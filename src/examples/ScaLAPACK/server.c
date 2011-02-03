/****************************************************************************/
/* DIET server for ScaLAPACK functions                                      */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Philippe COMBES (Philippe.Combes@ens-lyon.fr)                       */
/*    - Bert VAN HEUKELOM (Bert.Van-Heukelom@ens-lyon.fr)                   */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.7  2011/02/03 19:31:31  bdepardo
 * Remove unused variables
 *
 * Revision 1.6  2011/02/01 22:24:45  bdepardo
 * Remove memleak
 *
 * Revision 1.5  2011/01/25 18:50:35  bdepardo
 * Removed unused variables
 *
 * Revision 1.4  2011/01/23 19:20:00  bdepardo
 * Fixed memory and resources leaks, variables scopes, unread variables
 *
 * Revision 1.3  2010/03/05 15:52:08  ycaniou
 * Ordered things in CMakelist.txt and update Display (Batch, build_version...)
 * Fix version guess of compiler (was gcc only)
 * Use option to avoid %zd warning
 * Undo previous cast of size_t into int
 *
 * Revision 1.2  2003/08/09 17:32:47  pcombes
 * Update to the new diet_profile_desc_t.
 *
 * Revision 1.1.1.1  2003/04/10 13:21:39  pcombes
 * SCALAPCK is included in the autotools chain under "ScaLAPACK".
 *
 * Revision 1.4  2003/01/23 19:13:45  pcombes
 * Update to API 0.6.4
 *
 * Revision 1.3  2003/01/17 18:05:37  pcombes
 * Update to API 0.6.3
 *
 * Revision 1.2  2002/12/12 18:17:04  pcombes
 * Small bug fixes on prints (special thanks to Jean-Yves)
 ****************************************************************************/


#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>

#include "DIET_server.h"
#include "job.h"
#include "DIET_mutex.h"

extern void
dgemm_(char*, char*,  int*, int*, int*, double*, double*, int*, double*, int*,
       double*, double*, int*);

extern void
pdgemm_(char*, char*, int*, int*, int*, double*,
	double*, int*, int*, int*,
	double*, int*, int*, int*, double*,
	double*, int*, int*, int*);



int nbProcs;   // how many processes are running?
int myId;      // what is my processor ID in the MPI_COMM_WORLD communicator ?
int* myJobId;  // what job am i currently working on?
int zero = 0;

int delay = 1; // use artificial delay?


int* usedProcesses; // boolean array 

int first = 1;
char Ori='r', All='a', No='n';
char* configFileName;

MPI_Comm freeComm;
MPI_Group worldGroup, freeGroup;

diet_mutex_t mutex;

int wakeupMsgSize;

int jobCounter=0;

// some static definitions
int MSG_TYPE      =    0;
int MSG_ID        =    1;
int MSG_PROCS     =    2;
int MSG_SEDID     =    3;
int MSG_COLCOUNT  =    4;
int MSG_ROWCOUNT  =    5;
int MSG_BLOCKSIZE =    6;
int MSG_FIRSTPROC =    7;
int MSGTYPE_JOB   = 1000;
int MSGTYPE_FREE  = 1001;


#define print_matrix(mat, m, n, rm)        \
  {                                        \
    size_t i, j;                           \
    printf("%s (%s-major) = \n", #mat,     \
           (rm) ? "row" : "column");       \
    for (i = 0; i < (m); i++) {            \
      for (j = 0; j < (n); j++) {          \
        if (rm)                            \
	  printf("%3f ", (mat)[j + i*(n)]);\
        else                               \
	  printf("%3f ", (mat)[i + j*(m)]);\
      }                                    \
      printf("\n");                        \
    }                                      \
    printf("\n");                          \
  }

#ifdef DEBUG
#define LOG(n)      printf("Process %d : %s\n", myId, n)
#define LOG2I(n, m) printf("Process %d : %s %d\n", myId, n, m)
#else  // DEBUG
#define LOG(n)      
#define LOG2I(n, m) 
#endif // DEBUG


int
getJobId()
{
  return myJobId[diet_thread_id()];
}


#define STATUS(n)                                               \
  {                                                             \
    if(getJobId() != -1)                                        \
      printf("Process %d - Job %d : %s\n", myId, getJobId(), n);\
    else			 	                        \
      printf("Process %d : %s\n", myId, n);                     \
}

#define STATUSL(n)                                              \
  {                                                             \
    if (getJobId() != -1)                                       \
      printf("Process %d - Job %d : %s", myId, getJobId(), n);  \
    else			 	                        \
      printf("Process %d : %s", myId, n);                       \
}

#define STATUS2I(n, m)                                                 \
  {                                                                    \
    if (getJobId() != -1)                                              \
      printf("Process %d - Job %d : %s %d\n", myId, getJobId(), n, m); \
    else			 	                               \
      printf("Process %d : %s %d \n", myId, n, m);                     \
}

#define STATUS4(n, m, o, p, q)                                  \
  {                                                             \
    if (getJobId() != -1) {                                     \
      printf("Process %d - Job %d : ", myId, getJobId());       \
      printf(n, m, o, p, q);                                    \
      printf("\n");                                             \
    } else {			 	                        \
      printf("Process %d : ", myId);                            \
      printf(n, m, o, p, q);                                    \
      printf("\n");                                             \
    }				 	                        \
}



/****************************************************************************
 * SOLVE FUNCTION
 ****************************************************************************/


void
compute(int m, int n, int k, char tA, char tB,
	double alpha, double* A, double* B, double beta, double* C,
	int fromctxt, int toctxt, int allctxt, job_t* j);

job_t*
sed_getJob(int, int, int ,int );
void
sed_leaveJob(job_t*);

int
solve_pdgemm(diet_profile_t* pb)
{
  char    tA, tB;
  size_t  i, m, m_, n, n_, k, k_;
  size_t* procs = NULL;
  size_t* rows = NULL;
  size_t* cols = NULL;
  size_t* bs = NULL;
  double* alpha = NULL;
  double* beta = NULL;
  double* A = NULL;
  double* B = NULL;
  double* C = NULL;

  int res;
  int IsSqMatSUM = 0;
  diet_matrix_order_t oA, oB, oC;
  
  // values for partial martices and their redistribution
  diet_scalar_get(diet_parameter(pb,0), &procs, NULL);
  diet_scalar_get(diet_parameter(pb,1), &rows, NULL);
  diet_scalar_get(diet_parameter(pb,2), &cols, NULL);
  diet_scalar_get(diet_parameter(pb,3), &bs, NULL);
  diet_scalar_get(diet_parameter(pb,4), &alpha, NULL);
  diet_matrix_get(diet_parameter(pb,5), &A, NULL, &m, &k, &oA);
  tA    = (oA == DIET_ROW_MAJOR) ? 'T' : 'N';
  diet_matrix_get(diet_parameter(pb,6), &B, NULL, &k_, &n, &oB);
  tB    = (oB == DIET_ROW_MAJOR) ? 'T' : 'N';
  diet_scalar_get(diet_parameter(pb,7), &beta, NULL);
  diet_matrix_get(diet_parameter(pb,8), &C, NULL, NULL, NULL, &oC);

  STATUS("########################## Request received.");

  /* A and B NULL => MatScalMult, then m and n are wrong
   *  only B NULL => SqMatSUM, then set B to identity (and k_ for later test) */
  if (!A && !B) {
    m = m_;
    n = n_;
    k = (m > n) ? m : n;
    k_ = k;
  }
  if ((IsSqMatSUM = (A && !B))) {
    k_ = k;
    n = n_;
    if (m != n || n != k || m != k) {
      fprintf(stderr, "pdgemm Error: only square matrices can be summed.\n");
      return 1;
    }
    B = (double*) calloc(m * m, sizeof(double));
    for (i = 0; i < m; i++) {
      B[i + m * i] = 1.0;
    }
  }

  if ((k_ != k) || (m_ != m) || (n_ != n)) {
    fprintf(stderr, "pdgemm Error: invalid matrix dimensions: ");
    fprintf(stderr, "%zdx%zd = %zdx%zd * %zdx%zd\n", m_, n_, m, k, k_, n);
    free(B);
    return 1;
  }

  if ((m <= bs) || (n <= bs) || (k <= bs)) {

    printf("Solving dgemm_ ...");
    if (oC == DIET_ROW_MAJOR) {
      tA = (tA == 'T') ? 'N' : 'T';
      tB = (tB == 'T') ? 'N' : 'T';
      printf("dgemm args : m=%d, n=%d, k=%d, alpha=%f, beta=%f, tA=%c, tB=%c\n",
	     n, m, k, *alpha, *beta, tB, tA);
      dgemm_(&tB, &tA, &n, &m, &k, alpha,
	     B, (tB == 'T') ? &k : &n,
	     A, (tA == 'T') ? &m : &k,
	     beta, C, &n);  
    } else {
      printf("dgemm args : m=%zd, n=%zd, k=%zd, alpha=%f, beta=%f, tA=%c, tB=%c\n",
	     m, n, k, *alpha, *beta, tA, tB);
      dgemm_(&tA, &tB, &m, &n, &k, alpha,
	     A, (tA == 'T') ? &k : &m,
	     B, (tB == 'T') ? &n : &k,
	     beta, C, &m);
    }

  } else {

    double* E;
    // parameters for matrix size broadcast.
    int q       = 1;
    int w       = 5;
    int LDA     = 1;
    char* all   = "All";
    char* empty = " ";
    job_t* j;

    // locking Mutex
    
    LOG("Waiting for mutex");
    diet_mutex_lock(mutex);
    LOG("Mutex locked");

    LOG2I("Blocksize:", bs);

    j = sed_getJob(procs, rows, cols, bs);

    // m n k alpha and beta broadcast
    E = malloc(q * w* sizeof(double));
    E[0] = 1.0 * m;
    E[1] = 1.0 * n;
    E[2] = 1.0 * k;
    E[3] = alpha;
    E[4] = beta;
    
    LOG("Sending E.");

    dgebs2d_(&(j->contextAll), all, empty, &q, &w, E, &LDA, &zero, &zero);

    LOG("E sent");
    
    //printf("Solving pdgemm_ ...");

    if (oC == DIET_ROW_MAJOR) {
      tA = (tA == 'T') ? 'N' : 'T';
      tB = (tB == 'T') ? 'N' : 'T';
      compute(n, m, k, tB, tA, *alpha, B, A, *beta, C,
	      j->context1x1, -1, j->contextAll, j);
    } else {
      compute(m, n, k, tA, tB, *alpha, A, B, *beta, C,
	      j->context1x1, -1, j->contextAll, j);
    }
    //print_matrix(C, m, n, (oC == DIET_ROW_MAJOR));

    sed_leaveJob(j);

    // unlocking Mutex
    
    LOG("Unlocking mutex");
    diet_mutex_unlock(mutex);
    

    // further cleanup    
    //free(E);
    res = 0;
  }

  for (i = 0; i < 8; i++)
    diet_free_data(diet_parameter(pb,i));

  if (IsSqMatSUM) {
    free(B);
  }

  return res;
}




/****************************************************************************
 * TOOLS
 ****************************************************************************/

void
compute(int m, int n, int k, char tA, char tB,
	double alpha, double* A, double* B, double beta, double* C,
	int fromctxt, int toctxt, int allctxt, job_t* j)
{
  int descPA[9];
  int descPB[9];
  int descPC[9];
  int descA[9];
  int descB[9];
  int descC[9];
  int LOCqA, LOCpA;
  int LOCqB, LOCpB;
  int LOCqC, LOCpC;

  double* pA = NULL;
  double* pB = NULL;
  double* pC = NULL;

  int msg[1];
  MPI_Status status;


#ifdef DEBUG
  printf("Process %d : compute myRow=%d myCol=%d rowcount=%d colcount=%d\n",
	 myId, j->row, j->col, j->rowCount, j->colCount);
  printf("                     fromctxt=%d toctxt=%d ",fromctxt, toctxt);
  printf("allctxt=%d blocksize=%d\n", allctxt, j->blocksize);
#endif // DEBUG

  descset_(&descA, &m, &k, &(j->blocksize), &(j->blocksize),
	   &zero, &zero, &fromctxt, &m);
  descset_(&descB, &k, &n, &(j->blocksize), &(j->blocksize),
	   &zero, &zero, &fromctxt, &k);
  descset_(&descC, &m, &n, &(j->blocksize), &(j->blocksize),
	   &zero, &zero, &fromctxt, &m);

  LOCpA = numroc_(&m, &(j->blocksize),&(j->row),&zero,&(j->rowCount));
  LOCqA = numroc_(&k, &(j->blocksize),&(j->col),&zero,&(j->colCount));

  // todo malloc only on workers

  pA = (double*)malloc(LOCpA*LOCqA*sizeof(double));
  descset_(&descPA, &m, &k, &(j->blocksize), &(j->blocksize),
	   &zero, &zero, &toctxt, &LOCpA);  
  
  LOCpB = numroc_(&k, &(j->blocksize),&(j->row),&zero,&(j->rowCount));
  LOCqB = numroc_(&n, &(j->blocksize),&(j->col),&zero,&(j->colCount));
  pB = (double*)malloc(LOCpB*LOCqB*sizeof(double));    
  descset_(&descPB, &k, &n, &(j->blocksize), &(j->blocksize),
	   &zero, &zero, &toctxt, &LOCpB);  
  
  LOCpC = numroc_(&m, &(j->blocksize),&(j->row),&zero,&(j->rowCount));
  LOCqC = numroc_(&n, &(j->blocksize),&(j->col),&zero,&(j->colCount));
  pC = (double*)malloc(LOCpC*LOCqC*sizeof(double));    
  descset_(&descPC, &m, &n, &(j->blocksize), &(j->blocksize),
	   &zero, &zero, &toctxt, &LOCpC);  

  LOG("Ready for redistribution");

  LOG("Distributing A");
  Cpdgemr2d(m, k, A, first, first, descA , pA, first, first, descPA, allctxt); 
  LOG("Distributing B ");
  Cpdgemr2d(k, n, B, first, first, descB , pB, first, first, descPB, allctxt); 
  LOG("Distributing C ");
  Cpdgemr2d(m, n, C, first, first, descC , pC, first, first, descPC, allctxt); 
  

  //for(i=0;i<9;i++)printf(" %d", descPA[i]);
  //printf("\n");
  if (myId!=j->sedId) STATUS("Calculating... ");
  
  if (myId==j->sedId) {
    diet_mutex_unlock(mutex);
    LOG("Mutex unlocked");
  }

  if (myId!=j->sedId) {
    LOG("Starting pdgemm");
    pdgemm_(&tA, &tB, &m, &n, &k, &alpha, 
	    pA, &first, &first, descPA, 
	    pB, &first, &first, descPB, &beta,
	    pC, &first, &first, descPC);
  }

  LOG("Ready to collect C");
  
  if (myId==j->sedId) {
    MPI_Recv(msg, 1, MPI_INT, localLeader(j), j->id, MPI_COMM_WORLD, &status);
    STATUS("Waiting for result");
    if (delay) diet_thread_sleep(10,0);
  } else {	     
    if (myId==localLeader(j)) {
      MPI_Send(msg, 1, MPI_INT, 0, j->id, MPI_COMM_WORLD);
    }
  } 
 
  if (myId==j->sedId) {
    LOG("Waiting for mutex");
    diet_mutex_lock(mutex);
    LOG("Mutex locked");
  }


  Cpdgemr2d(m, n, pC, first, first, descPC, C, first, first, descC, allctxt);
  if (myId==j->sedId) STATUS("Calculation finished and result collected.");
  

  LOG("C collected");

  free(pA);
  free(pB);
  free(pC);
}


void
setJobId(int i)
{
  myJobId[diet_thread_id()]=i;
}


void
fillSubmap(int* bitvector, int* submap)
{
  int i;
  int count = 0;
  
  for (i=0;i<nbProcs;i++) {
    if (bitvector[i]) {
      submap[count]=i;
      count++;
    }
  }
}


// converts a wakeupmessage to a job 
job_t*
demarshallMessage(int* msg)
{
  int i;
  job_t* j;

  j = job_construct(nbProcs);
  j->id = msg[MSG_ID];
  j->sedId = msg[MSG_SEDID];
  j->blocksize = msg[MSG_BLOCKSIZE];
  j->colCount = msg[MSG_COLCOUNT];
  j->rowCount = msg[MSG_ROWCOUNT];
  j->procCount = msg[MSG_PROCS];
  for (i=0; i< nbProcs; i++) {
    j->procs[i]=msg[MSG_FIRSTPROC+i];
  }
  
  return j;

}  


// converts a job to a wakeupmessage
int*
marshallMessage(job_t* j)
{
  int i;
  int* msg;
  // creating MPI Message 
  msg = malloc(wakeupMsgSize*sizeof(int));
  msg[MSG_TYPE] = MSGTYPE_JOB;
  msg[MSG_SEDID] = j->sedId;
  msg[MSG_ID] = j->id;
  msg[MSG_PROCS] = j->procCount;
  // defining size of WORKERS Grid
  msg[MSG_COLCOUNT] = j->colCount;
  msg[MSG_ROWCOUNT] = j->rowCount;
  msg[MSG_BLOCKSIZE] = j->blocksize;
  for (i=0; i<nbProcs; i++) {
    msg[i+MSG_FIRSTPROC] = j->procs[i];
  }
  return msg;
}


int
localLeader(job_t* j)
{
  int i;
  for (i=0; i<nbProcs; i++) {
    if (j->procs[i])
      return i;
  }
  return 0;
}


void
sed_leaveJob(job_t* j)
{
  int i;
  MPI_Status status;
  MPI_Comm temp;
  int* msg;
  int lLeader=-1;


  MPI_Comm inter;

  // waiting for Message from local Leader

  lLeader = localLeader(j);
  
  // waking FreeJobs for reunion with workers

  LOG("Waking free Processes to reunite with finished Workers ... ");
  msg = marshallMessage(j);
  msg[MSG_TYPE]=MSGTYPE_FREE;
  for (i=0;i<nbProcs; i++) {
    if (!usedProcesses[i]) {
      LOG2I("Waking Process to reunite with working Workers", i );
      MPI_Send(msg, wakeupMsgSize, MPI_INT, i, 0, MPI_COMM_WORLD);
    }
  }
  free(msg);


  // creating intercomm and merging the two groups

  LOG("Creating intercomm");
  MPI_Intercomm_create(freeComm, 0, MPI_COMM_WORLD, lLeader, 1, &inter);
  LOG("Intercomm created");
  MPI_Intercomm_merge(inter, 0, &freeComm);
  LOG("Merging done");


  // leaving and destroying grids and communicators

  Cblacs_gridexit(j->contextAll);
  Cblacs_gridexit(j->context1x1);
  MPI_Comm_free(j->commAll);
  MPI_Comm_free(j->comm1x1);
  MPI_Comm_free(&inter);


  // getting new group of free Processes

  MPI_Comm_group(freeComm, &freeGroup);


  // updating usedProcesses

  for (i=0; i<nbProcs; i++) {
    if (j->procs[i])
      usedProcesses[i]=0;
  }


  // further cleanup

  job_destruct(j);
  setJobId(-1);


  // information output 

  MPI_Comm_size(freeComm, &i);
  STATUS2I("Free Workers ", i-1);
  MPI_Comm_rank(freeComm, &i);
  LOG2I("New rank in freeComm", i);

}


job_t*
sed_getJob(int procs, int rows, int cols, int bs)
{
  int i;
  int* msg = NULL;
  int* submapAll = NULL;
  int* submap1x1 = NULL;
  int* submapWorkers = NULL;
  int* submapTemp = NULL;
#ifdef DEBUG
  int rowCount, colCount, myR, myC;
#endif
  job_t* j = NULL;
  MPI_Group* group;
  MPI_Comm* tempComm;
  int remoteLeaderId=-1;

  STATUS4("Building grid. Processes %d Rows %d Cols %d Blocksize %d. ",
	  procs, rows, cols, bs);

  // creating job

  j = job_construct(nbProcs);
  j->id=jobCounter;
  j->rowCount=rows;
  j->colCount=cols;
  j->blocksize=bs; 
  jobCounter++;
  j->sedId = myId;
  j->procCount=0;
  setJobId(j->id);

  LOG2I("Rows", rows);
  LOG2I("Cols", cols);
  LOG2I("Blocksize", j->blocksize);


  // choosing free processes ... waiting until there are some free if not.

  do {
    j->procCount=0;
    for (i=0;i<nbProcs ; i++) {
      j->procs[i]=0;
    }

    STATUSL("Choosing Processes ... ");
    for (i=0; i<nbProcs && j->procCount < procs; i++) {
      if (!usedProcesses[i]) {
	if (remoteLeaderId==-1) remoteLeaderId=i;
	printf("%d ", i);
	j->procCount++;
	j->procs[i]=1;
      }
    }
    printf("\n");
    if (j->procCount!=procs) {
      STATUS("... not enough Processes found. Going to sleep for 3sec.");
      diet_mutex_unlock(mutex);
      diet_thread_sleep(3, 0);
      LOG("Waiting for mutex");
      diet_mutex_lock(mutex);
      LOG("Mutex locked");
    }
  } while (j->procCount!=procs);  


  // waking all free workers up with a MPI message

  STATUS("Waking... ");
  LOG("Creating wakeup message");
  // creating MPI Message 
  msg = marshallMessage(j);
  // sending Messages
  LOG("Sending Messages ");
  for (i=0; i<nbProcs; i++) {
    if (!usedProcesses[i]) {
      LOG2I("Waking Process ", i );
      MPI_Send(msg, wakeupMsgSize, MPI_INT, i, 0, MPI_COMM_WORLD);
      if(j->procs[i])usedProcesses[i]=1;
      // if(i%3==0)if(delay)diet_thread_sleep(1,0);
    }
  }
  free(msg);

  
  // defining Submaps 

  submapTemp = malloc((j->procCount+1)*sizeof(int));
  // submapWorkers
  submapWorkers = malloc(j->procCount*sizeof(int));
  fillSubmap(j->procs, submapTemp);
  MPI_Group_translate_ranks(worldGroup, j->procCount, submapTemp,
			    freeGroup, submapWorkers);
  // submapAll
  submapAll = malloc((j->procCount+1)*sizeof(int));
  submapTemp[0] = j->sedId;
  fillSubmap(j->procs, &(submapTemp[1]));
  MPI_Group_translate_ranks(worldGroup, j->procCount+1, submapTemp,
			    freeGroup, submapAll);
  // submap1x1
  submap1x1 = malloc(sizeof(int));
  submap1x1[0] = submapAll[0];
#ifdef DEBUG
  printf("Process %d Submap1x1=[%d]\n", myId, submap1x1[0]);
  printf("Process %d SubmapAll=[", myId);
  for(i=0;i<j->procCount+1;i++){
    printf("%d%s", submapAll[i], (i!=j->procCount?",":"]\n"));
  }
  printf("Process %d SubmapWorkers=[", myId);
  for(i=0;i<j->procCount;i++){
    printf("%d%s", submapWorkers[i], (i!=j->procCount-1?",":"]\n"));
  }
#endif // DEBUG
  free(submapTemp);


  // creating Communicators
  
  LOG("Creating communicators");
  // allocations 
  group = malloc(sizeof(MPI_Group));
  tempComm = malloc(sizeof(MPI_Comm));


  // create communicator with workers and Sed

  LOG("Creating comm all ");
  LOG2I("Creating GroupAll with Members=",j->procCount+1); 
  MPI_Group_incl(freeGroup, j->procCount+1, submapAll, group);
  LOG("Creating commAll");
  MPI_Comm_create(freeComm, *group, j->commAll);


  // create communicator for Workers only

  LOG("Creating commWorkers");
  MPI_Group_incl(freeGroup, j->procCount, submapWorkers, group);
  LOG("GroupWorkers created");
  MPI_Comm_create(freeComm, *group, j->commWorkers);
  LOG("commWorkers created");


  // creating new free Communicator

  LOG("Creating new freeComm"); 
  MPI_Group_excl(freeGroup, j->procCount, submapWorkers, group);
  MPI_Comm_create(freeComm, *group, tempComm);


  // creating communicator with only sed process 

  LOG("Creating 1x1 Comm ");
  MPI_Group_incl(freeGroup, 1, submap1x1, group);
  MPI_Comm_create(freeComm, *group, j->comm1x1);

  
  // coping new communicator for the free processes to the freeComm var.

  LOG("Coping freecom");
  MPI_Comm_dup(*tempComm, &freeComm);
  MPI_Comm_group(freeComm, &freeGroup);


  // freeing memory

  LOG("Freeing tempComm ");
  MPI_Comm_free(tempComm);
  free(tempComm);
  MPI_Group_free(group);
  free(group);
  
  LOG("Done creating comms.");


  // creating grids and maps for blacs 

  LOG("Creating Contexts");
  j->contextAll = Csys2blacs_handle(*(j->commAll));
  Cblacs_gridinit(&(j->contextAll), &Ori, 1, j->procCount+1);
#ifdef DEBUG
  Cblacs_gridinfo(j->contextAll, &rowCount, &colCount, &myR,&myC);
  printf("Process %d : Created GridAll Rows=%d, Cols=%d, myRow=%d myCol=%d    \n", myId, rowCount, colCount, myR, myC);    
#endif
  j->context1x1 = Csys2blacs_handle(*(j->comm1x1));
  Cblacs_gridinit(&(j->context1x1), &Ori, 1,1,1);
#ifdef DEBUG
  Cblacs_gridinfo(j->context1x1, &rowCount, &colCount, &myR,&myC);
  printf("Process %d : Created Grid1x1 Rows=%d, Cols=%d, myRow=%d myCol=%d    \n", myId, rowCount, colCount, myR, myC);    
#endif


  // cleanup & output

  free(submapAll);
  free(submap1x1);
  free(submapWorkers);
  LOG("Done creating Job");
  MPI_Comm_size(freeComm, &i);
  STATUS2I("Free Workers ", i-1);

  return j;
}



/****************************************************************************
 * WORKER FUNCTIONS
 ****************************************************************************/

void
worker_leaveJob(job_t* j)
{
  int i;
  int localLeader=0;
  int wasWorking=0;

  MPI_Comm temp;

  LOG("Leaving Job... ");


  // who is the local Leader ?

  for (i=0; i<nbProcs; i++) {
    if (j->procs[i]==1) {
      localLeader = i;
      break;
    }
  }      

  
  // sending Messages signaling that I am ready (busy workers only)
  wasWorking=j->procs[myId];


  // creating intercomm and merging free and woring workers

  LOG("Creating Intercomm");
  LOG2I("Local leader is", localLeader);
  if (wasWorking) {
    LOG("I was working");
    MPI_Intercomm_create(*(j->commWorkers), 0, MPI_COMM_WORLD,
			 j->sedId, 1, j->intercomm);    
    MPI_Intercomm_merge(*(j->intercomm), 1, &freeComm);
  } else {
    LOG("I have done nothing");
    MPI_Intercomm_create(freeComm, 0, MPI_COMM_WORLD,
			 localLeader, 1, j->intercomm );
    LOG("Merging");
    MPI_Intercomm_merge(*(j->intercomm), 0, &freeComm);
  }
  LOG("Merging done");


  // further cleanups 

  if (wasWorking) {
    Cblacs_gridexit(j->contextAll);
    Cblacs_gridexit(j->contextWorkers);
    
    MPI_Comm_free(j->commWorkers);
    MPI_Comm_free(j->commAll);
  }

  MPI_Comm_group(freeComm, &freeGroup);
  MPI_Comm_free(j->intercomm);
  MPI_Comm_size(freeComm, &i);
  LOG2I("New size of freeComm", i);
  MPI_Comm_rank(freeComm, &i);
  LOG2I("New rank in freeComm", i);

  job_destruct(j);
  setJobId(-1);
  

}


int
worker_formJob(job_t* j);

job_t*
worker_handleMessages()
{
  MPI_Status status;
  int    jobRequest;
  int*   msg;
  job_t* j;

  msg = malloc(wakeupMsgSize*sizeof(int));

  do {
    // waiting for wakeupmessage
    jobRequest=0;
    LOG("Going to sleep");
    MPI_Recv(msg, wakeupMsgSize, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD , &status);
    setJobId(msg[MSG_ID]);
#ifdef DEBUG
    printf("Process %d was woken.\n msg=[", myId);
    for (i=0; i<wakeupMsgSize; i++) {
      printf("%d%s", msg[i], (i!=(wakeupMsgSize-1)?",":"]\n"));
    }
#endif // DEBUG
    
    j = demarshallMessage(msg);
    
    if (msg[MSG_TYPE]==MSGTYPE_FREE) {

      // Worker was sleeping and has been waken to reunite with workers
      // of a just finished a job.
      LOG("I am awake! Reuniting with busy workers");

      LOG("Free Worker ready to reunite with working Workers.");
      worker_leaveJob(j);
     
      setJobId(-1);
    } else {

      // Worker was sleeping and has been waken to form new Communicators 
      // for a new Job
      LOG("I am awake! Forming new Job.");

      jobRequest = worker_formJob(j);
      if(jobRequest)LOG("I received a job!");
    }
  } while (!jobRequest);

  // if a job I have to work on has been received return it.
  
  return j;
}


int
worker_formJob(job_t* j)
{
  int* submapAll = NULL;
  int* submap1x1 = NULL;
  int* submapWorkers = NULL;
  int* submapTemp = NULL;
  int  rowCount, colCount, myR, myC;
  MPI_Group* group;
  MPI_Comm* tempComm;
  int jobRequest=0;
  int i;



  // defining Submaps
  
  submapTemp = malloc((j->procCount+1)*sizeof(int));
  // submapWorkers
  submapWorkers = malloc(j->procCount*sizeof(int));
  fillSubmap(j->procs, submapTemp);
  MPI_Group_translate_ranks(worldGroup, j->procCount, submapTemp, freeGroup, submapWorkers);
  // submapAll
  submapAll = malloc((j->procCount+1)*sizeof(int));
  submapTemp[0] = j->sedId;
  fillSubmap(j->procs, &(submapTemp[1]));
  MPI_Group_translate_ranks(worldGroup, j->procCount+1, submapTemp, freeGroup, submapAll);
  // submap1x1
  submap1x1 = malloc(sizeof(int));
  submap1x1[0] = submapAll[0];
#ifdef DEBUG
  printf("Process %d Submap1x1=[%d]\n", myId, submap1x1[0]);
  printf("Process %d SubmapAll=[", myId);
  for (i=0;i<j->procCount+1;i++) {
    printf("%d%s", submapAll[i], (i!=j->procCount?",":"]\n"));
  }
  printf("Process %d SubmapWorkers=[", myId);
  for (i=0;i<j->procCount;i++) {
    printf("%d%s", submapWorkers[i], (i!=j->procCount-1?",":"]\n"));
  }
#endif // DEBUG

  LOG2I("Blocksize", j->blocksize);
  
  // some other initializations
  
  jobRequest = j->procs[myId];
  group = malloc(sizeof(MPI_Group));
  
  
  // creating communicator with sed and workers
  
  LOG2I("Creating GroupAll with Members=",j->procCount+1); 
  MPI_Group_incl(freeGroup, j->procCount+1, submapAll, group);
  LOG("creating commAll");

  MPI_Comm_size(freeComm, &i);
  LOG2I("Sizeof freeComm", i);
  MPI_Comm_rank(freeComm, &i);
  LOG2I("Rank in freeComm", i);
  
  MPI_Comm_create(freeComm, *group, j->commAll);
  
  // creating workers communicator 
  
  MPI_Group_incl(freeGroup, j->procCount, submapWorkers, group);
  LOG("creatign commWorkers");
  MPI_Comm_create(freeComm, *group, j->commWorkers);
  
  
  // creating new free Communicator 
  
  tempComm = malloc(sizeof(MPI_Comm));
  LOG("Creating new freeComm"); 
  MPI_Group_excl(freeGroup, j->procCount, submapWorkers, group);
  MPI_Comm_create(freeComm, *group, tempComm);
  
  
  // creating 1x1 Communicator 
  
  LOG("Creating 1x1 Comm ");
  MPI_Group_incl(freeGroup, 1, submap1x1, group);
  MPI_Comm_create(freeComm, *group, j->comm1x1);
  
  
  // if Worker remains free copy the newly created communicator 
  // between the free Processes
  
  if (!jobRequest) {
    LOG("Coping freecom");
    MPI_Comm_dup(*tempComm, &freeComm);
    MPI_Comm_group(freeComm, &freeGroup);
    LOG("Freeing tempComm ");
    MPI_Comm_free(tempComm);  
  }
  
  
  LOG("Done creating comms.");
  
  
  // creating blacs contexts
  
  if (jobRequest) {
    int u;
    LOG("Creating contexts");
    // all context
    j->contextAll = Csys2blacs_handle(*(j->commAll));
    Cblacs_gridinit(&(j->contextAll), &Ori, 1, j->procCount+1);
    // workercontext
    j->contextWorkers = Csys2blacs_handle(*(j->commWorkers));
    for(u=0; u<j->procCount; u++){
      submapWorkers[u]=u;
    }

    LOG("Creating Worker context");
    LOG2I("Creating Worker context rowCount ", j->rowCount);
    LOG2I("Creating Worker context colCount ", j->colCount);
    
    
    Cblacs_gridmap(&(j->contextWorkers), submapWorkers,
		   j->rowCount, j->rowCount, j->colCount);
    Cblacs_gridinfo(j->contextWorkers, &rowCount, &colCount, &myR,&myC);
    j->row = myR;
    j->col = myC;

#ifdef DEBUG
    printf("Process %d: Created WorkerGrid R=%d, C=%d, myRow=%d myCol=%d\n",
	   myId, rowCount, colCount, myR, myC);

    Cblacs_gridinfo(j->contextAll, &rowCount, &colCount, &myR,&myC);
    printf("Process %d: Created  AllGrid   R=%d, C=%d, myRow=%d myCol=%d\n", 
	   myId, rowCount, colCount, myR, myC);    
#endif // DEBUG


    
    LOG("Done creating contexts");
    
  }
      
  // some cleanups
  
  free(submapTemp);
  free(tempComm);  
  MPI_Group_free(group);
  free(group);
  LOG2I("Freeing submapAll", submapAll);
  free(submapAll);
  LOG2I("Freeing submap1x1", submap1x1);
  free(submap1x1);
  LOG2I("Freeing submapWorkers", submapWorkers);
  free(submapWorkers);

  return jobRequest;
}


void
waitingLoop()
{

  // MPI Communication & Jobstuff
  MPI_Status status;
  
   // pdgemm variables
  int m, n, k;
  double* A = NULL,* B = NULL,* C = NULL;
  double alpha, beta;
  
   // transmission of pre calculation variables
  int q=1;
  int w=5;
  int LDA=1;
  double* E;

  job_t* j;
  
  // counters and other
  char* all= "All";
  char* empty=" ";
  
  E = malloc(q*w* sizeof(double));
  
  while (1) {
    
    LOG("Waiting ");
    
    j = worker_handleMessages();
    
    LOG("Waiting for E ");
    dgebr2d_(&(j->contextAll), all, empty, &q, &w, E, &LDA, &zero, &zero);
    m     = E[0]/1;
    n     = E[1]/1;
    k     = E[2]/1;
    alpha = E[3];
    beta  = E[4];
    
    LOG("E received");
        
    LOG2I("Computing job", j->id);
    compute(m, n, k, 'N', 'N', alpha, A, B, beta, C, -1, j->contextWorkers, j->contextAll, j);
    LOG2I("Done computing job", j->id);
    // waiting for all processes to be ready to leave ... 
    
    LOG2I("Ready to leave Job", j->id);
      //MPI_Barrier(*(myJob->commAll));
    
    worker_leaveJob(j);
    
  }
  free(E);
  
}


int
registration(int argc, char* argv[])
{
  diet_profile_desc_t* profile;
  diet_arg_t* arg;
  diet_convertor_t* cvt;
  size_t nbProcs =  4;
  size_t nbRows  =  2;
  size_t nbCols  =  2;
  size_t bs      = 16;

  diet_service_table_init(1);
  
  STATUS("Registering at DIET");
  
  cvt = diet_convertor_alloc("ScaLAPACK/pdgemm", 7, 8, 8);
  
  /*
   * Adding pdgemm
   */
  profile = diet_profile_desc_alloc("ScaLAPACK/pdgemm", 7, 8, 8);
  // nbProcs, nbRows, nbCols, bs, alpha, A, B, beta, C
  diet_generic_desc_set(diet_param_desc(profile,0), DIET_SCALAR, DIET_INT);
  diet_generic_desc_set(diet_param_desc(profile,1), DIET_SCALAR, DIET_INT);
  diet_generic_desc_set(diet_param_desc(profile,2), DIET_SCALAR, DIET_INT);
  diet_generic_desc_set(diet_param_desc(profile,3), DIET_SCALAR, DIET_INT);
  diet_generic_desc_set(diet_param_desc(profile,4), DIET_SCALAR, DIET_DOUBLE);
  diet_generic_desc_set(diet_param_desc(profile,5), DIET_MATRIX, DIET_DOUBLE);
  diet_generic_desc_set(diet_param_desc(profile,6), DIET_MATRIX, DIET_DOUBLE);
  diet_generic_desc_set(diet_param_desc(profile,7), DIET_SCALAR, DIET_DOUBLE);
  diet_generic_desc_set(diet_param_desc(profile,8), DIET_MATRIX, DIET_DOUBLE);

  if (diet_service_table_add(profile, NULL, solve_pdgemm)) return 1;
  diet_profile_desc_free(profile);

  /*
   * Adding dgemm
   */
  /* Set profile */
  profile = diet_profile_desc_alloc("dgemm", 3, 4, 4);
  // alpha, A, B, beta, C
  diet_generic_desc_set(diet_param_desc(profile,0), DIET_SCALAR, DIET_DOUBLE);
  diet_generic_desc_set(diet_param_desc(profile,1), DIET_MATRIX, DIET_DOUBLE);
  diet_generic_desc_set(diet_param_desc(profile,2), DIET_MATRIX, DIET_DOUBLE);
  diet_generic_desc_set(diet_param_desc(profile,3), DIET_SCALAR, DIET_DOUBLE);
  diet_generic_desc_set(diet_param_desc(profile,4), DIET_MATRIX, DIET_DOUBLE);
  /* Set convertor */
  arg = (diet_arg_t*) malloc(sizeof(diet_arg_t));
  diet_scalar_set(arg, &nbProcs, DIET_VOLATILE, DIET_INT);
  diet_arg_cvt_short_set(diet_arg_conv(cvt,0), -1, arg);
  arg = (diet_arg_t*) malloc(sizeof(diet_arg_t));
  diet_scalar_set(arg, &nbRows, DIET_VOLATILE, DIET_INT);
  diet_arg_cvt_short_set(diet_arg_conv(cvt,1), -1, arg);
  arg = (diet_arg_t*) malloc(sizeof(diet_arg_t));
  diet_scalar_set(arg, &nbCols, DIET_VOLATILE, DIET_INT);
  diet_arg_cvt_short_set(&(cvt->arg_convs[2]), -1, arg);
  arg = (diet_arg_t*) malloc(sizeof(diet_arg_t));
  diet_scalar_set(arg, &bs, DIET_VOLATILE, DIET_INT);
  diet_arg_cvt_short_set(diet_arg_conv(cvt,3), -1, arg);
  diet_arg_cvt_short_set(diet_arg_conv(cvt,4),  0, NULL);
  diet_arg_cvt_short_set(diet_arg_conv(cvt,5),  1, NULL);
  diet_arg_cvt_short_set(diet_arg_conv(cvt,6),  2, NULL);
  diet_arg_cvt_short_set(diet_arg_conv(cvt,7),  3, NULL);
  diet_arg_cvt_short_set(diet_arg_conv(cvt,8),  4, NULL);
  /* Add */
  if (diet_service_table_add(profile, cvt, solve_pdgemm)) return 1;
  diet_profile_desc_free(profile);


  /*
   * Adding SqMatSUM
   */
  /* Set profile */
  profile = diet_profile_desc_alloc("SqMatSUM", 0, 1, 1);
  diet_generic_desc_set(diet_param_desc(profile,0), DIET_MATRIX, DIET_DOUBLE);
  diet_generic_desc_set(diet_param_desc(profile,1), DIET_MATRIX, DIET_DOUBLE);
  /* Set convertor */
  arg = (diet_arg_t*) malloc(sizeof(diet_arg_t));
  diet_scalar_set(arg, &nbProcs, DIET_VOLATILE, DIET_INT);
  diet_arg_cvt_short_set(diet_arg_conv(cvt,0), -1, arg);
  arg = (diet_arg_t*) malloc(sizeof(diet_arg_t));
  diet_scalar_set(arg, &nbRows, DIET_VOLATILE, DIET_INT);
  diet_arg_cvt_short_set(diet_arg_conv(cvt,1), -1, arg);
  arg = (diet_arg_t*) malloc(sizeof(diet_arg_t));
  diet_scalar_set(arg, &nbCols, DIET_VOLATILE, DIET_INT);
  diet_arg_cvt_short_set(diet_arg_conv(cvt,2), -1, arg);
  arg = (diet_arg_t*) malloc(sizeof(diet_arg_t));
  diet_scalar_set(arg, &bs, DIET_VOLATILE, DIET_INT);
  diet_arg_cvt_short_set(diet_arg_conv(cvt,3), -1, arg);
  {
    double alpha = 1.0;
    arg = (diet_arg_t*) malloc(sizeof(diet_arg_t));
    diet_scalar_set(arg, &alpha, DIET_VOLATILE, DIET_DOUBLE);
  }
  diet_arg_cvt_short_set(diet_arg_conv(cvt,4), -1, arg);
  // beta is the same
  diet_arg_cvt_short_set(diet_arg_conv(cvt,7), -1, arg);
  diet_arg_cvt_short_set(diet_arg_conv(cvt,5),  0, NULL);
  {
    arg = (diet_arg_t*) calloc(1, sizeof(diet_arg_t));
    diet_matrix_set(arg, NULL, DIET_VOLATILE, DIET_DOUBLE, 0, 0, DIET_COL_MAJOR);
  }
  diet_arg_cvt_short_set(diet_arg_conv(cvt,6), -1, arg);
  diet_arg_cvt_short_set(diet_arg_conv(cvt,8),  1, NULL);
  /* Add */
  if (diet_service_table_add(profile, cvt, solve_pdgemm)) return 1;
  diet_profile_desc_free(profile);


  /*
   * Adding MatPROD
   */
  /* Set profile */
  profile = diet_profile_desc_alloc("MatPROD", 1, 1, 2);
  diet_generic_desc_set(diet_param_desc(profile,0), DIET_MATRIX, DIET_DOUBLE);
  diet_generic_desc_set(diet_param_desc(profile,1), DIET_MATRIX, DIET_DOUBLE);
  diet_generic_desc_set(diet_param_desc(profile,2), DIET_MATRIX, DIET_DOUBLE);
  /* Set convertor */
  arg = (diet_arg_t*) malloc(sizeof(diet_arg_t));
  diet_scalar_set(arg, &nbProcs, DIET_VOLATILE, DIET_INT);
  diet_arg_cvt_short_set(diet_arg_conv(cvt,0), -1, arg);
  arg = (diet_arg_t*) malloc(sizeof(diet_arg_t));
  diet_scalar_set(arg, &nbRows, DIET_VOLATILE, DIET_INT);
  diet_arg_cvt_short_set(diet_arg_conv(cvt,1), -1, arg);
  arg = (diet_arg_t*) malloc(sizeof(diet_arg_t));
  diet_scalar_set(arg, &nbCols, DIET_VOLATILE, DIET_INT);
  diet_arg_cvt_short_set(&(cvt->arg_convs[2]), -1, arg);
  arg = (diet_arg_t*) malloc(sizeof(diet_arg_t));
  diet_scalar_set(arg, &bs, DIET_VOLATILE, DIET_INT);
  diet_arg_cvt_short_set(diet_arg_conv(cvt,3), -1, arg);
  {
    double alpha = 1.0;
    arg = (diet_arg_t*) malloc(sizeof(diet_arg_t));
    diet_scalar_set(arg, &alpha, DIET_VOLATILE, DIET_DOUBLE);
  }
  diet_arg_cvt_short_set(diet_arg_conv(cvt,4), -1, arg);
  diet_arg_cvt_short_set(diet_arg_conv(cvt,5),  0, NULL);
  diet_arg_cvt_short_set(diet_arg_conv(cvt,6),  1, NULL);
  {
    double beta = 0.0;
    arg = (diet_arg_t*) malloc(sizeof(diet_arg_t));
    diet_scalar_set(arg, &beta, DIET_VOLATILE, DIET_DOUBLE);
  }
  diet_arg_cvt_short_set(diet_arg_conv(cvt,7), -1, arg);
  diet_arg_cvt_short_set(diet_arg_conv(cvt,8),  2, NULL);
  /* Add */
  if (diet_service_table_add(profile, cvt, solve_pdgemm)) return 1;
  diet_profile_desc_free(profile);


  /*
   * Adding MatScalMult
   */
  /* Set profile */
  profile = diet_profile_desc_alloc("MatScalMult", 0, 1, 1);
  diet_generic_desc_set(diet_param_desc(profile,0), DIET_SCALAR, DIET_DOUBLE);
  diet_generic_desc_set(diet_param_desc(profile,1), DIET_MATRIX, DIET_DOUBLE);
  /* Set convertor */
  arg = (diet_arg_t*) malloc(sizeof(diet_arg_t));
  diet_scalar_set(arg, &nbProcs, DIET_VOLATILE, DIET_INT);
  diet_arg_cvt_set(diet_arg_conv(cvt,0), DIET_CVT_IDENTITY, -1, arg);
  arg = (diet_arg_t*) malloc(sizeof(diet_arg_t));
  diet_scalar_set(arg, &nbRows, DIET_VOLATILE, DIET_INT);
  diet_arg_cvt_set(diet_arg_conv(cvt,1), DIET_CVT_IDENTITY, -1, arg);
  arg = (diet_arg_t*) malloc(sizeof(diet_arg_t));
  diet_scalar_set(arg, &nbCols, DIET_VOLATILE, DIET_INT);
  diet_arg_cvt_set(diet_arg_conv(cvt,2), DIET_CVT_IDENTITY, -1, arg);
  arg = (diet_arg_t*) malloc(sizeof(diet_arg_t));
  diet_scalar_set(arg, &bs, DIET_VOLATILE, DIET_INT);
  diet_arg_cvt_set(diet_arg_conv(cvt,3), DIET_CVT_IDENTITY, -1, arg);
  {
    double alpha = 0.0;
    arg = (diet_arg_t*) malloc(sizeof(diet_arg_t));
    diet_scalar_set(arg, &alpha, DIET_VOLATILE, DIET_DOUBLE);
  }
  diet_arg_cvt_short_set(diet_arg_conv(cvt,4), -1, arg);
  {
    arg = (diet_arg_t*) calloc(1, sizeof(diet_arg_t));
    diet_matrix_set(arg, NULL, DIET_VOLATILE, DIET_DOUBLE, 1, 1, DIET_COL_MAJOR);
  }  
  diet_arg_cvt_short_set(diet_arg_conv(cvt,5), -1, arg);
  diet_arg_cvt_short_set(diet_arg_conv(cvt,6), -1, arg);
  diet_arg_cvt_short_set(diet_arg_conv(cvt,7),  0, NULL);
  diet_arg_cvt_short_set(diet_arg_conv(cvt,8),  1, NULL);
  /* Add */
  if (diet_service_table_add(profile, cvt, solve_pdgemm)) return 1; 
  diet_profile_desc_free(profile);


  diet_print_service_table();

  return diet_SeD(argv[1], argc, argv);
}


int
parseArguments(int argc, char* argv[])
{
  /* Argument number checking  */

  if (argc != 2) {
    MPI_Finalize();
    if (myId==0) {
      fprintf(stderr, "Usage: %s <file.cfg> \n", argv[0]);
    }
    exit(2);
  }

  // nbProcs = atoi(argv[2]);


  /* Argument checking. This size has to be positive
     and less than matrix size. */
  //if ((nbProcCols <= 0) || (nbProcRows <= 0)
  //      || (nbProcRows * nbProcCols != nbProcs)) {
  //  MPI_Finalize();
  //  fprintf(stderr, "Proc %d: Bad arguments\n", myId);
  //  exit(1);
  // }

  configFileName = argv[1];
}


void
initialize(int*  argc, char** argv[])
{
  int q; // counter


  MPI_Init(argc, argv);

  printf("MPI_Comm_rank(%d, %d)\n", MPI_COMM_WORLD,myId);
  MPI_Comm_rank(MPI_COMM_WORLD,&myId);
  MPI_Comm_size(MPI_COMM_WORLD,&nbProcs);
  
  parseArguments(*argc, *argv);

  usedProcesses = calloc(nbProcs, sizeof(int));

  usedProcesses[0]=1;

  MPI_Comm_dup(MPI_COMM_WORLD, &freeComm);
  MPI_Comm_group(MPI_COMM_WORLD, &worldGroup);
  MPI_Comm_group(MPI_COMM_WORLD, &freeGroup);

  diet_mutex_initialize();
  diet_mutex_create(&mutex);

  wakeupMsgSize = MSG_FIRSTPROC+nbProcs;

  myJobId= malloc(nbProcs*sizeof(int));
  for (q=0; q<nbProcs; q++)
    myJobId[q]=-1;
}



pid_t
getpid()
{
  static pid_t process_pid;
  if (!process_pid)
    process_pid = __getpid();
  return process_pid;
}





/****************************************************************************
 * MAIN
 ****************************************************************************/

int
main(int argc, char* argv[])
{
  int ret = 0;
  
  /* Opening a MPI session. Asking for some info */
  
  initialize(&argc, &argv);    

  STATUS("Started");

  if(myId==0){

    ret = registration(argc, argv);
  }else{
    waitingLoop();
  }

  free(usedProcesses);
  free(myJobId);
  diet_mutex_finalize();
  MPI_Finalize();
  
  STATUS("Done");
  
  return ret;
}
