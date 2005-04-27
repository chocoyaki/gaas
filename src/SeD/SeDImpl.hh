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
#include "DietLogComponent.hh"

#define HAVE_JUXMEM 0

#if ! HAVE_JUXMEM
#include "DataMgrImpl.hh"     // DTM header file
#endif // ! HAVE_JUXMEM

#define HAVE_QUEUES 1

#if HAVE_QUEUES
#include "AccessController.hh"
#endif

#if HAVE_BATCH
#include "elbase.h"
#endif

/****************************************************************************/
/* SeD class                                                                */
/****************************************************************************/

class SeDImpl : public POA_SeD,
		public PortableServer::RefCountServantBase
{

public:                                              

  SeDImpl();
#if HAVE_JXTA
  SeDImpl(const char*);
#endif // HAVE_JXTA
  ~SeDImpl();
  
  int
  run(ServiceTable* services);
 
#if ! HAVE_JUXMEM
  /** Set this->dataMgr for DTM usage */
  int
  linkToDataMgr(DataMgrImpl* dataMgr);
#endif // ! HAVE_JUXMEM

  /**
   * Set the DietLogComponent of this SeD. If this function is not
   * called or the parameter is NULL, no monitoring information is
   * gathered.
   */
  void
  setDietLogComponent(DietLogComponent* dietLogComponent);

  virtual void
  getRequest(const corba_request_t& req);

  virtual CORBA::Long
  checkContract(corba_estimation_t& estimation,
		const corba_pb_desc_t& pb);

  virtual CORBA::Long
  solve(const char* pbName, corba_profile_t& pb,CORBA::Long reqID);

#if HAVE_BATCH
  virtual CORBA::Long
  solve_batch(const char* pbName, corba_profile_t& pb,CORBA::Long reqID);
#endif

  virtual void
  solveAsync(const char* pb_name, const corba_profile_t& pb,
	    CORBA::Long reqID, const char * volatileclientIOR);

  virtual CORBA::Long
  ping();

#if HAVE_BATCH
  int
  submitBatch(char* parallelService,char* configfilename, int synchro) ;
#endif

  const struct timeval* timeSinceLastSolve();

private:
  
  /** Reference of the parent */
  Agent_var parent;
  /** ID of this agent amongst the children of its parent */
  ChildID childID;

  /* (Fully qualified) local host name */
  char localHostName[257];

  /* Listening port */
  // size_t --> unsigned int
  unsigned int port;

  /* Service table */
  ServiceTable* SrvT;

#if ! HAVE_JUXMEM
  /* Data Manager associated to this SeD */
  DataMgrImpl* dataMgr;
#endif // ! HAVE_JUXMEM

  /* last queue timestamp */
  struct timeval lastSolveStart;

#if HAVE_BATCH
  /* Correspondance with Elagi
  ** -> must be initialized with a special call to Elagi */
  ELBASE_SchedulerServiceTypes batchID ;
  /* Correspondance between the Diet reqID and the Batch Job ID */
  /* Supposes that no more than 100 batch jobs are submitted on one SeD */
  typedef struct {
    int dietReqID ;
    int batchJobId ;
  } corresID ;
  #define MAX_RUNNING_NBSERVICES 100
  corresID tabCorresID[MAX_RUNNING_NBSERVICES] ;
  int tabCorresIDIndex ;
#endif

#if HAVE_QUEUES
  /* Should SeD restrict the number of concurrent solves? */
  bool useConcJobLimit;
  /* If useConcJobLimit == true, how many jobs can run at once? */
  int maxConcJobs;
  /* Enforce limit on concurrent solves with semaphore-like semantics
   * but supporting more features (priority enforcement, count reporting). */
  AccessController* accessController;
#endif

#if HAVE_JXTA
  /* endoint of JXTA SeD*/
  const char* uuid;
#endif // HAVE_JXTA

#if HAVE_FAST

  /** Use of FAST */
  // size_t --> unsigned int
  unsigned int fastUse;

  /* Fast calls mutex, this should be used until FAST becomes reentrant */
  omni_mutex fastMutex;

#endif // HAVE_FAST

  /**
   * The actual dietLogComponent of this SeD. If it contains NULL,
   * no monitoring information must be gathered, so it must be checked
   * each time before it is used.
   */
  DietLogComponent* dietLogComponent;


  /**************************************************************************/
  /* Private methods                                                        */
  /**************************************************************************/



  inline void 
  estimate(corba_estimation_t& estimation,
	   const corba_pb_desc_t& pb,
	   const ServiceTable::ServiceReference_t ref);

#if 0
  int
  dataLookup(long dataId);
  void
  addVar(diet_server_data_desc_t* data);
  void
  rmVar(long dataId);
  void
  rmDeprecatedVars();
#endif // 0
};


#endif // _SED_IMPL_HH_
