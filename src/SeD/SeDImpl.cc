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

#define DEVELOPPING_DATA_PERSISTENCY 0

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
}

SeDImpl::~SeDImpl()
{
  /* FIXME: Tables should be destroyed. */
}

int
SeDImpl::run(ServiceTable* services)
{
  SeqCorbaProfileDesc_t* profiles(NULL);
  
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
SeDImpl::linkToDataMgr(DataMgrImpl* dataMgr){
  this->dataMgr = dataMgr;
  return 0;
}


void
SeDImpl::getRequest(const corba_request_t& creq)
{
  corba_response_t resp;
  
  TRACE_TEXT(TRACE_MAIN_STEPS,
	     "\n**************************************************\n"
	     << "Got request " << creq.reqID << endl << endl);
  resp.reqID = creq.reqID;
  resp.myID  = childID;

  ServiceTable::ServiceReference_t serviceRef;
  serviceRef = SrvT->lookupService(&(creq.pb));
  if (serviceRef == -1) {
    resp.sortedIndexes.length(0);;
    resp.servers.length(0);

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

  stat_in("SeDImpl::solve.start");

  TRACE_TEXT(TRACE_MAIN_STEPS, "SeD::solve invoked on pb: " << path << endl);
  
  ref = SrvT->lookupService(path, &pb);
  if (ref == -1) {
   ERROR("SeD::" << __FUNCTION__ << ": service not found", 1);
  } 
   
  cvt = SrvT->getConvertor(ref);

#if DEVELOPPING_DATA_PERSISTENCY
  // added for data persistence 
  int i;
  char* s = NULL ;
  int* tabValue(NULL);

  profile.last_in = pb.last_in;
  profile.last_inout = pb.last_inout;
  profile.last_out = pb.last_out;
 
  // end for tests 
  tabValue=(int *)malloc(pb.last_out*sizeof(int));
  for (i = 0; i <= profile.last_inout; i++) tabValue[i] = -1; 
  for (i = 0; i <= profile.last_inout; i++) {
    s = (char *)malloc(sizeof(int));
    sprintf(s,"var%d",i);
    TRACE_VAR(s);
    pb.parameters[i].desc.id = CORBA::string_dup(s);
    if(pb.parameters[i].value.length() == 0){ tabValue[i]=0; TRACE_VAR(i);}
  }  

  unmrsh_in_args_to_profile(&profile, &pb, cvt);
  
  for (i = 0; i <= profile.last_inout; i++) {
    
    if(tabValue[i]==0){
      
      pb.parameters[i]= this->dataMgr->getData(pb.parameters[i].desc.id);
      cout << "DATA PRESENT = GETTING IT....." << endl;
      unmrsh_data((diet_data_t*)(&(profile.parameters[i])),&(pb.parameters[i]));
    } else {
      if( diet_is_persistent(pb.parameters[i]) ) {
	
	this->dataMgr->addData(pb.parameters[i],0);
      } else {
	cout << "Data Is not Persistent" << endl;
      }
    }
  }
  
#else  // DEVELOPPING_DATA_PERSISTENCY  
  
  unmrsh_in_args_to_profile(&profile, &pb, cvt);
  
#endif // DEVELOPPING_DATA_PERSISTENCY

  solve_res = (*(SrvT->getSolver(ref)))(&profile);
  
  mrsh_profile_to_out_args(&pb, &profile, cvt);

#if DEVELOPPING_DATA_PERSISTENCY  
  if(profile.last_inout > profile.last_in) {
    for (i = profile.last_in + 1 ; i <= profile.last_inout; i++) {
      if ( diet_is_persistent(profile.parameters[i])) {
	this->dataMgr->updateDataList(pb.parameters[i]); 
      }
    }
  }
  if(profile.last_out > profile.last_inout) {
    for (i = profile.last_inout + 1 ; i <= profile.last_out; i++) {
      profile.parameters[i].desc.mode = DIET_PERSISTENT;
      s = (char *)malloc(sizeof(int));
      sprintf(s,"var%d",i);
      TRACE_VAR(s);
      pb.parameters[i].desc.id = CORBA::string_dup(s);
      if ( diet_is_persistent(profile.parameters[i])) {
	cout << "adding out data" << endl;
        this->dataMgr->addData(pb.parameters[i],1); 
      }
    }
  }
#endif // DEVELOPPING_DATA_PERSISTENCY
  
  if (TRACE_LEVEL >= TRACE_MAIN_STEPS)
    cout << "SeD::solve complete\n"
	 << "************************************************************\n";
  
  stat_out("SeDImpl::solve.end");
  
  return solve_res;
}

void
SeDImpl::solveAsync(const char* path, const corba_profile_t& pb, 
		    CORBA::Long reqID, const char* volatileclientREF)
{
#if DEVELOPPING_DATA_PERSISTENCY
  int i;
  char* s = NULL;
  int* tabValue;
  corba_profile_t pbc = pb;
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
  
      stat_in("SeDImpl::solveAsync.start");

      TRACE_TEXT(TRACE_MAIN_STEPS,
		 "SeD::solveAsync invoked on pb: " << path << endl);
  
      ref = SrvT->lookupService(path, &pb);
      if (ref == -1) {
	ERROR("SeD::" << __FUNCTION__ << ": service not found",);
      }
      
      cvt = SrvT->getConvertor(ref);

#if DEVELOPPING_DATA_PERSISTENCY

      profile.last_in = pbc.last_in;
      profile.last_inout = pbc.last_inout;
      profile.last_out = pbc.last_out;
      
      // end for tests 
      tabValue=(int *)malloc(pbc.last_inout*sizeof(int));
      for (i = 0; i <= profile.last_inout; i++) tabValue[i] = -1; 
      for (i = 0; i <= profile.last_inout; i++) {
	s = (char *)malloc(sizeof(int));
	sprintf(s,"var%d",i);
	TRACE_VAR(s);
	pbc.parameters[i].desc.id = CORBA::string_dup(s);
	if(pbc.parameters[i].value.length() == 0){ tabValue[i]=0; TRACE_VAR(i);}
      }
      
      unmrsh_in_args_to_profile(&profile, &pbc, cvt);
      
      for (i = 0; i <= profile.last_inout; i++) {
	
	if(tabValue[i]==0){
	  
	  pbc.parameters[i]= this->dataMgr->getData(pbc.parameters[i].desc.id);
	  unmrsh_data((diet_data_t*)(&(profile.parameters[i])),&(pbc.parameters[i]));
	} else {
	  if( diet_is_persistent(pbc.parameters[i]) ) {
	    
	    this->dataMgr->addData(pbc.parameters[i],0);
	  } else {
	    cerr << "ERROR : SOLEV ASYNC UNABLE TO ADD OR RETRIEVE DATA" << endl;
	  }
	}
      }
      
      displayProfile(&profile, path);
      
#else // DEVELOPPING_DATA_PERSISTENCY
      
      unmrsh_in_args_to_profile(&profile, &(const_cast<corba_profile_t&>(pb)), cvt);

#endif // DEVELOPPING_DATA_PERSISTENCY

      solve_res = (*(SrvT->getSolver(ref)))(&profile);

#if ! DEVELOPPING_DATA_PERSISTENCY

      mrsh_profile_to_out_args(&(const_cast<corba_profile_t&>(pb)), &profile, cvt);

#else  // ! DEVELOPPING_DATA_PERSISTENCY
      
      mrsh_profile_to_out_args(&pbc, &profile, cvt);
      if(profile.last_inout > profile.last_in) {
	for (i = profile.last_in + 1 ; i <= profile.last_inout; i++) {
	  if ( diet_is_persistent(profile.parameters[i])) {
	    this->dataMgr->updateDataList(pbc.parameters[i]); 
	  }
	}
      }
      if(profile.last_out > profile.last_inout) {
	for (i = profile.last_inout + 1 ; i <= profile.last_out; i++) {
	  profile.parameters[i].desc.mode = DIET_PERSISTENT;
	  s = (char *)malloc(sizeof(int));
	  sprintf(s,"var%d",i);
	  TRACE_VAR(s);
	  pbc.parameters[i].desc.id = CORBA::string_dup(s);
	  if ( diet_is_persistent(profile.parameters[i])) {
	    
	    this->dataMgr->addData(pbc.parameters[i],1); 
	  }
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

      stat_out("SeDImpl::solveAsync.end");
      
      // send result data to client.
      TRACE_TEXT(TRACE_ALL_STEPS, "SeD::" << __FUNCTION__
		 << ": performing the call-back.\n");
      Callback_var cb_var = Callback::_narrow(cb);
      cb_var->notifyResults(path, pb, reqID);
      cb_var->solveResults(path, pb, reqID);
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
      estimation.commTimes[i] = 0;  // getTransferTime(pb.params[i].id);
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


