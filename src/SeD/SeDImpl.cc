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
 * Revision 1.8  2003/07/25 20:23:52  pcombes
 * Fix the warning about "uninitialized variables" n the FAST part.
 *
 * Revision 1.7  2003/07/04 09:47:57  pcombes
 * Use new ERROR, WARNING and TRACE macros.
 *
 * Revision 1.6  2003/06/23 13:35:06  pcombes
 * useAsyncAPI should be replaced by a "useBiDir" option. Remove it so far.
 *
 * Revision 1.5  2003/06/02 15:29:46  cpera
 * Update callback use.
 *
 * Revision 1.4  2003/06/02 09:06:46  cpera
 * Beta version of asynchronize DIET API.
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

#include "debug.hh"
#include "marshalling.hh"
#include "ORBMgr.hh"
#include "Parsers.hh"
#include "statistics.hh"
#include "Callback.hh"

/** The trace level. */
extern unsigned int TRACE_LEVEL;

#define SED_TRACE_FUNCTION(formatted_text)       \
  TRACE_TEXT(TRACE_ALL_STEPS, "SeD::");          \
  TRACE_FUNCTION(TRACE_ALL_STEPS,formatted_text)


SeDImpl::SeDImpl()
{
  SrvT    = NULL;
  childID = -1;
  parent  = NULL;
  localHostName[0] = '\0';
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
    Agent::_duplicate(Agent::_narrow(ORBMgr::getAgentReference(parent_name)));
  if (CORBA::is_nil(parent)) {
    ERROR("cannot locate agent " << parent_name, 1);
  }
  
  profiles = this->SrvT->getProfiles();
  if (TRACE_LEVEL >= TRACE_STRUCTURES)
    this->SrvT->dump(stdout);
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
  delete profiles;

  size_t* endPoint = (size_t*)
    Parsers::Results::getParamValue(Parsers::Results::ENDPOINT);
  // FIXME: How can I get the port used by the ORB ? and is it useful ?
  if (endPoint == NULL)
    this->port = 0;
  else
    this->port = *endPoint;

#if HAVE_FAST

  this->fastUse =
    *((size_t*)Parsers::Results::getParamValue(Parsers::Results::FASTUSE));

  if (this->fastUse > 0) {
    Parsers::Results::Address* tmp;
    size_t ldapUse(1), ldapPort(0), nwsUse(1), nwsNSPort(0), nwsFcstPort(0);
    char*  ldapHost = "";
    char*  ldapMask = "";
    char*  nwsNSHost = "";
    char*  nwsFcstHost = "";
    
    ldapUse = 
      *((size_t*)Parsers::Results::getParamValue(Parsers::Results::LDAPUSE));
    if (ldapUse) {
      tmp = (Parsers::Results::Address*)
	Parsers::Results::getParamValue(Parsers::Results::LDAPBASE);
      ldapHost = tmp->host;
      ldapPort = tmp->port;
      ldapMask = (char*)
	Parsers::Results::getParamValue(Parsers::Results::LDAPMASK);
    }

    nwsUse =
      *((size_t*)Parsers::Results::getParamValue(Parsers::Results::NWSUSE));
    if (nwsUse) {
      tmp = (Parsers::Results::Address*)
	Parsers::Results::getParamValue(Parsers::Results::NWSNAMESERVER);
      nwsNSHost = tmp->host;
      nwsNSPort = tmp->port;
      tmp = (Parsers::Results::Address*)
	Parsers::Results::getParamValue(Parsers::Results::NWSFORECASTER);
      nwsFcstHost = tmp->host;
      nwsFcstPort = tmp->port;
    }
    
    /* Initialize FAST with parsed parameters */
    if (!fast_init("ldap_use", ldapUse,
		   "ldap_server", ldapHost, ldapPort, "ldap_binddn", ldapMask,
		   "nws_use", nwsUse,
		   "nws_nameserver", nwsNSHost, nwsNSPort,
		   "nws_forecaster", nwsFcstHost, nwsFcstPort,
		   NULL))
      return 1;

  } // if (this->fastUse > 0)

#endif // HAVE_FAST
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
  unmrsh_in_args_to_profile(&profile, &pb, cvt);
  
  solve_res = (*(SrvT->getSolver(ref)))(&profile);
  
  mrsh_profile_to_out_args(&pb, &profile, cvt);

  /* Free data */
  // FIXME: persistent data should not be freed but referenced in the data list.

  TRACE_TEXT(TRACE_MAIN_STEPS, "SeD::" << __FUNCTION__ << " complete\n"
	     << "**************************************************\n");

  stat_out("SeDImpl::solve.end");

  return solve_res;
}

void
SeDImpl::solveAsync(const char* path, const corba_profile_t& pb, 
		    CORBA::Long reqID, const char* volatileclientREF)
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
  
      stat_in("SeDImpl::solveAsync.start");

      TRACE_TEXT(TRACE_MAIN_STEPS,
		 "SeD::solveAsync invoked on pb: " << path << endl);
  
      ref = SrvT->lookupService(path, &pb);
      if (ref == -1) {
	ERROR("SeD::" << __FUNCTION__ << ": service not found",);
      }
  
      cvt = SrvT->getConvertor(ref);
      unmrsh_in_args_to_profile(&profile, &(const_cast<corba_profile_t&>(pb)), cvt);
      displayProfile(&profile, path);
  
      solve_res = (*(SrvT->getSolver(ref)))(&profile);
      mrsh_profile_to_out_args(&(const_cast<corba_profile_t&>(pb)), &profile, cvt);
  

      /* Free data */
      // FIXME: persistent data should not be freed but referenced in the data list.

      TRACE_TEXT(TRACE_MAIN_STEPS, "SeD::" << __FUNCTION__ << " complete\n"
		 << "**************************************************\n");

      stat_out("SeDImpl::solveAsync.end");
      
      // send result data to client.
      TRACE_TEXT(TRACE_ALL_STEPS, "SeD::" << __FUNCTION__
		 << ": performing the call-back.\n");
      Callback_var cb_var = Callback::_narrow(cb);
      cb_var->notifyResults(path,pb, reqID);
      cb_var->solveResults(path,pb, reqID);
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
 * Called for first evaluation and when client connects to this SeD.
 * \c estimation.commTimes must be initalized.
 */
inline void 
SeDImpl::estimate(corba_estimation_t& estimation,
		  const corba_pb_desc_t& pb,
		  const ServiceTable::ServiceReference_t ref)
{
  double time(HUGE_VAL);
  double freeCPU(0);
  double freeMem(0);

#if HAVE_FAST

  if (this->fastUse) {
    sf_inst_desc_t inst;
  
    fastMutex.lock();
    unmrsh_pb_desc_to_sf(&inst, &pb, this->SrvT->getConvertor(ref));
    if ( !(fast_comp_time_best(this->localHostName, &inst, &time)) ) {
      WARNING("cannot estimate computation time. Try CPU and memory load");
      time = HUGE_VAL;
    }
    if ( !(fast_load(this->localHostName, &freeCPU)) ) {
      WARNING("cannot estimate free CPU fraction");
      freeCPU = 0;
    }
    if ( !(fast_memory(this->localHostName, &freeMem)) ) {
      WARNING("cannot estimate free memory");
      freeMem = 0;
    }
    fastMutex.unlock();
    CORBA::string_free(inst.path);
    delete [] inst.param_desc;
  }

#else  // HAVE_FAST

  WARNING("cannot estimate computation time: time set to inf");
  PAUSE(0,5000); // simulate FAST interrogation

#endif // HAVE_FAST

  estimation.tComp   = time;
  estimation.freeCPU = freeCPU;
  estimation.freeMem = freeMem;
  return;

  /* Evaluate comm times for persistent IN arguments only: comm times for
     volatile IN and persistent OUT arguments cannot be estimated here, and
     persistent OUT arguments will not move (comm times already set to 0). */
  for (int i = 0; i <= pb.last_inout; i++) {
    // FIXME: here the data localization service must be interrogated to
    // determine the transfer time of all IN and INOUT parameters.
    if ((pb.param_desc[i].mode > DIET_VOLATILE)
	&& (pb.param_desc[i].mode <= DIET_STICKY)
	&& (*(pb.param_desc[i].id) != '\0')) {
      estimation.commTimes[i] = 0;  // getTransferTime(pb.params[i].id);
    }
  }

  estimation.totalTime = time;
  if (estimation.totalTime != HUGE_VAL) {
    for (int i = 0; i <= pb.last_out; i++) {
      estimation.totalTime += estimation.commTimes[i];
      if (estimation.commTimes[i] == HUGE_VAL)
	break;
    }
  }
}
