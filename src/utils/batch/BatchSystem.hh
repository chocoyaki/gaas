/**
 * @file BatchSystem.hh
 *
 * @brief  Batch System Management with performance prediction
 *
 * @author  Yves Caniou (yves.caniou@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


#ifndef _BATCH_SYSTEM_HH_
#define _BATCH_SYSTEM_HH_

#include <iostream>
/* File checking */
#include <fcntl.h>       /* for O_RDONLY */
#include <sys/stat.h>
#include <omnithread.h>  // For omni_mutex
#include "DIET_data.h"



#define NBDIGITS_MAX_RESOURCES 4  // #char to code the # of computing resources
#define NBDIGITS_MAX_BATCH_JOB_ID 10  // #char to code the job ID, at least 8!
#define NBDIGITS_MAX_JOB_STATUS 15

// #define YC_DEBUG
#if defined YC_DEBUG
#define REMOVE_BATCH_TEMPORARY_FILE 0
#else
#define REMOVE_BATCH_TEMPORARY_FILE 1
#endif

class BatchSystem {
public:
  typedef enum {
	ERROR_JOB,
    CANCELED,
    TERMINATED,
    RUNNING,
    WAITING,
    SUBMITTED,
    PREEMPTED,  // Is this useful?
    UNDETERMINED,  // If error when trying to read the status. Begin again.
    NB_STATUS
  } batchJobState;

  static const char *emptyString;

  BatchSystem();

  virtual ~BatchSystem();

  // batchType
  int
  getBatchID();

  const char *
  getBatchName();

  const char *
  getBatchQueueName();

  const char *
  getNFSPath();

  const char *
  getTmpPath();

  /************************ Submitting Funtions *******************/

  /** Waits for the completion of the Diet Job defined by profile
      TODO
   */
  int
  wait4DietJobCompletion(diet_profile_t *profile);

  int
  checkIfDietJobCompleted(diet_profile_t *profile);

  /** Submit a job corresponding to given @param profile problem with a
      partially defined script contained in the string @param command .
      @param addon_prologue can be used for batch-dedicated (non inter-batch
      portable) script: corresponding commands are added before the termination
      of the batch prologue (e.g., QUEUE for LL).
      Returns -1 if error, 0 on succes.
   */
  virtual int
  diet_submit_parallel(diet_profile_t *profile,
                       const char *addon_prologue,
                       const char *command);

  /** Submit a job partially defined in a file
      TODO
   */
  int
  diet_submit_parallel(diet_profile_t *profile,
                       const FILE *fileName);

  /** Submit a job on a reservation or in concurrent with an already
      submitted job
      TODO
   */
  int
  diet_submit_parallel(int batchJobID, diet_profile_t *profile,
                       const char *command);

  /** Submits a job on a reservation or in concurrent with an already
      submitted job.
      TODO
   */
  int
  diet_submit_parallel(int batchJobID, diet_profile_t *profile,
                       const FILE *fileName);

  /*********************** Job Managing ******************************/
  /** Returns -1 on error, 0 otherwise */
  int
  storeBatchJobID(int batchJobID, int dietReqID, char *filename);

  /** Removes from the internal structure informations corresponding to the
      DIET request @param dietReqID .
      Returns -1 if not found.
   */
  int
  removeBatchJobID(int dietReqID);

  /** Returns the ID of batch job corresponding to the DIET
      request @param dietReqID .
      Returns -1 on error.
   */
  int
  getBatchJobID(int dietReqID);

  /** Waits for the completion of batch Job corresponding to Diet Request
      @param dietReqID . Checks all WAITING_BATCH_JOB_COMPLETION seconds.
   */
  int
  wait4BatchJobCompletion(int dietReqID);

  /** Reads the internal structure and give the state of batch job
      @param batchJobID .
      Returns NB_STATUS on error.
   */
  batchJobState
  getRecordedBatchJobStatus(int batchJobID);

  /** Updates internal structure for the batch job whose ID is
      @param batchJobID with the state @param job_status .
      Returns -1 if job has not been found.
   */
  int
  updateBatchJobStatus(int batchJobID, batchJobState job_status);

  /** If job not terminated, ask the batch system for the status of job
      whose ID is @param batchJobID .
      Updates the internal structure (status and unlink script file).
      Returns NB_STATUS on error, the status otherwise.
   */
  virtual batchJobState
  askBatchJobStatus(int batchJobID) = 0;

  /** If job whose id is @param batchJobID is:
      - not finished, returns 0
      - terminated, returns 1
      - not found, -1
   */
  virtual int
  isBatchJobCompleted(int batchJobID) = 0;

  /********** Batch static information accessing Functions **********/
  /* These should soon change for they assume a default queue and we
     want to be able to manage all queues of a system! */

  virtual int
  getNbTotResources() = 0;

  virtual int
  getNbResources() = 0;

  virtual int
  getMaxWalltime() = 0;

  virtual int
  getMaxProcs() = 0;

  /********** Batch dynamic information accessing Functions *********/
  /* These should soon change for they assume a default queue and we
     want to be able to manage all queues of a system! */

  virtual int
  getNbTotFreeResources() = 0;

  virtual int
  getNbFreeResources() = 0;

  /****************** Performance Prediction Functions ***************/

  /** Get the number of processors and the walltime for a Diet request
      by simulating the batch system.
      Returns -1 if unavailable, else 0
   */
  int
  getSimulatedProcAndWalltime(int *nbprocPtr, int *walltimePtr,
                              diet_profile_t *profilePtr);

protected:
  /****************** Utility functions ********************/
  /********** These should go in another class *************/

  /** Replace all occurences of @param occurence with @param by
      in the string @param input .
      Returns 1 if some replacement has been done, 0 if not, -1 on error.
   */
  int
  replaceAllOccurencesInString(char **input,
                               const char *occurence,
                               const char *by);

  /**
     Write @param n bytes of string @param buffer to the file
     descriptor @param fd .
     Returns 0 if error or the number of bytes writen (which can be 0).
   */

  size_t
  writen(int fd, const char *buffer, size_t n);

  /**
     Read at most @param n bytes from descriptor file @param fd to store
     them in @param buffer .
     Returns the number of bytes read (0 if error or no byte read).
   */
  int
  readn(int fd, char *buffer, int n);

  /** Check if @path is a writable directory. Quit the program if not. */
  void
  errorIfPathNotValid(const char *path);

  /* Creates a unique temporary file in the tmp path declared previously.
     \c pattern is used for information purpose. Returns a filename that
     the user must free when finished. */

  char *
  createUniqueTemporaryTmpFile(const char *pattern);

  /* Creates a unique temporary file in the NFS path declared previously.
     \c pattern is used for information purpose. Returns a filename that
     the user must free when finished. */

  char *
  createUniqueTemporaryNFSFile(const char *pattern);

  /** Read a number in the file \c filename. Only NBDIGITS_MAX_RESOURCES
      digits are read, thus the number of digits is assumed not to exceed
      this value. */
  int
  readNumberInFile(const char *filename);

  /** Exec a command (with system() system call) and get the result of
      the output in a unique temporary file made from \c pattern
      (maintly for debugging purpose). Assume the result in the file is an
      integer, read and return this value. Only NBDIGITS_MAX_RESOURCES
      digits are read, thus the number of digits is assumed not to exceed
      this value. */
  int
  launchCommandAndGetInt(const char *submitCommand,
                         const char *pattern);

  /** Exec a command (with system() system call) and get the result of
      the output in a unique temporary file made from \c pattern
      (maintly for debugging purpose). Returns the name of the file for
      further parsing. The user MUST unlink the file by his own. */
  char *
  launchCommandAndGetResultFilename(const char *submitCommand,
                                    const char *pattern);

  /************ Batch Configuration ************/
  /* (Fully qualified) frontal host name */
  char frontalName[256];

  int batch_ID;
  const char *batchName;   // The name of the batch system

  const char *batchQueueName;   // Only one queue managed

  /* The following must end with a '/' */
  const char *pathToNFS;  /* Some batch need this */
  const char *pathToTmp;

  /************ Batch Commands ************/
  const char *shell;  /* shell demandé pour lancer le script distant */
  const char *prefixe;
  const char *postfixe;

  /* The 2 following lines are LL tricks, for MPI or non-MPI jobs:
     LL needs to know if job is serial or //, but // can mean MPI with 1 proc.
   */
  const char *nodesNumber;   // If more than 1 proc
  const char *serial;       // If serial, proc == 1, DONT use nodesNumber!
  const char *coresNumber;   // for 0AR2.. not used
  const char *walltime;
  const char *submittingQueue;
  const char *minimumMemoryUsed;   // not managed for the moment

  const char *mail;
  const char *account;   // not used until so implements accounting in DIET

  const char *setSTDOUT;   // These are in place but not used
  const char *setSTDIN;   // -> I must define a DIET META VARIABLE which
  const char *setSTDERR;   // must be informed to take place after this line

  const char *submitCommand;
  const char *killCommand;
  const char *wait4Command;  /* Command to have info about a given jobID */
  const char *waitFilter;    /* Filter to access jobs status */
  const char *exitCode;

  const char *jid_extract_patterns;

  /************ Batch META-VARIABLES ************/
  const char *batchJobID;   // The script variable containing job ID
  const char *nodeFileName;
  const char *nodeIdentities;   // given by a string separated by a blank

  /* Correspondance between Diet reqIDs and Batch Job IDs stored as
     a chained list */

  typedef struct corresID_def {
    int dietReqID;
    int batchJobID;
    batchJobState status;  // Mostly used to log if ERROR/CANCELED | TERMINATED
    char *scriptFileName;   // To unlink the file when job is finished
    struct corresID_def *nextStruct;
  } corresID;
  corresID *batchJobQueue;

  omni_mutex corresBatchReqID_mutex;
};

#endif  // BATCH_SYSTEM
