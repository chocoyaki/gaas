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
 * Revision 1.1  2003/04/10 13:15:05  pcombes
 * Replace SeD_impl.hh. Add checkContract method.
 *
 ****************************************************************************/


#ifndef _SEDIMPL_HH_
#define _SEDIMPL_HH_

#include "SeD.hh"
#include "Agent.hh"
#include "common_types.hh"
#include "response.hh"

#include "dietTypes.hh"
#include "ServiceTable.hh"


/****************************************************************************/
/* SeD class                                                                */
/****************************************************************************/


class SeDImpl : public POA_SeD,
		public PortableServer::RefCountServantBase
{

public:                                              

  SeDImpl(int SeDPort);
  ~SeDImpl();
  
  int
  run(char* configFileName, ServiceTable* services);
  
  virtual void
  getRequest(const corba_request_t& req);

  virtual CORBA::Long
  checkContract(corba_estimation_t& estimation,
		const corba_pb_desc_t& pb);

  virtual CORBA::Long
  solve(const char* pbName, corba_profile_t& pb);

  virtual CORBA::Long
  ping();


private:

  /* SeD port */
  int port;
  
  int childID;
  Agent_var parent;

  /* (Fully qualified) local host name */
  char localHostName[257];

  /* Service table */
  ServiceTable* SrvT;
#if 0
  /* List of the variables held by the server */
  dietServerDataDescList* data;
#endif //0
  /* Trace level */
  //unsigned int traceLevel;

#if HAVE_FAST
  /* Fast calls mutex, this should be used until FAST becomes reentrant : */
  omni_mutex fast_mutex;
#endif // HAVE_FAST

  /* LDAP and NWS parameters for FAST
     They are not conditioned by HAVE_FAST, because they can be initialized
     in the configuration file. */
  int  ldapUse;
  char ldapHost[257];
  int  ldapPort;
  char ldapMask[257];
  int  nwsUse;
  char nwsNSHost[257];
  int  nwsNSPort;
  char nwsForecasterHost[257];
  int  nwsForecasterPort;


  /**************************************************************************/
  /* Private methods                                                        */
  /**************************************************************************/

  int
  parseConfigFile(char* configFileName, char* parentName);

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
