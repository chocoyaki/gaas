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
 * Revision 1.22  2004/03/25 14:30:42  bdelfabr
 * displayProfile removed in solveAsync
 *
 * Revision 1.21  2004/03/01 18:42:34  rbolze
 * add logservice
 *
 * Revision 1.20  2004/02/27 10:26:37  bdelfabr
 * let DIET_PERSISTENCE_MODE set to 1, coding standard
 *
 * Revision 1.19  2003/11/10 14:12:06  bdelfabr
 * estimate method modified
 * adding estimation time for data transfer in case of persistent data.
 * Data transfer time estimation is computed by FAST.
 *
 * Revision 1.18  2003/10/21 13:27:35  bdelfabr
 * Set persistence flag to 0
 *
 * Revision 1.17  2003/10/14 20:28:32  bdelfabr
 * adding method after solved problem to print the list of data owned by the
 * DataManager (PERSISTENT mode only)
 *
 * Revision 1.16  2003/10/10 14:53:45  bdelfabr
 * TabValue removed : no longer used
 *
 * Revision 1.15  2003/09/30 15:40:28  bdelfabr
 * manage Data Persistence with Solve and SolveAsync
 *
 * Revision 1.14  2003/09/29 09:33:50  pcombes
 * solve*: Delete the parameters allocated by unmrsh_in_args_to_profile.
 *
 * Revision 1.13  2003/09/29 09:25:01  ecaron
 * Take into account the new API of statistics module
 *
 * Revision 1.12  2003/09/24 09:15:38  pcombes
 * Merge corba_DataMgr_desc_t and corba_data_desc_t.
 *
 * Revision 1.11  2003/09/22 21:17:54  pcombes
 * Set all the modules and their interfaces for data persistency.
 *
 * Revision 1.9  2003/08/01 19:33:11  pcombes
 * Use FASTMgr.
 *
 * Revision 1.7  2003/07/04 09:47:57  pcombes
 * Use new ERROR, WARNING and TRACE macros.
 *
 * Revision 1.6  2003/06/23 13:35:06  pcombes
 * useAsyncAPI should be replaced by a "useBiDir" option. Remove it so far.
 *
 * Revision 1.3  2003/05/13 17:14:00  pcombes
 * Catch CORBA exceptions in serverSubscribe.
 *
 * Revision 1.2  2003/05/10 08:54:41  pcombes
 * New format for configuration files, new Parsers.
 *
 * Revision 1.1  2003/04/10 13:14:28  pcombes
 * Replace SeD_impl.cc. Implement contract checking. Use Parsers.
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
    childID = parent->serverSubscribe(this->_this(), localHostName, *profiles);
  } catch (CORBA::Exception& e) {
    CORBA::Any tmp;
    tmp <<= e;
    CORBA::TypeCode_var tc = tmp.type();
    ERROR("exception caught (" << tc->name() << ") while subscribing to "
	  << parent_name << ": either the latter is down, "
	  << "or there is a problem with the CORBA name server", 1);
  }
  //  delete profiles

  size_t* endPoint = (size_t*)
    Parsers::Results::getParamValue(Parsers::Results::ENDPOINT);
  // FIXME: How can I get the port used by the ORB ? and is it useful ?
  if (endPoint == NULL)
    this->port = 0;
  else
    this->port = *endPoint;
  
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
    resp.sortedIndexes.length(0);;
    resp.servers.length(0);
    cout << "service not found ??????????????????????????????????????" << endl;
  } else {
    resp.sortedIndexes.length(1);
    resp.servers.length(1);

    resp.sortedIndexes[0]        = 0;
    resp.servers[0].loc.ior      = SeD::_duplicate(_this());
    resp.servers[0].loc.hostName = CORBA::string_dup(localHostName);
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
SeDImpl::solve(const char* path, corba_profile_t& pb)
{
  ServiceTable::ServiceReference_t ref(-1);
  diet_profile_t profile;
  diet_convertor_t* cvt(NULL);
  int solve_res(0);
  stat_in("SeD","solve");


#if HAVE_LOGSERVICE
  if (dietLogComponent != NULL) {
    dietLogComponent->logBeginSolve(path, &pb);
  }
#endif

  TRACE_TEXT(TRACE_MAIN_STEPS, "SeD::solve invoked on pb: " << path << endl);
  
  ref = SrvT->lookupService(path, &pb);
  if (ref == -1) {
   ERROR("SeD::" << __FUNCTION__ << ": service not found", 1);
  } 
   
  cvt = SrvT->getConvertor(ref);
 
#if DEVELOPPING_DATA_PERSISTENCY
  // added for data persistence 
  int i;

 
  for (i=0; i <= pb.last_inout; i++) {
 
   
    if(pb.parameters[i].value.length() == 0){ /* In argument with NULL value : data is present */
      this->dataMgr->getData(pb.parameters[i]); 
    } else { /* data is not yet present but is persistent */
          if( diet_is_persistent(pb.parameters[i]) ) {
       	this->dataMgr->addData(pb.parameters[i],0);
      }
    }
  } 
 
  unmrsh_in_args_to_profile(&profile, &pb, cvt);
  
  
#else  // DEVELOPPING_DATA_PERSISTENCY  

  unmrsh_in_args_to_profile(&profile, &pb, cvt);

#endif // DEVELOPPING_DATA_PERSISTENCY

  solve_res = (*(SrvT->getSolver(ref)))(&profile);
 
  mrsh_profile_to_out_args(&pb, &profile, cvt);
 

#if DEVELOPPING_DATA_PERSISTENCY   
 
    for (i = pb.last_in + 1 ; i <= pb.last_inout; i++) {
      if ( diet_is_persistent(pb.parameters[i])) {
	this->dataMgr->updateDataList(pb.parameters[i]); 
      }
    }
 
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

  delete [] profile.parameters; // allocated by unmrsh_in_args_to_profile
  
  stat_out("SeD","solve");  

#if HAVE_LOGSERVICE
  if (dietLogComponent != NULL) {
    dietLogComponent->logEndSolve(path, &pb);
  }
#endif

  return solve_res;
}

void
SeDImpl::solveAsync(const char* path, const corba_profile_t& pb, 
		    CORBA::Long reqID, const char* volatileclientREF)
{

#if HAVE_LOGSERVICE
  if (dietLogComponent != NULL) {
    dietLogComponent->logBeginSolve(path, &pb);
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
      
#endif // DEVELOPPING_DATA_PERSISTENCY
      
      solve_res = (*(SrvT->getSolver(ref)))(&profile);
      
#if ! DEVELOPPING_DATA_PERSISTENCY
      
      mrsh_profile_to_out_args(&(const_cast<corba_profile_t&>(pb)), &profile, cvt);
      
#else  // ! DEVELOPPING_DATA_PERSISTENCY
      
      mrsh_profile_to_out_args(&(const_cast<corba_profile_t&>(pb)), &profile, cvt);
      for (i = profile.last_in + 1 ; i <= profile.last_inout; i++) {
	if ( diet_is_persistent(profile.parameters[i])) {
	  this->dataMgr->updateDataList(const_cast<corba_data_t&>(pb.parameters[i])); 
	}
      }
      
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

#if HAVE_LOGSERVICE
  if (dietLogComponent != NULL) {
    dietLogComponent->logEndSolve(path, &pb);
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

/**
 * Estimate a request, with FAST if available.
 */
inline void 
SeDImpl::estimate(corba_estimation_t& estimation,
		  const corba_pb_desc_t& pb,
		  const ServiceTable::ServiceReference_t ref)
{

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
  
  estimation.totalTime = estimation.tComp;
  if (estimation.totalTime != HUGE_VAL) {
    for (int i = 0; i <= pb.last_out; i++) {
      estimation.totalTime += estimation.commTimes[i];
      if (estimation.commTimes[i] == HUGE_VAL)
	break;
    }
  }
}


