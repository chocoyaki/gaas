/****************************************************************************/
/* Batch System Management with performance prediction: OAR v1.6            */
/*                                                                          */
/* Author(s):                                                               */
/*    - Yves Caniou (yves.caniou@ens-lyon.fr)                               */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.1  2007/04/16 22:35:55  ycaniou
 * Added the class for DIET to manage OAR v1.6
 *
 *
 ****************************************************************************/

#include "debug.hh"
#include "OAR1_6BatchSystem.hh"

const char * OAR1_6BatchSystem::statusNames[] = {
  "Error",
  "Error",
  "Terminated",
  "Running",
  "Waiting",
  "Launching",
} ;

OAR1_6BatchSystem::OAR1_6BatchSystem(int ID, const char * batchname)
{
  if( pathToNFS == NULL ) {
    ERROR_EXIT("OAR needs a path to a NFS directory to store its script\n") ;
  }
#if defined YC_DEBUG
  TRACE_TEXT(TRACE_ALL_STEPS,"Nom NFS: " << getNFSPath() << "\n") ;
#endif

  batch_ID = ID ;
  batchName = batchname ;
  
  prefixe = "#!/bin/sh\n" ;

  nodesNumber = "#OAR -l nodes=" ;
  nodeFileName = "$OAR_NODEFILE" ;
  nodeIdentities = "cat $OAR_NODEFILE" ;  
  walltime = "#OAR -l walltime=" ;
  submittingQueue = "#OAR -q " ;
  minimumMemoryUsed = BatchSystem::emptyString ;
  
  mail = BatchSystem::emptyString ;
  account = BatchSystem::emptyString ;
  setSTDOUT = BatchSystem::emptyString ;
  setSTDIN = BatchSystem::emptyString ;
  setSTDERR = BatchSystem::emptyString ;

  hostFile="$OAR_NODEFILE" ;
    
  /* cd, to be sure that OAR takes PWD and not /bin/pwd: for Grenoble */
  submitCommand = "cd . ; oarsub " ;
  killCommand = "oardel " ;
  wait4Command = "oarstat -j " ;
  waitFilter = "grep state | cut --delimiter== -f 2 | cut --delimiter=\" \" -f 2" ;
  exitCode = "0" ;
  
  jid_extract_patterns = "grep \"IdJob =\" | cut --delimiter== -f 2" ;

  /* OAR behaves with SQL scripts to reserve specials nodes */
  /* but the following line is not good enough: too less nodes */
  /*    ELBASE_NODETYPE,"#OAR -p \"hostname='%s'\"", */
}

OAR1_6BatchSystem::~OAR1_6BatchSystem()
{
  free( submitCommand ) ;
}

/*********************** Job Managing ******************************/

BatchSystem::batchJobState
OAR1_6BatchSystem::askBatchJobStatus(int batchJobID)
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
  while( (i<NB_STATUS) && 
	 (strcmp(chaine,OAR1_6BatchSystem::statusNames[i])!=0) ) {
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
OAR1_6BatchSystem::isBatchJobCompleted(int batchJobID)
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
OAR1_6BatchSystem::getNumberOfAvailableComputingResources()
{
  // oarnodes | grep [^ \t] | wc -l donne le nombre de noeuds
  {
    ERROR("This funtion is not implemented yet", 1) ;
  }
}