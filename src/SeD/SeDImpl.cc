/****************************************************************************/
/* DIET SeD implementation source code                                      */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Philippe COMBES (Philippe.Combes@ens-lyon.fr)                       */
/*    - Frederic LOMBARD (Frederic.Lombard@lifc.univ-fcomte.fr)             */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.133  2011/05/09 13:10:11  bdepardo
 * Added method diet_get_SeD_services to retreive the services of a SeD given
 * its name
 *
 * Revision 1.132  2011/04/21 16:00:46  bdepardo
 * Add log infos in async calls
 *
 * Revision 1.131  2011/04/05 14:01:07  bdepardo
 * IOR is printed only when the tracelevel is at least TRACE_MAIN_STEPS
 *
 * Revision 1.130  2011/03/01 13:37:51  bdepardo
 * SIGTERM can now also be used to properly terminate DIET
 *
 * Revision 1.129  2011/02/24 16:57:01  bdepardo
 * Use new parser
 *
 * Revision 1.128  2011/02/15 16:19:37  bdepardo
 * More robust disconnection from parent: catch exceptions
 *
 * Revision 1.127  2011/02/08 16:53:52  bdepardo
 * Fixed dynamics. They didn't work anymore
 *
 * Revision 1.126  2011/01/23 19:19:58  bdepardo
 * Fixed memory and resources leaks, variables scopes, unread variables
 *
 * Revision 1.125  2010/12/17 09:47:59  kcoulomb
 * * Set diet to use the new log with forwarders
 * * Fix a CoRI problem
 * * Add library version remove DTM flag from ccmake because deprecated
 *
 * Revision 1.124  2010/11/24 12:30:17  bdepardo
 * Added getName() method.
 * Moved name and hostname initialization from run() to initialize() method,
 * this allows the logComponent to retrieve the name of the SeD.
 *
 * Revision 1.123  2010/11/09 02:23:34  bdepardo
 * Changed SeD name generation: now uses the PID of the process.
 *
 * Revision 1.122  2010/07/27 12:43:06  glemahec
 * Bugs corrections
 *
 * Revision 1.121  2010/07/27 10:24:33  glemahec
 * Improve robustness & general performance
 *
 * Revision 1.120  2010/07/14 23:45:47  bdepardo
 * Warning correction
 *
 * Revision 1.119  2010/07/12 16:14:10  glemahec
 * DIET 2.5 beta 1 - Use the new ORB manager and allow the use of SSH-forwarders for all DIET CORBA objects
 *
 * Revision 1.118  2010/03/31 21:15:39  bdepardo
 * Changed C headers into C++ headers
 *
 * Revision 1.117  2010/03/31 19:37:54  bdepardo
 * Changed "\n" into std::endl
 *
 * Revision 1.116  2010/03/08 13:33:09  bisnard
 * new method to retrieve DAGDA agent ID (CORBA)
 *
 * Revision 1.115  2009/11/30 17:58:08  bdepardo
 * New methods to remove the SeD in a cleaner way.
 *
 * Revision 1.114  2009/10/26 09:14:23  bdepardo
 * Added methods for dynamic hierarchy modifications:
 * - bindParent(const char * parentName)
 * - disconnect()
 * - removeElement()
 *
 * Revision 1.113  2009/06/23 09:28:27  bisnard
 * new API method for EFT estimation
 *
 * Revision 1.112  2008/12/08 15:31:42  bdepardo
 * Added the possibility to remove a service given its profile description.
 * So now, one is able to remove a service given either the real profile,
 * or the profile description.
 *
 * Revision 1.111  2008/11/18 10:13:57  bdepardo
 * - Added the possibility to dynamically create and destroy a service
 *   (even if the SeD is already started). An example is available.
 *   This feature only works with DAGDA.
 * - Added policy commands for CMake 2.6
 * - Added the name of the service in the profile. It was only present in
 *   the profile description, but not in the profile. Currently, the name is
 *   copied in each solve function, but this should certainly be moved
 *   somewhere else.
 *
 * Revision 1.110  2008/06/25 09:53:39  bisnard
 * - Estimation vector sent with solve request to avoid storing it
 * for each submit request as it depends on the parameters value. The
 * estimation vector is used by SeD to updates internal Gantt chart and
 * provide earliest finish time to submitted requests.
 *
 * Revision 1.109  2008/06/01 14:06:57  rbolze
 * replace most ot the cout by adapted function from debug.cc
 * there are some left ...
 *
 * Revision 1.108  2008/05/19 14:45:07  bisnard
 * jobs added to the queue during submit instead of solve
 *
 * Revision 1.107  2008/05/16 12:25:55  bisnard
 * API give status of all jobs running or waiting on the SeD
 * (used to compute earliest finish time)
 *
 * Revision 1.106  2008/05/11 16:19:48  ycaniou
 * Check that pathToTmp and pathToNFS exist
 * Check and eventually correct if pathToTmp or pathToNFS finish or not by '/'
 * Rewrite of the propagation of the request concerning job parallel_flag
 * Implementation of Cori_batch system
 * Numerous information can be dynamically retrieved through batch systems
 *
 * Revision 1.105  2008/04/21 13:18:34  glemahec
 * Memory leak and volatile data management corrections.
 *
 * Revision 1.104  2008/04/19 09:16:45  ycaniou
 * Check that pathToTmp and pathToNFS exist
 * Check and eventually correct if pathToTmp or pathToNFS finish or not by '/'
 * Rewrite of the propagation of the request concerning job parallel_flag
 * Rewrite (and addition) of the propagation of the response concerning:
 *   job parallel_flag and serverType (batch or serial for the moment)
 * Complete debug info with batch stuff
 *
 * Revision 1.103  2008/04/18 13:47:23  glemahec
 * Everything about DAGDA is now in utils/DAGDA directory.
 *
 * Revision 1.102  2008/04/07 15:33:42  ycaniou
 * This should remove all HAVE_BATCH occurences (still appears in the doc, which
 *   must be updated.. soon :)
 * Add the definition of DIET_BATCH_JOBID wariable in batch scripts
 *
 * Revision 1.101  2008/04/06 15:53:10  glemahec
 * DIET_PERSISTENT_RETURN & DIET_STICKY_RETURN modes are now working.
 * Warning: The clients have to take into account that an out data declared as
 * DIET_PERSISTENT or DIET_STICKY is  only stored on the SeDs and not returned
 * to  the  client. DTM doesn't manage the  DIET_*_RETURN types it and  always
 * returns the out data to the client: A client which uses this bug should not
 * work when activating DAGDA.
 *
 * Revision 1.100  2008/04/03 21:18:44  glemahec
 * Source cleaning, bug correction and headers.
 *
 * Revision 1.99  2008/02/27 14:32:04  rbolze
 * the function ping() return getpid value instead of 0.
 * Add Trace information when calling the function ping
 *
 * Revision 1.98  2008/01/14 11:32:15  glemahec
 * SeDImpl, the SeD object implementation can now use Dagda as data manager.
 *
 * Revision 1.97  2008/01/01 19:40:35  ycaniou
 * Modifications for batch management
 *
 * Revision 1.96  2007/12/18 13:04:28  glemahec
 * This commit adds the "diet_estimate_waiting_jobs" function to obtain the
 * number of jobs waiting in the FIFO queue when using the max concurrent
 * jobs limit. This function has to be used in the SeD plugin schedulers.
 *
 * Revision 1.95  2007/07/13 10:00:26  ecaron
 * Remove deprecated code (ALTPREDICT part)
 *
 * Revision 1.94  2007/07/04 15:11:52  ycaniou
 * Correction of bug: when HAVE_BATCH, batchQueue was lost. strdup() and ok.
 *
 * Revision 1.93  2007/06/28 20:11:08  ycaniou
 * Changed the call to logBeginSolve() in accordance to the remove of
 * the dietReqID paramater which is now included in the profile as the
 * prototype defines it
 *
 * Revision 1.92  2007/06/28 18:23:19  rbolze
 * add dietReqID in the profile.
 * and propagate this change to all functions that  have both reqID and profile parameters.
 * TODO : look at the asynchronous mechanism (client->SED) to propage this change.
 *
 * Revision 1.91  2007/06/07 14:20:02  ycaniou
 * Ajout d'un 'defined HAVE_BATCH' manquant pour parallel_AsyncSolve()
 * -> corrige un warning/error sur cette fonction undefined pour HAVE_BATCH
 *
 * Utilisation correct de Macro ERROR_EXIT
 *
 * Revision 1.90  2007/05/16 08:39:32  mjan
 * Quelques ajustements avec JuxMem
 *
 * Revision 1.89  2007/04/16 22:43:43  ycaniou
 * Make all necessary changes to have the new option HAVE_ALT_BATCH operational.
 * This is indented to replace HAVE_BATCH.
 *
 * First draw to manage batch systems with a new Cori plug-in.
 *
 * Revision 1.88  2007/03/01 15:55:07  ycaniou
 * Added the updateTimeSinceLastSolve() feature
 *
 * Revision 1.87  2007/01/24 20:33:47  ycaniou
 * Indentation
 *
 * Revision 1.86  2006/11/16 09:55:52  eboix
 *   DIET_config.h is no longer used. --- Injay2461
 *
 * Revision 1.85  2006/10/19 21:26:36  mjan
 * JuxMem support in async mode. Reorganized data management (DTM and JuxMem) into functions in the spirit of last modifs by Yves.
 *
 * Revision 1.84  2006/09/18 19:46:07  ycaniou
 * Corrected a bug in file_transfer:server.c
 * Corrected memory leaks due to incorrect free of char *
 * ServiceTable prints if service is sequential or parallel
 * Fully complete examples, whith a batch, a parallel and a sequential server and
 *  a unique client
 *
 * Revision 1.83  2006/08/27 18:40:10  ycaniou
 * Modified parallel submission API
 * - client: diet_call_batch() -> diet_parallel_call()
 * - SeD: diet_profile_desc_set_batch() -> [...]_parallel()
 * - from now, internal fields are related to parallel not batch
 * and corrected a bug:
 * - 3 types of submission: request among only seq, only parallel, or all
 *   available services (second wasn't implemented, third bug)
 *
 * Revision 1.82  2006/08/21 07:24:34  bdepardo
 * Corrected bug: in async. mode, when HAVE_BATCH was enabled, if pb wasn't
 *  batch, the solve was still not made (missed the
 *  (*(SrvT->getSolver(ref)))(&profile) call).
 *
 * Revision 1.81  2006/08/18 13:46:22  ycaniou
 * Corrected bug: in async. mode, when HAVE_BATCH was enabled, if pb wasn't
 *   batch, the solve wasn't made. Tkx Benjamin.
 *
 * Revision 1.80  2006/08/17 11:04:18  ycaniou
 * Replaced (int) to (long) in order to compile well under 64 bits architectures
 *   (the last. Tkx Benjamin)
 *
 * Revision 1.79  2006/08/09 09:00:00  ycaniou
 * Conversion of a int to long in a cast of a pointer in order to compile well
 *   on 64 bits architectures
 *
 * Revision 1.78  2006/08/03 11:36:00  ycaniou
 * Removed a DEBUG_YC forgotten
 * Placed batch job management between mutexes
 * First look at async. call
 *
 * Revision 1.77  2006/07/25 14:34:39  ycaniou
 * Use TRACE_TIME to precise time of downloading, submitting and uploading
 *   datas
 * Use a chained list (and not an array anymore) to manage the correspondance
 *   between DIET requests and batch jobs.
 * Changed the prototype of solve_batch: reqID is in the profile when batch mode
 *   is enabled.
 *
 * Batch management for sync. calls is now fully operationnal (at least for oar ;)
 *
 * Revision 1.76  2006/07/11 08:59:09  ycaniou
 * .Batch queue is now read in the serveur config file (only one queue
 * supported).
 * .Transfered perf evaluation in diet server (still dummy function)
 *
 * Revision 1.75  2006/07/07 09:40:14  aamar
 * Change to callback invocation to respect the new interface (return the
 * execution status).
 *
 * Revision 1.74  2006/06/30 15:41:47  ycaniou
 * DIET is now capable to submit batch Jobs in synchronous mode. Still some
 *   tuning to do (hard coded NFS path for OAR, tests for synchro between
 *   SeD and the batch job in regard to delete files.., more examples).
 *
 * Put the Data transfer section (JuxMem and DTM) before and after the call to
 * the SeD solve, in inline functions
 *   - downloadSyncSeDData()
 *   - uploadSyncSeDData()
 *
 * Revision 1.73  2006/06/08 16:46:08  pkchouha
 * Went again from release 1.71 and
 * added lines to obtain getrequest time for Seds
 *
 * Revision 1.71  2006/05/22 20:00:52  hdail
 * - Introduced uniform output format for SeD configuration option output at launch
 *   time.
 * - Removed HAVE_QUEUES protections at SeD level as all code is protected by
 *   member variable useConcJobLimit, code has been well-tested, and HAVE_QUEUES
 *   was hardcoded to 1 anyway.
 * - Centralized all shared initialization work of the constructors to a private
 *   method to avoid code duplication and resulting errors.
 * - Fixed bug reported by Gael where SeD queues didn't work for
 *   asynchronous calls.
 *
 * Revision 1.70  2006/02/24 01:57:16  hdail
 * Change setting for time of last solve when using SeD queues -- now round robin
 * will be based on time last job was enqueued.
 *
 * Revision 1.69  2006/02/17 10:03:06  ecaron
 * warning fix: unitialized variables
 *
 * Revision 1.68  2006/01/31 10:07:40  mjan
 * Update on the API of JuxMem
 *
 * Revision 1.67  2006/01/25 21:07:59  pfrauenk
 * CoRI - plugin scheduler: the type diet_est_tag_t est replace by int
 *        some new fonctions in DIET_server.h to manage the estVector
 *
 * Revision 1.66  2006/01/19 21:35:42  pfrauenk
 * CoRI : when --enable-cori - round-robin is the default scheduler -
 *        CoRI is not called (any more) for collecting information
 *        (so no FAST possible any more)
 *
 * Revision 1.65  2006/01/15 13:50:11  pfrauenk
 * CoRI: unused function removed
 *
 * Revision 1.64  2006/01/13 10:40:39  mjan
 * Updating DIET for next JuxMem (0.2)
 *
 * Revision 1.63  2005/12/20 14:26:55  eboix
 *     Call for Martin Quinson's ruling !
 *     slimfast_api.h not included anymore in this file   --- Injay2461
 *
 * Revision 1.62  2005/12/20 09:02:17  pfrauenk
 * CoRI bug fixes thx to Raphaël Bolze
 *
 * Revision 1.61  2005/12/20 07:52:44  pfrauenk
 * CoRI functionality added: FAST is hided, information about number of processors,
 * amount of available memory and of free cpu are provided
 *
 * Revision 1.60  2005/11/10 14:37:51  eboix
 *     Clean-up of Cmake/DIET_config.h.in and related changes. --- Injay2461
 *
 * Revision 1.57  2005/08/31 14:39:57  alsu
 * New plugin scheduling interface: adapting estimation vector
 * manipulation calls to the new interface
 ****************************************************************************/

#include <iostream>
using namespace std;
#include <cmath>
#include <cstring>
#include <cstdlib>
#include <unistd.h>   // For gethostname()
#include <ctime>
#include <sys/types.h>


#include <sstream>


#ifdef HAVE_DYNAMICS
#include <csignal>
#endif // HAVE_DYNAMICS

/* CLEAN ME: this was a hilarious pun except that slimfast_api.h is nowhere
   to be found. The Changelog file of package fast-0.8.7 simply mentions:
   - For the LONG CHANGELOG entry of version 0.8.0:
   - SLiM is dead
   - slimfast_api.h renamed to fast.h
   - For entry 0.2.13:
   - slimfast_api.h used to be generated from the concatenation of
   several atomic files.h. [...]
   Also refer to src/utils/FASTMgr.cc, where the inclusion of slimfast_api.h
   is bound to the definition of __FAST_0_4__ preprocessing symbol...
   Hence it really looks like (to me at least) the following include should
   be simply removed ? Anyone knows better ?   --- Injay2461
   #if HAVE_FAST
   #include "slimfast_api.h"
   #endif // HAVE_FAST
*/

#include "SeDImpl.hh"
#include "Callback.hh"
#include "common_types.hh"
#include "debug.hh"
#include "est_internal.hh"

#if HAVE_CORI
#include "CORIMgr.hh"
#endif //HAVE_CORI

#include "marshalling.hh"
#include "ORBMgr.hh"
#include "statistics.hh"
#include "configuration.hh"

#if defined HAVE_ALT_BATCH
#include "BatchCreator.hh"
#endif

#include "DagdaFactory.hh"
#include "DIET_Dagda.hh"

/** The trace level. */
extern unsigned int TRACE_LEVEL;


SeDImpl::SeDImpl() {
  this->initialize();
}

/** Private method to centralize all shared variable initializations from
 * different constructors.  Call this only from a constructor. */
void
SeDImpl::initialize() {
  this->SrvT    = NULL;
  this->childID = -1;
  this->parent  = Agent::_nil();
  this->localHostName[0] = '\0';
  (this->lastSolveStart).tv_sec = -1;
  (this->lastSolveStart).tv_usec = -1;
#ifdef USE_LOG_SERVICE
  this->dietLogComponent = NULL;
#endif

#if defined HAVE_ALT_BATCH
  this->server_status = SERIAL ;
  this->batch = NULL ; /* This has to be removed when all SeD will instanciate
                          a class corresponding to what it is read in the .cfg
                          file: FIFO, batch, etc
                          For the moment, NULL is like FIFO
                       */
#endif

  if (gethostname(localHostName, 256)) {
    WARNING("could not get hostname");
  }
  localHostName[255] = '\0'; // If truncated, ensure null termination

  /* Bind this SeD to its name in the CORBA Naming Service */
  std::string name;
  if (!CONFIG_STRING(diet::NAME, name)) {
    /* Generate a name for this SeD and print it */
    std::stringstream oss;
    pid_t pid = getpid();
    oss << localHostName << "_" << pid << "_" << rand() % 10000;
    name = oss.str();
  }
  this->myName = new char[name.size()+1];
  strcpy(this->myName, name.c_str());
}

SeDImpl::~SeDImpl() {
  /* FIXME: Tables should be destroyed. */
  ORBMgr::getMgr()->unbind(SEDCTXT, myName);
  ORBMgr::getMgr()->fwdsUnbind(SEDCTXT, myName);
  stat_finalize();
}


#ifdef HAVE_DYNAMICS
/* Method to disconnect from the parent */
CORBA::Long
SeDImpl::disconnect() {
  long rv = 0;
  SeqCorbaProfileDesc_t* profiles(NULL);
  profiles = SrvT->getProfiles();

  /* Do we already have a parent?
   * If yes, we need to unsubscribe.
   */
  if (! CORBA::is_nil(this->parent)) {
    try {
      /* Unsubscribe from parent */
      this->parent->childUnsubscribe(childID, *profiles);
      this->parent = Agent::_nil();

      /* Unsubscribe data manager */
      this->dataManager->unsubscribeParent();

#ifdef USE_LOG_SERVICE
      /* Log */
      if (dietLogComponent != NULL)
        dietLogComponent->logDisconnect();
#endif
    } catch (CORBA::Exception& e) {
      CORBA::Any tmp;
      tmp <<= e;
      CORBA::TypeCode_var tc = tmp.type();
      WARNING("exception caught 11(" << tc->name() << ") while unsubscribing to "
              << "parent: either the latter is down, "
              << "or there is a problem with the CORBA name server");
      rv = 1;
    }
  }

  delete profiles;
  return rv;
}

/* Method to dynamically change the parent of the SeD */
CORBA::Long
SeDImpl::bindParent(const char * parentName) {
  long rv = 0;
  SeqCorbaProfileDesc_t* profiles(NULL);
  profiles = SrvT->getProfiles();

  /* Does the new parent exists? */
  Agent_var parentTmp;
  try {
    parentTmp =
      ORBMgr::getMgr()->resolve<Agent,Agent_var>(AGENTCTXT, parentName);
  } catch (...) {
    parentTmp = Agent::_nil();
  }

  if (CORBA::is_nil(parentTmp)) {
    if (CORBA::is_nil(this->parent)) {
      WARNING("cannot locate agent " << parentName << ", will now wait");
    } else {
      WARNING("cannot locate agent " << parentName << ", won't change current parent");
    }
    return 1;
  }

  /* Do we already have a parent?
   * If yes, we need to unsubscribe.
   */
  if (! CORBA::is_nil(this->parent)) {
    try {
      /* Unsubscribe from parent */
      if (childID != -1) {
        this->parent->childUnsubscribe(childID, *profiles);
      }
      this->parent = Agent::_nil();
      childID = -1;

      /* Unsubscribe data manager */
      this->dataManager->unsubscribeParent();
    } catch (CORBA::Exception& e) {
      CORBA::Any tmp;
      tmp <<= e;
      CORBA::TypeCode_var tc = tmp.type();
      WARNING("exception caught 22(" << tc->name() << ") while unsubscribing to "
              << "parent: either the latter is down, "
              << "or there is a problem with the CORBA name server");
    }
  }

  /* Now we try to subscribe to a new parent */
  this->parent = parentTmp;

  try {
    childID = this->parent->serverSubscribe(myName, localHostName,
                                            *profiles);

    TRACE_TEXT(TRACE_ALL_STEPS, "* Bound myself to parent: " << parentName << std::endl);

    /* Data manager also needs to connect to the new parent */
    this->dataManager->subscribeParent(parentName);
#ifdef USE_LOG_SERVICE
    /* Log */
    if (dietLogComponent != NULL)
      dietLogComponent->logNewParent("SeD", parentName);
#endif
  } catch (CORBA::Exception& e) {
    CORBA::Any tmp;
    tmp <<= e;
    CORBA::TypeCode_var tc = tmp.type();
    WARNING("exception caught 33(" << tc->name() << ") while subscribing to "
            << parentName << ": either the latter is down, "
            << "or there is a problem with the CORBA name server");
    rv = 1;
  }
  if (childID < 0) {
    WARNING(__FUNCTION__ << ": error subscribing server");
    this->parent = Agent::_nil();
    rv = 1;
  }

  delete profiles;
  return rv;
}


CORBA::Long
SeDImpl::removeElement() {
  /* Send signal to commit suicide */
  return raise(SIGINT); // Or SIGTERM
}

void
SeDImpl::removeElementClean() {
  SeqCorbaProfileDesc_t* profiles(NULL);
  profiles = SrvT->getProfiles();

  /* Do we already have a parent?
   * If yes, we need to unsubscribe.
   */
  if (! CORBA::is_nil(this->parent)) {
    /* Unsubscribe from parent */
    try {
      this->parent->childUnsubscribe(childID, *profiles);
    } catch (...) {
      // TODO
    }
    this->parent = Agent::_nil();

    /* Unsubscribe data manager */
    try {
      this->dataManager->unsubscribeParent();
    } catch (...) {
      // TODO
    }
  }

  delete profiles;

#ifdef USE_LOG_SERVICE
  /* Log */
  if (dietLogComponent != NULL)
    dietLogComponent->logRemoveElement();
#endif
}

#endif // HAVE_DYNAMICS


int
SeDImpl::run(ServiceTable* services) {
  /* initialize random seed: */
  srand(time(NULL));

  SeqCorbaProfileDesc_t* profiles(NULL);
  stat_init();

  TRACE_TEXT(TRACE_ALL_STEPS, "* Declared myself as: " << this->myName << std::endl);
  try {
    ORBMgr::getMgr()->bind(SEDCTXT, this->myName, this->_this(), true);
    ORBMgr::getMgr()->fwdsBind(SEDCTXT, this->myName,
                               ORBMgr::getMgr()->getIOR(_this()));
  } catch (...) {
    ERROR("could not declare myself as " << this->myName, 1);
  }

  this->SrvT = services;

#if defined HAVE_ALT_BATCH
  if( this->server_status == BATCH ) {
    // Read "batchName" if parallel jobs are to be submitted
    std::string batchname;
    if (!CONFIG_STRING(diet::BATCHNAME, batchname)) {
      ERROR("SeD can not launch parallel/batch jobs, no parallel/batch"
            " scheduler specified in the config file", 1) ;
    }
    batch = BatchCreator::getBatchSystem(batchname.c_str()) ;
    if( batch == NULL ) {
      ERROR("Parallel/batch scheduler not recognized", 1) ;
    }
    TRACE_TEXT(TRACE_MAIN_STEPS,
               "Parallel/batch submission enabled with "
               << batch->getBatchName()) ;
    /* TODO: Queues should be provided in the SeD.cfg, or
       should be recognized automatically
       -> for the moment, only one queue specified in the file
    */
    if( batch->getBatchQueueName() != NULL )
      TRACE_TEXT(TRACE_MAIN_STEPS, " using queue "
                 << batch->getBatchQueueName() ) ;
    TRACE_TEXT(TRACE_MAIN_STEPS, endl ) ;
    TRACE_TEXT(TRACE_MAIN_STEPS,"pathToNFS: " << batch->getNFSPath() << endl) ;
    TRACE_TEXT(TRACE_MAIN_STEPS,"pathToTmp: " << batch->getTmpPath() << endl) ;
  }
#endif

  std::string parent_name;
  if (!CONFIG_STRING(diet::PARENTNAME, parent_name)) {
#ifndef HAVE_DYNAMICS
    return 1;
#else
    WARNING("no parent specified, will now wait");
#endif // HAVE_DYNAMICS
  }

  try {
    parent =
      ORBMgr::getMgr()->resolve<Agent, Agent_var>(AGENTCTXT, parent_name);
  } catch (...) {
    parent = Agent::_nil();
  }

  if (CORBA::is_nil(parent)) {
#ifndef HAVE_DYNAMICS
    ERROR("cannot locate agent " << parent_name, 1);
#else
    WARNING("cannot locate agent " << parent_name << ", will now wait");
#endif // HAVE_DYNAMICS
  }

  profiles = SrvT->getProfiles();

#ifdef USE_LOG_SERVICE
  if (dietLogComponent != NULL) {
    for (CORBA::ULong i=0; i<profiles->length(); i++) {
      dietLogComponent->logAddService(&((*profiles)[i]));
    }
  }
#endif // USE_LOG_SERVICE

#ifdef HAVE_DYNAMICS
  if (! CORBA::is_nil(parent)) {
#endif // HAVE_DYNAMICS
    try {
      TRACE_TEXT(TRACE_ALL_STEPS, "parent->serverSubscribe(" << this->myName
                 << ", " << localHostName << ", *profiles)" << endl);
      childID = parent->serverSubscribe(this->myName, localHostName,
                                        *profiles);
      TRACE_TEXT(TRACE_ALL_STEPS, "subscribe !" << endl);
    } catch (CORBA::Exception& e) {
      CORBA::Any tmp;
      tmp <<= e;
      CORBA::TypeCode_var tc = tmp.type();
      ERROR("exception caught (" << tc->name() << ") while subscribing to "
            << parent_name << ": either the latter is down, "
            << "or there is a problem with the CORBA name server", 1);
    }
    if (childID < 0) {
      ERROR(__FUNCTION__ << ": error subscribing server", 1);
    }
#ifdef HAVE_DYNAMICS
  } // end: if (! CORBA::is_nil(parent))
#endif // HAVE_DYNAMICS
  delete profiles;

  // FIXME: How can I get the port used by the ORB ? and is it useful ?
  unsigned long endPoint;
  if (!CONFIG_ULONG(diet::DIETPORT, endPoint)) {
    this->port = 0;
  } else {
    this->port = endPoint;
  }

  bool useConcJob = false;
  CONFIG_BOOL(diet::USECONCJOBLIMIT, useConcJob);
  this->useConcJobLimit = useConcJob;

  int concJobsLimit = 1;
  CONFIG_INT(diet::MAXCONCJOBS, concJobsLimit);
  this->maxConcJobs = concJobsLimit;

  if (this->useConcJobLimit){
    this->accessController = new AccessController(this->maxConcJobs);
    this->jobQueue = new JobQueue(this->maxConcJobs);
    TRACE_TEXT(TRACE_ALL_STEPS, "* SeD Queue: enabled "
               << "(maximum " << this->maxConcJobs << " concurrent solves)" << endl);
  } else {
    this->accessController = NULL;
    this->jobQueue = new JobQueue();
    TRACE_TEXT(TRACE_ALL_STEPS, "* SeD Queue: disabled (no restriction"
               << " on concurrent solves)" << endl);
  }



  /* Print out service table */
  if (TRACE_LEVEL >= TRACE_STRUCTURES) {
    SrvT->dump(stdout);
  }

#if HAVE_CORI
  return CORIMgr::startCollectors();
#endif //HAVE_CORI

} // end: run

void
SeDImpl::setDataManager(DagdaImpl* dataManager) {
  this->dataManager=dataManager;
}

#ifdef USE_LOG_SERVICE
void
SeDImpl::setDietLogComponent(DietLogComponent* dietLogComponent) {
  this->dietLogComponent = dietLogComponent;
}
#endif

/*
** The server receives a request by an agent
*/
void
SeDImpl::getRequest(const corba_request_t& creq)
{
  corba_response_t resp;
  char statMsg[128];
#ifdef HAVE_DYNAMICS
  Agent_var parentTmp = this->parent;
#endif // HAVE_DYNAMICS

#ifdef HAVE_ALT_BATCH
  const char * jobSpec ;

  if( creq.pb.parallel_flag == 1 )
    jobSpec = "sequential" ;
  else
    jobSpec = "parallel" ;

  sprintf(statMsg, "getRequest %ld (%s)", (unsigned long) creq.reqID,
          jobSpec);
#else
  sprintf(statMsg, "getRequest %ld", (unsigned long) creq.reqID);
#endif

#ifdef HAVE_ALT_BATCH
  stat_in("SeD",statMsg);
  TRACE_TEXT(TRACE_MAIN_STEPS,
             endl << "**************************************************" << endl
             << "Got " << jobSpec << " request " << creq.reqID << endl
             << endl);
#else
  stat_in("SeD",statMsg);
  TRACE_TEXT(TRACE_MAIN_STEPS,
             endl << "**************************************************" << endl
             << "Got request " << creq.reqID << endl
             << endl);
#endif
  resp.reqID = creq.reqID;
  resp.myID  = childID;

  /** Commented to cut overhead of un-needed log messages
      if (dietLogComponent != NULL) {
      dietLogComponent->logAskForSeD(&creq);
      }
  */

  ServiceTable::ServiceReference_t serviceRef;
  serviceRef = SrvT->lookupService(&(creq.pb));
  if (serviceRef == -1) {
    resp.servers.length(0);
    TRACE_TEXT(TRACE_MAIN_STEPS,
               "service not found ??????????????????????????????????????"
               << endl) ;
  } else {
    resp.servers.length(1);

    resp.servers[0].loc.SeDName  = CORBA::string_dup(myName);
    resp.servers[0].loc.hostName = CORBA::string_dup(this->localHostName);
    resp.servers[0].loc.port     = this->port;
    resp.servers[0].estim.estValues.length(0);

#ifdef HAVE_ALT_BATCH
    resp.servers[0].loc.serverType = server_status ;
    resp.servers[0].loc.parallel_flag = creq.pb.parallel_flag ;
#endif

    /* Initialize some values */
    estVector_t ev = &(resp.servers[0].estim);
    for (int ctIter = 0 ; ctIter < creq.pb.last_out ; ctIter++) {
      diet_est_set_internal(ev, EST_COMMTIME, 0.0);
    }
    /* Fill the metrics */
    this->estimate(resp.servers[0].estim, creq.pb, serviceRef);

  }

  if (TRACE_LEVEL >= TRACE_STRUCTURES) {
    displayResponse(stdout, &resp);
  }

  /** Commented to cut overhead of un-needed log messages
      if (dietLogComponent != NULL) {
      dietLogComponent->logSedChosen(&creq,&resp);
      }
  */
  stat_out("SeD",statMsg);

#ifndef HAVE_DYNAMICS
  parent->getResponse(resp);
#else
  parentTmp->getResponse(resp);
#endif
}

CORBA::Long
SeDImpl::checkContract(corba_estimation_t& estimation,
                       const corba_pb_desc_t& pb)
{
  ServiceTable::ServiceReference_t ref(-1);
  ref = SrvT->lookupService(&(pb));
  if (ref == -1) {
    return 1;
  } else {
    this->estimate(estimation, pb, ref);
  }
  return 0;
}

void persistent_data_release(corba_data_t* arg) {

  switch((diet_data_type_t)(arg->desc.specific._d())) {
  case DIET_VECTOR: {
    corba_vector_specific_t vect;

    vect.size = 0;
    arg->desc.specific.vect(vect);
    break;
  }
  case DIET_MATRIX: {
    corba_matrix_specific_t mat;

    arg->desc.specific.mat(mat);
    arg->desc.specific.mat().nb_r  = 0;
    arg->desc.specific.mat().nb_c  = 0;

    break;
  }
  case DIET_STRING: {
    corba_string_specific_t str;

    str.length=0;
    arg->desc.specific.str(str);
    arg->desc.specific.str().length = 0;
    break;
  }
  case DIET_FILE: {
    corba_file_specific_t file;

    arg->desc.specific.file(file);
    arg->desc.specific.file().path = CORBA::string_dup("");
    arg->desc.specific.file().size = 0;
    break;
  }
  default:
    break;
  }
}

/** Called from client immediatly after knowing which server is selected
 ** and will be called by the client, before data transfer.

 ** Should disappear when data management by Gael is fully tested, because
 ** by default, data will be managed inside the solve() function and not inside
 ** the call.
 **/
void
SeDImpl::updateTimeSinceLastSolve() {
  gettimeofday(&(this->lastSolveStart), NULL) ;
}

int
SeDImpl::getNumJobsWaiting() {
  if (accessController) {
    return accessController->getNumWaiting();
  }
  return 0;
}

int
SeDImpl::getActiveJobVector(jobVector_t& jv) {
  if (jobQueue) {
    return jobQueue->getActiveJobTable(jv);
  }
  return 0;
}

double
SeDImpl::getEFT() {
  if (this->useConcJobLimit && jobQueue) {
    return jobQueue->estimateEFTwithFIFOSched();
  }
  return 0;
}

char*
SeDImpl::getName() {
  return CORBA::string_dup(myName);
}


CORBA::Long
SeDImpl::solve(const char* path, corba_profile_t& pb) {
  ServiceTable::ServiceReference_t ref(-1);
  diet_profile_t profile;
  diet_convertor_t* cvt(NULL);
  int solve_res(0);
  char statMsg[128];

  /* Record the SedImpl address */
  profile.SeDPtr = (const void*) this ;

  ref = SrvT->lookupService(path, &pb);
  if (ref == -1) {
    ERROR("SeD::" << __FUNCTION__ << ": service not found", 1);
  }

  /* Record time at which solve started (when not using queues)
   * and time at which job was enqueued (when using queues). */
  gettimeofday(&(this->lastSolveStart), NULL);

  /* Add the job in the list of queued jobs with its estimation vector */
  double estCompTime = diet_est_get_system(&pb.estim, EST_TCOMP, 10000000);
  this->jobQueue->addJobWaiting(pb.dietReqID, estCompTime, pb.estim);

#if defined HAVE_ALT_BATCH
  if( server_status == BATCH ) {
    return this->parallel_solve(path, pb, ref, profile) ;
  }
#endif

  if (this->useConcJobLimit){
    this->accessController->waitForResource();
  }

  sprintf(statMsg, "solve %ld", (unsigned long) pb.dietReqID);
  stat_in("SeD",statMsg);

#ifdef USE_LOG_SERVICE
  if (dietLogComponent != NULL) {
    dietLogComponent->logBeginSolve(path, &pb);
  }
#endif

  this->jobQueue->setJobStarted(pb.dietReqID);

  TRACE_TEXT(TRACE_MAIN_STEPS, "SeD::solve invoked on pb: " << path << endl);

  cvt = SrvT->getConvertor(ref);

  /* Data transfer */
  downloadSyncSeDData(profile,pb,cvt) ;

#ifdef USE_LOG_SERVICE
  if (dietLogComponent != NULL) {
    dietLogComponent->logEndDownload(path, &pb);
  }
#endif

  /* Copying the name of the service in the profile...
   * Not sure this should be done here, but currently it isn't done
   * anywhere else...
   */
  profile.pb_name = strdup(path);

  TRACE_TEXT(TRACE_MAIN_STEPS, "Calling getSolver" << endl);
  solve_res = (*(SrvT->getSolver(ref)))(&profile);    // SOLVE

  /* Data transfer */
  uploadSyncSeDData(profile,pb,cvt) ;

  TRACE_TEXT(TRACE_MAIN_STEPS,"SeD::solve complete" << endl
             << "************************************************************"<< endl);
  delete [] profile.parameters; // allocated by unmrsh_in_args_to_profile

  this->jobQueue->setJobFinished(profile.dietReqID);
  this->jobQueue->deleteJob(profile.dietReqID);

  stat_out("SeD",statMsg);
  stat_flush();

#ifdef USE_LOG_SERVICE
  if (dietLogComponent != NULL) {
    dietLogComponent->logEndSolve(path, &pb);
  }
#endif

  if (this->useConcJobLimit){
    this->accessController->releaseResource();
  }

  return solve_res;
}

#if defined HAVE_ALT_BATCH
void
SeDImpl::setServerStatus( diet_server_status_t status ) {
  this->server_status = status ;
}

diet_server_status_t
SeDImpl::getServerStatus() {
  return server_status ;
}

CORBA::Long
SeDImpl::parallel_solve(const char* path, corba_profile_t& pb,
                        ServiceTable::ServiceReference_t& ref,
                        diet_profile_t& profile)
{
  /*************************************************************
   **                  submit a parallel job                  **
   **
   ** For the moment,
   ** datas are received before batch submission. Maybe this has
   ** to be done in a fork, during the wait in the batch queue
   ** if we want to be the most efficient, but needs file names
   ** and perf. pred. from DTM or JuxMem.

   ** TODO: If a data is not a file, convert it as a file.
   **  Must I (can I?) do it here or give the functions to let
   **  the Sed programmer do it in the profile?
   *************************************************************/

  diet_convertor_t* cvt(NULL);
  int solve_res(0);
  char statMsg[128];

  /* Copying the name of the service in the profile...
   * Not sure this should be done here, but currently it isn't done
   * anywhere else...
   */
  profile.pb_name = strdup(path);

  /* Is there a sens to use Queue with Batch? */
  if (this->useConcJobLimit){
    this->accessController->waitForResource();
  }

  sprintf(statMsg, "solve %ld", (unsigned long) pb.dietReqID);
  stat_in("SeD",statMsg);

  this->jobQueue->setJobStarted(pb.dietReqID);

#ifdef USE_LOG_SERVICE
  if (dietLogComponent != NULL) {
    dietLogComponent->logBeginSolve(path, &pb);
  }
#endif

  TRACE_TEXT(TRACE_MAIN_STEPS, "SeD::parallel_solve() invoked on pb: "
             << path << endl);

  cvt = SrvT->getConvertor(ref);

  /* Data transfer */
  downloadSyncSeDData(profile,pb,cvt) ;

  TRACE_TEXT(TRACE_MAIN_STEPS, "Calling getSolver" << endl);
  solve_res = (*(SrvT->getSolver(ref)))(&profile);    // SOLVE
  TRACE_TIME(TRACE_MAIN_STEPS, "Submitting DIET job of ID "
             << profile.dietReqID <<
             " on batch system with ID " <<
             batch->getBatchJobID(profile.dietReqID)
             << endl) ;
  if( batch->wait4BatchJobCompletion(batch->getBatchJobID(profile.dietReqID))
      < 0 ) {
    ERROR("An error occured during the execution of the parallel job", 21) ;
  }
  batch->removeBatchJobID(profile.dietReqID) ;

  /* Data transfer */
  uploadSyncSeDData(profile,pb,cvt) ;

  TRACE_TEXT(TRACE_MAIN_STEPS,"SeD::parallel_solve() completed" << endl
             << "************************************************************" << endl);
  delete [] profile.parameters; // allocated by unmrsh_in_args_to_profile

  stat_out("SeD",statMsg);
  stat_flush();

#ifdef USE_LOG_SERVICE
  if (dietLogComponent != NULL) {
    dietLogComponent->logEndSolve(path, &pb);
  }
#endif

  this->jobQueue->setJobFinished(profile.dietReqID);
  this->jobQueue->deleteJob(profile.dietReqID);

  if (this->useConcJobLimit){
    this->accessController->releaseResource();
  }

  return solve_res;
}
#endif //HAVE_ALT_BATCH

void
SeDImpl::solveAsync(const char* path, const corba_profile_t& pb,
                    const char* volatileclientREF)
{

  // test validity of volatileclientREF
  // If nil, it is not necessary to solve ...
  try {
    //ServiceTable::ServiceReference_t ref(-1);
    CORBA::Object_var cb = ORBMgr::getMgr()->resolveObject(CLIENTCTXT, volatileclientREF);
    //ORBMgr::stringToObject(volatileclientREF);
    if (CORBA::is_nil(cb)) {
      ERROR("SeD::" << __FUNCTION__ << ": received a nil callback",);
    } else {
      ServiceTable::ServiceReference_t ref(-1);
      diet_profile_t profile;
      diet_convertor_t* cvt(NULL);
      char statMsg[128];

      /* Record the SedImpl address */
      profile.SeDPtr = (const void*) this ;

      ref = SrvT->lookupService(path, &pb);
      if (ref == -1) {
        ERROR("SeD::" << __FUNCTION__ << ": service not found",);
      }

      /* Record time at which solve started (when not using queues)
       * and time at which job was enqueued (when using queues). */
      gettimeofday(&(this->lastSolveStart), NULL);

      /* Add the job in the list of queued jobs with its estimation vector */
      double estCompTime = diet_est_get_system(&pb.estim, EST_TCOMP, 10000000);
      int reqID = pb.dietReqID;
      corba_estimation_t estim = pb.estim;
      this->jobQueue->addJobWaiting(reqID, estCompTime, estim);

#if defined HAVE_ALT_BATCH
      /* Use parallel_flag of the proposed SeD service to know what to do */
      const corba_profile_desc_t & sed_profile = SrvT->getProfile( ref ) ;
      if( sed_profile.parallel_flag == 2 )
        this->parallel_AsyncSolve(path, pb, ref,
                                  cb, profile) ;
      else {
#endif

        if (this->useConcJobLimit){
          this->accessController->waitForResource();
        }

        sprintf(statMsg, "solveAsync %ld", (unsigned long) pb.dietReqID);
        stat_in("SeD",statMsg);

#ifdef USE_LOG_SERVICE
        if (dietLogComponent != NULL) {
          dietLogComponent->logBeginSolve(path, &pb);
        }
#endif

        this->jobQueue->setJobStarted(pb.dietReqID);

        TRACE_TEXT(TRACE_MAIN_STEPS,
                   "SeD::solveAsync invoked on pb: " << path
                   << " (reqID " << profile.dietReqID << ")" << endl);


        cvt = SrvT->getConvertor(ref);

        downloadAsyncSeDData(profile, const_cast<corba_profile_t&>(pb), cvt);

#ifdef USE_LOG_SERVICE
        if (dietLogComponent != NULL) {
          dietLogComponent->logEndDownload(path, &pb);
        }
#endif

        /* Copying the name of the service in the profile...
         * Not sure this should be done here, but currently it isn't done
         * anywhere else...
         */
        profile.pb_name = strdup(path);

        TRACE_TEXT(TRACE_MAIN_STEPS, "Calling getSolver" << endl);
        int solve_res = (*(SrvT->getSolver(ref)))(&profile);    // SOLVE

        uploadAsyncSeDData(profile,  const_cast<corba_profile_t&>(pb), cvt);

        TRACE_TEXT(TRACE_MAIN_STEPS, "SeD::" << __FUNCTION__ << " complete" << endl
                   << "**************************************************" << endl);

        this->jobQueue->setJobFinished(profile.dietReqID);
        this->jobQueue->deleteJob(profile.dietReqID);

        stat_out("SeD",statMsg);
        stat_flush();

#ifdef USE_LOG_SERVICE
        if (dietLogComponent != NULL) {
          dietLogComponent->logEndSolve(path, &pb);
        }
#endif

        /* Release resource before returning the data.  Caution: this could be a
         * problem for applications with lots of data. */
        if (this->useConcJobLimit) {
          this->accessController->releaseResource();
        }

        // send result data to client.
        // TODO : change notifyResults and solveResults signature remove dietReqID
        TRACE_TEXT(TRACE_ALL_STEPS, "SeD::" << __FUNCTION__
                   << ": performing the call-back." << endl);
        Callback_var cb_var = Callback::_narrow(cb);

        cb_var->notifyResults(path, pb, pb.dietReqID);
        cb_var->solveResults(path, pb, pb.dietReqID, solve_res);

        /* FIXME: do we need to use diet_free_data on profile parameters as
         * we do in the solve(...) method? */
        delete [] profile.parameters; // allocated by unmrsh_in_args_to_profile
#if defined HAVE_ALT_BATCH
      }
#endif
    }
  } catch (const CORBA::Exception &e) {
    // Process any other User exceptions. Use the .id() method to
    // record or display useful information
    CORBA::Any tmp;
    tmp <<= e;
    CORBA::TypeCode_var tc = tmp.type();
    const char * p = tc->name();
    if (*p != '\0') {
      ERROR("exception caught 55in SeD::" << __FUNCTION__ << '(' << p << ')',);
    } else {
      ERROR("exception caught 66in SeD::" << __FUNCTION__
            << '(' << tc->id() << ')',);
    }
  } catch (...) {
    // Process any other exceptions. This would catch any other C++
    // exceptions and should probably never occur
    ERROR("unknown exception caught 77",);
  }
}

/* Note: ref is useful for convertors */
#if defined HAVE_ALT_BATCH
void
SeDImpl::parallel_AsyncSolve(const char * path, const corba_profile_t & pb,
                             ServiceTable::ServiceReference_t ref,
                             CORBA::Object_var & cb,
                             diet_profile_t & profile)
{
  /*************************************************************
   **                  submit a parallel job                  **
   **
   ** For the moment,
   ** datas are received before batch submission. Maybe this has
   ** to be done in a fork, during the wait in the batch queue
   ** if we want to be the most efficient, but needs file names
   ** and perf. pred. from DTM or JuxMem.

   ** TODO: If a data is not a file, convert it as a file.
   **  Must I (can I?) do it here or give the functions to let
   **  the Sed programmer do it in the profile?
   *************************************************************/

  try {
    if (CORBA::is_nil(cb)) {
      ERROR("SeD::" << __FUNCTION__ << ": received a nil callback",);
    } else {
      diet_convertor_t* cvt(NULL);
      int solve_res(0);
      char statMsg[128];

      /* This can be useful for parallel resolutions */
      if (this->useConcJobLimit){
        this->accessController->waitForResource();
      }

      sprintf(statMsg, "solve_AsyncParallel %ld", (unsigned long) pb.dietReqID);
      stat_in("SeD",statMsg);

#ifdef USE_LOG_SERVICE
      if (dietLogComponent != NULL) {
        dietLogComponent->logBeginSolve(path, &pb);
      }
#endif

      TRACE_TEXT(TRACE_MAIN_STEPS,
                 "SeD::solve_AsyncParallel invoked on pb: " << path
                 << " (reqID " << pb.dietReqID << ")" << endl);

      cvt = SrvT->getConvertor(ref);

      downloadAsyncSeDData(profile, const_cast<corba_profile_t&>(pb), cvt);

      /* Copying the name of the service in the profile...
       * Not sure this should be done here, but currently it isn't done
       * anywhere else...
       */
      profile.pb_name = strdup(path);


#if defined HAVE_ALT_BATCH
      solve_res = (*(SrvT->getSolver(ref)))(&profile);
      TRACE_TIME(TRACE_MAIN_STEPS, "Submitting DIET job of ID "
                 << profile.dietReqID <<
                 " on batch system with ID " <<
                 batch->getBatchJobID(profile.dietReqID)
                 << endl) ;
      if(
        batch->wait4BatchJobCompletion(batch->getBatchJobID(profile.dietReqID)
          )
        < 0 ) {
        ERROR_EXIT("An error occured during the execution of the parallel job") ;
      }
      batch->removeBatchJobID(profile.dietReqID) ;
#else
      int status ;
      TRACE_TIME(TRACE_MAIN_STEPS, "Submitting script for DIET job of ID "
                 << profile.dietReqID <<
                 " is of pid " <<
                 (long)((ProcessInfo)findBatchID(profile.dietReqID))->pid
                 << endl) ;
      /* This waits until the jobs ends
      ** and remove batchID/DIETreqID correspondance */
      if( (ELBASE_Poll(findBatchID(profile.dietReqID), 1, &status) == 0)
          && status == 0 ) {
        if( this->batchID == ELBASE_SHELLSCRIPT ) {
          ERROR_EXIT("An error occured during the execution of "
                     "the parallel job") ;
        } else {
          ERROR_EXIT("An error occured during the execution"
                     " of the batch job") ;
        }
      }
      removeBatchID(pb.dietReqID) ;
#endif // HAVE_ALT_BATCH

      uploadAsyncSeDData(profile,  const_cast<corba_profile_t&>(pb), cvt);

      TRACE_TEXT(TRACE_MAIN_STEPS, "SeD::" << __FUNCTION__ << " complete" << endl
                 << "**************************************************" << endl);

      stat_out("SeD",statMsg);
      stat_flush();

#ifdef USE_LOG_SERVICE
      if (dietLogComponent != NULL) {
        dietLogComponent->logEndSolve(path, &pb);
      }
#endif

      /* Release resource before returning the data.  Caution: this could be a
       * problem for applications with lots of data. */
      if (this->useConcJobLimit){
        this->accessController->releaseResource();
      }

      // send result data to client.
      TRACE_TEXT(TRACE_ALL_STEPS, "SeD::" << __FUNCTION__
                 << ": performing the call-back." << endl);
      Callback_var cb_var = Callback::_narrow(cb);
      cb_var->notifyResults(path, pb, pb.dietReqID);
      cb_var->solveResults(path, pb, pb.dietReqID, solve_res);
      /* FIXME: do we need to use diet_free_data on profile parameters as
       * we do in the solve(...) method? */
      delete [] profile.parameters; // allocated by unmrsh_in_args_to_profile
    }
  } catch (const CORBA::Exception &e) {

    //     // Process any other User exceptions. Use the .id() method to
    //     // record or display useful information
    //     CORBA::Any tmp;
    //     tmp <<= e;
    //     CORBA::TypeCode_var tc = tmp.type();
    //     const char * p = tc->name();
    //     if (*p != '\0') {
    //       ERROR("exception caught in SeD::" << __FUNCTION__ << '(' << p << ')',);
    //     } else {
    //       ERROR("exception caught in SeD::" << __FUNCTION__
    //             << '(' << tc->id() << ')',);
    //     }
    //   } catch (...) {
    //     // Process any other exceptions. This would catch any other C++
    //     // exceptions and should probably never occur
    //     ERROR("unknown exception caught",);
  }
}
#endif // HAVE_ALT_BATCH

/******************************** Data Management ***************************/
const struct timeval*
SeDImpl::timeSinceLastSolve()
{
  return (&(this->lastSolveStart));
}

CORBA::Long
SeDImpl::ping()
{
  TRACE_TEXT(TRACE_ALL_STEPS, "ping()" << endl);
  fflush(stdout);
  return getpid();
}

/****************************************************************************/
/* Private methods                                                          */
/****************************************************************************/

/**
 * Estimate a request.
 * Gather info about SeD by CoRI
 */
inline void
SeDImpl::estimate(corba_estimation_t& estimation,
                  const corba_pb_desc_t& pb,
                  const ServiceTable::ServiceReference_t ref)
{
  diet_perfmetric_t perfmetric_fn = SrvT->getPerfMetric(ref);
  estVector_t eVals = &(estimation);

  diet_profile_t profile;

  /*
  ** create a profile, based on the problem description, to
  ** be used in the performance metric function
  */
  profile.pb_name = strdup(pb.path);
  profile.last_in = pb.last_in;
  profile.last_inout = pb.last_inout;
  profile.last_out = pb.last_out;
  profile.SeDPtr = (const void*) this;
  profile.parameters = (diet_arg_t*) calloc ((pb.last_out+1),
                                             sizeof (diet_arg_t));
#ifdef HAVE_ALT_BATCH
  profile.parallel_flag = pb.parallel_flag ;
#endif

  /* populate the parameter structures */
  for (int i = 0 ; i <= pb.last_out ; i++) {
    const corba_data_desc_t* const cdd = &(pb.param_desc[i]);
    diet_arg_t* da = &(profile.parameters[i]);
    da->value = NULL;
    diet_data_desc_t* ddd = &(da->desc);
    unmrsh_data_desc(ddd, cdd);
  }

  if (perfmetric_fn == NULL){

    /** no metrics construction here: only RR Scheduling at
        the moment when Cori is installed*/

    /***** START CoRI-based metrics *****/
#if HAVE_CORI //dummy values

#ifdef HAVE_ALT_BATCH
    /* TODO:
       - If Batch, we have to make a RR that is more robust than
       just a RR on sites (cf mail from YC the 20 apr 2008)
       - We can add some values that can be transfered to the client
       for contract-client based checking
       - We can check here if client constraints given in a contract are
       respected, and if not, delete memory for the vector and do like
       in SeDImpl.cc:getRequest(), resp.servers.length(0) as a response
       by making SeDImpl.cc:estimate() return 0 or 1 and changing the
       code accordingly in getRequest().
    */
    switch(server_status) {
    case BATCH:
      if( pb.parallel_flag == 1 ) {
        TRACE_TEXT(TRACE_MAIN_STEPS,
                   "SeD::Have to provide information on sequential "
                   "resolution through "
                   << batch->getBatchName() << " Batch scheduler" << endl);
      } else {
        // Give information about parallel_job and parallel_resource
        TRACE_TEXT(TRACE_MAIN_STEPS,
                   "SeD::Have to provide information on parallel "
                   "resolution through " << batch->getBatchName() <<
                   " Batch scheduler" << endl);
      }
      break ;
      /* Set values like nb_resources, nb_free_resources, etc.
         See DIET_data.h */
      WARNING("Set Batch information in vector");
      break ;
    case SERIAL:
      /* Populate with random value */
      diet_est_set_internal(eVals, EST_TCOMP, HUGE_VAL);
      diet_est_set_internal(eVals, EST_EFT, HUGE_VAL);
      diet_est_set_internal(eVals, EST_FREECPU, 0);
      diet_est_set_internal(eVals, EST_FREEMEM, 0);
      diet_est_set_internal(eVals, EST_NBCPU, 1);
      break ;
    default:
      INTERNAL_ERROR_EXIT(__FUNCTION__ << "Type of server is not yet handled"
                          " for performance prediction!");
    }
#else // HAVE_ALT_BATCH
    /* Populate with random value */
    diet_est_set_internal(eVals, EST_TCOMP, HUGE_VAL);
    diet_est_set_internal(eVals, EST_EFT, HUGE_VAL);
    diet_est_set_internal(eVals, EST_FREECPU, 0);
    diet_est_set_internal(eVals, EST_FREEMEM, 0);
    diet_est_set_internal(eVals, EST_NBCPU, 1);
#endif // HAVE_ALT_BATCH

#endif  //!HAVE_CORI


    diet_est_set_internal(eVals,
                          EST_TOTALTIME,
                          diet_est_get_internal(eVals, EST_TCOMP, HUGE_VAL));

    {
      /*
      ** add in times for communication.  there are communication
      ** values ONLY when this method is called from the contract
      ** checking code; if we redo contract checking, we may be
      ** able to minimize/eliminate this section.
      **
      ** TODO: decide if this block should change according to the
      **       above comment
      */
      if (diet_est_get_internal(eVals, EST_TOTALTIME, HUGE_VAL) != HUGE_VAL) {
        double newTotalTime =
          diet_est_get_internal(eVals, EST_TOTALTIME, HUGE_VAL);

        for (int i = 0; i <= pb.last_out; i++) {
          if (diet_est_array_get_internal(eVals,
                                          EST_COMMTIME,
                                          i,
                                          HUGE_VAL) == HUGE_VAL) {
            diet_est_set_internal(eVals, EST_TOTALTIME, HUGE_VAL);
            break;
          }
          //         estimation.totalTime += estimation.commTimes[i];
          newTotalTime += diet_est_array_get_internal(eVals,
                                                      EST_COMMTIME,
                                                      i,
                                                      HUGE_VAL);
        }

        diet_est_set_internal(eVals, EST_TOTALTIME, newTotalTime);
      }
    }
    /***** END CoRI-based metrics *****/

    /***** START RR metrics *****/
    /* TODO: improve this (see Batch remarks higher */
    diet_estimate_lastexec(eVals, &profile);
    /***** END RR metrics *****/
  } else {
    /* populate with default values */
    diet_est_set_internal(eVals, EST_TCOMP, HUGE_VAL);
    diet_est_set_internal(eVals, EST_EFT, HUGE_VAL);
    /*
    ** just call the custom performance metric function!
    */
    (*perfmetric_fn)(&profile, eVals);
  }

  /* Evaluate comm times for persistent IN arguments only: comm times for
     volatile IN and persistent OUT arguments cannot be estimated here, and
     persistent OUT arguments will not move (comm times already set to 0). */
  for (int i = 0; i <= pb.last_inout; i++) {
    // FIXME: here the data localization service must be interrogated to
    // determine the transfer time of all IN and INOUT parameters.
    if ((pb.param_desc[i].mode > DIET_VOLATILE)
        && (pb.param_desc[i].mode <= DIET_STICKY)
        && (*(pb.param_desc[i].id.idNumber) != '\0')) {
      //       estimation.commTimes[i] = 0;
      diet_est_array_set_internal(eVals, EST_COMMTIME, i, 0.0);
    }
  }
}

inline void
SeDImpl::downloadAsyncSeDData(diet_profile_t& profile, corba_profile_t& pb,
                              diet_convertor_t* cvt)
{
  TRACE_TIME(TRACE_MAIN_STEPS, "SeD downloads client datas" << endl);

  dagda_download_data(profile, pb);
}

inline void
SeDImpl::downloadSyncSeDData(diet_profile_t& profile, corba_profile_t& pb,
                             diet_convertor_t* cvt)
{
  TRACE_TIME(TRACE_MAIN_STEPS, "SeD downloads client datas" << endl);

  dagda_download_data(profile, pb);
}

inline void
SeDImpl::uploadAsyncSeDData(diet_profile_t& profile, corba_profile_t& pb,
                            diet_convertor_t* cvt)
{
  TRACE_TIME(TRACE_MAIN_STEPS, "SeD uploads client datas" << endl);

  dagda_upload_data(profile, pb);

  /* Free data */
#if 0
  for(i=0;i<pb.last_out;i++)
    if(!diet_is_persistent(profile.parameters[i])) {
      // FIXME : adding file test
      CORBA::Char *p1 (NULL);
      p1 = pbc.parameters[i].value.get_buffer(1);
      _CORBA_Sequence<unsigned char>::freebuf((_CORBA_Char *)p1);
    }

#endif
}

inline void
SeDImpl::uploadSyncSeDData(diet_profile_t& profile, corba_profile_t& pb,
                           diet_convertor_t* cvt)
{
  TRACE_TIME(TRACE_MAIN_STEPS, "SeD uploads client datas" << endl);

  dagda_upload_data(profile, pb);
}

#if defined HAVE_ALT_BATCH
BatchSystem * // should be const
SeDImpl::getBatch()
{
  return batch ;
}

// int
// diet_submit_parallel(diet_profile_t * profile, const char * command)
// {
//   return batch->diet_submit_parallel(profile, command) ;
// }
// int
// diet_concurrent_submit_parallel(int batchJobID, diet_profile_t * profile,
//                      const char * command)
// {
//   return batch->diet_submit_parallel(batchJobID, profile,
//                                   command) ;
// }
#endif


int
SeDImpl::removeService(const diet_profile_t* const profile)
{
  int res = 0;
  corba_profile_desc_t corba_profile;
  diet_profile_desc_t profileDesc;
#ifdef HAVE_DYNAMICS
  Agent_var parentTmp = this->parent;
#endif // HAVE_DYNAMICS

  if (profile == NULL) {
    ERROR(__FUNCTION__ << ": NULL profile", -1);
  }

  if (this->SrvT == NULL) {
    ERROR(__FUNCTION__ << ": service table not yet initialized", -1);
  }

  if (childID < 0) {
    ERROR(__FUNCTION__ << ": server did not subscribe yet", 1);
  }

  { /* create the corresponding profile description */
    profileDesc.path = strdup(profile->pb_name);
    profileDesc.last_in = profile->last_in;
    profileDesc.last_inout = profile->last_inout;
    profileDesc.last_out = profile->last_out;
#if defined HAVE_ALT_BATCH
    profileDesc.parallel_flag = profile->parallel_flag ;
#endif
    int numArgs = profile->last_out + 1;
    profileDesc.param_desc =
      (diet_arg_desc_t*) calloc (numArgs, sizeof (diet_arg_desc_t));
    for (int argIter = 0 ; argIter < numArgs ; argIter++) {
      profileDesc.param_desc[argIter] =
        (profile->parameters[argIter]).desc.generic;
    }

    profileDesc.aggregator.agg_method = DIET_AGG_DEFAULT;
  }

  mrsh_profile_desc(&corba_profile, &profileDesc);
  if ((res = this->SrvT->rmService(&corba_profile)) != 0)
    return res;

#ifndef HAVE_DYNAMICS
  res = parent->childRemoveService(this->childID, corba_profile);
#else
  res = parentTmp->childRemoveService(this->childID, corba_profile);
#endif // HAVE_DYNAMICS

  return res;
}


int
SeDImpl::removeServiceDesc(const diet_profile_desc_t* profile)
{
  int res = 0;
  corba_profile_desc_t corba_profile;
#ifdef HAVE_DYNAMICS
  Agent_var parentTmp = this->parent;
#endif // HAVE_DYNAMICS


  if (profile == NULL) {
    ERROR(__FUNCTION__ << ": NULL profile", -1);
  }

  if (this->SrvT == NULL) {
    ERROR(__FUNCTION__ << ": service table not yet initialized", -1);
  }

  if (childID < 0) {
    ERROR(__FUNCTION__ << ": server did not subscribe yet", 1);
  }

  mrsh_profile_desc(&corba_profile, profile);
  if ((res = this->SrvT->rmService(&corba_profile)) != 0)
    return res;

#ifndef HAVE_DYNAMICS
  res = parent->childRemoveService(this->childID, corba_profile);
#else
  res = parentTmp->childRemoveService(this->childID, corba_profile);
#endif // HAVE_DYNAMICS

  return res;
}


int
SeDImpl::addService(const corba_profile_desc_t& profile)
{
  SeqCorbaProfileDesc_t profiles;
  ServiceTable::ServiceReference_t sref = this->SrvT->lookupService(&profile);
  profiles.length(1);
  profiles[0] = this->SrvT->getProfile(sref);

#ifdef USE_LOG_SERVICE
  if (dietLogComponent != NULL) {
    dietLogComponent->logAddService(&(profiles[0]));
  }
#endif

  return parent->addServices(this->childID, profiles);
}

// modif bisnard_logs_1
char *
SeDImpl::getDataMgrID() {
  return CORBA::string_dup(this->dataManager->getID()) ;
}
// end modif bisnard_logs_1


/**
 * Returns the list of Profile available
 */
SeqCorbaProfileDesc_t*
SeDImpl::getSeDProfiles(CORBA::Long& length) {
  TRACE_TEXT(TRACE_ALL_STEPS,"ask for list of services" << std::endl);
  return SrvT->getProfiles(length);
}




SeDFwdrImpl::SeDFwdrImpl(Forwarder_ptr fwdr, const char* objName) {
  this->forwarder = Forwarder::_duplicate(fwdr);
  this->objName = CORBA::string_dup(objName);
}

CORBA::Long SeDFwdrImpl::ping() {
  return forwarder->ping(objName);
}

#ifdef HAVE_DYNAMICS
CORBA::Long SeDFwdrImpl::bindParent(const char * parentName) {
  return forwarder->bindParent(parentName, objName);
}

CORBA::Long SeDFwdrImpl::disconnect() {
  return forwarder->disconnect(objName);
}

CORBA::Long SeDFwdrImpl::removeElement() {
  return forwarder->removeElement(false, objName);
}

#endif
void SeDFwdrImpl::getRequest(const corba_request_t& req) {
  return forwarder->getRequest(req, objName);
}

CORBA::Long SeDFwdrImpl::checkContract(corba_estimation_t& estimation,
                                       const corba_pb_desc_t& pb)
{
  return forwarder->checkContract(estimation, pb, objName);
}

void SeDFwdrImpl::updateTimeSinceLastSolve() {
  forwarder->updateTimeSinceLastSolve(objName);
}

CORBA::Long SeDFwdrImpl::solve(const char* pbName, corba_profile_t& pb) {
  return forwarder->solve(pbName, pb, objName);
}

void SeDFwdrImpl::solveAsync(const char* pb_name, const corba_profile_t& pb,
                             const char * volatileclientIOR)
{
  forwarder->solveAsync(pb_name, pb, volatileclientIOR, objName);
}

char* SeDFwdrImpl::getDataMgrID() {
  return forwarder->getDataMgrID(objName);
}

SeqCorbaProfileDesc_t*
SeDFwdrImpl::getSeDProfiles(CORBA::Long& length) {
  return forwarder->getSeDProfiles(length, objName);
}

