/****************************************************************************/
/* Batch System Management with performance prediction: PBS                 */
/*                                                                          */
/* Author(s):                                                               */
/*    - Benjamin Depardon (Benjamin.Depardon@ens-lyon.fr)                   */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.1  2008/08/17 08:10:19  bdepardo
 * Added PBS batch system
 * /!\ not tested yet
 *
 *
 ****************************************************************************/

#include "debug.hh"
#include "PBS_BatchSystem.hh"

const char * PBS_BatchSystem::statusNames[] = {
  "E", // not OK: there is no error state in PBS, either the job is
  "E", //         running, or it has exited
  "E", //         So, how to decide if all is ok? Parse the error file?
  "R", // Job is Running
  "H", // Job is Held
  "Q", // Job is Queued, eligible to run or be routed
  "S", // ?? Job is Suspended
  "W"  // ?? Job is waiting for its requested execution time to be reached
       //   or job specified a stage-in request which failed for some reason
} ;

PBS_BatchSystem::PBS_BatchSystem(int ID, const char * batchname)
{
  if( pathToNFS == NULL ) {
    ERROR_EXIT("PBS needs a path to a NFS directory to store its script\n") ;
  }
#if defined YC_DEBUG
  TRACE_TEXT(TRACE_ALL_STEPS,"Nom NFS: " << getNFSPath() << "\n") ;
#endif

  batch_ID = ID ;
  batchName = batchname ;
  
  shell    = BatchSystem::emptyString ;
  prefixe  = "#!/bin/sh\n\n" ;
  postfixe = "#PBS -V\n" ; // the -V option declares that all
                           // environment variables in the qsub
                           // command's environment are to be exported
                           // to the batch job

  nodesNumber       = "#PBS -l nodes=" ;
  serial            = "#PBS -l nodes=1" ;
  walltime          = "#PBS -l walltime=" ;
  submittingQueue   = "#PBS -q " ;
  minimumMemoryUsed = BatchSystem::emptyString ;
  
  /* TODO: When we use some ID for DIET client, change there! */
  mail      = "#PBS -m a\n#PBS -M " ; // -m, send mail when:
                                      // a: job is aborted by batch system
                                      // b: job begins execution
                                      // e: job ends execution
                                      // n: do not send mail
                                      // -M, recipient list
  account   = BatchSystem::emptyString ;
  setSTDOUT = "#PBS -o " ;
  setSTDIN  = BatchSystem::emptyString ;
  setSTDERR = "#PBS -e " ;

  submitCommand = "qsub " ;
  killCommand   = "qdel " ;
  wait4Command  = "qstat -f " ;
  waitFilter    = "grep job_state | cut --delimiter== --field=2 | cut --delimiter=\" \" --field=2" ;
  exitCode      = "0" ;
  
  jid_extract_patterns = "cut --delimiter=\\\" -f 2 | cut --delimiter=. -f 2" ;

  /* Information for META_VARIABLES */
  batchJobID     = "$PBS_JOBID" ;
  nodeFileName   = "$PBS_NODEFILE" ;
  nodeIdentities = "cat $PBS_NODEFILE" ;  
}

PBS_BatchSystem::~PBS_BatchSystem()
{
}

/*********************** Job Managing ******************************/

BatchSystem::batchJobState
PBS_BatchSystem::askBatchJobStatus(int batchJobID)
{
  char * filename ;
  int file_descriptor ;
  char * chaine ;
  int i=0 ;
  int nbread ;
  batchJobState status ;
  
  /* If job has completed, not ask batch system */
  status = getRecordedBatchJobStatus( batchJobID ) ;
  if( (status == TERMINATED) || (status == CANCELED) || (status == ERROR) )
    return status ;
  /* create a temporary file to get results and batch job ID */
  filename = createUniqueTemporaryTmpFile("DIET_batch_finish") ;
  file_descriptor = open(filename,O_RDONLY) ;
  if( file_descriptor == -1 ) {
    ERROR("Cannot open file", UNDETERMINED ) ;
  }

  /*** Ask batch system the job status ***/      
  chaine = (char*)malloc(sizeof(char)*(strlen(wait4Command)
				       + NBDIGITS_MAX_BATCH_ID
				       + strlen(waitFilter)
				       + strlen(filename)
				       + 7 + 1) ) ;
  /* See EOF to get an example of what we parse */
  sprintf(chaine,"%s %d | %s > %s",
	  wait4Command,batchJobID,waitFilter,filename) ;
#if defined YC_DEBUG
  TRACE_TEXT(TRACE_ALL_STEPS,"Execute:\n " << chaine << "\n") ;
#endif
  if( system(chaine) != 0 ) {
    ERROR("Cannot submit script", NB_STATUS) ;
  }

  /* Get job status */  
  for( int i = 0 ; i<=NBDIGITS_MAX_BATCH_ID ; i++ )
    chaine[i] = '\0' ;

  if( (nbread=readn(file_descriptor,chaine,NBDIGITS_MAX_JOB_STATUS)) == 0 ) {
    ERROR("Error with I/O file. Cannot read the batch status", NB_STATUS) ;
  }

  if( nbread == 0 )
    /* we consider that like OK */
    i=TERMINATED ;
  else {
    /* Adjust what have been read */
    if( chaine[nbread-1] == '\n' )
      chaine[nbread-1] = '\0' ;
    /* Compare to chaine+1 because of a space as a first char */
    while( (i<NB_STATUS) && 
	   (strcmp(chaine+1,PBS_BatchSystem::statusNames[i])!=0) ) {
      i++ ;
    }
  }
    
  if( i==NB_STATUS ) {
    ERROR("Cannot get batch job " << batchJobID << " status: " << chaine
	  << "\n",NB_STATUS) ;
  }
  /* Remove temporary file by closing it */
#if REMOVE_BATCH_TEMPORARY_FILE
  unlink( filename ) ;
#endif
  if( close(file_descriptor) != 0 ) {
    WARNING("Couln't remove I/O redirection file") ;
  }
  updateBatchJobStatus(batchJobID,(batchJobState)i) ;
  free(chaine) ;
  free(filename) ;
  return (batchJobState)i ;
}

int
PBS_BatchSystem::isBatchJobCompleted(int batchJobID)
{
  int status = getRecordedBatchJobStatus(batchJobID) ;
  
  if( (status == TERMINATED) || (status == CANCELED) || (status == ERROR) )
    return 1 ;
  status = askBatchJobStatus(batchJobID) ;
  if( (status == TERMINATED) || (status == CANCELED) || (status == ERROR) )
    return 1 ;
  else if( status == NB_STATUS )
    return -1 ;
  return 0 ;
}

/********** Batch static information accessing Functions **********/

int
PBS_BatchSystem::getNbResources()
{
  INTERNAL_WARNING(__FUNCTION__ << " not yet implemented\n\n") ;
  return getNbTotResources() ;
}

int
PBS_BatchSystem::getNbTotResources()
{
  return launchCommandAndGetInt( "qstat -Bf | grep resources_assigned.nodect | cut --delimiter== --field=2 | cut --delimiter=\" \" --field=2",
				 "DIET_getNbResources") ;
}

int
PBS_BatchSystem::getMaxWalltime()
{
  return launchCommandAndGetInt("qstat -Qf | grep mtime | cut --delimiter== --field=2 | cut --delimiter=\" \" --field=2",
				 "DIET_getNbResources") ;
}

int
PBS_BatchSystem::getMaxProcs()
{
  INTERNAL_WARNING(__FUNCTION__ << " not yet implemented\n\n") ;
  return getNbResources() ;
}

/********** Batch dynamic information accessing Functions *********/

int
PBS_BatchSystem::getNbTotFreeResources()
{
  INTERNAL_WARNING(__FUNCTION__ << " not yet implemented\n\n") ;
  return getNbResources() ;
}

int
PBS_BatchSystem::getNbFreeResources()
{
  INTERNAL_WARNING(__FUNCTION__ << " not yet implemented\n\n") ;
  return getNbResources() ;
}

/*************************** Performance Prediction *************************/


/************************* Examples of commands ****************************/
/*
[eddy@breeze ~]$ qstat -f 70.breeze.ics.hawaii.edu
Job Id: 70.breeze.ics.hawaii.edu
    Job_Name = Anacle_8
    Job_Owner = davidls@breeze.ics.hawaii.edu
    resources_used.cput = 43:38:36
    resources_used.mem = 54412kb
    resources_used.vmem = 152520kb
    resources_used.walltime = 24:06:40
    job_state = R
    queue = default
    server = breeze.ics.hawaii.edu
    Checkpoint = u
    ctime = Fri Aug 15 17:26:21 2008
    Error_Path = breeze.ics.hawaii.edu:/home/davidls/anacle/scripts/Anacle_8.e
	70
    exec_host = compute-0-0.local/1+compute-0-0.local/0
    Hold_Types = n
    Join_Path = n
    Keep_Files = n
    Mail_Points = a
    mtime = Fri Aug 15 17:26:22 2008
    Output_Path = breeze.ics.hawaii.edu:/home/davidls/anacle/scripts/Anacle_8.
	o70
    Priority = 0
    qtime = Fri Aug 15 17:26:21 2008
    Rerunable = True
    Resource_List.nodect = 1
    Resource_List.nodes = 1:ppn=2
    session_id = 19620
    etime = Fri Aug 15 17:26:21 2008

*/
