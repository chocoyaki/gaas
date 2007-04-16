/****************************************************************************/
/* Batch System Management with performance prediction                      */
/*                                                                          */
/* Author(s):                                                               */
/*    - Yves Caniou (yves.caniou@ens-lyon.fr)                               */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.1  2007/04/16 22:35:19  ycaniou
 * Added the generic class for batch systems
 *
 *
 ****************************************************************************/

#ifndef _BATCH_SYSTEM_HH_
#define _BATCH_SYSTEM_HH_

#include <iostream>
#include <omnithread.h>       // For omni_mutex
#include "DIET_data.h"

#define NBDIGITS_MAX_BATCH_ID 10
#define NBDIGITS_MAX_JOB_STATUS 15

//#define YC_DEBUG
#if defined YC_DEBUG
#define REMOVE_BATCH_TEMPORARY_FILE 0
#else
#define REMOVE_BATCH_TEMPORARY_FILE 1
#endif
//#define YC_DEBUG_

class BatchSystem
{

public :

  typedef enum {
    ERROR,
    CANCELED,
    TERMINATED,
    RUNNING,
    WAITING,
    SUBMITTED,
    NB_STATUS
  } batchJobState ;

  static const char * emptyString ;

  BatchSystem() ;
  virtual ~BatchSystem() ;

  //  batchType
  int
  getBatchID() ;
  
  const char *
  getBatchName() ;
  
  const char *
  getBatchQueueName() ;

  const char *
  getNFSPath() ;
  
  const char *
  getTmpPath() ;
  
  /************************ Submitting Funtions *******************/

  /** Waits for the completion of the Diet Job defined by profile
      TODO
  */
  int
  wait4DietJobCompletion(diet_profile_t * profile) ;
  
  int
  checkIfDietJobCompleted(diet_profile_t * profile) ;
  
  /** Submit a job partially defined by a script contained in the string
      @param command.
      Returns -1 if error, 0 on succes.
  */
  int
  diet_submit_parallel(diet_profile_t * profile,
		       const char * command) ;

  /** Submit a job partially defined in a file 
      TODO
  */
  int
  diet_submit_parallel(diet_profile_t * profile,
		       const FILE * fileName) ;

  /** Submit a job on a reservation or in concurrent with an already
      submitted job 
      TODO
  */
  int
  diet_submit_parallel(int batchJobID, diet_profile_t * profile,
		       const char * command) ;

  /** Submits a job on a reservation or in concurrent with an already
      submitted job.
      TODO
  */
  int
  diet_submit_parallel(int batchJobID, diet_profile_t * profile,
		       const FILE * fileName) ;

  /*********************** Job Managing ******************************/    
  /** Returns -1 on error, 0 otherwise */
  int
  storeBatchJobID(int batchJobID, int dietReqID, char * filename) ;

  /** Removes from the internal structure informations corresponding to the
      DIET request @param dietReqID.
      Returns -1 if not found.
  */
  int
  removeBatchJobID(int dietReqID) ;

  /** Returns the ID of batch job corresponding to the DIET
      request @param dietReqID.
      Returns -1 on error.
  */
  int
  getBatchJobID(int dietReqID) ;
  
  /** Waits for the completion of batch Job corresponding to Diet Request
      @param dietReqID. Checks all WAITING_BATCH_JOB_COMPLETION seconds.
  */
  int
  wait4BatchJobCompletion(int dietReqID) ;

  /** Reads the internal structure and give the state of batch job 
      @param batchJobID.
      Returns NB_STATUS on error.
  */
  batchJobState
  getRecordedBatchJobStatus(int batchJobID) ;

  /** Updates internal structure for the batch job whose ID is 
      @param batchJobID with the state @param job_status.
      Returns -1 if job has not been found.
  */
  int
  updateBatchJobStatus(int batchJobID, batchJobState job_status) ;
    
  /** If job not terminated, ask the batch system for the status of job
      whose ID is @param batchJobID.
      Updates the internal structure (status and unlink script file).
      Returns NB_STATUS on error, the status otherwise.
  */
  virtual batchJobState
  askBatchJobStatus(int batchJobID) = 0 ;
  
  /** If job whose id is @param batchJobID is:
      - not finished, returns 0
      - terminated, returns 1
      - not found, -1
  */
  virtual int
  isBatchJobCompleted(int batchJobID) = 0 ;
      
  /****************** Performance Prediction Functions ***************/
  //  int
  //getNumberOfAvailableComputingResources()=0 ;

  /** Get the number of processors and the walltime for a Diet request
      by simulating the batch system.
      Returns -1 if unavailable, else 0
  */
  int
  getSimulatedProcAndWalltime(int * nbprocPtr, int * walltimePtr,
			      diet_profile_t * profilePtr) ;

protected :

  /****************** Utility functions ********************/
  /********** These should go in another class *************/

  /** Replace all occurences of @param occurence with @param by
      in the string @param input.
      Returns 1 if some replacement has been done, 0 if not, -1 on error.
  */
  int
  replaceAllOccurencesInString(char ** input, 
			       const char * occurence,
			       const char * by) ;
  
  /**
     Write @param n bytes of string @param vptr to the file 
     descriptor @param fd.
     Returns 0 if error or the number of bytes writen (which can be 0).
  */

  size_t
  writen(int fd, const char * buffer, size_t n) ;
  
  /** 
      Read at most @param n bytes from descriptor file @param fd to store
      them in @param buffer.
      Returns the number of bytes read (0 if error or no byte read).
  */
  int
  readn(int fd, char * buffer, int n) ;

  /************ Batch Configuration ************/
  /* (Fully qualified) frontal host name */
  char frontalName[256];
    
  int batch_ID ;
  const char * batchName ;
    
  const char * batchQueueName ; // Only one queue managed

  /* The following must end with a '/' */
  const char * pathToNFS ; /* Some batch need this */
  const char * pathToTmp ; 
  
  /************ Batch Commands ************/
  const char * prefixe ;
  
  const char * nodesNumber ;
  const char * nodeFileName ;
  const char * nodeIdentities ; // given by a string separated by a blank
  const char * walltime ;
  const char * submittingQueue ;
  const char * minimumMemoryUsed ; // not managed for the moment
  
  const char * mail ;
  const char * account ; // not used until so implements accounting in DIET
  const char * setSTDOUT ;
  const char * setSTDIN ;
  const char * setSTDERR ;

  const char * hostFile ;
    
  char * submitCommand ;
  char * killCommand ;
  char * wait4Command ;
  char * waitFilter ;
  char * exitCode ;
  
  char * jid_extract_patterns ;

  /* Correspondance between Diet reqIDs and Batch Job IDs stored as 
     a chained list */

  typedef struct corresID_def {
    int dietReqID ;
    int batchJobID ;
    batchJobState status ; // Mostly used to log if ERROR/CANCELED | TERMINATED
    char * scriptFileName ; // To unlink the file when job is finished
    struct corresID_def *nextStruct ;
  } corresID ;
  corresID * batchJobQueue ;

  omni_mutex corresBatchReqID_mutex ;

} ;

#endif // BATCH_SYSTEM
