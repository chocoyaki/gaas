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
 * Revision 1.1  2003/04/10 13:14:28  pcombes
 * Replace SeD_impl.cc. Implement contract checking. Use Parsers.
 *
 ****************************************************************************/


#include <iostream>
using namespace std;
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "SeDImpl.hh"

#include "debug.hh"
#include "marshalling.hh"
#include "ORBMgr.hh"
#include "Parsers.hh"
#include "statistics.hh"

#if HAVE_FAST
#include "slimfast_api.h"
#endif // HAVE_FAST

extern unsigned int TRACE_LEVEL;


SeDImpl::SeDImpl(int SeDPort)
{
  port    = SeDPort;
  SrvT    = NULL;
  childID = -1;
  parent  = NULL;
  localHostName[0] = '\0';
  //  data    = new dietServerDataDescList();
#if HAVE_FAST
  ldapUse     = 1;
  ldapHost[0] = '\0';
  ldapPort    = 0;
  ldapMask[0] = '\0';
  nwsUse               = 1;
  nwsNSHost[0]         = '\0';
  nwsNSPort            = 0;
  nwsForecasterHost[0] = '\0';
  nwsForecasterPort    = 0;
#endif // HAVE_FAST
}

SeDImpl::~SeDImpl()
{
  /* FIXME: Tables should be destroyed. */
}


int
SeDImpl::run(char* configFileName, ServiceTable* services)
{
  char parent_name[257];
  SeqCorbaProfileDesc_t* profiles(NULL);
  
  localHostName[257] = '\0';
  if (gethostname(localHostName, 256)) {
    perror("Could not initialize the SeD");
    return 1;
  }

  if (parseConfigFile(configFileName, (char*)parent_name))
    return 1;
  SrvT = services;

//   SrvT->traceLevel = traceLevel;
//   data_set_trace_level(traceLevel);
//   mrsh_set_trace_level(traceLevel);
  ORBMgr::setTraceLevel();

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

#if HAVE_FAST
  if (!fast_init("nws_use", this->nwsUse,
		 "nws_nameserver", this->nwsNSHost, this->nwsNSPort,
		 "nws_forecaster",
		 this->nwsForecasterHost, this->nwsForecasterPort,
		 "ldap_use", this->ldapUse,
		 "ldap_server", this->ldapHost, this->ldapPort,
		 "ldap_binddn", this->ldapMask))
    return 1; 
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

#if 0
  /* Free data */
  // FIXME: persistent data should not be freed but referenced in the data list.
  for (i = 0; i <= pb_profile.last_inout; i++) {
    diet_server_data_desc_t* newData = new diet_server_data_desc_t;
    newData->id = -1;
    newData->data = inout_args.seq[i];
    if (TRACE_LEVEL >= TRACE_ALL_STEPS)
      cout << "Adding " << i << "th INOUT arg\n";
    addVar(newData);
  }
  for (i = 0; i < pb_profile.last_out; i++) {
    diet_server_data_desc_t* newData = new diet_server_data_desc_t;
    newData->id = -1;
    if (TRACE_LEVEL >= TRACE_ALL_STEPS)
      cout << "Adding " << i << "th OUT arg\n";
    newData->data = out_args.seq[i];
    addVar(newData);
  }
#endif // 0
  if (TRACE_LEVEL >= TRACE_MAIN_STEPS)
    cout << "SeD::solve complete\n"
	 << "************************************************************\n";

  stat_out("SeDImpl::solve.end");

  return solve_res;
}


CORBA::Long
SeDImpl::ping()
{
  //cout << "I was pinged !\n";
  return 0;
}


/****************************************************************************/
/* Private methods                                                          */
/****************************************************************************/


int
SeDImpl::parseConfigFile(char* configFileName, char* parentName)
{
  int res =
    Parsers::beginParsing(configFileName)
    || Parsers::parseName(parentName)
    || Parsers::parseTraceLevel()//(&TRACE_LEVEL)
    || Parsers::parseFASTEntries(&this->ldapUse,   this->ldapHost,
				 &this->ldapPort,  this->ldapMask,
				 &this->nwsUse,    this->nwsNSHost,
				 &this->nwsNSPort, this->nwsForecasterHost,
				 &this->nwsForecasterPort);

  if (!res && TRACE_LEVEL >= TRACE_STRUCTURES) {
    cout << "TRACE_LEVEL = "         << TRACE_LEVEL       << endl
	 << "LDAP_USE = "            << ldapUse           << endl
	 << "LDAP_HOST = "           << ldapHost          << " | "
	 << "LDAP_PORT = "           << ldapPort          << " | "
	 << "LDAP_MASK = "           << ldapMask          << endl
	 << "NWS_USE = "             << nwsUse            << endl
	 << "NWS_NAMESERVER_HOST = " << nwsNSHost         << " | "
	 << "NWS_NAMESERVER_PORT = " << nwsNSPort         << endl
	 << "NWS_FORECASTER_HOST = " << nwsForecasterHost << " | "
	 << "NWS_FORECASTER_PORT = " << nwsForecasterPort << endl;
  }

  Parsers::endParsing();
      
  return res;
}

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
  sf_inst_desc_t inst;
  
  fast_mutex.lock();
  unmrsh_pb_desc_to_sf(&inst, &pb, this->SrvT->getConvertor(ref));
  if ( !(fast_comp_time_best(this->localHostName, &inst, &time)) ) {
    cerr << "Warning: Cannot estimate computation time. "
	 << "Try CPU and memory load.\n";
    time = HUGE_VAL;
  }
  fast_mutex.unlock();
  CORBA::string_free(inst.path);
  delete [] inst.param_desc;
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

#if 0
/* Returns true if the data is on this server */

int SeDImpl::dataLookup(long dataID)
{
  dietServerDataDescListIterator* iter =
    new dietServerDataDescListIterator(data);

  while (iter->next()) {      
    if (((dietServerDataDescListElt*)(iter->curr()))->data->id == dataID) {
      delete(iter);
      return(1);
    }
  }

  delete(iter);
  return(0);
}


void
SeDImpl::addVar(diet_server_data_desc_t* data_desc)
{
  dietServerDataDescListElt* newData = new dietServerDataDescListElt();
  if (TRACE_LEVEL >= TRACE_ALL_STEPS)
    cout << "addVar";
  newData->data = data_desc;

  // Only update the SeD table
  data->append(newData);
  if (data_desc->id >= 0) {
    if (TRACE_LEVEL >= TRACE_ALL_STEPS)
      cout << " and forward\n";
    parent->createData(data_desc->id, childID);
  }
  cout << endl;
}

void
SeDImpl::rmVar(long dataID)
{
  /* FIXME: The parent may be updated */

  dietServerDataDescListIterator* iter =
    new dietServerDataDescListIterator(data);
  
  while (iter->next()) {
    if (((dietServerDataDescListElt*)(iter->curr()))->data->id == dataID) {
      dietServerDataDescListElt* tmp =
	(dietServerDataDescListElt*)(iter->curr());
      _CORBA_Sequence<unsigned char>::freebuf((_CORBA_Octet*)(tmp->data->data.value));
      delete(tmp->data);
      // Only updates the SeD
      tmp->del();
      delete(tmp);
      delete(iter);
      break;
    }
  }
  
  if (TRACE_LEVEL >= TRACE_MAIN_STEPS)
    cout << "Warning, trying to remove an unknown data" << endl;
  delete(iter);
}

void
SeDImpl::rmDeprecatedVars()
{
  /* FIXME: The parent may be updated */

  dietServerDataDescListIterator* iter =
    new dietServerDataDescListIterator(data);
  
  while (iter->next()) {
    dietServerDataDescListElt* tmp =
      (dietServerDataDescListElt*)(iter->curr());
    if (tmp->data->id == -1) {
      tmp->del();
      _CORBA_Sequence<unsigned char>::freebuf((_CORBA_Octet*)(tmp->data->data.value));
      delete(tmp->data);
      delete(tmp);
    }
  }
  delete(iter);
}
#endif // 0

