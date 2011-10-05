/****************************************************************************/
/* DIET SeD implementation header                                           */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Philippe COMBES (Philippe.Combes@ens-lyon.fr)                       */
/*    - Frederic LOMBARD (Frederic.Lombard@lifc.univ-fcomte.fr)             */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.53  2011/05/09 13:10:11  bdepardo
 * Added method diet_get_SeD_services to retreive the services of a SeD given
 * its name
 *
 * Revision 1.52  2011/03/03 00:23:11  bdepardo
 * Resolved a few fix me
 *
 * Revision 1.51  2010/12/17 09:47:59  kcoulomb
 * * Set diet to use the new log with forwarders
 * * Fix a CoRI problem
 * * Add library version remove DTM flag from ccmake because deprecated
 *
 * Revision 1.50  2010/11/24 12:30:17  bdepardo
 * Added getName() method.
 * Moved name and hostname initialization from run() to initialize() method,
 * this allows the logComponent to retrieve the name of the SeD.
 *
 * Revision 1.49  2010/07/12 16:14:10  glemahec
 * DIET 2.5 beta 1 - Use the new ORB manager and allow the use of SSH-forwarders for all DIET CORBA objects
 *
 * Revision 1.48  2010/03/08 13:33:09  bisnard
 * new method to retrieve DAGDA agent ID (CORBA)
 *
 * Revision 1.47  2009/11/30 17:58:08  bdepardo
 * New methods to remove the SeD in a cleaner way.
 *
 * Revision 1.46  2009/10/26 09:14:23  bdepardo
 * Added methods for dynamic hierarchy modifications:
 * - bindParent(const char * parentName)
 * - disconnect()
 * - removeElement()
 *
 * Revision 1.45  2009/06/23 09:28:27  bisnard
 * new API method for EFT estimation
 *
 * Revision 1.44  2008/12/08 15:31:42  bdepardo
 * Added the possibility to remove a service given its profile description.
 * So now, one is able to remove a service given either the real profile,
 * or the profile description.
 *
 * Revision 1.43  2008/11/18 10:13:57  bdepardo
 * - Added the possibility to dynamically create and destroy a service
 *   (even if the SeD is already started). An example is available.
 *   This feature only works with DAGDA.
 * - Added policy commands for CMake 2.6
 * - Added the name of the service in the profile. It was only present in
 *   the profile description, but not in the profile. Currently, the name is
 *   copied in each solve function, but this should certainly be moved
 *   somewhere else.
 *
 * Revision 1.42  2008/07/16 00:45:56  ecaron
 * Remove HAVE_ALTPREDICT (deprecated code)
 *
 * Revision 1.41  2008/05/16 12:25:56  bisnard
 * API give status of all jobs running or waiting on the SeD
 * (used to compute earliest finish time)
 *
 * Revision 1.40  2008/05/11 16:19:48  ycaniou
 * Check that pathToTmp and pathToNFS exist
 * Check and eventually correct if pathToTmp or pathToNFS finish or not by '/'
 * Rewrite of the propagation of the request concerning job parallel_flag
 * Implementation of Cori_batch system
 * Numerous information can be dynamically retrieved through batch systems
 *
 * Revision 1.39  2008/04/07 15:33:42  ycaniou
 * This should remove all HAVE_BATCH occurences (still appears in the doc, which
 *   must be updated.. soon :)
 * Add the definition of DIET_BATCH_JOBID wariable in batch scripts
 *
 * Revision 1.38  2008/01/14 11:32:15  glemahec
 * SeDImpl, the SeD object implementation can now use Dagda as data manager.
 *
 * Revision 1.37  2008/01/01 19:40:35  ycaniou
 * Modifications for batch management
 *
 * Revision 1.36  2007/12/18 13:04:28  glemahec
 * This commit adds the "diet_estimate_waiting_jobs" function to obtain the
 * number of jobs waiting in the FIFO queue when using the max concurrent
 * jobs limit. This function has to be used in the SeD plugin schedulers.
 *
 * Revision 1.35  2007/06/28 18:23:19  rbolze
 * add dietReqID in the profile.
 * and propagate this change to all functions that  have both reqID and profile parameters.
 * TODO : look at the asynchronous mechanism (client->SED) to propage this change.
 *
 * Revision 1.34  2007/05/16 08:39:33  mjan
 * Quelques ajustements avec JuxMem
 *
 * Revision 1.33  2007/04/16 22:43:43  ycaniou
 * Make all necessary changes to have the new option HAVE_ALT_BATCH operational.
 * This is indented to replace HAVE_BATCH.
 *
 * First draw to manage batch systems with a new Cori plug-in.
 *
 * Revision 1.32  2007/03/01 15:55:07  ycaniou
 * Added the updateTimeSinceLastSolve() feature
 *
 * Revision 1.31  2006/10/19 21:26:36  mjan
 * JuxMem support in async mode. Reorganized data management (DTM and JuxMem) into functions in the spirit of last modifs by Yves.
 *
 * Revision 1.30  2006/08/27 18:40:10  ycaniou
 * Modified parallel submission API
 * - client: diet_call_batch() -> diet_parallel_call()
 * - SeD: diet_profile_desc_set_batch() -> [...]_parallel()
 * - from now, internal fields are related to parallel not batch
 * and corrected a bug:
 * - 3 types of submission: request among only seq, only parallel, or all
 *   available services (second wasn't implemented, third bug)
 *
 * Revision 1.29  2006/08/03 11:36:00  ycaniou
 * Removed a DEBUG_YC forgotten
 * Placed batch job management between mutexes
 * First look at async. call
 *
 * Revision 1.28  2006/07/25 14:34:39  ycaniou
 * Use TRACE_TIME to precise time of downloading, submitting and uploading
 *   datas
 * Use a chained list (and not an array anymore) to manage the correspondance
 *   between DIET requests and batch jobs.
 * Changed the prototype of solve_batch: reqID is in the profile when batch mode
 *   is enabled.
 *
 * Batch management for sync. calls is now fully operationnal (at least for oar ;)
 *
 * Revision 1.27  2006/07/11 08:59:09  ycaniou
 * .Batch queue is now read in the serveur config file (only one queue
 * supported).
 * .Transfered perf evaluation in diet server (still dummy function)
 *
 * Revision 1.26  2006/06/30 15:41:47  ycaniou
 * DIET is now capable to submit batch Jobs in synchronous mode. Still some
 *   tuning to do (hard coded NFS path for OAR, tests for synchro between
 *   SeD and the batch job in regard to delete files.., more examples).
 *
 * Put the Data transfer section (JuxMem and DTM) before and after the call to
 * the SeD solve, in inline functions
 *   - downloadSyncSeDData()
 *   - uploadSyncSeDData()
 *
 * Revision 1.25  2006/05/22 20:00:52  hdail
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
 * Revision 1.24  2006/03/08 12:42:48  ycaniou
 * Compilation against the new libUtilsSeDBatch, compiled with gcc
 *  -> #include batch.h requires extern "C"
 *
 * Revision 1.23  2006/01/19 21:35:42  pfrauenk
 * CoRI : when --enable-cori - round-robin is the default scheduler -
 *        CoRI is not called (any more) for collecting information
 *        (so no FAST possible any more)
 *
 * Revision 1.22  2006/01/13 10:40:39  mjan
 * Updating DIET for next JuxMem (0.2)
 *
 * Revision 1.20  2005/08/30 09:20:20  ycaniou
 * Corrected things in DIET_server.cc (diet_submit_batch...)
 * Link libDIET with Elagi and Appleseeds only if BATCH is asked in the
 *   configuration (corrected the Makefile)
 * Changed things in SeDImpl.[ch] for batch submission to work. Only synchronous
 *   mode made.
 *
 * Revision 1.19  2005/05/18 14:18:09  mjan
 * Initial adding of JuxMem support inside DIET. dmat_manips examples tested without JuxMem and with JuxMem
 *
 * Revision 1.18  2005/05/02 16:46:33  ycaniou
 * Added the function diet_submit_batch(), the stuff in the makefile to compile
 *  with appleseeds..
 *
 * Revision 1.17  2005/04/27 01:41:34  ycaniou
 * Added the stuff for a correct compilation, for a correct registration of
 * a batch profile, and for its execution.
 * Added the solve_batch() function
 *
 * Revision 1.16  2005/04/13 08:46:29  hdail
 * Beginning of adoption of new persistency model: DTM is enabled by default and
 * JuxMem will be supported via configure flags.  DIET will always provide at
 * least one type of persistency.  As a first step, persistency across DTM and
 * JuxMem is not supported so all persistency handling should be surrounded by
 *     #if HAVE_JUXMEM
 *       // JuxMem code
 *     #else
 *       // DTM code
 *     #endif
 * This check-in prepares for the JuxMem check-in by cleaning up old
 * DEVELOPPING_DATA_PERSISTENCY flags and surrounding DTM code with
 * #if ! HAVE_JUXMEM / #endif flags to be replaced by above format by Mathieu's
 * check-in.  Currently the HAVE_JUXMEM flag is set in SeDImpl.hh - to be replaced
 * by Mathieu's check-in of a configure system for JuxMem.
 *
 * Revision 1.15  2005/04/08 13:02:43  hdail
 * The code for LogCentral has proven itself stable and it seems bug free.
 * Since no external libraries are required to compile in LogCentral, its now
 * going to be compiled in by default always ... its usage is easily controlled by
 * configuration file.
 *
 * Revision 1.14  2004/12/08 15:02:51  alsu
 * plugin scheduler first-pass validation testing complete.  merging into
 * main CVS trunk; ready for more rigorous testing.
 *
 * Revision 1.13.2.2  2004/11/26 15:19:44  alsu
 * adding timeSinceLastSolve() to give enable access to the last-solve
 * timestamp
 *
 * Revision 1.13.2.1  2004/10/26 14:12:52  alsu
 * (Tag: AS-plugin-sched)
 *  - branch created to avoid conflicting with release 1.2 (imminent)
 *  - initial commit on branch, new dynamic performance info structure in
 *    the profile
 *
 * Revision 1.13  2004/10/04 13:53:41  hdail
 * Added ability to restrict number of concurrent jobs running in the SeD.
 *
 * Revision 1.12  2004/10/04 13:52:32  hdail
 * Added ability to restrict number of concurrent jobs running in the SeD.
 *
 * Revision 1.11  2004/07/29 18:52:11  rbolze
 * Change solve function now , DIET_client send the reqID of the request when
 * he call the solve function.
 * Nothing is change for DIET's API
 *
 * Revision 1.10  2004/07/05 14:56:13  rbolze
 * correct bug on 64 bit plat-form, when parsing cfg file :
 * remplace size_t by unsigned int for config options
 *
 * Revision 1.9  2004/06/11 15:45:39  ctedesch
 * add DIET/JXTA
 *
 * Revision 1.8  2004/03/01 18:43:23  rbolze
 * add logservice
 ****************************************************************************/


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

#include "Forwarder.hh"
#include "SeDFwdr.hh"

/****************************************************************************/
/* SeD class                                                                */
/****************************************************************************/
class SeDImpl : public POA_SeD,
                public PortableServer::RefCountServantBase
{

public:

  SeDImpl();
  ~SeDImpl();

#ifdef HAVE_DYNAMICS
  virtual CORBA::Long
  bindParent(const char * parentName);
  virtual CORBA::Long
  disconnect();
  virtual CORBA::Long
  removeElement();
  void removeElementClean();
#endif // HAVE_DYNAMICS

  int
  run(ServiceTable* services);

  void
  setDataManager(DagdaImpl* dataManager);

#ifdef USE_LOG_SERVICE
  /**
   * Set the DietLogComponent of this SeD. If this function is not
   * called or the parameter is NULL, no monitoring information is
   * gathered.
   */
  void
  setDietLogComponent(DietLogComponent* dietLogComponent);
#endif

  virtual void
  getRequest(const corba_request_t& req);

  virtual CORBA::Long
  checkContract(corba_estimation_t& estimation,
                const corba_pb_desc_t& pb);

  virtual void
  updateTimeSinceLastSolve() ;

  virtual CORBA::Long
  solve(const char* pbName, corba_profile_t& pb);

  char* getName();

#if defined HAVE_ALT_BATCH
  /* Set if server is SERIAL, BATCH,.. */
  void
  setServerStatus( diet_server_status_t status ) ;

  diet_server_status_t
  getServerStatus() ;

  virtual CORBA::Long
  parallel_solve(const char* pbName, corba_profile_t& pb,
                 ServiceTable::ServiceReference_t& ref,
                 diet_profile_t& profile) ;

  void
  parallel_AsyncSolve(const char* path, const corba_profile_t& pb,
                      ServiceTable::ServiceReference_t ref,
                      CORBA::Object_var & cb,
                      diet_profile_t& profile) ;

  char* getLocalHostName() ;
#endif
#if HAVE_ALT_BATCH
  BatchSystem * // should be const
  getBatch() ;

  //   int
  //   diet_submit_parallel(diet_profile_t * profile, const char * command) ;

  //   int
  //   diet_concurrent_submit_parallel(int batchJobID, diet_profile_t * profile,
  //                                   const char * command) ;
#endif


  virtual void
  solveAsync(const char* pb_name, const corba_profile_t& pb,
             const char * volatileclientIOR);

  virtual CORBA::Long
  ping();

  const struct timeval* timeSinceLastSolve();

  /* Access to the queue size in the AccessController object. */
  int getNumJobsWaiting();

  /**
   * Retrieve the list of all jobs currently waiting or running
   * @param jv  a table of diet_job_t (caller resp. for freeing this table)
   * @return number of jobs in the table (0 if failure)
   */
  int getActiveJobVector(jobVector_t& jv);

  /**
   * Get the Earliest Finish Time of the SeD ie time until it's available
   * This will take into account the SeD's current job queue (jobs either
   * running or waiting) and SeD's concurrency constraint (nb of procs).
   * @return  the estimated EFT in ms from now
   */
  double getEFT();


  int removeService(const diet_profile_t* const profile);
  int removeServiceDesc(const diet_profile_desc_t* profile);
  int addService(const corba_profile_desc_t& profile);
  virtual char* getDataMgrID(); // modif bisnard_logs_1

  virtual  SeqCorbaProfileDesc_t*
  getSeDProfiles(CORBA::Long& length);

private:
#ifdef HAVE_ALT_BATCH
  /* Status of SeD: Batch, Serial, other? */
  diet_server_status_t server_status ;
#endif

  /** Reference of the parent */
  Agent_var parent;

  /** Identity in the CORBA Naming Service */
  char* myName;

  /** ID of this agent amongst the children of its parent */
  ChildID childID;

  /* (Fully qualified) local host name */
  char localHostName[256];

  /** Listening port */
  unsigned int port;

  /** Service table */
  ServiceTable* SrvT;

  DagdaImpl* dataManager;

  /** Time at which last solve started (when not using queues) and when
   * last job was enqueued (when using queues) */
  struct timeval lastSolveStart;

#if HAVE_ALT_BATCH
  BatchSystem * batch ;
#endif

#if HAVE_SEDSCHEDULER
  SeDScheduler * sched ;
#endif

  /* Queue: should SeD restrict the number of concurrent solves? */
  bool useConcJobLimit;
  /* Queue: If useConcJobLimit == true, how many jobs can run at once? */
  int maxConcJobs;
  /* Queue: Enforce limit on concurrent solves with semaphore-like semantics
   * but supporting more features (priority enforcement, count reporting). */
  AccessController* accessController;
  /* Queue: Maintains the list of running or waiting jobs
   * used for computing the estimation of earliest finish time */
  JobQueue* jobQueue;

#ifdef USE_LOG_SERVICE
  /**
   * The actual dietLogComponent of this SeD. If it contains NULL,
   * no monitoring information must be gathered, so it must be checked
   * each time before it is used.
   */
  DietLogComponent* dietLogComponent;
#endif

  /**************************************************************************/
  /* Private methods                                                        */
  /**************************************************************************/

  /** Private method to centralize all shared variable initializations
   * in various constructors. */
  virtual void initialize();

  inline void
  estimate(corba_estimation_t& estimation,
           const corba_pb_desc_t& pb,
           const ServiceTable::ServiceReference_t ref);

  /**
   * TODO: if possible merge async and sync function. Currently, the DTM code
   * if different
   */

  inline void
  downloadSyncSeDData(diet_profile_t& profile, corba_profile_t& pb,
                      diet_convertor_t* cvt) ;

  inline void
  uploadSyncSeDData(diet_profile_t& profile, corba_profile_t& pb,
                    diet_convertor_t* cvt) ;

  inline void
  downloadAsyncSeDData(diet_profile_t& profile, corba_profile_t& pb,
                       diet_convertor_t* cvt) ;

  inline void
  uploadAsyncSeDData(diet_profile_t& profile, corba_profile_t& pb,
                     diet_convertor_t* cvt) ;

};

class SeDFwdrImpl : public POA_SeD,
                    public PortableServer::RefCountServantBase
{
protected:
  Forwarder_ptr forwarder;
  char* objName;

public:
  SeDFwdrImpl(Forwarder_ptr fwdr, const char* objName);
  virtual CORBA::Long ping();
#ifdef HAVE_DYNAMICS
  virtual CORBA::Long bindParent(const char * parentName);
  virtual CORBA::Long disconnect();
  virtual CORBA::Long removeElement();
#endif
  virtual void getRequest(const corba_request_t& req);
  virtual CORBA::Long checkContract(corba_estimation_t& estimation,
                                    const corba_pb_desc_t& pb);

  virtual void updateTimeSinceLastSolve() ;

  virtual CORBA::Long solve(const char* pbName, corba_profile_t& pb);
  virtual void solveAsync(const char* pb_name, const corba_profile_t& pb,
                          const char * volatileclientIOR);
  virtual char* getDataMgrID();

  virtual  SeqCorbaProfileDesc_t*
  getSeDProfiles(CORBA::Long& length);
};

#endif // _SED_IMPL_HH_
