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

#if defined HAVE_ALT_BATCH
#include "BatchCreator.hh"
#endif

/**
 * Performance measurement for bw and latency of JuxMem
 */
#define JUXMEM_LATENCY_THROUGHPUT 0

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

#if !HAVE_CORI && HAVE_FAST
  this->fastUse = 1;
#endif //!HAVE_CORI && HAVE_FAST

#if defined HAVE_BATCH
  batchQueue = NULL ;
  initCorresBatchDietReqID() ;
#endif //HAVE_BATCH
}

SeDImpl::~SeDImpl()
{
#if defined HAVE_BATCH
  free(batchQueue) ;
#endif //HAVE_BATCH
  
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

#if defined HAVE_BATCH
  int only_batch_services = this->SrvT->testIfAllBatchServices() ;
  // A SeD can only launch parallel/batch or -exclusive- sequential jobs.
  // Then, all profiles must be either parallel/batch or sequential.
  // No mix allowed. 
  if( only_batch_services == 1 ) {
    // Read "batchName" if parallel jobs are to be submitted
    char * batchname = (char*) 
      Parsers::Results::getParamValue(Parsers::Results::BATCHNAME) ;
    if (batchname == NULL) {
      ERROR("SeD can not launch parallel/batch jobs, no parallel/batch"
	    " scheduler specified in the config file", 1) ;
    }
    if( !(ELBASE_ExistBatchScheduler(batchname,&this->batchID)) ) {
      ERROR("Parallel/batch scheduler not recognized", 1) ;
    }
    TRACE_TEXT(TRACE_MAIN_STEPS,
    	       "Parallel/batch submission enabled with " 
	       << ELBASE_GiveBatchName(this->batchID) ) ;
    if( this->batchID != ELBASE_SHELL ) {
      /* Search for batch queues */
      this->batchQueue = strdup((char*)
	Parsers::Results::getParamValue(Parsers::Results::BATCHQUEUE)) ;
      if( batchQueue != NULL )
	TRACE_TEXT(TRACE_MAIN_STEPS, " using queue " 
		   << batchQueue ) ;
    }
    TRACE_TEXT(TRACE_MAIN_STEPS, "\n" ) ;
  } else if( only_batch_services == -1 ) {
    ERROR("SeD is not allowed to launch parallel/batch and sequential job"
	  "at the same time",
	  1) ;
  }
#endif
#if defined HAVE_ALT_BATCH
  int only_batch_services = this->SrvT->testIfAllParallelServices() ;
  // A SeD can only launch parallel/batch or -exclusive- sequential jobs.
  // Then, all profiles must be either parallel/batch or sequential.
  // No mix allowed. 
  if( only_batch_services == 1 ) {
    // Read "batchName" if parallel jobs are to be submitted
    char * batchname = (char*) 
      Parsers::Results::getParamValue(Parsers::Results::BATCHNAME) ;
    if (batchname == NULL) {
      ERROR("SeD can not launch parallel/batch jobs, no parallel/batch"
	    " scheduler specified in the config file", 1) ;
    }
    batch = BatchCreator::getBatchSystem(batchname) ;
    if( batch == NULL ) {
      ERROR("Parallel/batch scheduler not recognized", 1) ;
    }
    TRACE_TEXT(TRACE_MAIN_STEPS,
    	       "Parallel/batch submission enabled with " 
	       << batch->getBatchName()) ;
    if( batch->getBatchQueueName() != NULL )
      TRACE_TEXT(TRACE_MAIN_STEPS, " using queue " 
		 << batch->getBatchQueueName() ) ;
    TRACE_TEXT(TRACE_MAIN_STEPS, "\n" ) ;
    TRACE_TEXT(TRACE_MAIN_STEPS,"pathToNFS: " << batch->getNFSPath() << "\n") ;
    TRACE_TEXT(TRACE_MAIN_STEPS,"pathToTmp: " << batch->getTmpPath() << "\n") ;
  } else if( only_batch_services == -1 ) {
    ERROR("SeD is not allowed to launch parallel/batch and sequential job"
	  "at the same time",
	  1) ;
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


#if not defined HAVE_BATCH && not defined HAVE_ALT_BATCH
    estVector_t ev = &(resp.servers[0].estim);
    for (int ctIter = 0 ; ctIter < creq.pb.last_out ; ctIter++) {
      diet_est_set_internal(ev, EST_COMMTIME, 0.0);
    }
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

  if (TRACE_LEVEL >= TRACE_STRUCTURES) {
    displayResponse(stdout, &resp);
  }

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
/** Called from client immediatly after knowing which server is selected
 ** before data transfer */
void
SeDImpl::updateTimeSinceLastSolve() 
{
  gettimeofday(&(this->lastSolveStart), NULL) ;
}

/* TODO: When HAVE_ALT_BATCH is enabled by default, change the prototype
   because reqID is integrated in the profile */
CORBA::Long
SeDImpl::solve(const char* path, corba_profile_t& pb)
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

#if defined HAVE_BATCH || defined HAVE_ALT_BATCH
  /* Use parallel_flag of the proposed SeD service to know what to do */
  const corba_profile_desc_t & sed_profile = SrvT->getProfile( ref ) ;
  if( sed_profile.parallel_flag == 2 )
    return this->parallel_solve(path, pb, ref, profile) ;
#endif

  if (this->useConcJobLimit){
    this->accessController->waitForResource();
  }

  sprintf(statMsg, "solve %ld", pb.dietReqID);
  stat_in("SeD",statMsg);

  if (dietLogComponent != NULL) {
    dietLogComponent->logBeginSolve(path, &pb);
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
    dietLogComponent->logEndSolve(path, &pb);
  }

  if (this->useConcJobLimit){
    this->accessController->releaseResource();
  }

  return solve_res;
}

#if defined HAVE_BATCH 
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
#endif

#if defined HAVE_BATCH || defined HAVE_ALT_BATCH
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
  int i ;

  /* Is there a sens to use Queue with Batch? */
  if (this->useConcJobLimit){
    this->accessController->waitForResource();
  }

  sprintf(statMsg, "solve %ld", (unsigned long) pb.dietReqID);
  stat_in("SeD",statMsg);

  if (dietLogComponent != NULL) {
    dietLogComponent->logBeginSolve(path, &pb);
  }

  TRACE_TEXT(TRACE_MAIN_STEPS, "SeD::parallel_solve() invoked on pb: "
	     << path << endl);
  
  cvt = SrvT->getConvertor(ref);

  /* Data transfer */
  downloadSyncSeDData(profile,pb,cvt) ;
    
  TRACE_TEXT(TRACE_MAIN_STEPS, "Calling getSolver\n");
  solve_res = (*(SrvT->getSolver(ref)))(&profile);    // SOLVE

#if defined HAVE_ALT_BATCH
  TRACE_TIME(TRACE_MAIN_STEPS, "Submitting DIET job of ID "
	     << profile.dietReqID <<
	     " on batch system with ID " <<
	     batch->getBatchJobID(profile.dietReqID)
	     << "\n") ;
  if( batch->wait4BatchJobCompletion(batch->getBatchJobID(profile.dietReqID)) 
      < 0 ) {
    ERROR("An error occured during the execution of the parallel job", 21) ;
  }
  batch->removeBatchJobID(profile.dietReqID) ;
#else  
  int status ;
  TRACE_TIME(TRACE_MAIN_STEPS, "Submitting script for DIET job of ID "
	     << profile.dietReqID <<
	     " is of pid " << 
	     (long)((ProcessInfo)findBatchID(profile.dietReqID))->pid 
	     << "\n") ;
  /* This waits until the jobs ends
  ** and remove batchID/DIETreqID correspondance */
  if( (ELBASE_Poll(findBatchID(profile.dietReqID), 1, &status) == 0)
      && status == 0 ) {
    if( this->batchID == ELBASE_SHELLSCRIPT ) {
      ERROR("An error occured during the execution of the parallel job", 21) ;
    } else {
      ERROR("An error occured during the execution of the batch job", 21) ;
    }
  }
  removeBatchID(pb.dietReqID) ;
#endif

  /* Data transfer */
  uploadSyncSeDData(profile,pb,cvt) ;
 
  if (TRACE_LEVEL >= TRACE_MAIN_STEPS)
    cout << "SeD::parallel_solve() completed\n"
         << "************************************************************\n";

  for (i = 0; i <= cvt->last_in; i++) {
    diet_free_data(&(profile.parameters[i]));
  }
  delete [] profile.parameters; // allocated by unmrsh_in_args_to_profile
 
  stat_out("SeD",statMsg);
  stat_flush();

  if (dietLogComponent != NULL) {
    dietLogComponent->logEndSolve(path, &pb);
  }

  if (this->useConcJobLimit){
    this->accessController->releaseResource();
  }

  return solve_res;
}
#endif //HAVE_BATCH

/* TODO: When HAVE_ALT_BATCH is enabled by default, change the prototype 
   because reqID is integrated in the profile */
void
SeDImpl::solveAsync(const char* path, const corba_profile_t& pb, 
                     const char* volatileclientREF)
{

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

      /* Record the SedImpl address */
      profile.SeDPtr = (const void*) this ;

      ref = SrvT->lookupService(path, &pb);
      if (ref == -1) {
        ERROR("SeD::" << __FUNCTION__ << ": service not found",);
      }

      /* Record time at which solve started (when not using queues) 
       * and time at which job was enqueued (when using queues). */
      gettimeofday(&(this->lastSolveStart), NULL);
#if defined HAVE_BATCH || defined HAVE_ALT_BATCH
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
	
	sprintf(statMsg, "solveAsync %ld", pb.dietReqID);
	stat_in("SeD",statMsg);
	
	if (dietLogComponent != NULL) {
	  dietLogComponent->logBeginSolve(path, &pb);
	}
	
	TRACE_TEXT(TRACE_MAIN_STEPS,
		   "SeD::solveAsync invoked on pb: " << path 
		   << " (reqID " << profile.dietReqID << ")" << endl);
	
	
	cvt = SrvT->getConvertor(ref);
	
	downloadAsyncSeDData(profile, const_cast<corba_profile_t&>(pb), cvt);
	
	solve_res = (*(SrvT->getSolver(ref)))(&profile);    // SOLVE
	
	uploadAsyncSeDData(profile,  const_cast<corba_profile_t&>(pb), cvt);
	
	TRACE_TEXT(TRACE_MAIN_STEPS, "SeD::" << __FUNCTION__ << " complete\n"
		   << "**************************************************\n");
	
	stat_out("SeD",statMsg);
	stat_flush();
	
	if (dietLogComponent != NULL) {
	  dietLogComponent->logEndSolve(path, &pb);
	}
	
	/* Release resource before returning the data.  Caution: this could be a
	 * problem for applications with lots of data. */
	if (this->useConcJobLimit){
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
	delete [] profile.parameters; // allocated by unmrsh_in_args_to_profile
#if defined HAVE_BATCH || defined HAVE_ALT_BATCH
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

/* Note: ref is useful for convertors */
#if defined HAVE_BATCH || defined HAVE_ALT_BATCH
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
    
      if (dietLogComponent != NULL) {
	dietLogComponent->logBeginSolve(path, &pb);
      }
    
      TRACE_TEXT(TRACE_MAIN_STEPS,
		 "SeD::solve_AsyncParallel invoked on pb: " << path 
		 << " (reqID " << pb.dietReqID << ")" << endl);
    
    
      cvt = SrvT->getConvertor(ref);
    
      downloadAsyncSeDData(profile, const_cast<corba_profile_t&>(pb), cvt);
    
#if defined HAVE_BATCH || defined HAVE_ALT_BATCH
      solve_res = (*(SrvT->getSolver(ref)))(&profile);
#if defined HAVE_ALT_BATCH
      TRACE_TIME(TRACE_MAIN_STEPS, "Submitting DIET job of ID "
		 << profile.dietReqID <<
		 " on batch system with ID " <<
		 batch->getBatchJobID(profile.dietReqID)
		 << "\n") ;
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
		 << "\n") ;
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
#endif // HAVE_BATCH || ALT_BATCH

      uploadAsyncSeDData(profile,  const_cast<corba_profile_t&>(pb), cvt);

      TRACE_TEXT(TRACE_MAIN_STEPS, "SeD::" << __FUNCTION__ << " complete\n"
		 << "**************************************************\n");

      stat_out("SeD",statMsg);
      stat_flush();
    
      if (dietLogComponent != NULL) {
	dietLogComponent->logEndSolve(path, &pb);
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

#if HAVE_JUXMEM
inline void
SeDImpl::uploadSeDDataJuxMem(diet_profile_t* profile)
{
#if JUXMEM_LATENCY_THROUGHPUT
  float latency = 0;
  float throughput = 0;
  /**
   * To store time
   */
  struct timeval t_begin;
  struct timeval t_end;
  struct timeval t_result;
#endif
  int i = 0;

  for (i = 0; i <= profile->last_out; i++) {
    if (profile->parameters[i].desc.mode == DIET_PERSISTENT ||
	profile->parameters[i].desc.mode == DIET_PERSISTENT_RETURN) {

      /** IN and INOUT case */
      if (i <= profile->last_inout) {
	TRACE_TEXT(TRACE_MAIN_STEPS, "Releasing data with ID = " << profile->parameters[i].desc.id << " from JuxMem\n");
#if JUXMEM_LATENCY_THROUGHPUT
	gettimeofday(&t_begin, NULL);
#endif
	this->juxmem->release(profile->parameters[i].value);
#if JUXMEM_LATENCY_THROUGHPUT
	gettimeofday(&t_end, NULL);
	timersub(&t_end, &t_begin, &t_result);
	latency = (t_result.tv_usec + (t_result.tv_sec * 1000. * 1000)) / 1000.;
	throughput = (data_sizeof(&(profile->parameters[i].desc)) / (1024. * 1024.)) / (latency / 1000.);
	fprintf(stderr, "INOUT %s release. Latency: %f, Throughput: %f\n", profile->parameters[i].desc.id, latency, throughput);
#endif
      } else {       /** OUT case */
	/** The data does not exist yet */
	if (strlen(profile->parameters[i].desc.id) == 0) {
	  /* The local memory is attached inside JuxMem */
	  profile->parameters[i].desc.id = 
	    this->juxmem->attach(profile->parameters[i].value, 
				 data_sizeof(&(profile->parameters[i].desc)), 
				 1, 1, EC_PROTOCOL, BASIC_SOG);
	  TRACE_TEXT(TRACE_MAIN_STEPS, "A data space with ID = " 
		     << profile->parameters[i].desc.id 
		     << " for OUT data has been attached inside JuxMem!\n");
	  /* The local memory is flush inside JuxMem */
	  this->juxmem->msync(profile->parameters[i].value);
	} else { /* Simply release the lock */
	  /** FIXME: should we handle this case */
	  this->juxmem->release(profile->parameters[i].value);
	}
      }

      this->juxmem->unmap(profile->parameters[i].value);
    }
  }
}
#endif

#if HAVE_JUXMEM
inline void
SeDImpl::downloadSeDDataJuxMem(diet_profile_t* profile)
{
#if JUXMEM_LATENCY_THROUGHPUT
  float latency = 0;
  float throughput = 0;
  /**
   * To store time
   */
  struct timeval t_begin;
  struct timeval t_end;
  struct timeval t_result;
#endif
  int i = 0;

  for (i = 0; i <= profile->last_out; i++) {
    if (profile->parameters[i].desc.mode == DIET_PERSISTENT ||
	profile->parameters[i].desc.mode == DIET_PERSISTENT_RETURN) {
   
      /* IN case -> acquire the data in read mode */
      if (i <= profile->last_in) {
	assert(profile->parameters[i].desc.id != NULL);
	profile->parameters[i].value = this->juxmem->mmap(NULL, data_sizeof(&(profile->parameters[i].desc)), profile->parameters[i].desc.id, 0);
	TRACE_TEXT(TRACE_MAIN_STEPS, "Acquiring IN data with ID = " << profile->parameters[i].desc.id << " from JuxMem\n");
#if JUXMEM_LATENCY_THROUGHPUT
	gettimeofday(&t_begin, NULL);
#endif
	this->juxmem->acquireRead(profile->parameters[i].value);
#if JUXMEM_LATENCY_THROUGHPUT
	gettimeofday(&t_end, NULL);
	timersub(&t_end, &t_begin, &t_result);
	latency = (t_result.tv_usec + (t_result.tv_sec * 1000. * 1000)) / 1000.;	
	throughput = (data_sizeof(&(profile->parameters[i].desc)) / (1024. * 1024.)) / (latency / 1000.);
	fprintf(stderr, "IN %s acquireRead. Latency: %f, Throughput: %f\n", profile->parameters[i].desc.id, latency, throughput);
#endif
	continue;
      }
      /* INOUT case -> acquire the data in write mode */
      if (i > profile->last_in && i <= profile->last_inout) {
	assert(profile->parameters[i].desc.id != NULL);
	profile->parameters[i].value = this->juxmem->mmap(NULL, data_sizeof(&(profile->parameters[i].desc)), profile->parameters[i].desc.id, 0);
	TRACE_TEXT(TRACE_MAIN_STEPS, "Acquiring INOUT data with ID = " << profile->parameters[i].desc.id << " from JuxMem ...\n");
#if JUXMEM_LATENCY_THROUGHPUT
	gettimeofday(&t_begin, NULL);
#endif
	this->juxmem->acquire(profile->parameters[i].value);
#if JUXMEM_LATENCY_THROUGHPUT
	gettimeofday(&t_end, NULL);
	timersub(&t_end, &t_begin, &t_result);
	latency = (t_result.tv_usec + (t_result.tv_sec * 1000. * 1000)) / 1000.;
	throughput = (data_sizeof(&(profile->parameters[i].desc)) / (1024. * 1024.)) / (latency / 1000.);
	fprintf(stderr, "IN/INOUT %s acquire. Latency: %f, Throughput: %f\n", profile->parameters[i].desc.id, latency, throughput);
#endif
	continue;
      }
      /* OUT case -> acquire the data in write mode if exists in JuxMem */
      if (i > profile->last_inout) {
	if (profile->parameters[i].desc.id == NULL || (strlen(profile->parameters[i].desc.id) == 0)) {
	  TRACE_TEXT(TRACE_MAIN_STEPS, "New data for OUT\n");
	} else {
	  /** FIXME: not clear if we should handle such a case */
	  assert(profile->parameters[i].desc.id != NULL);
	  profile->parameters[i].value = this->juxmem->mmap(NULL, data_sizeof(&(profile->parameters[i].desc)), profile->parameters[i].desc.id, 0);
	  this->juxmem->acquire(profile->parameters[i].value);
	  TRACE_TEXT(TRACE_MAIN_STEPS, "Acquiring OUT data with ID = " << profile->parameters[i].desc.id << " from JuxMem ...\n");
	}
      }
    }
  }
}
#endif

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
    diet_estimate_lastexec(eVals, &profile);
    /***** END RR metrics *****/
  }
  else {
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

//   cout << "AS: [" << __FUNCTION__ << "] num values = " << estimation.estValues.length() << endl;
}

inline void
SeDImpl::downloadAsyncSeDData(diet_profile_t& profile, corba_profile_t& pb,
			      diet_convertor_t* cvt)
{
  TRACE_TIME(TRACE_MAIN_STEPS, "SeD downloads client datas\n");

#if HAVE_JUXMEM
  unmrsh_in_args_to_profile(&profile, &(const_cast<corba_profile_t&>(pb)), cvt);
  downloadSeDDataJuxMem(&profile);
#else
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
      unmrsh_in_args_to_profile(&profile, &(const_cast<corba_profile_t&>(pb)),
				cvt);
      //      displayProfile(&profile, path);
#endif // ! HAVE_JUXMEM
}

inline void
SeDImpl::downloadSyncSeDData(diet_profile_t& profile, corba_profile_t& pb,
			     diet_convertor_t* cvt)
{
  TRACE_TIME(TRACE_MAIN_STEPS, "SeD downloads client datas\n");
  
#if HAVE_JUXMEM
  unmrsh_in_args_to_profile(&profile, &pb, cvt);
  downloadSeDDataJuxMem(&profile);
#else // DTM case
  int i ;
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
SeDImpl::uploadAsyncSeDData(diet_profile_t& profile, corba_profile_t& pb,
			    diet_convertor_t* cvt)
{
  TRACE_TIME(TRACE_MAIN_STEPS, "SeD uploads client datas\n");

#if HAVE_JUXMEM
      uploadSeDDataJuxMem(&profile);
      mrsh_profile_to_out_args(&(const_cast<corba_profile_t&>(pb)), &profile, cvt);
#else // DTM case
  int i ;
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
#endif // HAVE_JUXMEM
}

inline void
SeDImpl::uploadSyncSeDData(diet_profile_t& profile, corba_profile_t& pb,
			   diet_convertor_t* cvt)
{
  TRACE_TIME(TRACE_MAIN_STEPS, "SeD uploads client datas\n");
  
#if HAVE_JUXMEM
  uploadSeDDataJuxMem(&profile);
  mrsh_profile_to_out_args(&pb, &profile, cvt);
#else // DTM case
  int i ;
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


#if defined HAVE_BATCH
void
SeDImpl::initCorresBatchDietReqID()
{
  this->batchJobQueue = NULL ;
}
/**
 * Store the batch job ID of the parallel task (in fact the pid of the script)
 just submitted in correspondance with the DIET request ID
 */
void
SeDImpl::storeBatchID(ELBASE_Process *batch_jobID, int diet_reqID)
{
  SeDImpl::corresID *tmp ;

  tmp = (corresID*)malloc(sizeof(corresID)) ;
  if( tmp == NULL ) {
    ERROR("Not enough memory to store new batch information\n",);
  }

  corresBatchReqID_mutex.lock() ;

  tmp->nextStruct = this->batchJobQueue ;
  
  this->batchJobQueue = tmp ;
  this->batchJobQueue->batchJobID = batch_jobID ;
  this->batchJobQueue->dietReqID = diet_reqID ;

  corresBatchReqID_mutex.unlock() ;
}

  /* For the moment, storage done in a table
  ** TODO: use something like a red/black tree? */
//   int i=0 ;
  
//   while( (i<tabCorresIDIndex) && (tabCorresID[i].dietReqID != -1) )
//     i++ ;
//   if( tabCorresIDIndex == MAX_RUNNING_NBSERVICES ) {
//     INTERNAL_ERROR("not enough place to insert new batch job", 1);
//   } else if( i == tabCorresIDIndex )
//     tabCorresIDIndex++ ;
//   tabCorresID[i].batchJobID = batch_jobID ;
//   tabCorresID[i].dietReqID = diet_reqID ;
// }
/* This function must be called after a ELBASE_Poll, 
**     which desallocate the ELBASE_Process structure!
** ( TODO: Look what happens when stop/kill ).
** It removes the batchJobID/DietReqID correspondance
*/
void
SeDImpl::removeBatchID(int diet_reqID)
{
  corresID *tmp=batchJobQueue ;
  corresID *tmp2 ;

  corresBatchReqID_mutex.lock() ;

  tmp = batchJobQueue ;
  if( tmp != NULL ) {
    if( tmp->dietReqID != diet_reqID ) {
      while( (tmp->nextStruct != NULL) && 
	     (tmp->nextStruct->dietReqID != diet_reqID) )
	tmp = tmp->nextStruct ;
      if( tmp->nextStruct == NULL ) {
	INTERNAL_ERROR("incoherence relating with batch job ID"
		       " and diet request ID"
		       " when removing batch info"
		       , 1);
      }
      // remove the struct
      tmp2=tmp->nextStruct ;
      tmp->nextStruct = tmp2->nextStruct ;
      free(tmp2) ;
    } else { // remove the head
      batchJobQueue=batchJobQueue->nextStruct ;
      free( tmp ) ;
    }
  } else { // tmp == NULL
    INTERNAL_ERROR("incoherence relating with batch job ID and diet request ID"
		   " when removing batch info"
		   , 1);
  }

  corresBatchReqID_mutex.unlock() ;
}
/** 
 * Return the pid of the script that launched the batch job
 */
ELBASE_Process
SeDImpl::findBatchID(int diet_reqID)
{
  corresID *tmp=batchJobQueue ;

  corresBatchReqID_mutex.lock() ;
  
  while( (tmp != NULL) && (tmp->dietReqID != diet_reqID) )
    tmp=tmp->nextStruct ;
  corresBatchReqID_mutex.unlock() ;

  if( tmp == NULL) {
    INTERNAL_ERROR("Incoherence relating with batch job ID and diet request ID"
		   , 1);
  }
  return *(tmp->batchJobID) ;
}
/** 
 * Return the name of the batch queue
 */
char*
SeDImpl::getBatchQueue()
{
  return batchQueue ;
}
#endif

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
// 			const char * command)
// {
//   return batch->diet_submit_parallel(batchJobID, profile,
// 				     command) ;
// }
#endif
