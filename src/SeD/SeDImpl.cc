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
 *
 * Revision 1.56  2005/08/30 09:20:20  ycaniou
 * Corrected things in DIET_server.cc (diet_submit_batch...)
 * Link libDIET with Elagi and Appleseeds only if BATCH is asked in the
 *   configuration (corrected the Makefile)
 * Changed things in SeDImpl.[ch] for batch submission to work. Only synchronous
 *   mode made.
 *
 * Revision 1.55  2005/08/04 10:44:40  bdelfabr
 * fixing persistentce bug for out data in solve async
 *
 * Revision 1.54  2005/06/14 16:17:12  mjan
 * Added support of DIET_FILE inside JuxMem-DIET for TLSE code
 *
 * Revision 1.47  2005/05/15 15:38:59  alsu
 * implementing aggregation interface
 *
 * Revision 1.46  2005/05/02 16:46:33  ycaniou
 * Added the function diet_submit_batch(), the stuff in the makefile to compile
 *  with appleseeds..
 *
 * Revision 1.45  2005/04/27 01:41:34  ycaniou
 * Added the stuff for a correct compilation, for a correct registration of
 * a batch profile, and for its execution.
 * Added the solve_batch() function
 *
 * Revision 1.44  2005/04/13 08:46:29  hdail
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
 * Revision 1.43  2005/04/08 13:02:43  hdail
 * The code for LogCentral has proven itself stable and it seems bug free.
 * Since no external libraries are required to compile in LogCentral, its now
 * going to be compiled in by default always ... its usage is easily controlled by
 * configuration file.
 *
 * Revision 1.42  2005/03/30 07:41:09  rbolze
 * No more ASK_FOR_SED and SED_CHOSEN message in SeDs
 *
 * Revision 1.41  2004/12/15 18:09:58  alsu
 * cleaner, easier to document interface: changing diet_perfmetric_t back
 * to the simpler one-argument (of type diet_profile_t) version, and
 * hiding a copy of the pointer back to the SeD in the profile.
 *
 * Revision 1.40  2004/12/08 15:02:51  alsu
 * plugin scheduler first-pass validation testing complete.  merging into
 * main CVS trunk; ready for more rigorous testing.
 *
 * Revision 1.39  2004/12/03 09:53:51  bdelfabr
 * cleanup data when persistent
 *
 * Revision 1.38  2004/12/02 09:33:09  bdelfabr
 * cleanup memory management for in persistent data.
 *
 * Revision 1.37  2004/11/25 21:56:37  hdail
 * Among other fixes, use diet_free_data on the IN parameters of the DIET_profile.
 * This seems to resolve the major part of the memory leak problem we have.
 *
 * Revision 1.36  2004/11/25 11:40:32  hdail
 * Add request ID to statistics output to allow tracing of stats for each request.
 *
 * Revision 1.35.2.9  2004/11/30 15:46:50  alsu
 * minor cleanup, adding a note to reconsider an unecessary block of code
 * in the case that contract checking is reworked, as planned.
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

#include "acDIET_config.h"
#include "SeDImpl.hh"
#include "Callback.hh"
#include "common_types.hh"
#include "debug.hh"
#include "est_internal.hh"

#if HAVE_CORI
#include "CORIMgr.hh"
#else //HAVE_CORI
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
  this->SrvT    = NULL;
  this->childID = -1;
  this->parent  = Agent::_nil();
  this->localHostName[0] = '\0';
#if HAVE_ALTPREDICT
  this->locationID[0] = '\0';
#endif
  (this->lastSolveStart).tv_sec = -1;
  (this->lastSolveStart).tv_usec = -1;

#if HAVE_FAST
  this->fastUse = 1;
#endif // HAVE_FAST

  this->dietLogComponent = NULL;
#if HAVE_BATCH
  this->tabCorresIDIndex = 0 ;
  for( int i=0 ; i<MAX_RUNNING_NBSERVICES ; i++ )
    tabCorresID[i].dietReqID = -1 ;
#endif
}

#if HAVE_JXTA
SeDImpl::SeDImpl(const char* uuid = '\0')
{
  this->SrvT    = NULL;
  this->childID = -1;
  this->parent  = Agent::_nil();
  this->localHostName[0] = '\0';
#if HAVE_ALTPREDICT
  this->locationID[0] = '\0';
#endif
  this->uuid = uuid;
  (this->lastSolveStart).tv_sec = -1;
  (this->lastSolveStart).tv_usec = -1;

#if HAVE_FAST
  this->fastUse = 1;
#endif // HAVE_FAST

#if HAVE_CORI
 /**initialize CORI evaluation*/
  CORIMgr::init(); 
 #endif //HAVE_CORI

  this->dietLogComponent = NULL;

#if HAVE_BATCH
  this->tabCorresIDIndex = 0 ;
  for( int i=0 ; i<MAX_RUNNING_NBSERVICES ; i++ )
    tabCorresID[i].dietReqID = -1 ;
#endif //HAVE_BATCH
}
#endif //HAVE_JXTA

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
  // A SeD can only launch batch or non batch jobs.
  // Then, all profiles must be either batch or not batch. No mix allowed. 
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
  if (parent_name == NULL)
    return 1;
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

  if (TRACE_LEVEL >= TRACE_STRUCTURES)
    SrvT->dump(stdout);
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
  if (endPoint == NULL)
    this->port = 0;
  else
    this->port = *endPoint;

#if HAVE_ALTPREDICT
  char * tmpName = (char*)
      Parsers::Results::getParamValue(Parsers::Results::LOCATIONID);
  if (tmpName != NULL) {
    strcpy(this->locationID, tmpName);
  } else {
    strcpy(this->locationID, "");
  } 
#endif // HAVE_ALTPREDICT

#if HAVE_QUEUES
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
    this->maxConcJobs = 1;
  } else {
    this->maxConcJobs = *tmpIntPtr;
  }

  if (this->useConcJobLimit){
    this->accessController = new AccessController(this->maxConcJobs);
    TRACE_TEXT(TRACE_MAIN_STEPS, "Concurrent jobs restricted to " 
          << this->maxConcJobs << "\n");
  } else {
    this->accessController = NULL;
  }
#endif // HAVE_QUEUES
 
  

#if HAVE_CORI
#if HAVE_FAST
  this->fastUse=*((size_t*)Parsers::Results::getParamValue(Parsers::Results::FASTUSE));
  if (this->fastUse)
    CORIMgr::add(EST_COLL_FAST,NULL);
#endif //HAVE_FAST
  CORIMgr::add(EST_COLL_EASY,NULL);
#else
   // Init FAST (HAVE_FAST is managed by the FASTMgr class)
  return FASTMgr::init();
#endif //HAVE_CORI  

  return 0;
}

#if ! HAVE_JUXMEM
/** Set this->dataMgr */
int
SeDImpl::linkToDataMgr(DataMgrImpl* dataMgr)
{
  this->dataMgr = dataMgr;
  return 0;
}
#endif // ! HAVE_JUXMEM

#if HAVE_JUXMEM
/** Set this->JuxMem */
int
SeDImpl::linkToJuxMem(JuxMem::Wrapper* juxmem)
{
  this->juxmem = juxmem;
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
    // FIXME: What do I have to do for a batch, non batch, parallel, etc.?
    // for the moment, do the same but..
    estVector_t ev = &(resp.servers[0].estim);
    for (int ctIter = 0 ; ctIter < creq.pb.last_out ; ctIter++) {
      diet_est_set_internal(ev, EST_COMMTIME, 0.0);
    }
#endif  // HAVE_BATCH

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

    arg->desc.specific.vect(vect);
    arg->desc.specific.vect().size = 0;
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

#if HAVE_BATCH
  if( pb.batch_flag == 1 ) {
    return this->solve_batch(path, pb, reqID) ;
  }
#endif

  ref = SrvT->lookupService(path, &pb);
  if (ref == -1) {
   ERROR("SeD::" << __FUNCTION__ << ": service not found", 1);
  } 

#if HAVE_QUEUES
  if (this->useConcJobLimit){
    this->accessController->waitForResource();
  }
#endif // HAVE_QUEUES

  sprintf(statMsg, "solve %ld", (unsigned long) reqID);
  stat_in("SeD",statMsg);

  if (dietLogComponent != NULL) {
    dietLogComponent->logBeginSolve(path, &pb,reqID);
  }

  TRACE_TEXT(TRACE_MAIN_STEPS, "SeD::solve invoked on pb: " << path << endl);

  cvt = SrvT->getConvertor(ref);

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
	this->juxmem->acquireRead(profile.parameters[i].value, profile.parameters[i].desc.id);
	continue;
      }
      /* INOUT case -> acquire the data in write mode */
      if (i > profile.last_in && i <= profile.last_inout) {
	assert(profile.parameters[i].desc.id != NULL);
	profile.parameters[i].value = this->juxmem->mmap(NULL, data_sizeof(&(profile.parameters[i].desc)), profile.parameters[i].desc.id, 0);
	TRACE_TEXT(TRACE_MAIN_STEPS, "Acquiring INOUT data with ID = " << profile.parameters[i].desc.id << " from JuxMem ...\n");
	this->juxmem->acquire(profile.parameters[i].value, profile.parameters[i].desc.id);
	continue;
      }
      /* OUT case -> acquire the data in write mode if exists in JuxMem */
      if (i > profile.last_inout) {
	if (profile.parameters[i].desc.id == NULL || (strlen(profile.parameters[i].desc.id) == 0)) {
	  TRACE_TEXT(TRACE_MAIN_STEPS, "New data for OUT\n");
	} else {
	  assert(profile.parameters[i].desc.id != NULL);
	  profile.parameters[i].value = this->juxmem->mmap(NULL, data_sizeof(&(profile.parameters[i].desc)), profile.parameters[i].desc.id, 0);
	  this->juxmem->acquire(profile.parameters[i].value, profile.parameters[i].desc.id);
	  TRACE_TEXT(TRACE_MAIN_STEPS, "Acquiring OUT data with ID = " << profile.parameters[i].desc.id << " from JuxMem ...\n");
	}
      }
    }
  }
#else // DTM case
  // added for data persistence 
  for (i=0; i <= pb.last_inout; i++) {
 
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
  for (i=0; i <= pb.last_inout; i++) {
    if( diet_is_persistent(pb.parameters[i]) && 
        (pb.parameters[i].desc.specific._d() == DIET_FILE)) {
      char* in_path   = CORBA::string_dup(profile.parameters[i].desc.specific.file.path);
      this->dataMgr->changePath(pb.parameters[i], in_path);
    }
  }
#endif // HAVE_JUXMEM 
  
  /* record the timestamp of this solve */
  gettimeofday(&(this->lastSolveStart), NULL);
  TRACE_TEXT(TRACE_MAIN_STEPS, "Calling getSolver\n");
  solve_res = (*(SrvT->getSolver(ref)))(&profile);    // SOLVE

#if HAVE_JUXMEM
  for (i = 0; i <= profile.last_out; i++) {
    if (profile.parameters[i].desc.mode == DIET_PERSISTENT ||
	profile.parameters[i].desc.mode == DIET_PERSISTENT_RETURN) {

      /** IN and INOUT case */
      if (i <= profile.last_inout) {
	TRACE_TEXT(TRACE_MAIN_STEPS, "Releasing data with ID = " << profile.parameters[i].desc.id << " from JuxMem ...\n");
	this->juxmem->release(profile.parameters[i].value, profile.parameters[i].desc.id);
      } else {       /** OUT case */
	/** The data does not exist yet */
	if (strlen(profile.parameters[i].desc.id) == 0) {
	  /* The local memory is attached inside JuxMem */
	  profile.parameters[i].desc.id = this->juxmem->attach(profile.parameters[i].value, 
							       data_sizeof(&(profile.parameters[i].desc)), 
							       1, 1, EC_PROTOCOL, BASIC_SOG);
	  TRACE_TEXT(TRACE_MAIN_STEPS, "A data space with ID = " << profile.parameters[i].desc.id << " for OUT data has been attached inside JuxMem!\n");
	  /* The local memory is flush inside JuxMem */
	  this->juxmem->msync(profile.parameters[i].value, profile.parameters[i].desc.id);
	} else { /* Simply release the lock */
	  this->juxmem->release(profile.parameters[i].value, profile.parameters[i].desc.id);
	}
      }

      this->juxmem->unmap(profile.parameters[i].value, profile.parameters[i].desc.id);
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

#if HAVE_QUEUES
  if (this->useConcJobLimit){
    this->accessController->releaseResource();
  }
#endif // HAVE_QUEUES

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
SeDImpl::solve_batch(const char* path, corba_profile_t& pb, CORBA::Long reqID)
{
  /*************************************************************
   **                  submit a batch job                     **
   **
   ** For the moment, 
   ** datas are received before batch submission. Maybe this has
   ** to be done after, during the wait in the batch queue
   ** if we want to be the most efficient?

   ** FIXME: If a data is not a file, convert it as a file.
   **  Must I do it here or give the functions to let the Sed programmer
   **  do it in the profile?
   *************************************************************/

  ServiceTable::ServiceReference_t ref(-1);
  diet_profile_t profile;
  diet_convertor_t* cvt(NULL);
  int solve_res(0);
  char statMsg[128];
  int i;//, arg_idx;

  ref = SrvT->lookupService(path, &pb);
  if (ref == -1) {
    ERROR("SeD::" << __FUNCTION__ << ": batch service not found", 1);
  } 

#if HAVE_QUEUES
  if (this->useConcJobLimit){
    this->accessController->waitForResource();
  }
#endif // HAVE_QUEUES

  sprintf(statMsg, "solve %ld", (unsigned long) reqID);
  stat_in("SeD",statMsg);

  if (dietLogComponent != NULL) {
    dietLogComponent->logBeginSolve(path, &pb,reqID);
  }

  TRACE_TEXT(TRACE_MAIN_STEPS, "SeD::batch_solve invoked on pb: " 
	     << path << endl);
  
  cvt = SrvT->getConvertor(ref);

#if HAVE_JUXMEM
  unmrsh_in_args_to_profile(&profile, &pb, cvt);

  for (i= 0; i <= profile.last_inout; i++) {
    if (profile.parameters[i].desc.mode == DIET_PERSISTENT ||
	profile.parameters[i].desc.mode == DIET_PERSISTENT_RETURN) {
      
      /* IN case -> acquire the data in read mode */
      if (i <= profile.last_in) {
	assert(profile.parameters[i].desc.id != NULL);
	profile.parameters[i].value = this->juxmem->mmap(NULL, data_sizeof(&(profile.parameters[i].desc)), profile.parameters[i].desc.id, 0);
	TRACE_TEXT(TRACE_MAIN_STEPS, "Acquiring IN data with ID = " << profile.parameters[i].desc.id << " from JuxMem ...\n");
	this->juxmem->acquireRead(profile.parameters[i].value, profile.parameters[i].desc.id);
	continue;
      }
      /* INOUT case -> acquire the data in write mode */
      if (i > profile.last_in && i <= profile.last_inout) {
	assert(profile.parameters[i].desc.id != NULL);
	profile.parameters[i].value = this->juxmem->mmap(NULL, data_sizeof(&(profile.parameters[i].desc)), profile.parameters[i].desc.id, 0);
	TRACE_TEXT(TRACE_MAIN_STEPS, "Acquiring INOUT data with ID = " << profile.parameters[i].desc.id << " from JuxMem ...\n");
	this->juxmem->acquire(profile.parameters[i].value, profile.parameters[i].desc.id);
	continue;
      }
      /* OUT case -> acquire the data in write mode if exists in JuxMem */
      if (i > profile.last_inout) {
	if (profile.parameters[i].desc.id == NULL || (strlen(profile.parameters[i].desc.id) == 0)) {
	  TRACE_TEXT(TRACE_MAIN_STEPS, "New data for OUT\n");
	} else {
	  assert(profile.parameters[i].desc.id != NULL);
	  profile.parameters[i].value = this->juxmem->mmap(NULL, data_sizeof(&(profile.parameters[i].desc)), profile.parameters[i].desc.id, 0);
	  this->juxmem->acquire(profile.parameters[i].value, profile.parameters[i].desc.id);
	  TRACE_TEXT(TRACE_MAIN_STEPS, "Acquiring OUT data with ID = " << profile.parameters[i].desc.id << " from JuxMem ...\n");
	}
      }
    }
  }
#else // DTM case
  // added for data persistence 
  for (i=0; i <= pb.last_inout; i++) {
 
    if(pb.parameters[i].value.length() == 0){ 
      /* In argument with NULL value : data is present */
      this->dataMgr->getData(pb.parameters[i]); 
    } else { /* data is not yet present but is persistent */
      if( diet_is_persistent(pb.parameters[i])) {
        this->dataMgr->addData(pb.parameters[i],0);
      }
    }
  }
 
  unmrsh_in_args_to_profile(&profile, &pb, cvt);
  for (i=0; i <= pb.last_inout; i++) {
    if( diet_is_persistent(pb.parameters[i]) && 
        (pb.parameters[i].desc.specific._d() == DIET_FILE)) {
      char* in_path   = 
	CORBA::string_dup(profile.parameters[i].desc.specific.file.path);
      this->dataMgr->changePath(pb.parameters[i], in_path);
    }
  }
#endif // HAVE_JUXMEM 
  
  /* Record the SedImpl address in order to access batchID and such information
   */
  profile.SeDPtr = (const void*) this ;
  
  /* record the timestamp of this solve */
  gettimeofday(&(this->lastSolveStart), NULL);
  TRACE_TEXT(TRACE_MAIN_STEPS, "Calling getSolver\n");
  solve_res = (*(SrvT->getSolver(ref)))(&profile);    // SOLVE

  // FIXME: We still have to wait for job completion
  //   and complete data production. How to implement something
  //   that could transfer data by given amount once computed?

  // while( status != finished )
  // sleep( 10 )
  // or better if we can
  printf("Must wait for job completion !") ;
  // NOTE: Ok with batch, elagi seems to make it possible, but for //jobs?

#if HAVE_JUXMEM
  for (i = 0; i <= profile.last_out; i++) {
    if (profile.parameters[i].desc.mode == DIET_PERSISTENT ||
	profile.parameters[i].desc.mode == DIET_PERSISTENT_RETURN) {

      /** IN and INOUT case */
      if (i <= profile.last_inout) {
	TRACE_TEXT(TRACE_MAIN_STEPS, "Releasing data with ID = " << profile.parameters[i].desc.id << " from JuxMem ...\n");
	this->juxmem->release(profile.parameters[i].value, profile.parameters[i].desc.id);
      } else {       /** OUT case */
	/** The data does not exist yet */
	if (strlen(profile.parameters[i].desc.id) == 0) {
	  /* The local memory is attached inside JuxMem */
	  profile.parameters[i].desc.id = this->juxmem->attach(profile.parameters[i].value, 
							       data_sizeof(&(profile.parameters[i].desc)), 
							       1, 1, EC_PROTOCOL, BASIC_SOG);
	  TRACE_TEXT(TRACE_MAIN_STEPS, "A data space with ID = " << profile.parameters[i].desc.id << " for OUT data has been attached inside JuxMem!\n");
	  /* The local memory is flush inside JuxMem */
	  this->juxmem->msync(profile.parameters[i].value, profile.parameters[i].desc.id);
	} else { /* Simply release the lock */
	  this->juxmem->release(profile.parameters[i].value, profile.parameters[i].desc.id);
	}
      }

      this->juxmem->unmap(profile.parameters[i].value, profile.parameters[i].desc.id);
    }
  }
  mrsh_profile_to_out_args(&pb, &profile, cvt);
#else // DTM case
  for(i=0 ; i<=pb.last_in ; i++){
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

#if HAVE_QUEUES
  if (this->useConcJobLimit){
    this->accessController->releaseResource();
  }
#endif // HAVE_QUEUES

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

  // TODO: enable RRSolve & Queue handling here to match solveSync
  if (dietLogComponent != NULL) {
    dietLogComponent->logBeginSolve(path, &pb,reqID);
  }

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

      stat_in("SeD","solveAsync");

      TRACE_TEXT(TRACE_MAIN_STEPS,
                 "SeD::solveAsync invoked on pb: " << path << endl);

      ref = SrvT->lookupService(path, &pb);
      if (ref == -1) {
        ERROR("SeD::" << __FUNCTION__ << ": service not found",);
      }
 
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
      
      /* record the timestamp of this solve */
      gettimeofday(&(this->lastSolveStart), NULL);

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
      persistent_data_release(const_cast<corba_data_t*>(&(pb.parameters[i])));
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

      stat_out("SeD","solveAsync");
      stat_flush();

      if (dietLogComponent != NULL) {
        dietLogComponent->logEndSolve(path, &pb,reqID);
      }
     
      // send result data to client.
      TRACE_TEXT(TRACE_ALL_STEPS, "SeD::" << __FUNCTION__
                 << ": performing the call-back.\n");
      Callback_var cb_var = Callback::_narrow(cb);
      cb_var->notifyResults(path, pb, reqID);
      cb_var->solveResults(path, pb, reqID);

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
    /***** START CoRI-based metrics *****/

#if HAVE_CORI 
#if HAVE_FAST  
    if (this->fastUse){ 
      diet_estimate_cori(eVals,EST_ALLINFOS,EST_COLL_FAST,&profile);
      //fast don't need a tag-> EST_ALLINFOS== dummydiet_in
    }
    else{
#endif // HAVE_FAST

   diet_est_set_internal(eVals,EST_TCOMP, HUGE_VAL); 
   diet_estimate_cori(eVals,EST_FREEMEM,EST_COLL_EASY,NULL); 
   diet_estimate_cori(eVals,EST_NBCPU,EST_COLL_EASY,NULL); 
   diet_estimate_cori(eVals,EST_FREECPU,EST_COLL_EASY,NULL); 
    
#if HAVE_FAST  
  }
#endif // HAVE_FAST
#else //HAVE_CORI
   /***** START FAST-based metrics *****/
   diet_estimate_fast(eVals, &profile);
#endif  //HAVE_CORI


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

