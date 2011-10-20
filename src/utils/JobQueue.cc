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
 * Revision 1.9  2010/03/08 13:18:36  bisnard
 * corrected bug in EFT calculation when task duration exceeds the estimation
 *
 * Revision 1.8  2009/08/17 15:24:31  bdepardo
 * JobQueue wasn't thread safe. Added most of the code into critical sections.
 *
 * Revision 1.7  2009/06/23 09:29:35  bisnard
 * implemented scheduling simulation method to estimate SeD's EFT
 *
 * Revision 1.6  2008/11/08 19:12:38  bdepardo
 * A few warnings removal
 *
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
#include <map>

JobQueue::JobQueue(int maxConcurrentJobsNb)
  : nbActiveJobs(0) , nbProc(maxConcurrentJobsNb) {
}

JobQueue::~JobQueue() { }

void
JobQueue::addJobWaiting(int dietReqID, double jobEstCompTime,
                        corba_estimation_t& ev) {
  // make a copy of the estimation vector
  // (the estimation vector is stored only to be able to provide it when
  // getActiveJobTable is called; data inside ev is not used)
  estVector_t estVect = new corba_estimation_t(ev);
  diet_job_t newJob = { estVect, DIET_JOB_WAITING, jobEstCompTime, -1 };
  this->myLock.lock();        /** LOCK */
  myJobs[dietReqID] = newJob;
  myWaitingQueue.push_back(dietReqID);
  this->nbActiveJobs++;
  this->myLock.unlock();      /** UNLOCK */
  TRACE_TEXT(TRACE_ALL_STEPS, "JobQueue: adding job "
             << dietReqID << " in status WAITING"
             << " (duration est.=" << jobEstCompTime << " ms)\n");
}

bool
JobQueue::setJobStarted(int dietReqID) {
  // set start time
  struct timeval current_time;
  gettimeofday(&current_time, NULL);
  double currTime = (double)(current_time.tv_sec*1000 +
                             current_time.tv_usec/1000);
  // myJobs[dietReqID].startTime = current_time.tv_sec;
  // time is store in ms
  this->myLock.lock();        /** LOCK */
  myJobs[dietReqID].status = DIET_JOB_RUNNING;
  myJobs[dietReqID].startTime = currTime;
  // remove from waiting queue
  myWaitingQueue.remove(dietReqID);
  this->myLock.unlock();      /** UNLOCK */
  return true;
}

bool
JobQueue::setJobFinished(int dietReqID) {
  this->myLock.lock();  /** LOCK */
  myJobs[dietReqID].status = DIET_JOB_FINISHED;
  this->nbActiveJobs--;
  this->myLock.unlock();  /** UNLOCK */
  return true;
}

bool
JobQueue::deleteJob(int dietReqID) {
  this->myLock.lock();  /** LOCK */
  std::map<int, diet_job_t>::iterator p = myJobs.find(dietReqID);
  if (p != myJobs.end()) {
    diet_job_t job = p->second;
    if (job.status != DIET_JOB_FINISHED) {
      this->nbActiveJobs--;
      WARNING("JobQueue::deleteJob: job deleted still active ("
              << dietReqID << ")\n");
    }
    if (job.estVector != NULL) {
      delete job.estVector;
    } else {
      WARNING("JobQueue::deleteJob: null estimation vector for job "
              << dietReqID << "\n");
    }
    myJobs.erase(p);

    TRACE_TEXT(TRACE_ALL_STEPS, "job " << dietReqID
               << " deleted / new map size=" << myJobs.size() << "\n");

    std::map<int, diet_job_t>::iterator q = myJobs.begin();
    for (; q != myJobs.end(); ++ q) {
      TRACE_TEXT(TRACE_ALL_STEPS, " Queue contains job " << q->first
                 << " in status " << (q->second).status << "\n");
    }
    this->myLock.unlock();      /** UNLOCK */
    return true;
  } else {
    this->myLock.unlock();      /** UNLOCK */
    WARNING(" JobQueue::deleteJob: could not find job "<< dietReqID << "\n");
    return false;
  }
}

int
JobQueue::getActiveJobTable(jobVector_t& jobVector) {
  this->myLock.lock();        /** LOCK */
  jobVector = (diet_job_t *) malloc(sizeof(diet_job_t) * this->nbActiveJobs);
  std::map<int, diet_job_t>::iterator p = myJobs.begin();
  int nbJobs = 0;
  while (nbJobs < this->nbActiveJobs && p != myJobs.end()) {
    diet_job_t job = p->second;
    // add job in the output vector only if active (ie waiting or running)
    if (job.status == DIET_JOB_WAITING || job.status == DIET_JOB_RUNNING) {
      jobVector[nbJobs++] = p->second;
    }
    ++ p;
  }
  if (nbJobs < this->nbActiveJobs) {
    WARNING("getActiveJobTable [WARNING]: mismatch btw counter and map"
            << "nbActiveJobs=" << this->nbActiveJobs << " / nbJobs="
            << nbJobs << "\n");
  }
  this->myLock.unlock();      /** UNLOCK */
  return nbJobs;
}

double
JobQueue::estimateEFTwithFIFOSched() {

  // initialize EFT=>processor map
  std::multimap<double, int> procMap;
  for (int i = 0; i < nbProc; ++i) {
    procMap.insert(std::make_pair(0, 0));
  }

  // initialize current time
  struct timeval currentTime;
  gettimeofday(&currentTime, NULL);
  double currTime = (double)(currentTime.tv_sec*1000 +
                             currentTime.tv_usec/1000);

  // process RUNNING jobs
  int nbJobsRunning = 0;  // used only for trace
  this->myLock.lock();        /** LOCK */
  std::map<int, diet_job_t>::iterator jobsIter = myJobs.begin();
  for (; jobsIter != myJobs.end(); ++jobsIter) {
    diet_job_t job = jobsIter->second;
    if (job.status == DIET_JOB_RUNNING) {
      nbJobsRunning++;
      double remainCompTime =  job.startTime + job.estCompTime - currTime;
      double newEFT = procMap.begin()->first;
      if (remainCompTime > 0) {
        // if elapsed time < estimated time, estimated remaining time is known
        newEFT += remainCompTime;
      } else {
        // if elapsed time > estimated time, remaining time is unknown
        // we use an estimation of 10% of the computation time
        newEFT += job.estCompTime / 10;
      }
      // add this job to the processor with lowest EFT
      procMap.erase(procMap.begin());
      procMap.insert(std::make_pair(newEFT, 0));
    }
  }

  // process WAITING jobs
  std::list<int>::iterator waitJobIter = myWaitingQueue.begin();
  for (; waitJobIter != myWaitingQueue.end(); ++waitJobIter) {
    int jobReqID = *waitJobIter;

    std::map<int, diet_job_t>::iterator jobsIter = myJobs.find(jobReqID);
    if (jobsIter == myJobs.end()) {
      continue;
    }

    diet_job_t job = jobsIter->second;

    // add this job to the proc with lowest EFT
    double newEFT = procMap.begin()->first + job.estCompTime;
    procMap.erase(procMap.begin());
    procMap.insert(std::make_pair(newEFT, 0));
  }
  this->myLock.unlock();  /** UNLOCK */

  TRACE_TEXT(TRACE_ALL_STEPS, "Computing EFT: " << nbJobsRunning
             << " jobs running / " << myWaitingQueue.size()
             << " jobs waiting\n");
  // Take the lowest EFT value
  return procMap.begin()->first;
}

