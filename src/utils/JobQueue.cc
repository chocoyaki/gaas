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
 * Revision 1.2  2008/05/19 14:45:08  bisnard
 * jobs added to the queue during submit instead of solve
 *
 * Revision 1.1  2008/05/16 12:26:39  bisnard
 * new class JobQueue to manage list of jobs on the SeD
 *
 ****************************************************************************/

#include "JobQueue.hh"


JobQueue::JobQueue() : nbActiveJobs(0) { };
JobQueue::~JobQueue() { };

bool
JobQueue::addJobEstimated(int dietReqID, corba_estimation_t& ev) {
  // make a copy of the estimation vector
  estVector_t estVect = new corba_estimation_t(ev);
  diet_job_t newJob = { estVect, DIET_JOB_ESTIMATED, -1 };
  this->myLock.lock();        /** LOCK */
  myJobs[dietReqID] = newJob;
  cout << "JobQueue: adding job " << dietReqID << " in status ESTIMATED" << endl;
  this->myLock.unlock();      /** UNLOCK */
  return true;
}

bool
JobQueue::setJobWaiting(int dietReqID) {
  myJobs[dietReqID].status = DIET_JOB_WAITING;
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
  return true;
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
      cout << "[Warning] JobQueue::deleteJob: job deleted still active ("
          << dietReqID << ")" << endl;
    }
    if (job.estVector != NULL)
      delete job.estVector;
    else
      cout << "[Warning] JobQueue::deleteJob: null estimation vector for job "
          << dietReqID << endl;
    myJobs.erase(p);
    cout << "job " << dietReqID << " deleted / new map size=" << myJobs.size() << endl;
    for (map<int, diet_job_t>::iterator q = myJobs.begin(); q != myJobs.end(); q++) {
      cout << " Queue contains job " << q->first << " in status " << (q->second).status << endl;
    }
    return true;
  }
  else {
    cout << "[Warning] JobQueue::deleteJob: could not find job "<< dietReqID << endl;
    return false;
  }
}

int
JobQueue::getActiveJobTable(jobVector_t& jobVector) {
  this->myLock.lock();        /** LOCK */
  jobVector = (diet_job_t *) malloc(sizeof(diet_job_t) * this->nbActiveJobs);
  map<int, diet_job_t>::iterator p = myJobs.begin();
  int nbJobs=0;
  while (nbJobs < this->nbActiveJobs && p != myJobs.end()) {
    diet_job_t job = p->second;
    // add job in the output vector only if active (ie waiting or running)
    if (job.status == DIET_JOB_WAITING || job.status == DIET_JOB_RUNNING) {
      jobVector[nbJobs++] = p->second;
    }
    p++;
  }
  this->myLock.unlock();      /** UNLOCK */
  if (nbJobs < this->nbActiveJobs) {
    cout << "getActiveJobTable [WARNING]: mismatch btw counter and map"
        << "nbActiveJobs=" << this->nbActiveJobs << " / nbJobs="
        << nbJobs << endl;
  }
  return nbJobs;
}

