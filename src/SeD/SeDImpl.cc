/**
 * @file SeDImpl.cc
 *
 * @brief  DIET SeD implementation source code
 *
 * @author  Philippe COMBES (Philippe.Combes@ens-lyon.fr)
 *          Frederic LOMBARD (Frederic.Lombard@lifc.univ-fcomte.fr)
 *
 * @section Licence
 *   |LICENCE|
 */


#include <cmath>
#include <csignal>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <unistd.h>   // For gethostname()
#include <ctime>
#include <sys/types.h>


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
   #endif  // HAVE_FAST
 */

#include "SeDImpl.hh"
#include "Callback.hh"
#include "common_types.hh"
#include "debug.hh"
#include "est_internal.hh"

#include "CORIMgr.hh"

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
  this->SrvT = NULL;
  this->childID = -1;
  this->parent = Agent::_nil();
  this->localHostName[0] = '\0';
  (this->lastSolveStart).tv_sec = -1;
  (this->lastSolveStart).tv_usec = -1;
#ifdef USE_LOG_SERVICE
  this->dietLogComponent = NULL;
#endif

#if defined HAVE_ALT_BATCH
  this->server_status = SERIAL;
  this->batch = NULL; /* This has to be removed when all SeD will instanciate
                         a class corresponding to what it is read in the .cfg
                         file: FIFO, batch, etc
                         For the moment, NULL is like FIFO
                       */
#endif

  if (gethostname(localHostName, 256)) {
    WARNING("could not get hostname");
  }
  localHostName[255] = '\0';  // If truncated, ensure null termination

  /* Bind this SeD to its name in the CORBA Naming Service */
  std::string name;
  if (!CONFIG_STRING(diet::NAME, name)) {
    /* Generate a name for this SeD and print it */
    std::stringstream oss;
    pid_t pid = getpid();
    oss << localHostName << "_" << pid << "_" << rand() % 10000;
    name = oss.str();
  }
  this->myName = new char[name.size() + 1];
  strcpy(this->myName, name.c_str());
} // initialize

SeDImpl::~SeDImpl() {
  /* FIXME: Tables should be destroyed. */
  ORBMgr::getMgr()->unbind(SEDCTXT, myName);
  ORBMgr::getMgr()->fwdsUnbind(SEDCTXT, myName);
  stat_finalize();
}

/* Method to disconnect from the parent */
CORBA::Long
SeDImpl::disconnect() {
  long rv = 0;
  SeqCorbaProfileDesc_t *profiles(NULL);
  profiles = SrvT->getProfiles();

  /* Do we already have a parent?
   * If yes, we need to unsubscribe.
   */
  if (!CORBA::is_nil(this->parent)) {
    try {
      /* Unsubscribe from parent */
      this->parent->childUnsubscribe(childID, *profiles);
      this->parent = Agent::_nil();

      /* Unsubscribe data manager */
      this->dataManager->unsubscribeParent();

#ifdef USE_LOG_SERVICE
      /* Log */
      if (dietLogComponent != NULL) {
        dietLogComponent->logDisconnect();
      }
#endif
    } catch (CORBA::Exception &e) {
      CORBA::Any tmp;
      tmp <<= e;
      CORBA::TypeCode_var tc = tmp.type();
      WARNING(
        "exception caught 11(" << tc->name() << ") while unsubscribing to "
                               << "parent: either the latter is down, "
                               <<
        "or there is a problem with the CORBA name server");
      rv = 1;
    }
  }

  delete profiles;
  return rv;
} // disconnect

/* Method to dynamically change the parent of the SeD */
CORBA::Long
SeDImpl::bindParent(const char *parentName) {
  long rv = 0;
  SeqCorbaProfileDesc_t *profiles(NULL);
  profiles = SrvT->getProfiles();

  /* Does the new parent exists? */
  Agent_var parentTmp;
  try {
    parentTmp =
      ORBMgr::getMgr()->resolve<Agent, Agent_var>(AGENTCTXT, parentName);
  } catch (...) {
    parentTmp = Agent::_nil();
  }

  if (CORBA::is_nil(parentTmp)) {
    if (CORBA::is_nil(this->parent)) {
      WARNING("cannot locate agent " << parentName << ", will now wait");
    } else {
      WARNING(
        "cannot locate agent " << parentName << ", won't change current parent");
    }
    return 1;
  }

  /* Do we already have a parent?
   * If yes, we need to unsubscribe.
   */
  if (!CORBA::is_nil(this->parent)) {
    try {
      /* Unsubscribe from parent */
      if (childID != -1) {
        this->parent->childUnsubscribe(childID, *profiles);
      }
      this->parent = Agent::_nil();
      childID = -1;

      /* Unsubscribe data manager */
      this->dataManager->unsubscribeParent();
    } catch (CORBA::Exception &e) {
      CORBA::Any tmp;
      tmp <<= e;
      CORBA::TypeCode_var tc = tmp.type();
      WARNING(
        "exception caught 22(" << tc->name() << ") while unsubscribing to "
                               << "parent: either the latter is down, "
                               <<
        "or there is a problem with the CORBA name server");
    }
  }

  /* Now we try to subscribe to a new parent */
  this->parent = parentTmp;

  try {
    childID = this->parent->serverSubscribe(myName, localHostName,
                                            *profiles);

    TRACE_TEXT(TRACE_ALL_STEPS,
               "* Bound myself to parent: " << parentName << "\n");

    /* Data manager also needs to connect to the new parent */
    this->dataManager->subscribeParent(parentName);
#ifdef USE_LOG_SERVICE
    /* Log */
    if (dietLogComponent != NULL) {
      dietLogComponent->logNewParent("SeD", parentName);
    }
#endif
  } catch (CORBA::Exception &e) {
    CORBA::Any tmp;
    tmp <<= e;
    CORBA::TypeCode_var tc = tmp.type();
    WARNING(
      "exception caught 33(" << tc->name() << ") while subscribing to "
                             << parentName <<
      ": either the latter is down, "
                             <<
      "or there is a problem with the CORBA name server");
    rv = 1;
  }
  if (childID < 0) {
    WARNING(__FUNCTION__ << ": error subscribing server");
    this->parent = Agent::_nil();
    rv = 1;
  }

  delete profiles;
  return rv;
} // bindParent


CORBA::Long
SeDImpl::removeElement() {
  /* Send signal to commit suicide */
  return raise(SIGINT);  // Or SIGTERM
}

void
SeDImpl::removeElementClean() {
  SeqCorbaProfileDesc_t *profiles(NULL);
  profiles = SrvT->getProfiles();

  /* Do we already have a parent?
   * If yes, we need to unsubscribe.
   */
  if (!CORBA::is_nil(this->parent)) {
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
  if (dietLogComponent != NULL) {
    dietLogComponent->logRemoveElement();
  }
#endif
} // removeElementClean


int
SeDImpl::run(ServiceTable *services) {
  /* initialize random seed: */
  srand(time(NULL));

  SeqCorbaProfileDesc_t *profiles(NULL);
  stat_init();

  TRACE_TEXT(TRACE_ALL_STEPS, "* Declared myself as: " << this->myName << "\n");
  try {
    ORBMgr::getMgr()->bind(SEDCTXT, this->myName, this->_this(), true);
    ORBMgr::getMgr()->fwdsBind(SEDCTXT, this->myName,
                               ORBMgr::getMgr()->getIOR(_this()));
  } catch (...) {
    ERROR("could not declare myself as " << this->myName, 1);
  }

  this->SrvT = services;

#if defined HAVE_ALT_BATCH
  if (this->server_status == BATCH) {
    // Read "batchName" if parallel jobs are to be submitted
    std::string batchname;
    if (!CONFIG_STRING(diet::BATCHNAME, batchname)) {
      ERROR("SeD can not launch parallel/batch jobs, no parallel/batch"
            " scheduler specified in the config file", 1);
    }
    batch = BatchCreator::getBatchSystem(batchname.c_str());
    if (batch == NULL) {
      ERROR("Parallel/batch scheduler not recognized", 1);
    }
    TRACE_TEXT(TRACE_MAIN_STEPS,
               "Parallel/batch submission enabled with "
               << batch->getBatchName());
    /* TODO: Queues should be provided in the SeD.cfg, or
       should be recognized automatically
       -> for the moment, only one queue specified in the file
     */
    if (batch->getBatchQueueName() != NULL) {
      TRACE_TEXT(TRACE_MAIN_STEPS, " using queue "
                 << batch->getBatchQueueName());
    }
    TRACE_TEXT(TRACE_MAIN_STEPS, "\n");
    TRACE_TEXT(TRACE_MAIN_STEPS, "pathToNFS: " << batch->getNFSPath() << "\n");
    TRACE_TEXT(TRACE_MAIN_STEPS, "pathToTmp: " << batch->getTmpPath() << "\n");
  }
#endif // if defined HAVE_ALT_BATCH

  std::string parent_name;
  if (!CONFIG_STRING(diet::PARENTNAME, parent_name)) {
    return 1;
  }

  try {
    parent =
      ORBMgr::getMgr()->resolve<Agent, Agent_var>(AGENTCTXT, parent_name);
  } catch (...) {
    parent = Agent::_nil();
  }

  if (CORBA::is_nil(parent)) {
    WARNING("cannot locate agent " << parent_name << ", will now wait");
  }

  profiles = SrvT->getProfiles();

#ifdef USE_LOG_SERVICE
  if (dietLogComponent != NULL) {
    for (CORBA::ULong i = 0; i < profiles->length(); i++) {
      dietLogComponent->logAddService(&((*profiles)[i]));
    }
  }
#endif  // USE_LOG_SERVICE

  if (!CORBA::is_nil(parent)) {
    try {
      TRACE_TEXT(
        TRACE_ALL_STEPS, "parent->serverSubscribe(" << this->myName
                                                    << ", " <<
        localHostName << ", *profiles)\n");
      childID = parent->serverSubscribe(this->myName, localHostName,
                                        *profiles);
      TRACE_TEXT(TRACE_ALL_STEPS, "subscribe !\n");
    } catch (CORBA::Exception &e) {
      CORBA::Any tmp;
      tmp <<= e;
      CORBA::TypeCode_var tc = tmp.type();
      ERROR(
        "exception caught (" << tc->name() << ") while subscribing to "
                             << parent_name <<
        ": either the latter is down, "
                             <<
        "or there is a problem with the CORBA name server", 1);
    }
    if (childID < 0) {
      ERROR(__FUNCTION__ << ": error subscribing server", 1);
    }
  } // end: if (! CORBA::is_nil(parent))

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

  if (this->useConcJobLimit) {
    this->accessController = new AccessController(this->maxConcJobs);
    this->jobQueue = new JobQueue(this->maxConcJobs);
    TRACE_TEXT(TRACE_ALL_STEPS, "* SeD Queue: enabled "
               << "(maximum " << this->maxConcJobs << " concurrent solves)\n");
  } else {
    this->accessController = NULL;
    this->jobQueue = new JobQueue();
    TRACE_TEXT(TRACE_ALL_STEPS, "* SeD Queue: disabled (no restriction"
               << " on concurrent solves)\n");
  }



  /* Print out service table */
  if (TRACE_LEVEL >= TRACE_STRUCTURES) {
    SrvT->dump(stdout);
  }

  return CORIMgr::startCollectors();
} // end: run

void
SeDImpl::setDataManager(DagdaImpl *dataManager) {
  this->dataManager = dataManager;
}

#ifdef USE_LOG_SERVICE
void
SeDImpl::setDietLogComponent(DietLogComponent *dietLogComponent) {
  this->dietLogComponent = dietLogComponent;
}
#endif

/*
** The server receives a request by an agent
*/
void
SeDImpl::getRequest(const corba_request_t &creq) {
  corba_response_t resp;
  char statMsg[128];
  Agent_var parentTmp = this->parent;

#ifdef HAVE_ALT_BATCH
  const char *jobSpec;

  if (creq.pb.parallel_flag == 1) {
    jobSpec = "sequential";
  } else {
    jobSpec = "parallel";
  }

  sprintf(statMsg, "getRequest %ld (%s)", (unsigned long) creq.reqID,
          jobSpec);
#else // ifdef HAVE_ALT_BATCH
  sprintf(statMsg, "getRequest %ld", (unsigned long) creq.reqID);
#endif // ifdef HAVE_ALT_BATCH

#ifdef HAVE_ALT_BATCH
  stat_in("SeD", statMsg);
  TRACE_TEXT(TRACE_MAIN_STEPS,
             "\n**************************************************\n"
             << "Got " << jobSpec << " request " << creq.reqID << "\n"
             << "\n");
#else
  stat_in("SeD", statMsg);
  TRACE_TEXT(TRACE_MAIN_STEPS,
             "\n**************************************************\n"
             << "Got request " << creq.reqID << "\n"
             << "\n");
#endif // ifdef HAVE_ALT_BATCH
  resp.reqID = creq.reqID;
  resp.myID = childID;

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
               << "\n");
  } else {
    resp.servers.length(1);

    resp.servers[0].loc.SeDName = CORBA::string_dup(myName);
    resp.servers[0].loc.hostName = CORBA::string_dup(this->localHostName);
    resp.servers[0].loc.port = this->port;
    resp.servers[0].estim.estValues.length(0);

#ifdef HAVE_ALT_BATCH
    resp.servers[0].loc.serverType = server_status;
    resp.servers[0].loc.parallel_flag = creq.pb.parallel_flag;
#endif

    /* Initialize some values */
    estVector_t ev = &(resp.servers[0].estim);
    for (int ctIter = 0; ctIter < creq.pb.last_out; ctIter++) {
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
  stat_out("SeD", statMsg);

  parentTmp->getResponse(resp);
} // getRequest

CORBA::Long
SeDImpl::checkContract(corba_estimation_t &estimation,
                       const corba_pb_desc_t &pb) {
  ServiceTable::ServiceReference_t ref(-1);
  ref = SrvT->lookupService(&(pb));
  if (ref == -1) {
    return 1;
  } else {
    this->estimate(estimation, pb, ref);
  }
  return 0;
}

void
persistent_data_release(corba_data_t *arg) {
  switch ((diet_data_type_t) (arg->desc.specific._d())) {
  case DIET_VECTOR: {
    corba_vector_specific_t vect;

    vect.size = 0;
    arg->desc.specific.vect(vect);
    break;
  }
  case DIET_MATRIX: {
    corba_matrix_specific_t mat;

    arg->desc.specific.mat(mat);
    arg->desc.specific.mat().nb_r = 0;
    arg->desc.specific.mat().nb_c = 0;

    break;
  }
  case DIET_STRING: {
    corba_string_specific_t str;

    str.length = 0;
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
  } // switch
} // persistent_data_release

/** Called from client immediatly after knowing which server is selected
** and will be called by the client, before data transfer.

** Should disappear when data management by Gael is fully tested, because
** by default, data will be managed inside the solve() function and not inside
** the call.
**/
void
SeDImpl::updateTimeSinceLastSolve() {
  gettimeofday(&(this->lastSolveStart), NULL);
}

int
SeDImpl::getNumJobsWaiting() {
  if (accessController) {
    return accessController->getNumWaiting();
  }
  return 0;
}

int
SeDImpl::getActiveJobVector(jobVector_t &jv) {
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

char *
SeDImpl::getName() {
  return CORBA::string_dup(myName);
}


CORBA::Long
SeDImpl::solve(const char *path, corba_profile_t &pb) {
  ServiceTable::ServiceReference_t ref(-1);
  diet_profile_t profile;
  diet_convertor_t *cvt(NULL);
  int solve_res(0);
  char statMsg[128];

  /* Record the SedImpl address */
  profile.SeDPtr = (const void *) this;

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
  if (server_status == BATCH) {
    return this->parallel_solve(path, pb, ref, profile);
  }
#endif

  if (this->useConcJobLimit) {
    this->accessController->waitForResource();
  }

  sprintf(statMsg, "solve %ld", (unsigned long) pb.dietReqID);
  stat_in("SeD", statMsg);

#ifdef USE_LOG_SERVICE
  if (dietLogComponent != NULL) {
    dietLogComponent->logBeginSolve(path, &pb);
  }
#endif

  this->jobQueue->setJobStarted(pb.dietReqID);

  TRACE_TEXT(TRACE_MAIN_STEPS, "SeD::solve invoked on pb: " << path << "\n");

  cvt = SrvT->getConvertor(ref);

  /* Data transfer */
  downloadSyncSeDData(profile, pb, cvt);

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

  TRACE_TEXT(TRACE_MAIN_STEPS, "Calling getSolver\n");
  solve_res = (*(SrvT->getSolver(ref)))(&profile);    // SOLVE

  /* Data transfer */
  uploadSyncSeDData(profile, pb, cvt);

  TRACE_TEXT(
    TRACE_MAIN_STEPS, "SeD::solve complete\n"
    << "************************************************************"
    << "\n");
  delete [] profile.parameters;  // allocated by unmrsh_in_args_to_profile

  this->jobQueue->setJobFinished(profile.dietReqID);
  this->jobQueue->deleteJob(profile.dietReqID);

  stat_out("SeD", statMsg);
  stat_flush();

#ifdef USE_LOG_SERVICE
  if (dietLogComponent != NULL) {
    dietLogComponent->logEndSolve(path, &pb);
  }
#endif

  if (this->useConcJobLimit) {
    this->accessController->releaseResource();
  }

  return solve_res;
} // solve

#if defined HAVE_ALT_BATCH
void
SeDImpl::setServerStatus(diet_server_status_t status) {
  this->server_status = status;
}

diet_server_status_t
SeDImpl::getServerStatus() {
  return server_status;
}

CORBA::Long
SeDImpl::parallel_solve(const char *path, corba_profile_t &pb,
                        ServiceTable::ServiceReference_t &ref,
                        diet_profile_t &profile) {
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

  diet_convertor_t *cvt(NULL);
  int solve_res(0);
  char statMsg[128];

  /* Copying the name of the service in the profile...
   * Not sure this should be done here, but currently it isn't done
   * anywhere else...
   */
  profile.pb_name = strdup(path);

  /* Is there a sens to use Queue with Batch? */
  if (this->useConcJobLimit) {
    this->accessController->waitForResource();
  }

  sprintf(statMsg, "solve %ld", (unsigned long) pb.dietReqID);
  stat_in("SeD", statMsg);

  this->jobQueue->setJobStarted(pb.dietReqID);

#ifdef USE_LOG_SERVICE
  if (dietLogComponent != NULL) {
    dietLogComponent->logBeginSolve(path, &pb);
  }
#endif

  TRACE_TEXT(TRACE_MAIN_STEPS, "SeD::parallel_solve() invoked on pb: "
             << path << "\n");

  cvt = SrvT->getConvertor(ref);

  /* Data transfer */
  downloadSyncSeDData(profile, pb, cvt);

  TRACE_TEXT(TRACE_MAIN_STEPS, "Calling getSolver\n");
  solve_res = (*(SrvT->getSolver(ref)))(&profile);    // SOLVE
  TRACE_TIME(TRACE_MAIN_STEPS, "Submitting DIET job of ID "
             << profile.dietReqID <<
             " on batch system with ID " <<
             batch->getBatchJobID(profile.dietReqID)
             << "\n");
  if (batch->wait4BatchJobCompletion(batch->getBatchJobID(profile.dietReqID))
      < 0) {
    ERROR("An error occured during the execution of the parallel job", 21);
  }
  batch->removeBatchJobID(profile.dietReqID);

  /* Data transfer */
  uploadSyncSeDData(profile, pb, cvt);

  TRACE_TEXT(
    TRACE_MAIN_STEPS, "SeD::parallel_solve() completed\n"
    <<
    "************************************************************\n");
  delete [] profile.parameters;  // allocated by unmrsh_in_args_to_profile

  stat_out("SeD", statMsg);
  stat_flush();

#ifdef USE_LOG_SERVICE
  if (dietLogComponent != NULL) {
    dietLogComponent->logEndSolve(path, &pb);
  }
#endif

  this->jobQueue->setJobFinished(profile.dietReqID);
  this->jobQueue->deleteJob(profile.dietReqID);

  if (this->useConcJobLimit) {
    this->accessController->releaseResource();
  }

  return solve_res;
} // parallel_solve
#endif  // HAVE_ALT_BATCH

void
SeDImpl::solveAsync(const char *path, const corba_profile_t &pb,
                    const char *volatileclientREF) {
  // test validity of volatileclientREF
  // If nil, it is not necessary to solve ...
  try {
    // ServiceTable::ServiceReference_t ref(-1);
    CORBA::Object_var cb = ORBMgr::getMgr()->resolveObject(CLIENTCTXT,
                                                           volatileclientREF);
    // ORBMgr::stringToObject(volatileclientREF);
    if (CORBA::is_nil(cb)) {
      ERROR("SeD::" << __FUNCTION__ << ": received a nil callback", );
    } else {
      ServiceTable::ServiceReference_t ref(-1);
      diet_profile_t profile;
      diet_convertor_t *cvt(NULL);
      char statMsg[128];

      /* Record the SedImpl address */
      profile.SeDPtr = (const void *) this;

      ref = SrvT->lookupService(path, &pb);
      if (ref == -1) {
        ERROR("SeD::" << __FUNCTION__ << ": service not found", );
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
      const corba_profile_desc_t &sed_profile = SrvT->getProfile(ref);
      if (sed_profile.parallel_flag == 2) {
        this->parallel_AsyncSolve(path, pb, ref,
                                  cb, profile);
      } else {
#endif

      if (this->useConcJobLimit) {
        this->accessController->waitForResource();
      }

      sprintf(statMsg, "solveAsync %ld", (unsigned long) pb.dietReqID);
      stat_in("SeD", statMsg);

#ifdef USE_LOG_SERVICE
      if (dietLogComponent != NULL) {
        dietLogComponent->logBeginSolve(path, &pb);
      }
#endif

      this->jobQueue->setJobStarted(pb.dietReqID);

      TRACE_TEXT(
        TRACE_MAIN_STEPS,
        "SeD::solveAsync invoked on pb: " << path
                                          << " (reqID " <<
        profile.dietReqID << ")\n");


      cvt = SrvT->getConvertor(ref);

      downloadAsyncSeDData(profile, const_cast<corba_profile_t &>(pb), cvt);

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

      TRACE_TEXT(TRACE_MAIN_STEPS, "Calling getSolver\n");
      int solve_res = (*(SrvT->getSolver(ref)))(&profile);      // SOLVE

      uploadAsyncSeDData(profile, const_cast<corba_profile_t &>(pb), cvt);

      TRACE_TEXT(
        TRACE_MAIN_STEPS, "SeD::" << __FUNCTION__ << " complete\n"
                                  <<
        "**************************************************\n");

      this->jobQueue->setJobFinished(profile.dietReqID);
      this->jobQueue->deleteJob(profile.dietReqID);

      stat_out("SeD", statMsg);
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
                                          << ": performing the call-back.\n");
      Callback_var cb_var = Callback::_narrow(cb);

      cb_var->notifyResults(path, pb, pb.dietReqID);
      cb_var->solveResults(path, pb, pb.dietReqID, solve_res);

      /* FIXME: do we need to use diet_free_data on profile parameters as
       * we do in the solve(...) method? */
      delete [] profile.parameters;    // allocated by unmrsh_in_args_to_profile
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
    const char *p = tc->name();
    if (*p != '\0') {
      ERROR("exception caught 55in SeD::" << __FUNCTION__ << '(' << p << ')', );
    } else {
      ERROR("exception caught 66in SeD::" << __FUNCTION__
                                          << '(' << tc->id() << ')', );
    }
  } catch (...) {
    // Process any other exceptions. This would catch any other C++
    // exceptions and should probably never occur
    ERROR("unknown exception caught 77", );
  }
} // solveAsync

/* Note: ref is useful for convertors */
#if defined HAVE_ALT_BATCH
void
SeDImpl::parallel_AsyncSolve(const char *path, const corba_profile_t &pb,
                             ServiceTable::ServiceReference_t ref,
                             CORBA::Object_var &cb,
                             diet_profile_t &profile) {
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
      ERROR("SeD::" << __FUNCTION__ << ": received a nil callback", );
    } else {
      diet_convertor_t *cvt(NULL);
      int solve_res(0);
      char statMsg[128];

      /* This can be useful for parallel resolutions */
      if (this->useConcJobLimit) {
        this->accessController->waitForResource();
      }

      sprintf(statMsg, "solve_AsyncParallel %ld", (unsigned long) pb.dietReqID);
      stat_in("SeD", statMsg);

#ifdef USE_LOG_SERVICE
      if (dietLogComponent != NULL) {
        dietLogComponent->logBeginSolve(path, &pb);
      }
#endif

      TRACE_TEXT(
        TRACE_MAIN_STEPS,
        "SeD::solve_AsyncParallel invoked on pb: " << path
                                                   << " (reqID " <<
        pb.dietReqID << ")\n");

      cvt = SrvT->getConvertor(ref);

      downloadAsyncSeDData(profile, const_cast<corba_profile_t &>(pb), cvt);

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
                 << "\n");
      if (
        batch->wait4BatchJobCompletion(batch->getBatchJobID(profile.dietReqID)
                                       )
        < 0) {
        ERROR_EXIT("An error occured during the execution of the parallel job");
      }
      batch->removeBatchJobID(profile.dietReqID);
#else // if defined HAVE_ALT_BATCH
      int status;
      TRACE_TIME(TRACE_MAIN_STEPS, "Submitting script for DIET job of ID "
                 << profile.dietReqID <<
                 " is of pid " <<
                 (long) ((ProcessInfo) findBatchID(profile.dietReqID))->pid
                 << "\n");
      /* This waits until the jobs ends
      ** and remove batchID/DIETreqID correspondance */
      if ((ELBASE_Poll(findBatchID(profile.dietReqID), 1, &status) == 0)
          && status == 0) {
        if (this->batchID == ELBASE_SHELLSCRIPT) {
          ERROR_EXIT("An error occured during the execution of "
                     "the parallel job");
        } else {
          ERROR_EXIT("An error occured during the execution"
                     " of the batch job");
        }
      }
      removeBatchID(pb.dietReqID);
#endif  // HAVE_ALT_BATCH

      uploadAsyncSeDData(profile, const_cast<corba_profile_t &>(pb), cvt);

      TRACE_TEXT(
        TRACE_MAIN_STEPS, "SeD::" << __FUNCTION__ << " complete\n"
                                  <<
        "**************************************************\n");

      stat_out("SeD", statMsg);
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
      TRACE_TEXT(TRACE_ALL_STEPS, "SeD::" << __FUNCTION__
                                          << ": performing the call-back.\n");
      Callback_var cb_var = Callback::_narrow(cb);
      cb_var->notifyResults(path, pb, pb.dietReqID);
      cb_var->solveResults(path, pb, pb.dietReqID, solve_res);
      /* FIXME: do we need to use diet_free_data on profile parameters as
       * we do in the solve(...) method? */
      delete [] profile.parameters;  // allocated by unmrsh_in_args_to_profile
    }
  } catch (const CORBA::Exception &e) {
    // // Process any other User exceptions. Use the .id() method to
    // // record or display useful information
    // CORBA::Any tmp;
    // tmp <<= e;
    // CORBA::TypeCode_var tc = tmp.type();
    // const char * p = tc->name();
    // if (*p != '\0') {
    // ERROR("exception caught in SeD::" << __FUNCTION__ << '(' << p << ')',);
    // } else {
    // ERROR("exception caught in SeD::" << __FUNCTION__
    // << '(' << tc->id() << ')',);
    // }
    // } catch (...) {
    // // Process any other exceptions. This would catch any other C++
    // // exceptions and should probably never occur
    // ERROR("unknown exception caught",);
  }
} // parallel_AsyncSolve
#endif  // HAVE_ALT_BATCH

/******************************** Data Management ***************************/
const struct timeval *
SeDImpl::timeSinceLastSolve() {
  return (&(this->lastSolveStart));
}

CORBA::Long
SeDImpl::ping() {
  TRACE_TEXT(TRACE_ALL_STEPS, "ping()\n");
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
SeDImpl::estimate(corba_estimation_t &estimation,
                  const corba_pb_desc_t &pb,
                  const ServiceTable::ServiceReference_t ref) {
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
  profile.SeDPtr = (const void *) this;
  profile.parameters = (diet_arg_t *) calloc((pb.last_out + 1),
                                             sizeof(diet_arg_t));
#ifdef HAVE_ALT_BATCH
  profile.parallel_flag = pb.parallel_flag;
#endif

  /* populate the parameter structures */
  for (int i = 0; i <= pb.last_out; i++) {
    const corba_data_desc_t *const cdd = &(pb.param_desc[i]);
    diet_arg_t *da = &(profile.parameters[i]);
    da->value = NULL;
    diet_data_desc_t *ddd = &(da->desc);
    unmrsh_data_desc(ddd, cdd);
  }

  if (perfmetric_fn == NULL) {
    /** no metrics construction here: only RR Scheduling at
        the moment when Cori is installed*/

    /***** START CoRI-based metrics *****/
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
    switch (server_status) {
    case BATCH:
      if (pb.parallel_flag == 1) {
        TRACE_TEXT(TRACE_MAIN_STEPS,
                   "SeD::Have to provide information on sequential "
                   "resolution through "
                   << batch->getBatchName() << " Batch scheduler\n");
      } else {
        // Give information about parallel_job and parallel_resource
        TRACE_TEXT(TRACE_MAIN_STEPS,
                   "SeD::Have to provide information on parallel "
                   "resolution through " << batch->getBatchName() <<
                   " Batch scheduler\n");
      }
      break;
      /* Set values like nb_resources, nb_free_resources, etc.
         See DIET_data.h */
      WARNING("Set Batch information in vector");
      break;
    case SERIAL:
      /* Populate with random value */
      diet_est_set_internal(eVals, EST_TCOMP, HUGE_VAL);
      diet_est_set_internal(eVals, EST_EFT, HUGE_VAL);
      diet_est_set_internal(eVals, EST_FREECPU, 0);
      diet_est_set_internal(eVals, EST_FREEMEM, 0);
      diet_est_set_internal(eVals, EST_NBCPU, 1);
      break;
    default:
      INTERNAL_ERROR_EXIT(__FUNCTION__ << "Type of server is not yet handled"
                                          " for performance prediction!");
    } // switch
#else // HAVE_ALT_BATCH
      /* Populate with random value */
    diet_est_set_internal(eVals, EST_TCOMP, HUGE_VAL);
    diet_est_set_internal(eVals, EST_EFT, HUGE_VAL);
    diet_est_set_internal(eVals, EST_FREECPU, 0);
    diet_est_set_internal(eVals, EST_FREEMEM, 0);
    diet_est_set_internal(eVals, EST_NBCPU, 1);
#endif  // HAVE_ALT_BATCH

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
          // estimation.totalTime += estimation.commTimes[i];
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
      // estimation.commTimes[i] = 0;
      diet_est_array_set_internal(eVals, EST_COMMTIME, i, 0.0);
    }
  }
} // estimate

inline void
SeDImpl::downloadAsyncSeDData(diet_profile_t &profile, corba_profile_t &pb,
                              diet_convertor_t *cvt) {
  TRACE_TIME(TRACE_MAIN_STEPS, "SeD downloads client datas\n");

  dagda_download_data(profile, pb);
}

inline void
SeDImpl::downloadSyncSeDData(diet_profile_t &profile, corba_profile_t &pb,
                             diet_convertor_t *cvt) {
  TRACE_TIME(TRACE_MAIN_STEPS, "SeD downloads client datas\n");

  dagda_download_data(profile, pb);
}

inline void
SeDImpl::uploadAsyncSeDData(diet_profile_t &profile, corba_profile_t &pb,
                            diet_convertor_t *cvt) {
  TRACE_TIME(TRACE_MAIN_STEPS, "SeD uploads client datas\n");

  dagda_upload_data(profile, pb);

  /* Free data */
#if 0
  for (i = 0; i < pb.last_out; i++)
    if (!diet_is_persistent(profile.parameters[i])) {
      // FIXME : adding file test
      CORBA::Char *p1(NULL);
      p1 = pbc.parameters[i].value.get_buffer(1);
      _CORBA_Sequence<unsigned char>::freebuf((_CORBA_Char *) p1);
    }

#endif
} // uploadAsyncSeDData

inline void
SeDImpl::uploadSyncSeDData(diet_profile_t &profile, corba_profile_t &pb,
                           diet_convertor_t *cvt) {
  TRACE_TIME(TRACE_MAIN_STEPS, "SeD uploads client datas\n");

  dagda_upload_data(profile, pb);
}

#if defined HAVE_ALT_BATCH
BatchSystem *
// should be const
SeDImpl::getBatch() {
  return batch;
}
#endif


int
SeDImpl::removeService(const diet_profile_t *const profile) {
  int res = 0;
  corba_profile_desc_t corba_profile;
  diet_profile_desc_t profileDesc;
  Agent_var parentTmp = this->parent;

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
    profileDesc.parallel_flag = profile->parallel_flag;
#endif
    int numArgs = profile->last_out + 1;
    profileDesc.param_desc =
      (diet_arg_desc_t *) calloc(numArgs, sizeof(diet_arg_desc_t));
    for (int argIter = 0; argIter < numArgs; argIter++) {
      profileDesc.param_desc[argIter] =
        (profile->parameters[argIter]).desc.generic;
    }

    profileDesc.aggregator.agg_method = DIET_AGG_DEFAULT;
  }

  mrsh_profile_desc(&corba_profile, &profileDesc);
  if ((res = this->SrvT->rmService(&corba_profile)) != 0) {
    return res;
  }

  res = parentTmp->childRemoveService(this->childID, corba_profile);

  return res;
} // removeService


int
SeDImpl::removeServiceDesc(const diet_profile_desc_t *profile) {
  int res = 0;
  corba_profile_desc_t corba_profile;
  Agent_var parentTmp = this->parent;

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
  if ((res = this->SrvT->rmService(&corba_profile)) != 0) {
    return res;
  }

  res = parentTmp->childRemoveService(this->childID, corba_profile);

  return res;
} // removeServiceDesc


int
SeDImpl::addService(const corba_profile_desc_t &profile) {
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
} // addService

// modif bisnard_logs_1
char *
SeDImpl::getDataMgrID() {
  return CORBA::string_dup(this->dataManager->getID());
}
// end modif bisnard_logs_1


/**
 * Returns the list of Profile available
 */
SeqCorbaProfileDesc_t *
SeDImpl::getSeDProfiles(CORBA::Long &length) {
  TRACE_TEXT(TRACE_ALL_STEPS, "ask for list of services\n");
  return SrvT->getProfiles(length);
}




SeDFwdrImpl::SeDFwdrImpl(Forwarder_ptr fwdr, const char *objName) {
  this->forwarder = Forwarder::_duplicate(fwdr);
  this->objName = CORBA::string_dup(objName);
}

CORBA::Long
SeDFwdrImpl::ping() {
  return forwarder->ping(objName);
}

CORBA::Long
SeDFwdrImpl::bindParent(const char *parentName) {
  return forwarder->bindParent(parentName, objName);
}

CORBA::Long
SeDFwdrImpl::disconnect() {
  return forwarder->disconnect(objName);
}

CORBA::Long
SeDFwdrImpl::removeElement() {
  return forwarder->removeElement(false, objName);
}

void
SeDFwdrImpl::getRequest(const corba_request_t &req) {
  return forwarder->getRequest(req, objName);
}

CORBA::Long
SeDFwdrImpl::checkContract(corba_estimation_t &estimation,
                           const corba_pb_desc_t &pb) {
  return forwarder->checkContract(estimation, pb, objName);
}

void
SeDFwdrImpl::updateTimeSinceLastSolve() {
  forwarder->updateTimeSinceLastSolve(objName);
}

CORBA::Long
SeDFwdrImpl::solve(const char *pbName, corba_profile_t &pb) {
  return forwarder->solve(pbName, pb, objName);
}

void
SeDFwdrImpl::solveAsync(const char *pb_name, const corba_profile_t &pb,
                        const char *volatileclientIOR) {
  forwarder->solveAsync(pb_name, pb, volatileclientIOR, objName);
}

char *
SeDFwdrImpl::getDataMgrID() {
  return forwarder->getDataMgrID(objName);
}

SeqCorbaProfileDesc_t *
SeDFwdrImpl::getSeDProfiles(CORBA::Long &length) {
  return forwarder->getSeDProfiles(length, objName);
}
