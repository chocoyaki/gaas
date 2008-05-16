/****************************************************************************/
/* Container for SeD jobs status information                                */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Benjamin ISNARD (benjamin.isnard@ens-lyon.fr)                       */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.1  2008/05/16 12:26:39  bisnard
 * new class JobQueue to manage list of jobs on the SeD
 *
 ****************************************************************************/

#include "JobQueue.hh"


JobQueue::JobQueue() : nbActiveJobs(0) { };
JobQueue::~JobQueue() { };

bool
JobQueue::addJob(diet_profile_t& profile, diet_job_status_t status) {
  diet_job_t newJob = { &profile, status, -1 };
  this->myLock.lock();        /** LOCK */
  myJobs[profile.dietReqID] = newJob;
  this->myLock.unlock();      /** UNLOCK */
  if (status != DIET_JOB_FINISHED)
    this->nbActiveJobs++;
  return true;
}

bool
JobQueue::setJobStarted(int dietReqID) {
  myJobs[dietReqID].status = DIET_JOB_RUNNING;
  // set start time
  struct timeval current_time;
  gettimeofday(&current_time, NULL);
  myJobs[dietReqID].startTime = current_time.tv_sec;
}

bool
JobQueue::setJobFinished(int dietReqID) {
  myJobs[dietReqID].status = DIET_JOB_FINISHED;
  this->nbActiveJobs--;
}

bool
JobQueue::deleteJob(int dietReqID) {
  map<int, diet_job_t>::iterator p = myJobs.find(dietReqID);
  if (p != myJobs.end()) {
    diet_job_t job = p->second;
    if (job.status != DIET_JOB_FINISHED) {
      this->nbActiveJobs--;
    }
    myJobs.erase(p);
    return true;
  }
  else return false;
}

int
JobQueue::getActiveJobTable(jobVector_t& jobVector) {
  this->myLock.lock();        /** LOCK */
  jobVector = (diet_job_t *) malloc(sizeof(diet_job_t) * this->nbActiveJobs);
  map<int, diet_job_t>::iterator p = myJobs.begin();
  int i=0;
  while (p != myJobs.end()) {
    jobVector[i] = p->second;
    p++;
    i++;
  }
  this->myLock.unlock();      /** UNLOCK */
  return this->nbActiveJobs;
}

