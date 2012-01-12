/**
 * @file SeDImpl.hh
 *
 * @brief  DIET SeD implementation header
 *
 * @author  Philippe COMBES (Philippe.Combes@ens-lyon.fr)
 *          Frederic LOMBARD (Frederic.Lombard@lifc.univ-fcomte.fr)
 *
 * @section Licence
 *   |LICENCE|
 */


#ifndef _SEDIMPL_HH_
#define _SEDIMPL_HH_

#include "SeD.hh"

#include "Agent.hh"
#include "common_types.hh"
#include "ChildID.hh"
#include "Counter.hh"
#include "DIET_data_internal.hh"
#include "response.hh"
#include "ServiceTable.hh"
#include "AccessController.hh"
#include "JobQueue.hh"

#ifdef USE_LOG_SERVICE
#include "DietLogComponent.hh"
#endif

#include "DagdaImpl.hh"

#if HAVE_ALT_BATCH
extern "C" {
#include "DIET_server.h" /* For inclusion of diet_server_status_t */
}
#include "BatchSystem.hh"
#endif
#if HAVE_SEDSCHEDULER
#include "SeDScheduler.hh"
#endif
#ifdef WIN32
   #define SHAREDLIB __declspec(dllexport)
#else
   #define SHAREDLIB
#endif
#include "Forwarder.hh"
#include "SeDFwdr.hh"

/****************************************************************************/
/* SeD class                                                                */
/****************************************************************************/
class SeDImpl : public POA_SeD,
public PortableServer::RefCountServantBase {
public:
  SeDImpl();

  ~SeDImpl();

  virtual CORBA::Long
  bindParent(const char *parentName);

  virtual CORBA::Long
  disconnect();

  virtual CORBA::Long
  removeElement();

  void
  removeElementClean();

  int
  run(ServiceTable *services);

  void
  setDataManager(DagdaImpl *dataManager);

#ifdef USE_LOG_SERVICE
  /**
   * Set the DietLogComponent of this SeD. If this function is not
   * called or the parameter is NULL, no monitoring information is
   * gathered.
   */
  void
  setDietLogComponent(DietLogComponent *dietLogComponent);
#endif

  virtual void
  getRequest(const corba_request_t &req);

  virtual CORBA::Long
  checkContract(corba_estimation_t &estimation,
                const corba_pb_desc_t &pb);

  virtual void
  updateTimeSinceLastSolve();

  virtual CORBA::Long
  solve(const char *pbName, corba_profile_t &pb);

  char *
  getName();

#if defined HAVE_ALT_BATCH
  /* Set if server is SERIAL, BATCH,.. */
  void
  setServerStatus(diet_server_status_t status);

  diet_server_status_t
  getServerStatus();

  virtual CORBA::Long
  parallel_solve(const char *pbName, corba_profile_t &pb,
                 ServiceTable::ServiceReference_t &ref,
                 diet_profile_t &profile);

  void
  parallel_AsyncSolve(const char *path, const corba_profile_t &pb,
                      ServiceTable::ServiceReference_t ref,
                      CORBA::Object_var &cb,
                      diet_profile_t &profile);

  char *
  getLocalHostName();
#endif /* if defined HAVE_ALT_BATCH */
#if HAVE_ALT_BATCH
  BatchSystem *
  getBatch();  // should be const
#endif

  virtual void
  solveAsync(const char *pb_name, const corba_profile_t &pb,
             const char *volatileclientIOR);

  virtual CORBA::Long
  ping();

  const struct timeval *
  timeSinceLastSolve();

  /* Access to the queue size in the AccessController object. */
  int
  getNumJobsWaiting();

  /**
   * Retrieve the list of all jobs currently waiting or running
   * @param jv  a table of diet_job_t (caller resp. for freeing this table)
   * @return number of jobs in the table (0 if failure)
   */
  int
  getActiveJobVector(jobVector_t &jv);

  /**
   * Get the Earliest Finish Time of the SeD ie time until it's available
   * This will take into account the SeD's current job queue (jobs either
   * running or waiting) and SeD's concurrency constraint (nb of procs).
   * @return  the estimated EFT in ms from now
   */
  double
  getEFT();


  int
  removeService(const diet_profile_t *const profile);

  int
  removeServiceDesc(const diet_profile_desc_t *profile);

  int
  addService(const corba_profile_desc_t &profile);

  virtual char *
  getDataMgrID();  // modif bisnard_logs_1

  virtual SeqCorbaProfileDesc_t *
  getSeDProfiles(CORBA::Long &length);

private:
#ifdef HAVE_ALT_BATCH
  /* Status of SeD: Batch, Serial, other? */
  diet_server_status_t server_status;
#endif

  /** Reference of the parent */
  Agent_var parent;

  /** Identity in the CORBA Naming Service */
  char *myName;

  /** ID of this agent amongst the children of its parent */
  ChildID childID;

  /* (Fully qualified) local host name */
  char localHostName[256];

  /** Listening port */
  unsigned int port;

  /** Service table */
  ServiceTable *SrvT;

  DagdaImpl *dataManager;

  /** Time at which last solve started (when not using queues) and when
   * last job was enqueued (when using queues) */
  struct timeval lastSolveStart;

#if HAVE_ALT_BATCH
  BatchSystem *batch;
#endif

#if HAVE_SEDSCHEDULER
  SeDScheduler *sched;
#endif

  /* Queue: should SeD restrict the number of concurrent solves? */
  bool useConcJobLimit;
  /* Queue: If useConcJobLimit == true, how many jobs can run at once? */
  int maxConcJobs;
  /* Queue: Enforce limit on concurrent solves with semaphore-like semantics
  * but supporting more features (priority enforcement, count reporting). */
  AccessController *accessController;
  /* Queue: Maintains the list of running or waiting jobs
   * used for computing the estimation of earliest finish time */
  JobQueue *jobQueue;

#ifdef USE_LOG_SERVICE
  /**
   * The actual dietLogComponent of this SeD. If it contains NULL,
   * no monitoring information must be gathered, so it must be checked
   * each time before it is used.
   */
  DietLogComponent *dietLogComponent;
#endif

  /**************************************************************************/
  /* Private methods                                                        */
  /**************************************************************************/

  /** Private method to centralize all shared variable initializations
   * in various constructors. */
  virtual void
  initialize();

  inline void
  estimate(corba_estimation_t &estimation,
           const corba_pb_desc_t &pb,
           const ServiceTable::ServiceReference_t ref);

  /**
   * TODO: if possible merge async and sync function. Currently, the DTM code
   * if different
   */
  inline void
  downloadSyncSeDData(diet_profile_t &profile, corba_profile_t &pb,
                      diet_convertor_t *cvt);

  inline void
  uploadSyncSeDData(diet_profile_t &profile, corba_profile_t &pb,
                    diet_convertor_t *cvt);

  inline void
  downloadAsyncSeDData(diet_profile_t &profile, corba_profile_t &pb,
                       diet_convertor_t *cvt);

  inline void
  uploadAsyncSeDData(diet_profile_t &profile, corba_profile_t &pb,
                     diet_convertor_t *cvt);
};

class SHAREDLIB SeDFwdrImpl : public POA_SeD,
public PortableServer::RefCountServantBase {
public:
  SeDFwdrImpl(Forwarder_ptr fwdr, const char *objName);

  virtual CORBA::Long
  ping();

  virtual CORBA::Long
  bindParent(const char *parentName);

  virtual CORBA::Long
  disconnect();

  virtual CORBA::Long
  removeElement();

  virtual void
  getRequest(const corba_request_t &req);

  virtual CORBA::Long
  checkContract(corba_estimation_t &estimation,
                const corba_pb_desc_t &pb);

  virtual void
  updateTimeSinceLastSolve();

  virtual CORBA::Long
  solve(const char *pbName, corba_profile_t &pb);

  virtual void
  solveAsync(const char *pb_name, const corba_profile_t &pb,
             const char *volatileclientIOR);

  virtual char *
  getDataMgrID();

  virtual SeqCorbaProfileDesc_t *
  getSeDProfiles(CORBA::Long &length);

protected:
  Forwarder_ptr forwarder;
  char *objName;
};

#endif  // _SED_IMPL_HH_
