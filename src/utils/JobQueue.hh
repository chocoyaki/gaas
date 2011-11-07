/**
 * @file  JobQueue.hh
 *
 * @brief  Container for SeD jobs status information
 *
 * @author  Benjamin ISNARD (benjamin.isnard@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENCE|
 */


#ifndef _JOBQUEUE_HH_
#define _JOBQUEUE_HH_

#include <ctime>
#include <list>
#include <map>
#include <omnithread.h>
#include "DIET_data_internal.hh"
#include "debug.hh"

class JobQueue {
public:
  explicit
  JobQueue(int maxConcurrentJobsNb = 1);

  ~JobQueue();

  /**
   * Add a job in the waiting queue for the SeD
   * (called when the SOLVE request arrives)
   * @param dietReqID the identifier of the request
   * @param jobEstCompTime  the estimated duration of the job
   * @param ev  the vector of estimations (for bw compatibility)
   */
  void
  addJobWaiting(int dietReqID, double jobEstCompTime, corba_estimation_t &ev);

  /**
   * Change the status of the job to RUNNING
   * (called when the SOLVE request gets a ressource)
   * @param dietReqId the identifier of the request
   */
  bool
  setJobStarted(int dietReqId);

  /**
   * Change the status of the job to FINISHED
   * @param dietReqId the identifier of the request
   */
  bool
  setJobFinished(int dietReqId);

  bool
  deleteJob(int dietReqId);

  /**
   * @deprecated (kept for backward compatibility)
   * Get the stored list of jobs
   * @param jobVector ref to job table
   */
  int
  getActiveJobTable(jobVector_t &jobVector);

  /**
   * Earliest Finish Time estimation using FIFO scheduling
   * This method consider all running and waiting jobs in the queue
   * and calculates the EFT using maxConcurrentJobsNb processors.
   * @return  the EFT value from now (in ms)
   */
  double
  estimateEFTwithFIFOSched();

private:
  // / Container for the list of jobs
  std::map<int, diet_job_t>  myJobs;

  // / FIFO queue to store waiting jobs (for EFT processing)
  std::list<int>   myWaitingQueue;

  // / Lock for thread-safe access to variables
  omni_mutex myLock;

  // / Counter of active jobs (ie not finished)
  int nbActiveJobs;

  // / Number of processors
  int nbProc;
};

#endif  // _JOBQUEUE_HH_
