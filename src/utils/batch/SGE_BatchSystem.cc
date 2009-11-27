/****************************************************************************/
/* Batch System Management with performance prediction: SGE                 */
/*                                                                          */
/* Author(s):                                                               */
/*    - Yves Caniou (yves.caniou@ens-lyon.fr)                               */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.5  2009/11/27 03:24:30  ycaniou
 * Add user_command possibility before the end of Batch prologue (only
 * to be used for batch dependent code!)
 * Memory leak/segfault--
 * New easy Batch basic example
 * Management of OAR2_X Batch scheduler
 *
 * Revision 1.4  2009/11/19 14:45:01  ycaniou
 * Walltime in profile is in seconds
 * Renamed Global var
 * Management of time HH:MM:SS in batch scripts
 *
 * Revision 1.3  2008/05/11 16:19:51  ycaniou
 * Check that pathToTmp and pathToNFS exist
 * Check and eventually correct if pathToTmp or pathToNFS finish or not by '/'
 * Rewrite of the propagation of the request concerning job parallel_flag
 * Implementation of Cori_batch system
 * Numerous information can be dynamically retrieved through batch systems
 *
 * Revision 1.2  2008/04/07 13:11:44  ycaniou
 * Correct "deprecated conversion from string constant to 'char*'" warnings
 * First attempt to code functions to dynamicaly get batch information
 * 	(e.g.,  getNbMaxResources(), etc.)
 *
 * Revision 1.1  2008/01/04 12:49:46  ycaniou
 * Added those two files, but there is no glue for SGE at the moment
 *
 *
 ****************************************************************************/

#include "debug.hh"
#include "SGE_BatchSystem.hh"

const char * const SGE_BatchSystem::statusNames[] = {
  "Error", // not OK: sucks, because error or terminated
  "Error", // not OK  it seems that LL reports no info with llq
  "Error", // not OK  so, how to decide if all is ok? Parse the error file?
  "running",
  "waiting",
  "pending",
  "preempted"
} ;

SGE_BatchSystem::SGE_BatchSystem(int ID, const char * batchname)
{
  /* FIXME: Dont know if this is true -> Check it! */

  if( pathToNFS == NULL ) {
    ERROR_EXIT("SGE needs a path to a NFS directory to store its script\n") ;
  }
#if defined YC_DEBUG
  TRACE_TEXT(TRACE_ALL_STEPS,"Nom NFS: " << getNFSPath() << "\n") ;
#endif

  batch_ID = ID ;
  batchName = batchname ;
  
  shell = "-S " ;
  prefixe = "#!/bin/sh" ;
  postfixe = BatchSystem::emptyString ;

  nodesNumber = "#@ job_type=parallel\n#@ node=" ;
  serial = "#@ job_type = serial" ;
  coresNumber = BatchSystem::emptyString ;
  walltime = "\n#@ wall_clock_limit=" ;
  submittingQueue = "\n#@ Class= " ;
  minimumMemoryUsed = BatchSystem::emptyString ;
  
  mail = "\n#@ notification=always\n#@ notify_user=" ;
  account = "\n#@ account_no=" ;
  setSTDOUT = "\n#@ output=" ;
  setSTDIN = BatchSystem::emptyString ;
  setSTDERR = "\n#@ error=" ;

  /* Here is an output of a submission */
  submitCommand = "llsubmit " ;
  killCommand = "llcancel " ;
  wait4Command = "llq -j " ;
  waitFilter = "grep step | cut --delimiter=1 --field=3 | cut --delimiter=\",\" -- field=1" ;
  exitCode = "0" ;
  
  jid_extract_patterns = "cut --delimiter=\" -f 2 | cut --delimiter=. -f 2" ;

  /* Information for META_VARIABLES */
  batchJobID = "$LOADL_STEP_ID" ;
  nodeFileName = "$SAVEHOSTFILE" ;
  nodeIdentities = "cat $MP_SAVEHOSTFILE" ;  
}

SGE_BatchSystem::~SGE_BatchSystem()
{
}

/*********************** Job Managing ******************************/

BatchSystem::batchJobState
SGE_BatchSystem::askBatchJobStatus(int batchJobID)
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
  filename = (char*)malloc(sizeof(char)*strlen(pathToTmp) + 30 ) ;
  sprintf(filename,"%sDIET_batch_finish.XXXXXX", pathToTmp) ;
  file_descriptor = mkstemp( filename ) ;
  if( file_descriptor == -1 ) {
    ERROR("Cannot create batch I/O redirection file", NB_STATUS) ;
  }
#if defined YC_DEBUG
  TRACE_TEXT(TRACE_ALL_STEPS,"Fichier_finish: " << filename << "\n") ;
#endif

  /* Ask batch system the job status */      
  chaine = (char*)malloc(sizeof(char)*(strlen(wait4Command)
				       + NBDIGITS_MAX_BATCH_JOB_ID
				       + strlen(waitFilter)
				       + strlen(filename)
				       + 7 + 1) ) ;

  /* To determine the job status, we parse something like:

     bash-2.05b$ llq -j 15771
     Id                       Owner      Submitted   ST PRI Class        Running on
     ------------------------ ---------- ----------- -- --- ------------ -----------
     meso-d.15771.0           diet        7/3  19:22 R  50  q0_30m       meso-d
     
     1 job step(s) in query, 0 waiting, 0 pending, 1 running, 0 held, 0 preempted
     bash-2.05b$ llq -j 15768
     llq: There is currently no job status to report.
  */

  sprintf(chaine,"%s %d | %s > %s",
	  wait4Command,batchJobID,waitFilter,filename) ;
  if( system(chaine) != 0 ) {
    ERROR("Cannot submit script", NB_STATUS) ;
  }
  /* Get job status */  
  for( int i = 0 ; i<=NBDIGITS_MAX_BATCH_JOB_ID ; i++ )
    chaine[i] = '\0' ;
  if( (nbread=readn(file_descriptor,chaine,NBDIGITS_MAX_JOB_STATUS))
      == 0 ) {
    ERROR("Error with I/O file. Cannot read the batch status", NB_STATUS) ;
  }
  /* Adjust what have been read */
  if( chaine[nbread-1] == '\n' )
    chaine[nbread-1] = '\0' ;
  /* Compare to chaine+1 because of a space as a first char */
  while( (i<NB_STATUS) && 
	 (strcmp(chaine+1,SGE_BatchSystem::statusNames[i])!=0) ) {
    i++ ;
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
SGE_BatchSystem::isBatchJobCompleted(int batchJobID)
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
SGE_BatchSystem::getNbTotResources()
{
  INTERNAL_WARNING(__FUNCTION__ << " not yet implemented\n\n") ;
  return 16 ;
}

int
SGE_BatchSystem::getNbResources()
{
  INTERNAL_WARNING(__FUNCTION__ << " not yet implemented\n\n") ;
  return 16 ;
}

int
SGE_BatchSystem::getMaxWalltime()
{
  INTERNAL_WARNING(__FUNCTION__ << " not yet implemented\n\n") ;
  return 500 ;
}

int
SGE_BatchSystem::getMaxProcs()
{
  INTERNAL_WARNING(__FUNCTION__ << " not yet implemented\n\n") ;
  return getNbResources() ;
}

/********** Batch dynamic information accessing Functions *********/

int
SGE_BatchSystem::getNbTotFreeResources()
{
  INTERNAL_WARNING(__FUNCTION__ << " not yet implemented\n\n") ;
  return getNbResources() ;
}

int
SGE_BatchSystem::getNbFreeResources()
{
  INTERNAL_WARNING(__FUNCTION__ << " not yet implemented\n\n") ;
  return getNbResources() ;
}

/*************************** Performance Prediction *************************/

