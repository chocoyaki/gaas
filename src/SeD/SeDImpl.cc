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

extern unsigned int TRACE_LEVEL;


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
    perror("Could not initialize the SeD");
    return 1;
  }

  SrvT = services;

  char* parent_name = (char*)
    Parsers::Results::getParamValue(Parsers::Results::PARENTNAME);
  if (parent_name == NULL)
    return 1;

  parent =
    Agent::_duplicate(Agent::_narrow(ORBMgr::getAgentReference(parent_name)));
  if (CORBA::is_nil(parent)) {
    cerr << "Cannot locate agent " << parent_name << ".\n";
    return 1;
  }
  
  profiles = SrvT->getProfiles();
  if (TRACE_LEVEL >= TRACE_STRUCTURES)
    SrvT->dump(stdout);
  childID = parent->serverSubscribe(_this(), localHostName, *profiles);
  delete profiles;

  size_t* endPoint = (size_t*)
    Parsers::Results::getParamValue(Parsers::Results::ENDPOINT);
  // FIXME: How can I get the port used by the ORB ?
  //        and is it useful ?
  if (endPoint == NULL)
    this->port = 0;
  else
    this->port = *endPoint;

#if HAVE_FAST

  this->fastUse =
    *((size_t*)Parsers::Results::getParamValue(Parsers::Results::FASTUSE));

  if (this->fastUse > 0) {
    Parsers::Results::Address* tmp;
    size_t ldapUse, ldapPort, nwsUse, nwsNSPort, nwsFcstPort;
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

  if (TRACE_LEVEL >= TRACE_MAIN_STEPS)
    cout << "\n************************************************************\n"
	 << "Got request " << creq.reqID << endl << endl;  
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
  if (TRACE_LEVEL >= TRACE_MAIN_STEPS)
    cout << "************************************************************\n";

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

  if (TRACE_LEVEL >= TRACE_MAIN_STEPS)
    cout << "\n************************************************************\n"
	 << "SeD::solve invoked on pb: " << path << endl;
  
  ref = SrvT->lookupService(path, &pb);
  if (ref == -1) {
    cerr << "ERROR in SeD solve: service not found.\n";
    return 1;
  }
  
  cvt = SrvT->getConvertor(ref);
  unmrsh_in_args_to_profile(&profile, &pb, cvt);
  
  solve_res = (*(SrvT->getSolver(ref)))(&profile);
  
  mrsh_profile_to_out_args(&pb, &profile, cvt);

  /* Free data */
  // FIXME: persistent data should not be freed but referenced in the data list.

  if (TRACE_LEVEL >= TRACE_MAIN_STEPS)
    cout << "SeD::solve complete\n"
	 << "************************************************************\n";

  stat_out("SeDImpl::solve.end");

  return solve_res;
}


CORBA::Long
SeDImpl::ping()
{
  if (TRACE_LEVEL >= TRACE_ALL_STEPS)
    cout << "SeD::ping()\n";
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
  
    fast_mutex.lock();
    unmrsh_pb_desc_to_sf(&inst, &pb, this->SrvT->getConvertor(ref));
    if ( !(fast_comp_time_best(this->localHostName, &inst, &time)) ) {
      cerr << "Warning: Cannot estimate computation time. "
	   << "Try CPU and memory load.\n";
      time = HUGE_VAL;
    }
    if ( !(fast_load(this->localHostName, &freeCPU)) ) {
      cerr << "Warning: Cannot estimate free CPU fraction.\n";
      freeCPU = 0;
    }
    if ( !(fast_memory(this->localHostName, &freeMem)) ) {
      cerr << "Warning: Cannot estimate free memory.\n";
      freeMem = 0;
    }
    fast_mutex.unlock();
    CORBA::string_free(inst.path);
    delete [] inst.param_desc;
  }

#else  // HAVE_FAST

  cerr << "Warning: Cannot estimate computation time: time set to inf.\n";

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
