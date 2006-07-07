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
 * Revision 1.59  2005/09/07 07:40:16  hdail
 * Exclusion of all SeD-level comm time prediction and modification under
 * HAVE_ALTPREDICT
 *
 * Revision 1.58  2005/09/05 16:02:52  hdail
 * Addition of locationID as member variable and to parsing.  SeD initializes
 * data location data in response with all locally available information about
 * parameters. (experimental and protected by HAVE_ALTPREDICT).
 *
 * Revision 1.57  2005/08/31 14:39:57  alsu
 * New plugin scheduling interface: adapting estimation vector
 * manipulation calls to the new interface
 ****************************************************************************/

#include <iostream>
using namespace std;
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>   // For gethostname()
#include <time.h>

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

#include "DIET_config.h"
#include "SeDImpl.hh"
#include "Callback.hh"
#include "common_types.hh"
#include "debug.hh"
#include "est_internal.hh"

#if HAVE_CORI
#include "CORIMgr.hh"
#else 
#include "FASTMgr.hh"
#endif //HAVE_CORI

#include "marshalling.hh"
#include "ORBMgr.hh"
#include "Parsers.hh"
#include "statistics.hh"

/** The trace level. */
extern unsigned int TRACE_LEVEL;

#define SED_TRACE_FUNCTION(formatted_text)       \
  TRACE_TEXT(TRACE_ALL_STEPS, "SeD::");          \
  TRACE_FUNCTION(TRACE_ALL_STEPS,formatted_text)

SeDImpl::SeDImpl()
{
  this->initialize();
}

#if HAVE_JXTA
SeDImpl::SeDImpl(const char* uuid = '\0')
{
  this->uuid = uuid;
  this->initialize();
}
#endif //HAVE_JXTA

/** Private method to centralize all shared variable initializations from
 * different constructors.  Call this only from a constructor. */
void
SeDImpl::initialize()
{
  this->SrvT    = NULL;
  this->childID = -1;
  this->parent  = Agent::_nil();
  this->localHostName[0] = '\0';
  (this->lastSolveStart).tv_sec = -1;
  (this->lastSolveStart).tv_usec = -1;
  this->dietLogComponent = NULL;

#if HAVE_ALTPREDICT
  this->locationID[0] = '\0';
#endif

#if !HAVE_CORI && HAVE_FAST
  this->fastUse = 1;
#endif //!HAVE_CORI && HAVE_FAST

#if HAVE_BATCH
  this->tabCorresIDIndex = 0 ;
  for( int i=0 ; i<MAX_RUNNING_NBSERVICES ; i++ )
    tabCorresID[i].dietReqID = -1 ;
#endif //HAVE_BATCH
}

SeDImpl::~SeDImpl()
{
  /* FIXME: Tables should be destroyed. */
  stat_finalize();  
}

int
SeDImpl::run(ServiceTable* services)
{
  SeqCorbaProfileDesc_t* profiles(NULL);
  stat_init();  
  if (gethostname(localHostName, 256)) {
    ERROR("could not get hostname", 1);
  }
  localHostName[255] = '\0'; // If truncated, ensure null termination 
 
  this->SrvT = services;

#if HAVE_BATCH
  // A SeD can only launch batch or -exclusive- non batch jobs.
  // Then, all profiles must be either batch or non batch. No mix allowed. 
  if( this->SrvT->testIfAllBatchServices() ) {
    // Read batchName if parallel jobs are to be submitted
    char* batchname = (char*) 
      Parsers::Results::getParamValue(Parsers::Results::BATCHNAME) ;
    if (batchname == NULL) {
      ERROR("SeD can not launch batch jobs, no batch scheduler specified"
	    " in the config file", 1) ;
    }
    if( !(ELBASE_ExistBatchScheduler(batchname,&this->batchID)) ) {
      ERROR("Batch scheduler not recognized", 1) ;
    }
    TRACE_TEXT(TRACE_MAIN_STEPS,
    	       "Batch submission enabled with " << ELBASE_GiveBatchName(this->batchID) << "\n") ;
  } else if (this->SrvT->existBatchService()) {
    ERROR("SeD is not allowed to launch batch and non batch job", 1) ;
  }
#endif

  char* parent_name = (char*)
    Parsers::Results::getParamValue(Parsers::Results::PARENTNAME);
  if (parent_name == NULL) {
    return 1;  
  }
  parent =
    Agent::_duplicate(Agent::_narrow(ORBMgr::getObjReference(ORBMgr::AGENT,
                                                             parent_name)));
  if (CORBA::is_nil(parent)) {
    ERROR("cannot locate agent " << parent_name, 1);
  }
  
  profiles = SrvT->getProfiles();

  if (dietLogComponent != NULL) {
    for (CORBA::ULong i=0; i<profiles->length(); i++) {
      dietLogComponent->logAddService(&((*profiles)[i]));
    }
  }

  try {
    childID = parent->serverSubscribe(this->_this(), localHostName,
#if HAVE_JXTA
                                      uuid,
#endif //HAVE_JXTA
                                      *profiles);
  } catch (CORBA::Exception& e) {
    CORBA::Any tmp;
    tmp <<= e;
    CORBA::TypeCode_var tc = tmp.type();
    ERROR("exception caught (" << tc->name() << ") while subscribing to "
          << parent_name << ": either the latter is down, "
          << "or there is a problem with the CORBA name server", 1);
  }
  if (childID < 0) {
    ERROR(__FUNCTION__ << ": error subscribing server\n", 1);
  }
  delete profiles;

  unsigned int* endPoint = (unsigned int*)
    Parsers::Results::getParamValue(Parsers::Results::DIETPORT);
  // FIXME: How can I get the port used by the ORB ? and is it useful ?
  if (endPoint == NULL) {
    this->port = 0;
  } else {
    this->port = *endPoint;
  }

#if HAVE_ALTPREDICT
  char * tmpName = (char*)
      Parsers::Results::getParamValue(Parsers::Results::LOCATIONID);
  if (tmpName != NULL) {
    strcpy(this->locationID, tmpName);
  } else {
    strcpy(this->locationID, "");
  } 
#endif // HAVE_ALTPREDICT

  bool* tmpBoolPtr;
  int* tmpIntPtr;
  tmpBoolPtr = (bool*)
    Parsers::Results::getParamValue(Parsers::Results::USECONCJOBLIMIT);
  if(tmpBoolPtr == NULL){
    this->useConcJobLimit = false;
  } else {
    this->useConcJobLimit = *tmpBoolPtr;
  }

  tmpIntPtr = (int*)
    Parsers::Results::getParamValue(Parsers::Results::MAXCONCJOBS);
  if(tmpIntPtr == NULL){
    /* If queues requested, but no limit specified, restrict to 1 */
    this->maxConcJobs = 1;
  } else {
    this->maxConcJobs = *tmpIntPtr;
  }

  if (this->useConcJobLimit){
    this->accessController = new AccessController(this->maxConcJobs);
    TRACE_TEXT(TRACE_ALL_STEPS, "* SeD Queue: enabled "
        << "(maximum " << this->maxConcJobs << " concurrent solves)\n");
  } else {
    this->accessController = NULL;
    TRACE_TEXT(TRACE_ALL_STEPS, "* SeD Queue: disabled (no restriction"
        << " on concurrent solves)\n");
  }

  /* Print out service table */
  if (TRACE_LEVEL >= TRACE_STRUCTURES) {
    SrvT->dump(stdout);
  }

#if HAVE_CORI  
 return CORIMgr::startCollectors();
#else //HAVE_CORI  
 return FASTMgr::init();
#endif //HAVE_CORI  

}

#if HAVE_JUXMEM
/** Set this->juxmem */
int
SeDImpl::linkToJuxMem(JuxMem::Wrapper* juxmem)
{
  this->juxmem = juxmem;
  return 0;
}
#else
/** Set this->dataMgr */
int
SeDImpl::linkToDataMgr(DataMgrImpl* dataMgr)
{
  this->dataMgr = dataMgr;
  return 0;
}
#endif // HAVE_JUXMEM

void
SeDImpl::setDietLogComponent(DietLogComponent* dietLogComponent) {
  this->dietLogComponent = dietLogComponent;
}

/* 
** The server receives a request by an agent
*/
void
SeDImpl::getRequest(const corba_request_t& creq)
{
  corba_response_t resp;
  char statMsg[128];

  sprintf(statMsg, "getRequest %ld", (unsigned long) creq.reqID);
  stat_in("SeD",statMsg);
  TRACE_TEXT(TRACE_MAIN_STEPS,
             "\n**************************************************\n"
             << "Got request " << creq.reqID << endl << endl);
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
#if HAVE_ALTPREDICT
    resp.dataLoc.length(0);
#endif
    cout << "service not found ??????????????????????????????????????" << endl;
  } else {
    resp.servers.length(1);

    resp.servers[0].loc.ior      = SeD::_duplicate(_this());
    resp.servers[0].loc.hostName = CORBA::string_dup(this->localHostName);
    resp.servers[0].loc.port     = this->port;
    resp.servers[0].estim.estValues.length(0);
#if HAVE_JXTA
    resp.servers[0].loc.uuid = CORBA::string_dup(uuid);
#endif //HAVE_JXTA
#if HAVE_ALTPREDICT
    resp.servers[0].loc.locationID = CORBA::string_dup(this->locationID);

    /* Fill in available information about all parameters */
    resp.dataLoc.length(creq.pb.last_out + 1);
    for (int i = 0; i <= creq.pb.last_out; i++) {
      /** Get copy of data ID */
      resp.dataLoc[i].idNumber = 
          CORBA::string_dup(creq.pb.param_desc[i].id.idNumber);
      /** Calculate total amount of data needed for parameter transfer */
      resp.dataLoc[i].bytes = data_sizeof(&(creq.pb.param_desc[i]));
      /** If data is local, fill in location info */
      if (this->dataMgr->dataLookup(resp.dataLoc[i].idNumber)) {
        resp.dataLoc[i].hostName = CORBA::string_dup(this->localHostName);
        resp.dataLoc[i].locationID = CORBA::string_dup(this->locationID);
      } else {
        resp.dataLoc[i].hostName = CORBA::string_dup("");
        resp.dataLoc[i].locationID = CORBA::string_dup("");
      } 
    }
#endif


#if ! HAVE_BATCH
  #if ! HAVE_ALTPREDICT
    estVector_t ev = &(resp.servers[0].estim);
    for (int ctIter = 0 ; ctIter < creq.pb.last_out ; ctIter++) {
      diet_est_set_internal(ev, EST_COMMTIME, 0.0);
    }
  #else
    // Predictions are done at the agents
  #endif // HAVE_ALTPREDICT

#else // HAVE_BATCH
    // TODO: What do I have to do for a batch, non batch, parallel, etc.?
    // for the moment, do the same but..
    estVector_t ev = &(resp.servers[0].estim);
    for (int ctIter = 0 ; ctIter < creq.pb.last_out ; ctIter++) {
      diet_est_set_internal(ev, EST_COMMTIME, 0.0);
    }
#endif  // !HAVE_BATCH

    this->estimate(resp.servers[0].estim, creq.pb, serviceRef);
  }

#if ! HAVE_ALTPREDICT
  if (TRACE_LEVEL >= TRACE_STRUCTURES) {
    displayResponse(stdout, &resp);
  }
#else
  displayResponse(stdout, &resp);
#endif // ! HAVE_ALTPREDICT

/** Commented to cut overhead of un-needed log messages
  if (dietLogComponent != NULL) {
    dietLogComponent->logSedChosen(&creq,&resp);
  }
*/
  stat_out("SeD",statMsg);

  parent->getResponse(resp);
}

CORBA::Long
SeDImpl::checkContract(corba_estimation_t& estimation,
                       const corba_pb_desc_t& pb)
{  
  ServiceTable::ServiceReference_t ref(-1);
  ref = SrvT->lookupService(&(pb));
  if (ref == -1)
    return 1;
  else
    this->estimate(estimation, pb, ref);
  return 0;
}


void persistent_data_release(corba_data_t* arg){

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

CORBA::Long
SeDImpl::solve(const char* path, corba_profile_t& pb, CORBA::Long reqID)
{
  ServiceTable::ServiceReference_t ref(-1);
  diet_profile_t profile;
  diet_convertor_t* cvt(NULL);
  int solve_res(0);
  char statMsg[128];
  int i;//, arg_idx;

  /* Record the SedImpl address */
  profile.SeDPtr = (const void*) this ;

  ref = SrvT->lookupService(path, &pb);
  if (ref == -1) {
   ERROR("SeD::" << __FUNCTION__ << ": service not found", 1);
  } 

  /* Record time at which solve started (when not using queues) 
   * and time at which job was enqueued (when using queues). */
  gettimeofday(&(this->lastSolveStart), NULL);

#if HAVE_BATCH
  if( pb.batch_flag == 1 ) {
    return this->solve_batch(path, pb, reqID, ref, profile) ;
  }
#endif

  if (this->useConcJobLimit){
    this->accessController->waitForResource();
  }

  sprintf(statMsg, "solve %ld", (unsigned long) reqID);
  stat_in("SeD",statMsg);

  if (dietLogComponent != NULL) {
    dietLogComponent->logBeginSolve(path, &pb,reqID);
  }

  TRACE_TEXT(TRACE_MAIN_STEPS, "SeD::solve invoked on pb: " << path << endl);

  cvt = SrvT->getConvertor(ref);

  /* Data transfer */
  downloadSyncSeDData(profile,pb,cvt) ;
  
  TRACE_TEXT(TRACE_MAIN_STEPS, "Calling getSolver\n");
  solve_res = (*(SrvT->getSolver(ref)))(&profile);    // SOLVE

  /* Data transfer */
  uploadSyncSeDData(profile,pb,cvt) ;
   
  if (TRACE_LEVEL >= TRACE_MAIN_STEPS)
    cout << "SeD::solve complete\n"
         << "************************************************************\n";

  for (i = 0; i <= cvt->last_in; i++) {
    diet_free_data(&(profile.parameters[i]));
  }
  delete [] profile.parameters; // allocated by unmrsh_in_args_to_profile
 
  stat_out("SeD",statMsg);
  stat_flush();

  if (dietLogComponent != NULL) {
    dietLogComponent->logEndSolve(path, &pb,reqID);
  }

  if (this->useConcJobLimit){
    this->accessController->releaseResource();
  }

  return solve_res;
}

#if HAVE_BATCH
char*
SeDImpl::getLocalHostName()
{
  return strdup(this->localHostName) ;
}

ELBASE_SchedulerServiceTypes
SeDImpl::getBatchSchedulerID()
{
  return this->batchID ;
}

CORBA::Long
SeDImpl::solve_batch(const char* path, corba_profile_t& pb,
		     CORBA::Long reqID, ServiceTable::ServiceReference_t& ref,
		     diet_profile_t& profile)
{
  /*************************************************************
   **                  submit a batch job                     **
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
  int i, status;

  /* Is there a sens to use Queue with Batch? */
  if (this->useConcJobLimit){
    this->accessController->waitForResource();
  }

  sprintf(statMsg, "solve %ld", (unsigned long) reqID);
  stat_in("SeD",statMsg);

  if (dietLogComponent != NULL) {
    dietLogComponent->logBeginSolve(path, &pb, reqID);
  }

  TRACE_TEXT(TRACE_MAIN_STEPS, "SeD::batch_solve invoked on pb: " 
	     << path << endl);
  
  cvt = SrvT->getConvertor(ref);

  /* Data transfer */
  downloadSyncSeDData(profile,pb,cvt) ;
    
  TRACE_TEXT(TRACE_MAIN_STEPS, "Calling getSolver\n");
  solve_res = (*(SrvT->getSolver(ref)))(&profile);    // SOLVE

  // TODO: We still have to wait for job completion and complete data
  // production. How implement something that could transfer data by
  // a given amount once computed several times?

  /* We implement the batch job termination watching by looking if the
  ** job that have effectively launched the batch script is still alive

  Only for async call!
  if( ELBASE_Poll((ELBASE_Process)findBatchID(profile.dietJobID), 1, &status) 
      && status == 0 )
    ERROR("An error occured during the execution of the batch job", 1);
  */

  // TODO: look if still ok for // jobs (normally yes)

  /* Data transfer */
  uploadSyncSeDData(profile,pb,cvt) ;
 
  if (TRACE_LEVEL >= TRACE_MAIN_STEPS)
    cout << "SeD::solve_batch complete\n"
         << "************************************************************\n";

  for (i = 0; i <= cvt->last_in; i++) {
    diet_free_data(&(profile.parameters[i]));
  }
  delete [] profile.parameters; // allocated by unmrsh_in_args_to_profile
 
  stat_out("SeD",statMsg);
  stat_flush();

  if (dietLogComponent != NULL) {
    dietLogComponent->logEndSolve(path, &pb,reqID);
  }

  if (this->useConcJobLimit){
    this->accessController->releaseResource();
  }

  return solve_res;
}
#endif //HAVE_BATCH

void
SeDImpl::solveAsync(const char* path, const corba_profile_t& pb, 
                    CORBA::Long reqID, const char* volatileclientREF)
{
#if HAVE_BATCH
  if( pb.batch_flag == 1 )
    ERROR("Asynchronous batch resolution not yet implemented",) ;
#endif

  // test validity of volatileclientREF
  // If nil, it is not necessary to solve ...
  try {
    //ServiceTable::ServiceReference_t ref(-1);
    CORBA::Object_var cb = ORBMgr::stringToObject(volatileclientREF);
    if (CORBA::is_nil(cb)) {
      ERROR("SeD::" << __FUNCTION__ << ": received a nil callback",);
    } else {
      ServiceTable::ServiceReference_t ref(-1);
      diet_profile_t profile;
      diet_convertor_t* cvt(NULL);
      int solve_res(0);
      char statMsg[128];

      ref = SrvT->lookupService(path, &pb);
      if (ref == -1) {
        ERROR("SeD::" << __FUNCTION__ << ": service not found",);
      }

      /* Record time at which solve started (when not using queues) 
       * and time at which job was enqueued (when using queues). */
      gettimeofday(&(this->lastSolveStart), NULL);
      if (this->useConcJobLimit){
        this->accessController->waitForResource();
      }

      sprintf(statMsg, "solveAsync %ld", (unsigned long) reqID);
      stat_in("SeD",statMsg);

      if (dietLogComponent != NULL) {
        dietLogComponent->logBeginSolve(path, &pb,reqID);
      }

      TRACE_TEXT(TRACE_MAIN_STEPS,
                 "SeD::solveAsync invoked on pb: " << path 
                 << " (reqID " << reqID << ")" << endl);

 
      cvt = SrvT->getConvertor(ref);

#if ! HAVE_JUXMEM
      int i;
 
      for (i = 0; i <= pb.last_inout; i++) {    
        if(pb.parameters[i].value.length() == 0){
          this->dataMgr->getData(const_cast<corba_data_t&>(pb.parameters[i]));
        } else {
          if( diet_is_persistent(pb.parameters[i]) ) {
            this->dataMgr->addData(const_cast<corba_data_t&>(pb.parameters[i]),
                                   0);
          }
        }
      }
      unmrsh_in_args_to_profile(&profile, &(const_cast<corba_profile_t&>(pb)), cvt);
      //      displayProfile(&profile, path);
      
#endif // ! HAVE_JUXMEM
      
#if HAVE_BATCH
      // Still to be done...
      // nbprocs and walltime are set
      if( profile.batch_flag != 1 ) {
	if( profile.nbprocs == 1 )
	  solve_res = (*(SrvT->getSolver(ref)))(&profile);    // SOLVE
	else
	  solve_res = (*(SrvT->getSolver(ref)))(&profile);    // SOLVE_//
      } else {
	solve_res = (*(SrvT->getSolver(ref)))(&profile) ;    // SOLVE_Batch
      }
#else
      solve_res = (*(SrvT->getSolver(ref)))(&profile);    // SOLVE
#endif // HAVE_BATCH
      
#if ! HAVE_JUXMEM

      for(i=0;i<=pb.last_in;i++){
        if(diet_is_persistent(pb.parameters[i])) {
          if (pb.parameters[i].desc.specific._d() != DIET_FILE) {
            CORBA::Char *p1 (NULL);
            const_cast<SeqChar&>(pb.parameters[i].value).replace(0,0,p1,1);
          }
          persistent_data_release(
              const_cast<corba_data_t*>(&(pb.parameters[i])));
        }
      }
  
      mrsh_profile_to_out_args(&(const_cast<corba_profile_t&>(pb)), &profile, cvt);
      /*      for (i = profile.last_in + 1 ; i <= profile.last_inout; i++) {
        if ( diet_is_persistent(profile.parameters[i])) {
          this->dataMgr->updateDataList(const_cast<corba_data_t&>(pb.parameters[i])); 
        }
        }*/
      
      for (i = pb.last_inout + 1 ; i <= pb.last_out; i++) {
        if ( diet_is_persistent(pb.parameters[i])) {
          this->dataMgr->addData(const_cast<corba_data_t&>(pb.parameters[i]),
                                 1); 
        }
      }
      
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
      // FIXME: persistent data should not be freed but referenced in the data list.
      
#endif // ! HAVE_JUXMEM

      TRACE_TEXT(TRACE_MAIN_STEPS, "SeD::" << __FUNCTION__ << " complete\n"
                 << "**************************************************\n");

      stat_out("SeD",statMsg);
      stat_flush();

      if (dietLogComponent != NULL) {
        dietLogComponent->logEndSolve(path, &pb,reqID);
      }

      /* Release resource before returning the data.  Caution: this could be a
       * problem for applications with lots of data. */
      if (this->useConcJobLimit){
        this->accessController->releaseResource();
      }
     
      // send result data to client.
      TRACE_TEXT(TRACE_ALL_STEPS, "SeD::" << __FUNCTION__
                 << ": performing the call-back.\n");
      Callback_var cb_var = Callback::_narrow(cb);
      cb_var->notifyResults(path, pb, reqID);
      cb_var->solveResults(path, pb, reqID, solve_res);
      /* FIXME: do we need to use diet_free_data on profile parameters as
       * we do in the solve(...) method? */
      delete [] profile.parameters; // allocated by unmrsh_in_args_to_profile
    }
  
  } catch (const CORBA::Exception &e) {
    // Process any other User exceptions. Use the .id() method to
    // record or display useful information
    CORBA::Any tmp;
    tmp <<= e;
    CORBA::TypeCode_var tc = tmp.type();
    const char * p = tc->name();
    if (*p != '\0') {
      ERROR("exception caught in SeD::" << __FUNCTION__ << '(' << p << ')',);
    } else {
      ERROR("exception caught in SeD::" << __FUNCTION__
            << '(' << tc->id() << ')',);
    }
  } catch (...) {
    // Process any other exceptions. This would catch any other C++
    // exceptions and should probably never occur
    ERROR("unknown exception caught",);
  }
}

const struct timeval*
SeDImpl::timeSinceLastSolve()
{
  return (&(this->lastSolveStart));
}

CORBA::Long
SeDImpl::ping()
{
  SED_TRACE_FUNCTION("");
  return 0;
}

/****************************************************************************/
/* Private methods                                                          */
/****************************************************************************/

/**
 * Estimate a request, with FAST if available. 
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
    diet_est_set_internal(eVals, EST_TCOMP, HUGE_VAL);
    diet_est_set_internal(eVals, EST_FREECPU, 0);
    diet_est_set_internal(eVals, EST_FREEMEM, 0);
    diet_est_set_internal(eVals, EST_NBCPU, 1);
#else //HAVE_CORI
   /***** START FAST-based metrics *****/
   diet_estimate_fast(eVals, &profile);
#endif  //!HAVE_CORI


#if ! HAVE_ALTPREDICT
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
#endif // HAVE_ALTPREDICT
    /***** END CoRI-based metrics *****/

    /***** START RR metrics *****/
    diet_estimate_lastexec(eVals, &profile);
    /***** END RR metrics *****/
  }
  else {
    /*
    ** just call the custom performance metric function!
    */
    (*perfmetric_fn)(&profile, eVals);
  }

#if ! HAVE_ALTPREDICT
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
#endif

//   cout << "AS: [" << __FUNCTION__ << "] num values = " << estimation.estValues.length() << endl;
}

inline void
SeDImpl::downloadSyncSeDData(diet_profile_t& profile, corba_profile_t& pb,
			     diet_convertor_t* cvt)
{
  int i ;

  TRACE_TEXT(TRACE_MAIN_STEPS, "SeD downloads client datas\n");
  
#if HAVE_JUXMEM
  unmrsh_in_args_to_profile(&profile, &pb, cvt);

  for (i = 0; i <= profile.last_out; i++) {
    if (profile.parameters[i].desc.mode == DIET_PERSISTENT ||
	profile.parameters[i].desc.mode == DIET_PERSISTENT_RETURN) {
   
      /* IN case -> acquire the data in read mode */
      if (i <= profile.last_in) {
	assert(profile.parameters[i].desc.id != NULL);
	profile.parameters[i].value = this->juxmem->mmap(NULL, data_sizeof(&(profile.parameters[i].desc)), profile.parameters[i].desc.id, 0);
	TRACE_TEXT(TRACE_MAIN_STEPS, "Acquiring IN data with ID = " << profile.parameters[i].desc.id << " from JuxMem ...\n");
	this->juxmem->acquireRead(profile.parameters[i].value);
	continue;
      }
      /* INOUT case -> acquire the data in write mode */
      if (i > profile.last_in && i <= profile.last_inout) {
	assert(profile.parameters[i].desc.id != NULL);
	profile.parameters[i].value = this->juxmem->mmap(NULL, data_sizeof(&(profile.parameters[i].desc)), profile.parameters[i].desc.id, 0);
	TRACE_TEXT(TRACE_MAIN_STEPS, "Acquiring INOUT data with ID = " << profile.parameters[i].desc.id << " from JuxMem ...\n");
	this->juxmem->acquire(profile.parameters[i].value);
	continue;
      }
      /* OUT case -> acquire the data in write mode if exists in JuxMem */
      if (i > profile.last_inout) {
	if (profile.parameters[i].desc.id == NULL || (strlen(profile.parameters[i].desc.id) == 0)) {
	  TRACE_TEXT(TRACE_MAIN_STEPS, "New data for OUT\n");
	} else {
	  /** FIXME: not clear if we should handle such a case */
	  assert(profile.parameters[i].desc.id != NULL);
	  profile.parameters[i].value = this->juxmem->mmap(NULL, data_sizeof(&(profile.parameters[i].desc)), profile.parameters[i].desc.id, 0);
	  this->juxmem->acquire(profile.parameters[i].value);
	  TRACE_TEXT(TRACE_MAIN_STEPS, "Acquiring OUT data with ID = " << profile.parameters[i].desc.id << " from JuxMem ...\n");
	}
      }
    }
  }
#else // DTM case

  // For data persistence

  for (i=0 ; i <= pb.last_inout ; i++) {
    if(pb.parameters[i].value.length() == 0) { 
      /* In argument with NULL value : data is present */
      this->dataMgr->getData(pb.parameters[i]); 
    } else { /* data is not yet present but is persistent */
      if(diet_is_persistent(pb.parameters[i])) {
        this->dataMgr->addData(pb.parameters[i],0);
      }
    }
  }
  unmrsh_in_args_to_profile(&profile, &pb, cvt);
  for (i=0 ; i <= pb.last_inout ; i++) {
    if( diet_is_persistent(pb.parameters[i]) && 
        (pb.parameters[i].desc.specific._d() == DIET_FILE)) {
      char* in_path = 
	CORBA::string_dup(profile.parameters[i].desc.specific.file.path);
      this->dataMgr->changePath(pb.parameters[i], in_path);
    }
  }
#endif // HAVE_JUXMEM 
}

inline void
SeDImpl::uploadSyncSeDData(diet_profile_t& profile, corba_profile_t& pb,
			   diet_convertor_t* cvt)
{
  int i ;

  TRACE_TEXT(TRACE_MAIN_STEPS, "SeD uploads client datas\n");
  
#if HAVE_JUXMEM
  for (i = 0; i <= profile.last_out; i++) {
    if (profile.parameters[i].desc.mode == DIET_PERSISTENT ||
	profile.parameters[i].desc.mode == DIET_PERSISTENT_RETURN) {

      /** IN and INOUT case */
      if (i <= profile.last_inout) {
	TRACE_TEXT(TRACE_MAIN_STEPS, "Releasing data with ID = " 
		   << profile.parameters[i].desc.id << " from JuxMem ...\n");
	this->juxmem->release(profile.parameters[i].value);
      } else {       /** OUT case */
	/** The data does not exist yet */
	if (strlen(profile.parameters[i].desc.id) == 0) {
	  /* The local memory is attached inside JuxMem */
	  profile.parameters[i].desc.id = 
	    this->juxmem->attach(profile.parameters[i].value, 
				 data_sizeof(&(profile.parameters[i].desc)), 
				 1, 1, EC_PROTOCOL, BASIC_SOG);
	  TRACE_TEXT(TRACE_MAIN_STEPS, "A data space with ID = " 
		     << profile.parameters[i].desc.id 
		     << " for OUT data has been attached inside JuxMem!\n");
	  /* The local memory is flush inside JuxMem */
	  this->juxmem->msync(profile.parameters[i].value);
	} else { /* Simply release the lock */
	  /** FIXME: should we handle this case */
	  this->juxmem->release(profile.parameters[i].value);
	}
      }

      this->juxmem->unmap(profile.parameters[i].value);
      profile.parameters[i].value = NULL;
    }
  }
  mrsh_profile_to_out_args(&pb, &profile, cvt);
#else // DTM case
  for(i=0;i<=pb.last_in;i++){
    if(diet_is_persistent(pb.parameters[i])) {
      if (pb.parameters[i].desc.specific._d() != DIET_FILE) {
        CORBA::Char *p1 (NULL);
        pb.parameters[i].value.replace(0,0,p1,1);
      }
      persistent_data_release(&(pb.parameters[i]));
    }
  }
  mrsh_profile_to_out_args(&pb, &profile, cvt);
  
  for (i = pb.last_inout + 1 ; i <= pb.last_out; i++) {
    if ( diet_is_persistent(pb.parameters[i])) {
      this->dataMgr->addData(pb.parameters[i],1); 
    }
  }
  this->dataMgr->printList();
#endif // HAVE_JUXMEM 
}


#ifdef HAVE_BATCH
  /**
   * Store the batch job ID of the parallel task just submitted in
   * correspondance with the DIET request ID
   */
void
SeDImpl::storeBatchID(int batch_jobID, int diet_reqID)
{
  /* For the moment, storage done in a table
  ** TODO: use something like a red/black tree? */
  int i=0 ;
  
  while( (i<tabCorresIDIndex) && (tabCorresID[i].dietReqID != -1) )
    i++ ;
  if( tabCorresIDIndex == MAX_RUNNING_NBSERVICES ) {
    INTERNAL_ERROR("not enough place to insert new batch job", 1);
  } else if( i == tabCorresIDIndex )
    tabCorresIDIndex++ ;
  tabCorresID[i].batchJobID = batch_jobID ;
  tabCorresID[i].dietReqID = diet_reqID ;
}

/** 
 * Return the pid of the script that launched the batch job
 */
int
SeDImpl::findBatchID(int diet_reqID)
{
  int i=0 ;

  while( (i<tabCorresIDIndex) && (tabCorresID[i].dietReqID != diet_reqID) )
    i++ ;
  if( i == tabCorresIDIndex )
    INTERNAL_ERROR("incoherence relating with batch job ID and diet request ID"
		   , 1);
  return tabCorresID[i].batchJobID ;
}
#endif


