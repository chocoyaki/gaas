/****************************************************************************/
/* DIET server for BLAS functions with plugin-scheduler                     */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Philippe COMBES (Philippe.Combes@ens-lyon.fr)                       */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.4  2006/01/16 16:48:16  pfrauenk
 * CoRI add new performance functions
 *
 * Revision 1.3  2006/01/16 09:34:14  pfrauenk
 * CoRI: adding again more plug-in schedulers
 *
 * Revision 1.2  2005/12/19 16:58:08  pfrauenk
 * CoRI Adding a new scheduler to the example
 *
 * Revision 1.1  2005/12/15 09:42:36  pfrauenk
 * CoRI example added Peter Frauenkron
 ****************************************************************************/
//#include "DIET_config.h"

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


#include "DIET_server.h"

#if HAVE_CORI
/** void set_up_scheduler
 * add the scheduler to the profile
 */
void
set_up_scheduler(char * schedulertype,diet_profile_desc_t* profile);

/*
** performance_RRNBPROC: the performance function to use in the DIET
**   plugin scheduling facility
** The SeD must maintain a vector with the last executions
** the scalar of user defined tag must be the value of the eldest
** value. The size of the vector is (finally -after enough executions)
** the number of CPUs. In this manner, scheduling takes care of NBCPU 
** without occuping only the SeD with the most CPUs
*/
void
performance_RRNBPROC (diet_profile_t* pb,
		      estVector_t perfValues);

/*
 * performanceLOAD_AVG: the performance function to use in the DIET
 *   plugin scheduling facility
 * A simple call to the CORI_EASY_collector to receive the EST_AVGFREECPU:
 * the load average of the SeD
 * scheduling criteria is the loadavg: the SeD with the lowest 
 * load average receive the job
 */
void
performance_Load_Avg(diet_profile_t* pb,
		    estVector_t perfValues);

void
performance_Mem_Free(diet_profile_t* pb,
		     estVector_t perfValues);
void
performance_Disk_Write(diet_profile_t* pb,
		       estVector_t perfValues);
void
performance_Disk_Read(diet_profile_t* pb,
		      estVector_t perfValues);
void
performance_All_Hardwarestatic(diet_profile_t* pb,
			       estVector_t perfValues);

void 
performance_Total_Disk_Size(diet_profile_t* pb,
				 estVector_t perfValues);

void 
performance_Free_Disk_Size(diet_profile_t* pb,
				estVector_t perfValues);

#endif //HAVE_CORI

/**
 * dgemm_ prototype
 */
extern void
dgemm_(char*   tA,    char* tB,  int* m,   int* n,    int* k,
       double* alpha, double* A, int* lda, double* B, int* ldb,
       double* beta,  double* C, int* ldc);


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


/* This server can offer 5 services (chosen by user at launch time): */
/*   - T = matrix translation                                        */
/*   - MatSUM = matrix addition (and thus SqMatSUM and SqMatSUM_opt) */
/*   - MatPROD = matrix product                                      */

#define NB_SRV 5
static const char* SRV[NB_SRV] =
  {"dgemm", "MatPROD", "SqMatSUM", "SqMatSUM_opt", "MatScalMult"};


/*
 * SOLVE FUNCTION for dgemm_
 * C MUST BE COLUMN-MAJOR
 */

int
solve_dgemm(diet_profile_t* pb)
{
  char    tA, tB;
  diet_matrix_order_t oA, oB, oC;
  size_t  i, k, k_, m, m_, n, n_;
  double* alpha = NULL;
  double* beta  = NULL;
  double* A = NULL;
  double* B = NULL;
  double* C = NULL;
  int     IsSqMatSUM = 0;

  diet_matrix_get(diet_parameter(pb,0), &A, NULL, &m, &k, &oA);
  tA = (oA == DIET_ROW_MAJOR) ? 'T' : 'N';
  diet_matrix_get(diet_parameter(pb,1), &B, NULL, &k_, &n, &oB);
  tB = (oB == DIET_ROW_MAJOR) ? 'T' : 'N';
  diet_scalar_get(diet_parameter(pb,2), &alpha, NULL);
  diet_scalar_get(diet_parameter(pb,3), &beta,  NULL);
  diet_matrix_get(diet_parameter(pb,4), &C, NULL, &m_, &n_, &oC);
  
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
      fprintf(stderr, "dgemm Error: only square matrices can be summed.\n");
      return 1;
    }
    B = (double*) calloc(m * m, sizeof(double));
    for (i = 0; i < m; i++) {
      B[i + m * i] = 1.0;
    }
  }

  if ((k_ != k) || (m_ != m) || (n_ != n)) {
    fprintf(stderr, "dgemm Error: invalid matrix dimensions: ");
    fprintf(stderr, "%dx%d = %dx%d * %dx%d\n", m_, n_, m, k, k_, n);
    return 1;
  }

  // DEBUG
  
  printf("Solving dgemm_ ...");
  if (oC == DIET_ROW_MAJOR) {
    tA = (tA == 'T') ? 'N' : 'T';
    tB = (tB == 'T') ? 'N' : 'T';
    fprintf(stderr,
	    "dgemm args : m=%d, n=%d, k=%d, alpha=%f, beta=%f, tA=%c, tB=%c\n",
	    n, m, k, *alpha, *beta, tB, tA);
    dgemm_(&tB, &tA, &n, &m, &k, alpha,
	   B, (tB == 'T') ? &k : &n,
	   A, (tA == 'T') ? &m : &k,
	   beta, C, &n);  
  } else {
    fprintf(stderr,
	    "dgemm args : m=%d, n=%d, k=%d, alpha=%f, beta=%f, tA=%c, tB=%c\n",
	    m, n, k, *alpha, *beta, tA, tB);
    dgemm_(&tA, &tB, &m, &n, &k, alpha,
	   A, (tA == 'T') ? &k : &m,
	   B, (tB == 'T') ? &n : &k,
	   beta, C, &m);
  }
  printf(" done.\n");

  diet_free_data(diet_parameter(pb,0));
  diet_free_data(diet_parameter(pb,1));
  diet_free_data(diet_parameter(pb,2));
  diet_free_data(diet_parameter(pb,3));

  if (IsSqMatSUM) {
    free(B);
  }

  return 0;
}

/*
 * MAIN
 */

int
main(int argc, char* argv[])
{
  int res;
  diet_profile_desc_t* profile = NULL;
  diet_convertor_t* cvt = NULL;
  diet_arg_t* arg = NULL;

  if (argc != 3) {
    fprintf(stderr, "Usage: %s <file.cfg> <LOADAVG|MEMFREE|WRITE|READ|RRALLHARDWARE|RRNBPROC|NWS>\n", argv[0]);
    return 1;
  }

  diet_service_table_init(NB_SRV);

  cvt = diet_convertor_alloc("BLAS/dgemm", 3, 4, 4);


  /*
   * Adding dgemm
   */
  /* Set profile */
  profile = diet_profile_desc_alloc(SRV[0], 3, 4, 4);
  diet_generic_desc_set(diet_param_desc(profile,0),
			DIET_SCALAR, DIET_DOUBLE); // alpha
  diet_generic_desc_set(diet_param_desc(profile,1),
			DIET_MATRIX, DIET_DOUBLE); // A
  diet_generic_desc_set(diet_param_desc(profile,2),
			DIET_MATRIX, DIET_DOUBLE); // B
  diet_generic_desc_set(diet_param_desc(profile,3),
			DIET_SCALAR, DIET_DOUBLE); // beta
  diet_generic_desc_set(diet_param_desc(profile,4),
			DIET_MATRIX, DIET_DOUBLE); // C
  /* Set convertor */
  diet_arg_cvt_short_set(diet_arg_conv(cvt,0), 1, NULL);
  diet_arg_cvt_short_set(diet_arg_conv(cvt,1), 2, NULL);
  diet_arg_cvt_short_set(diet_arg_conv(cvt,2), 0, NULL);
  diet_arg_cvt_short_set(diet_arg_conv(cvt,3), 3, NULL);
  diet_arg_cvt_short_set(diet_arg_conv(cvt,4), 4, NULL);
#if HAVE_CORI
 /* Add a plugin scheduler for this case */
  set_up_scheduler(argv[2],profile);
#endif //HAVE_CORI
  /* Add */
  if (diet_service_table_add(profile, cvt, solve_dgemm)) return 1;
  diet_profile_desc_free(profile);


  /*
   * Adding MatPROD
   */
  /* Set profile */
  profile = diet_profile_desc_alloc(SRV[1], 1, 1, 2);
  diet_generic_desc_set(diet_param_desc(profile,0), DIET_MATRIX, DIET_DOUBLE);
  diet_generic_desc_set(diet_param_desc(profile,1), DIET_MATRIX, DIET_DOUBLE);
  diet_generic_desc_set(diet_param_desc(profile,2), DIET_MATRIX, DIET_DOUBLE);
  /* Set convertor */
  diet_arg_cvt_short_set(diet_arg_conv(cvt,0), 0, NULL);
  diet_arg_cvt_short_set(diet_arg_conv(cvt,1), 1, NULL);
  {
    double alpha = 1.0;
    arg = (diet_arg_t*) calloc(1, sizeof(diet_arg_t));
    diet_scalar_set(arg, &alpha, DIET_VOLATILE, DIET_DOUBLE);
  }
  diet_arg_cvt_short_set(diet_arg_conv(cvt,2), -1, arg);
  {
    double beta = 0.0;
    arg = (diet_arg_t*) calloc(1, sizeof(diet_arg_t));
    diet_scalar_set(arg, &beta, DIET_VOLATILE, DIET_DOUBLE);
  }
  diet_arg_cvt_short_set(diet_arg_conv(cvt,3), -1, arg);
  diet_arg_cvt_short_set(diet_arg_conv(cvt,4),  2, NULL);
#if HAVE_CORI
 /* Add a plugin scheduler for this case */
  set_up_scheduler(argv[2],profile);
#endif //HAVE_CORI
  /* Add */
  if (diet_service_table_add(profile, cvt, solve_dgemm)) return 1;
  diet_profile_desc_free(profile);


  /*
   * Adding SqMatSUM (IN, IN, OUT)
   */
  /* Set profile */
  profile = diet_profile_desc_alloc(SRV[2], 1, 1, 2);
  diet_generic_desc_set(diet_param_desc(profile,0), DIET_MATRIX, DIET_DOUBLE);
  diet_generic_desc_set(diet_param_desc(profile,1), DIET_MATRIX, DIET_DOUBLE);
  diet_generic_desc_set(diet_param_desc(profile,2), DIET_MATRIX, DIET_DOUBLE);
  /* Set convertor
     NB: the difficulty here is that the INOUT parameter of the dgemm
         corresponds to the second IN AND the OUT parameter of the profile. */
  diet_arg_cvt_short_set(diet_arg_conv(cvt,0), 0, NULL);
  {
    arg = (diet_arg_t*) calloc(1, sizeof(diet_arg_t));
    diet_matrix_set(arg, NULL, DIET_VOLATILE,
		    DIET_DOUBLE, 0, 0, DIET_COL_MAJOR);
  }
  diet_arg_cvt_short_set(diet_arg_conv(cvt,1), -1, arg);
  {
    double alpha = 1.0;
    arg = (diet_arg_t*) calloc(1, sizeof(diet_arg_t));
    diet_scalar_set(arg, &alpha, DIET_VOLATILE, DIET_DOUBLE);
  }
  diet_arg_cvt_short_set(diet_arg_conv(cvt,2), -1, arg);
  // beta is the same
  diet_arg_cvt_short_set(diet_arg_conv(cvt,3), -1, arg);
  // Use all arguments of diet_arg_cvt_set, since out_arg_idx == 2 (the OUT
  // parameter of the profile) and in_arg_idx == 1 (the second IN parameter).
  diet_arg_cvt_set(diet_arg_conv(cvt,4), DIET_CVT_IDENTITY, 1, NULL, 2);
#if HAVE_CORI
 /* Add a plugin scheduler for this case */
  set_up_scheduler(argv[2],profile);
#endif //HAVE_CORI
  /* Add */
  if (diet_service_table_add(profile, cvt, solve_dgemm)) return 1;
  diet_profile_desc_free(profile);


  /*
   * Adding SqMatSUM_opt (IN, INOUT)
   */
  /* Set profile */
  profile = diet_profile_desc_alloc(SRV[3], 0, 1, 1);
  diet_generic_desc_set(diet_param_desc(profile,0), DIET_MATRIX, DIET_DOUBLE);
  diet_generic_desc_set(diet_param_desc(profile,1), DIET_MATRIX, DIET_DOUBLE);
  /* Set convertor */
  diet_arg_cvt_short_set(diet_arg_conv(cvt,0), 0, NULL);
  {
    arg = (diet_arg_t*) calloc(1, sizeof(diet_arg_t));
    diet_matrix_set(arg, NULL, DIET_VOLATILE,
		    DIET_DOUBLE, 0, 0, DIET_COL_MAJOR);
  }
  diet_arg_cvt_short_set(diet_arg_conv(cvt,1), -1, arg);
  {
    double alpha = 1.0;
    arg = (diet_arg_t*) calloc(1, sizeof(diet_arg_t));
    diet_scalar_set(arg, &alpha, DIET_VOLATILE, DIET_DOUBLE);
  }
  diet_arg_cvt_short_set(diet_arg_conv(cvt,2), -1, arg);
  // beta is the same
  diet_arg_cvt_short_set(diet_arg_conv(cvt,3), -1, arg);
  diet_arg_cvt_short_set(diet_arg_conv(cvt,4), 1, NULL);
#if HAVE_CORI
 /* Add a plugin scheduler for this case */
  set_up_scheduler(argv[2],profile);
#endif //HAVE_CORI
  /* Add */
  if (diet_service_table_add(profile, cvt, solve_dgemm)) return 1;
  diet_profile_desc_free(profile);


  /*
   * Adding MatScalMult
   */
  /* Set profile */
  profile = diet_profile_desc_alloc(SRV[4], 0, 1, 1);
  // beta and C
  diet_generic_desc_set(diet_param_desc(profile,0), DIET_SCALAR, DIET_DOUBLE);
  diet_generic_desc_set(diet_param_desc(profile,1), DIET_MATRIX, DIET_DOUBLE);
  /* Set convertor */
  {
    arg = (diet_arg_t*) calloc(1, sizeof(diet_arg_t));
    // 1x1 matrix (to force dimensions set, because 0 is ignored)
    diet_matrix_set(arg, NULL, DIET_VOLATILE, DIET_DOUBLE, 1, 1, DIET_COL_MAJOR);
  }
  diet_arg_cvt_short_set(diet_arg_conv(cvt,0), -1, arg);
  diet_arg_cvt_short_set(diet_arg_conv(cvt,1), -1, arg);
  {
    double alpha = 0.0;
    arg = (diet_arg_t*) calloc(1, sizeof(diet_arg_t));
    diet_scalar_set(arg, &alpha, DIET_VOLATILE, DIET_DOUBLE);
  }
  diet_arg_cvt_short_set(diet_arg_conv(cvt,2), -1, arg);
  diet_arg_cvt_short_set(diet_arg_conv(cvt,3),  0, NULL);
  diet_arg_cvt_short_set(diet_arg_conv(cvt,4),  1, NULL);

#if HAVE_CORI
 /* Add a plugin scheduler for this case */
  set_up_scheduler(argv[2],profile);
#endif //HAVE_CORI

  /* Add */
  if (diet_service_table_add(profile, cvt, solve_dgemm)) return 1;
  diet_profile_desc_free(profile);

  /* The same cvt has been used for all services, free it now */
  diet_convertor_free(cvt);
  
  res = diet_SeD(argv[1], argc, argv);
  // Not reached
  return res;
}

#if HAVE_CORI

struct liste_t{
   int actual_size;
   double *array;
}list_lastexec;

void 
set_up_scheduler(char * schedulertype, diet_profile_desc_t* profile){
  if (strcmp(schedulertype,"NWS")){
   //NWSScheduler is called by default when FAST doesn't exist-> skip 
   return;

  diet_aggregator_desc_t *agg;
    agg = diet_profile_desc_aggregator(profile);
    
  if (strcmp(schedulertype,"LOADAVG")){    
    diet_service_use_perfmetric(performance_Load_Avg);
    diet_aggregator_set_type(agg, DIET_AGG_PRIORITY);
    diet_aggregator_priority_min(agg, EST_AVGFREECPU); 
  }
  if (strcmp(schedulertype,"RRALLHARDWARE")){ 
    list_lastexec.actual_size=0;
    list_lastexec.array=NULL;
    diet_service_use_perfmetric(performance_All_Hardware);
    diet_aggregator_set_type(agg, DIET_AGG_PRIORITY);
    diet_aggregator_priority_min(agg, EST_USERDEFINED); 
  }
  if (strcmp(schedulertype,"MEMFREE")){
    diet_service_use_perfmetric(performance_Mem_Free);
    diet_aggregator_set_type(agg, DIET_AGG_PRIORITY);
    diet_aggregator_priority_max(agg, EST_FREEMEM);
  }
  if (strcmp(schedulertype,"WRITE")){
    diet_service_use_perfmetric(performance_Disk_Write);
    diet_aggregator_set_type(agg, DIET_AGG_PRIORITY);
    diet_aggregator_priority_max(agg, EST_DISKACCESWRITE);
  }
  if (strcmp(schedulertype,"READ")){
    diet_service_use_perfmetric(performance_Disk_Read);
    diet_aggregator_set_type(agg, DIET_AGG_PRIORITY);
    diet_aggregator_priority_max(agg, EST_DISKACCESWRITE);
  }
  if (strcmp(schedulertype,"RRNBPROC")){
    list_lastexec.actual_size=0;
    list_lastexec.array=NULL;
    diet_service_use_perfmetric(performance_RRNBPROC);
    diet_aggregator_set_type(agg, DIET_AGG_PRIORITY);
    diet_aggregator_priority_max(agg,EST_USERDEFINED);
  }
}

void 
add_new_value(double valToAdd, int size){
  if (list_lastexec.actual_size==0){
    list_lastexec.actual_size=1;
    list_lastexec.array=(double *)calloc(size,sizeof(double));
    list_lastexec.array[0]=valToAdd;
  }
  else
    if (size== list_lastexec.actual_size){ //the maximal size is reached
      int i;
      for (i=size-1;i<0;i--)
      list_lastexec.array[i]=list_lastexec.array[i-1];
    }
    else{
      list_lastexec.actual_size++;
      int i;
      for (i=list_lastexec.actual_size;i<0;i--)
      list_lastexec.array[i]=list_lastexec.array[i-1];
      list_lastexec.array[0]=valToAdd;
    }
}

void
performance_RRNBPROC(diet_profile_t* pb,estVector_t perfValues)
{
  diet_estimate_cori(perfValues,EST_NBCPU, EST_COLL_EASY,NULL);
  
  /*store the timestamp since last execution */
  diet_estimate_lastexec(perfValues, pb);

 // stock the value in list_lastexec (the most recent is on index 0)
  add_new_value(diet_est_get(perfValues,EST_TIMESINCELASTSOLVE,0),
		(int)diet_est_get(perfValues,EST_NBCPU,1));
 
  //if the stocked number of executions is less then the number of CPUs, then
  // stock in the perfValues on tag userdefine only the number of cpu * 10000
  // this is to prevent that an low number-CPU-SeD receive too fast a new task
  if (list_lastexec.actual_size<diet_est_get(perfValues,EST_NBCPU,1)){
    diet_est_set(perfValues,0,diet_est_get(perfValues,EST_NBCPU,1)*100000);
  }
  else{  
     //stock the last element of vector_values in the scalar of perfValues
    diet_est_set(perfValues,0,list_lastexec.array[list_lastexec.actual_size-1]);
  }
}

void
performance_Load_Avg(diet_profile_t* pb,
		    estVector_t perfValues )
{
  int minute = 15;
  diet_estimate_cori(perfValues,EST_AVGFREECPU, EST_COLL_EASY,&minute);
}

void
performance_Mem_Free(diet_profile_t* pb,
		     estVector_t perfValues)
{
  diet_estimate_cori(perfValues,EST_FREEMEM, EST_COLL_EASY,NULL);
}

void
performance_Disk_Write(diet_profile_t* pb,
		       estVector_t perfValues)
{
  char*path="./";
  diet_estimate_cori(perfValues,EST_DISKACCESWRITE, EST_COLL_EASY,path);
}

void
performance_Disk_Read(diet_profile_t* pb,
		      estVector_t perfValues)
{
  char*path="./";
  diet_estimate_cori(perfValues,EST_DISKACCESREAD, EST_COLL_EASY,path);
}

void performance_Total_Disk_Size(diet_profile_t* pb,
				 estVector_t perfValues)
{
  char*path="./";
  diet_estimate_cori(perfValues,EST_TOTALSIZEDISK, EST_COLL_EASY,path);
}

void performance_Free_Disk_Size(diet_profile_t* pb,
				estVector_t perfValues)
{
  char*path="./";
  diet_estimate_cori(perfValues,EST_FREESIZEDISK, EST_COLL_EASY,path);
}

const double MULTIPLICATOR_MEM= 1/1000
const double MULTIPLICATOR_NBCPU=1
const double MULTIPLICATOR_FREQUENCE=1/500
const double MULTIPLICATOR_BOGOMIPS=1/500

/**
 * like performance_RRNBPROC, the number of last executions in a list 
 * depends on the a criteria.
 */
void
performance_All_Hardwarestatic(diet_profile_t* pb,
			       estVector_t perfValues)
{

/*store the timestamp since last execution */
  diet_estimate_lastexec(perfValues, pb);

  diet_estimate_cori(perfValues,EST_CPUSPEED, EST_COLL_EASY,NULL);
  diet_estimate_cori(perfValues,EST_TOTALMEM, EST_COLL_EASY,NULL);
  diet_estimate_cori(perfValues,EST_BOGOMIPS, EST_COLL_EASY,NULL);
  diet_estimate_cori(perfValues,EST_NBCPU, EST_COLL_EASY,NULL);
  /* the number of entries depends on the Hardware values
  * each harware information gives 'points' to the SeD*/
  int nbproc=(int)diet_est_get(perfValues,EST_NBCPU,1);
  int nbproc_points=nbproc*MULTIPLICATOR_NBCPU;
  int mem_points= diet_est_get(perfValues,
			       EST_NBCPU,
			       0)*MULTIPLICATOR_MEM;
  int frequence_points=0;
  for (int i=0, i<=nbproc,i++)
    frequence_points+=diet_est_get(perfValues,
				   EST_NBCPU,
				   i,
				   0)*MULTIPLICATOR_FREQUENCE;
  int bogmips_points=0;
  for (int i=0, i<=nbproc,i++)
    bogmips_points+=diet_est_get(perfValues,
				 EST_BOGOMIPS,
				 i,
				 0)*MULTIPLICATOR_BOGOMIPS;
  
  int total_points=nbproc_points+mem_points+frequence_points+bogmips_points;
  
  add_new_value(diet_est_get(perfValues,
			     EST_TIMESINCELASTSOLVE,
			     0),
		total_points);
 
  // if the stocked number of executions is less then the total_points, then
  // stock in the perfValues on tag userdefine only total_points * 10000
  // this is to prevent that an low total_points-SeD receive too fast a new task
  if (list_lastexec.actual_size<diet_est_get(perfValues,EST_NBCPU,1)){
    diet_est_set(perfValues,0,total_points*100000);
  }
  else{  
     //stock the last element of vector_values in the scalar of perfValues
    diet_est_set(perfValues,0,list_lastexec.array[list_lastexec.actual_size-1]);
  }
}




#endif //HAVE_CORI
