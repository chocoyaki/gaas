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
 * Revision 1.35  2004/10/15 08:19:13  hdail
 * Removed references to corba_response_t->sortedIndexes - no longer useful.
 *
 * Revision 1.34  2004/10/06 15:56:13  bdelfabr
 * bug persistent data fixed (hope so one more time)
 *
 * Revision 1.33  2004/10/06 11:59:04  bdelfabr
 * corrected inout bug (I hope so)
 *
 * Revision 1.32  2004/10/05 08:23:09  bdelfabr
 * fixing bug for persistent file : add a changePath method thta gives the good file access path
 *
 * Revision 1.31  2004/10/04 13:52:32  hdail
 * Added ability to restrict number of concurrent jobs running in the SeD.
 *
 * Revision 1.30  2004/07/29 18:52:11  rbolze
 * Change solve function now , DIET_client send the reqID of the request when
 * he call the solve function.
 * Nothing is change for DIET's API
 *
 * Revision 1.29  2004/07/08 12:27:39  alsu
 * approximating a round-robin scheduler by default
 *
 * Revision 1.28  2004/07/05 14:56:13  rbolze
 * correct bug on 64 bit plat-form, when parsing cfg file :
 * remplace size_t by unsigned int for config options
 *
 * Revision 1.27  2004/06/11 15:45:39  ctedesch
 * add DIET/JXTA
 *
 * Revision 1.26  2004/06/09 15:10:38  mcolin
 * add stat_flush in statistics API in order to flush write access to
 * statistic file for agent and sed which never end and can't call
 * stat_finalize
 ****************************************************************************/


#include <iostream>
using namespace std;
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#if HAVE_FAST
#include "slimfast_api.h"
#endif // HAVE_FAST

#include "SeDImpl.hh"

#include "Callback.hh"
#include "common_types.hh"
#include "debug.hh"
#include "FASTMgr.hh"
#include "marshalling.hh"
#include "ORBMgr.hh"
#include "Parsers.hh"
#include "statistics.hh"

#define DEVELOPPING_DATA_PERSISTENCY 1

/** The trace level. */
extern unsigned int TRACE_LEVEL;

#define SED_TRACE_FUNCTION(formatted_text)       \
  TRACE_TEXT(TRACE_ALL_STEPS, "SeD::");          \
  TRACE_FUNCTION(TRACE_ALL_STEPS,formatted_text)

/*
** if FAST is not present, we set up the variables
** and functions needed to do round-robin, by default
*/
#ifndef HAVE_FAST
static struct timeval *RRtimestamps = NULL;
static int RRtimestampsCapacity = 0;
static int RRensureSize(int serviceNum) {
  /* check to see if we have space for the requested service timestamp */
  /* get the current timestamp */
  if (serviceNum >= RRtimestampsCapacity) {
    /* need to increase the size of the array */
    int newCapacity = serviceNum+1;
    RRtimestamps = (struct timeval *) realloc (RRtimestamps,
                                               (newCapacity *
                                                sizeof (struct timeval)));
    if (RRtimestamps == NULL) {
      return (-1);
    }

    /* initialize new timestamp structures */
    for (int i = RRtimestampsCapacity ; i < newCapacity ; i++) {
      (RRtimestamps[i]).tv_sec = (RRtimestamps[i]).tv_usec = 0;
    }
    RRtimestampsCapacity = newCapacity;
  }
  return (0);
}
static void RRsolve(int serviceNum) {
  if (RRensureSize(serviceNum) == -1) {
    fprintf(stderr, "SeDImpl::solve: unable to realloc rr array\n");
    return;
  }
  gettimeofday(&(RRtimestamps[serviceNum]), NULL);
}
static double RRperformanceMetric(int serviceNum) {
  if (RRensureSize(serviceNum) == -1) {
    fprintf(stderr, "SeDImpl::estimate: unable to realloc rr array\n");
  }

  /* get the current time */
  struct timeval current;
  gettimeofday(&current, NULL);
  double elapsed = ((double) current.tv_sec -
                    (double) (RRtimestamps[serviceNum]).tv_sec +
                    (((double) current.tv_usec -
                      (double) (RRtimestamps[serviceNum]).tv_usec) /
                     1000000.0));

  /*
  ** to prevent underflow on the first usage of a server,
  ** we use a big number as the numerator
  */
  return (1000000.0 / elapsed);
}
#endif /* ! HAVE_FAST */



SeDImpl::SeDImpl()
{
this->SrvT    = NULL;
  this->childID = -1;
  this->parent  = Agent::_nil();
  this->localHostName[0] = '\0';
#if HAVE_FAST
  this->fastUse = 1;
#endif // HAVE_FAST
#if HAVE_LOGSERVICE
  this->dietLogComponent = NULL;
#endif
}

#if HAVE_JXTA
SeDImpl::SeDImpl(const char* uuid = '\0')
{
  this->SrvT    = NULL;
  this->childID = -1;
  this->parent  = Agent::_nil();
  this->localHostName[0] = '\0';
  this->uuid = uuid;
#if HAVE_FAST
  this->fastUse = 1;
#endif // HAVE_FAST
#if HAVE_LOGSERVICE
  this->dietLogComponent = NULL;
#endif
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
  localHostName[257] = '\0';
  if (gethostname(localHostName, 256)) {
    ERROR("could not get hostname", 1);
  }
 
  this->SrvT = services;

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

#if HAVE_LOGSERVICE
  if (dietLogComponent != NULL) {
    for (CORBA::ULong i=0; i<profiles->length(); i++) {
      dietLogComponent->logAddService(&((*profiles)[i]));
    }
  }
#endif

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
  delete profiles;

  unsigned int* endPoint = (unsigned int*)
    Parsers::Results::getParamValue(Parsers::Results::DIETPORT);
  // FIXME: How can I get the port used by the ORB ? and is it useful ?
  if (endPoint == NULL)
    this->port = 0;
  else
    this->port = *endPoint;

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
  
  // Init FAST (HAVE_FAST is managed by the FASTMgr class)
  return FASTMgr::init();
}

/** Set this->dataMgr */
int
SeDImpl::linkToDataMgr(DataMgrImpl* dataMgr)
{
  this->dataMgr = dataMgr;
  return 0;
}

#if HAVE_LOGSERVICE
void
SeDImpl::setDietLogComponent(DietLogComponent* dietLogComponent) {
  this->dietLogComponent = dietLogComponent;
}
#endif


void
SeDImpl::getRequest(const corba_request_t& creq)
{
  corba_response_t resp;
  
  TRACE_TEXT(TRACE_MAIN_STEPS,
	     "\n**************************************************\n"
	     << "Got request " << creq.reqID << endl << endl);
  resp.reqID = creq.reqID;
  resp.myID  = childID;
#if HAVE_LOGSERVICE
  if (dietLogComponent != NULL) {
    dietLogComponent->logAskForSeD(&creq);
  }
#endif

  ServiceTable::ServiceReference_t serviceRef;
  serviceRef = SrvT->lookupService(&(creq.pb));
  if (serviceRef == -1) {
    resp.servers.length(0);
    cout << "service not found ??????????????????????????????????????" << endl;
  } else {
    resp.servers.length(1);

    resp.servers[0].loc.ior      = SeD::_duplicate(_this());
    resp.servers[0].loc.hostName = CORBA::string_dup(localHostName);
#if HAVE_JXTA
    resp.servers[0].loc.uuid = CORBA::string_dup(uuid);
#endif //HAVE_JXTA
    resp.servers[0].loc.port     = this->port;
    resp.servers[0].estim.commTimes.length(creq.pb.last_out + 1);
    for (int i = 0; i <= creq.pb.last_out; i++)
      resp.servers[0].estim.commTimes[i] = 0;
    this->estimate(resp.servers[0].estim, creq.pb, serviceRef);
  }

  // Just for debugging
  if (TRACE_LEVEL >= TRACE_STRUCTURES)
    displayResponse(stdout, &resp);

#if HAVE_LOGSERVICE
  if (dietLogComponent != NULL) {
    dietLogComponent->logSedChosen(&creq,&resp);
  }
#endif

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



CORBA::Long
SeDImpl::solve(const char* path, corba_profile_t& pb, CORBA::Long reqID)
{
  ServiceTable::ServiceReference_t ref(-1);
  diet_profile_t profile;
  diet_convertor_t* cvt(NULL);
  int solve_res(0);
  char statMsg[128];
  int i;//, arg_idx;

  ref = SrvT->lookupService(path, &pb);
  if (ref == -1) {
   ERROR("SeD::" << __FUNCTION__ << ": service not found", 1);
  } 
   
#ifndef HAVE_FAST
  /*
  ** tell the default round-robin scheduler that we got a solve request!
  */
  RRsolve(ref);
#endif /* ! HAVE_FAST */
   

#if HAVE_QUEUES
  if (this->useConcJobLimit){
    this->accessController->waitForResource();
  }
#endif // HAVE_QUEUES

  sprintf(statMsg, "solve %ld", (unsigned long) reqID);
  stat_in("SeD",statMsg);

#if HAVE_LOGSERVICE
  if (dietLogComponent != NULL) {
    dietLogComponent->logBeginSolve(path, &pb,reqID);
  }
#endif

  TRACE_TEXT(TRACE_MAIN_STEPS, "SeD::solve invoked on pb: " << path << endl);

  cvt = SrvT->getConvertor(ref);
 
#if DEVELOPPING_DATA_PERSISTENCY
  // added for data persistence 
  for (i=0; i <= pb.last_inout; i++) {
 
    if(pb.parameters[i].value.length() == 0){ /* In argument with NULL value : data is present */
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
        (pb.parameters[i].desc.specific._d() == DIET_FILE))
    {
      char* in_path   = CORBA::string_dup(profile.parameters[i].desc.specific.file.path);
      this->dataMgr->changePath(pb.parameters[i], in_path);
    }
  }

#else  // DEVELOPPING_DATA_PERSISTENCY  

  unmrsh_in_args_to_profile(&profile, &pb, cvt);

#endif // DEVELOPPING_DATA_PERSISTENCY
  
  solve_res = (*(SrvT->getSolver(ref)))(&profile);    // SOLVE
  
#if DEVELOPPING_DATA_PERSISTENCY 

  for(i=0;i<pb.last_in;i++){
    if(diet_is_persistent(profile.parameters[i])) {
      if (pb.parameters[i].desc.specific._d() == DIET_FILE) {
	pb.parameters[i].desc.specific.file().path= (char *)NULL;
      } else { 
	CORBA::Char *p1 (NULL);
	pb.parameters[i].value.replace(0,0,p1,0);
      }
    }
  }

#endif // DEVELOPPING_DATA_PERSISTENCY   
    
    mrsh_profile_to_out_args(&pb, &profile, cvt);
    
    
#if DEVELOPPING_DATA_PERSISTENCY   
 
  /*    for (i = pb.last_in + 1 ; i <= pb.last_inout; i++) {
      if ( diet_is_persistent(pb.parameters[i])) {
	this->dataMgr->updateDataList(pb.parameters[i]); 
      }
      }*/
 
    for (i = pb.last_inout + 1 ; i <= pb.last_out; i++) {
      if ( diet_is_persistent(pb.parameters[i])) {
	this->dataMgr->addData(pb.parameters[i],1); 
      }
    }
    this->dataMgr->printList();
#endif // DEVELOPPING_DATA_PERSISTENCY
 
  if (TRACE_LEVEL >= TRACE_MAIN_STEPS)
    cout << "SeD::solve complete\n"
	 << "************************************************************\n";

  for (i = 0; i <= cvt->last_in; i++) {
    diet_free_data(&(profile.parameters[i]));
  }
  delete [] profile.parameters; // allocated by unmrsh_in_args_to_profile
 
  stat_out("SeD",statMsg);
  stat_flush();

#if HAVE_LOGSERVICE
  if (dietLogComponent != NULL) {
    dietLogComponent->logEndSolve(path, &pb,reqID);
  }
#endif

#if HAVE_QUEUES
  if (this->useConcJobLimit){
    this->accessController->releaseResource();
  }
#endif // HAVE_QUEUES

  return solve_res;
}

void
SeDImpl::solveAsync(const char* path, const corba_profile_t& pb, 
		    CORBA::Long reqID, const char* volatileclientREF)
{
  // TODO: enable RRSolve & Queue handling here to match solveSync
#if HAVE_LOGSERVICE
  if (dietLogComponent != NULL) {
    dietLogComponent->logBeginSolve(path, &pb,reqID);
  }
#endif

#if DEVELOPPING_DATA_PERSISTENCY
 

#endif // DEVELOPPING_DATA_PERSISTENCY

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

#if DEVELOPPING_DATA_PERSISTENCY
      int i;
 
      for (i = 0; i <= pb.last_inout; i++) {    
	if(pb.parameters[i].value.length() == 0){
	  
	  this->dataMgr->getData(const_cast<corba_data_t&>(pb.parameters[i]));
	} else {
	  if( diet_is_persistent(pb.parameters[i]) ) {
	    
	    this->dataMgr->addData(const_cast<corba_data_t&>(pb.parameters[i]),0);
	  }
	}
      }
      unmrsh_in_args_to_profile(&profile, &(const_cast<corba_profile_t&>(pb)), cvt);
      
      
      
      //      displayProfile(&profile, path);
      
#else // DEVELOPPING_DATA_PERSISTENCY
      
      unmrsh_in_args_to_profile(&profile, &(const_cast<corba_profile_t&>(pb)), cvt);
       for (i=0; i <= pb.last_inout; i++) {
     if( diet_is_persistent(pb.parameters[i])&& (pb.parameters[i].desc.specific._d() == DIET_FILE))
       {
	 char* in_path   = CORBA::string_dup(profile.parameters[i].desc.specific.file.path);
	 this->dataMgr->changePath(pb.parameters[i], in_path);
       }
   }
#endif // DEVELOPPING_DATA_PERSISTENCY
      
      solve_res = (*(SrvT->getSolver(ref)))(&profile);
      
#if ! DEVELOPPING_DATA_PERSISTENCY
      
      mrsh_profile_to_out_args(&(const_cast<corba_profile_t&>(pb)), &profile, cvt);
      
#else  // ! DEVELOPPING_DATA_PERSISTENCY
      
      
      mrsh_profile_to_out_args(&(const_cast<corba_profile_t&>(pb)), &profile, cvt);

      /*      for (i = profile.last_in + 1 ; i <= profile.last_inout; i++) {
	if ( diet_is_persistent(profile.parameters[i])) {
	  this->dataMgr->updateDataList(const_cast<corba_data_t&>(pb.parameters[i])); 
	}
	}*/
      
      for (i = profile.last_inout + 1 ; i <= profile.last_out; i++) {

	if ( diet_is_persistent(profile.parameters[i])) {
	  
	  this->dataMgr->addData(const_cast<corba_data_t&>(pb.parameters[i]),1); 
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
      
#endif // ! DEVELOPPING_DATA_PERSISTENCY

      TRACE_TEXT(TRACE_MAIN_STEPS, "SeD::" << __FUNCTION__ << " complete\n"
		 << "**************************************************\n");

      stat_out("SeD","solveAsync");
      stat_flush();

#if HAVE_LOGSERVICE
  if (dietLogComponent != NULL) {
    dietLogComponent->logEndSolve(path, &pb,reqID);
  }
#endif
     
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

CORBA::Long
SeDImpl::ping()
{
  SED_TRACE_FUNCTION("");
  return 0;
}


/****************************************************************************/
/* Private methods                                                          */
/****************************************************************************/

/*
static void
corbaPbDesc2dietProfile(const corba_pb_desc_t& pb, diet_profile_t& prof)
{
}
*/

/**
 * Estimate a request, with FAST if available.
 */
inline void 
SeDImpl::estimate(corba_estimation_t& estimation,
		  const corba_pb_desc_t& pb,
		  const ServiceTable::ServiceReference_t ref)
{
  diet_perfmetric_t perfmetric_fn = SrvT->getPerfMetric(ref);

  FASTMgr::estimate(this->localHostName,
		    estimation, pb, SrvT->getConvertor(ref));

  /* Evaluate comm times for persistent IN arguments only: comm times for
     volatile IN and persistent OUT arguments cannot be estimated here, and
     persistent OUT arguments will not move (comm times already set to 0). */
  for (int i = 0; i <= pb.last_inout; i++) {
    // FIXME: here the data localization service must be interrogated to
    // determine the transfer time of all IN and INOUT parameters.
    if ((pb.param_desc[i].mode > DIET_VOLATILE)
	&& (pb.param_desc[i].mode <= DIET_STICKY)
	&& (*(pb.param_desc[i].id.idNumber) != '\0')) {    
      estimation.commTimes[i] = 0;  
#if DEVELOPPING_DATA_PERSISTENCY
      /*   cout << "in ESTIMATE" << endl;
      if(this->dataMgr->dataLookup(CORBA::string_dup(pb.param_desc[i].id.idNumber)))	
	estimation.commTimes[i] = 0;  // getTransferTime(pb.params[i].id);
      else {
	char *remoteHostName=NULL;
	remoteHostName = this->dataMgr->whichDataMgr(pb.param_desc[i].id.idNumber);
	//	if (*remoteHostName == '\0')
	//  cout << " PROBLEM" << endl;
	//else 
	unsigned int size =(long unsigned int) data_sizeof(&(pb.param_desc[i]));
	estimation.commTimes[i]=FASTMgr::commTime(localHostName,remoteHostName,size,false);
	// getTransferTime(pb.params[i].id);
	}*/
#endif //  DEVELOPPING_DATA_PERSISTENCY
    }
  }

  if (perfmetric_fn == NULL) {
#ifdef HAVE_FAST
    estimation.totalTime = estimation.tComp;
    if (estimation.totalTime != HUGE_VAL) {
      for (int i = 0; i <= pb.last_out; i++) {
        estimation.totalTime += estimation.commTimes[i];
        if (estimation.commTimes[i] == HUGE_VAL)
          break;
      }
    }
#else /* HAVE_FAST */
    /*
    ** without FAST, we use a round-robin if no custom performance
    ** metric has been specified.
    */
    estimation.totalTime = estimation.tComp = RRperformanceMetric(ref);
#endif /* HAVE_FAST */
  }
  else {
    diet_profile_t profile;

    /*
    ** create a profile, based on the problem description, to
    ** be used in the performance metric function
    */
    profile.pb_name = strdup(pb.path);
    profile.last_in = pb.last_in;
    profile.last_inout = pb.last_inout;
    profile.last_out = pb.last_out;
    profile.parameters = (diet_arg_t*) calloc ((pb.last_out+1),
                                               sizeof (diet_arg_t));

    /* populate the parameter structures */
    for (int i = 0 ; i <= pb.last_out ; i++) {
      const corba_data_desc_t cdd = pb.param_desc[i];
      diet_arg_t* da = &(profile.parameters[i]);
      da->value = NULL;
      diet_data_desc_t* ddd = &(da->desc);
      ddd->id = strdup(cdd.id.idNumber);
      ddd->mode = (diet_persistence_mode_t) cdd.mode;
      struct diet_data_generic* ddg_s = &(ddd->generic);
      ddg_s->base_type = (diet_base_type_t) cdd.base_type;

      /*
      ** TODO: figure out if this is acceptable (calling a seemingly
      **       private method of an inner class), and if the
      **       correspondence of these two member variables is fixed
      */
      ddg_s->type = (diet_data_type_t) cdd.specific._d();
    }

    /*
    ** profile is ready, call the custom performance
    ** metric function!
    */
    estimation.tComp = estimation.totalTime = (*perfmetric_fn)(&profile);
  }
}


