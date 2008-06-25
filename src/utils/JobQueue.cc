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
 * Revision 1.5  2008/06/25 09:52:47  bisnard
 * - Estimation vector sent with solve request to avoid storing it
 * for each submit request as it depends on the parameters value. The
 * estimation vector is used by SeD to updates internal Gantt chart and
 * provide earliest finish time to submitted requests.
 * ==> added parameter to diet_call_common & diet_call_async_common
 *
 * Revision 1.4  2008/06/01 14:06:55  rbolze
 * replace most ot the cout by adapted function from debug.cc
 * there are some left ...
 *
 * Revision 1.3  2008/05/28 12:30:36  rbolze
 * change unit of the startTime value store in the diet_job_t structure.
 * it was in second, now it is in millisecond
 *
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
JobQueue::addJobWaiting(int dietReqID, corba_estimation_t& ev) {
  // make a copy of the estimation vector
  estVector_t estVect = new corba_estimation_t(ev);
  diet_job_t newJob = { estVect, DIET_JOB_WAITING, -1 };
  this->myLock.lock();        /** LOCK */
  myJobs[dietReqID] = newJob;
  this->nbActiveJobs++;
  TRACE_TEXT (TRACE_ALL_STEPS,"JobQueue: adding job " << dietReqID << " in status WAITING" << endl);
  this->myLock.unlock();      /** UNLOCK */
  return true;
}

bool
JobQueue::setJobStarted(int dietReqID) {
  myJobs[dietReqID].status = DIET_JOB_RUNNING;
  // set start time
  struct timeval current_time;
  gettimeofday(&current_time, NULL);
  // myJobs[dietReqID].startTime = current_time.tv_sec;
  // time is store in ms
  myJobs[dietReqID].startTime = (double)(current_time.tv_sec*1000 + current_time.tv_usec/1000);
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
      WARNING("JobQueue::deleteJob: job deleted still active ("
          << dietReqID << ")" << endl);
    }
    if (job.estVector != NULL)
      delete job.estVector;
    else
      WARNING("JobQueue::deleteJob: null estimation vector for job "
          << dietReqID << endl);
    myJobs.erase(p);
    TRACE_TEXT (TRACE_ALL_STEPS,"job " << dietReqID << " deleted / new map size=" << myJobs.size() << endl);
    for (map<int, diet_job_t>::iterator q = myJobs.begin(); q != myJobs.end(); q++) {
	    TRACE_TEXT (TRACE_ALL_STEPS," Queue contains job " <<
			  q->first << " in status " <<
			  (q->second).status << endl);
    }
    return true;
  }
  else {
    WARNING(" JobQueue::deleteJob: could not find job "<< dietReqID << endl);
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
    WARNING("getActiveJobTable [WARNING]: mismatch btw counter and map"
        << "nbActiveJobs=" << this->nbActiveJobs << " / nbJobs="
        << nbJobs << endl);
  }
  return nbJobs;
}

