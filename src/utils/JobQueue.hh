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

#ifndef _JOBQUEUE_HH_
#define _JOBQUEUE_HH_

using namespace std;
#include "DIET_data_internal.hh"
#include "debug.hh"
#include <omnithread.h>
#include <map>
#include <time.h>

class JobQueue {

  public:

    JobQueue();
    ~JobQueue();

    bool
        addJob(diet_profile_t& profile, diet_job_status_t status = DIET_JOB_WAITING);

    bool
        setJobStarted(int dietReqId);

    bool
        setJobFinished(int dietReqId);

    bool
        deleteJob(int dietReqId);

    int
        getActiveJobTable(jobVector_t& jobVector);

  private:

    /// Container for the list of jobs
    map<int, diet_job_t>  myJobs;

    /// Lock for thread-safe access to variables
    omni_mutex myLock;

    /// Counter of active jobs (ie not finished)
    int nbActiveJobs;

}; // class JobQueue

#endif // _JOBQUEUE_HH_
