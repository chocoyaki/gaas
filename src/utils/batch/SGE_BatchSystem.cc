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
 * Revision 1.1  2008/01/04 12:49:46  ycaniou
 * Added those two files, but there is no glue for SGE at the moment
 *
 *
 ****************************************************************************/

#include "debug.hh"
#include "SGE_BatchSystem.hh"

const char * SGE_BatchSystem::statusNames[] = {
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
  prefixe = "#!/bin/sh\n\n" ;
  postfixe = BatchSystem::emptyString ;

  nodesNumber = "#@ job_type=parallel\n#@ node=" ;
  serial = "#@ job_type = serial" ;
  walltime = "#@ wall_clock_limit=" ;
  submittingQueue = "#@ Class= " ;
  minimumMemoryUsed = BatchSystem::emptyString ;
  
  mail = "#@ notification=always\n#@ notify_user=" ;
  account = "#@ account_no=" ;
  setSTDOUT = "#@ output=" ;
  setSTDIN = BatchSystem::emptyString ;
  setSTDERR = "#@ error=" ;

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
  free( submitCommand ) ;
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
				       + NBDIGITS_MAX_BATCH_ID
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
  for( int i = 0 ; i<=NBDIGITS_MAX_BATCH_ID ; i++ )
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

/*************************** Performance Prediction *************************/

int
SGE_BatchSystem::getNumberOfAvailableComputingResources()
{
  // oarnodes | grep [^ \t] | wc -l donne le nombre de noeuds
  {
    ERROR("This funtion is not implemented yet", 1) ;
  }
}