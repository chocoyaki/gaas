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
 * Revision 1.11  2004/07/29 18:52:11  rbolze
 * Change solve function now , DIET_client send the reqID of the request when
 * he call the solve function.
 * Nothing is change for DIET's API
 *
 * Revision 1.10  2004/07/05 14:56:13  rbolze
 * correct bug on 64 bit plat-form, when parsing cfg file :
 * remplace size_t by unsigned int for config options
 *
 * Revision 1.9  2004/06/11 15:45:39  ctedesch
 * add DIET/JXTA
 *
 * Revision 1.8  2004/03/01 18:43:23  rbolze
 * add logservice
 *
 * Revision 1.7  2003/12/01 14:49:30  pcombes
 * Rename dietTypes.hh to DIET_data_internal.hh, for more coherency.
 *
 * Revision 1.6  2003/09/22 21:17:54  pcombes
 * Set all the modules and their interfaces for data persistency.
 *
 * Revision 1.4  2003/06/23 13:35:06  pcombes
 * useAsyncAPI should be replaced by a "useBiDir" option. Remove it so far.
 *
 * Revision 1.3  2003/06/02 09:06:46  cpera
 * Beta version of asynchronize DIET API.
 *
 * Revision 1.2  2003/05/10 08:54:41  pcombes
 * New format for configuration files, new Parsers.
 *
 * Revision 1.1  2003/04/10 13:15:05  pcombes
 * Replace SeD_impl.hh. Add checkContract method.
 ****************************************************************************/


#ifndef _SEDIMPL_HH_
#define _SEDIMPL_HH_

#include "SeD.hh"

#include "Agent.hh"
#include "common_types.hh"
#include "ChildID.hh"
#include "Counter.hh"
#include "DataMgrImpl.hh"
#include "DIET_data_internal.hh"
#include "response.hh"
#include "ServiceTable.hh"

#if HAVE_LOGSERVICE
#include "DietLogComponent.hh"
#endif


/****************************************************************************/
/* SeD class                                                                */
/****************************************************************************/

class SeDImpl : public POA_SeD,
		public PortableServer::RefCountServantBase
{

public:                                              

  SeDImpl();
#if HAVE_JXTA
  SeDImpl(const char*);
#endif // HAVE_JXTA
  ~SeDImpl();
  
  int
  run(ServiceTable* services);
  
  /** Set this->dataMgr */
  int
  linkToDataMgr(DataMgrImpl* dataMgr);

#if HAVE_LOGSERVICE
  /**
   * Set the DietLogComponent of this SeD. If this function is not
   * called or the parameter is NULL, no monitoring information is
   * gathered.
   */
  void
  setDietLogComponent(DietLogComponent* dietLogComponent);
#endif

  virtual void
  getRequest(const corba_request_t& req);

  virtual CORBA::Long
  checkContract(corba_estimation_t& estimation,
		const corba_pb_desc_t& pb);

  virtual CORBA::Long
  solve(const char* pbName, corba_profile_t& pb,CORBA::Long reqID);

  virtual void
  solveAsync(const char* pb_name, const corba_profile_t& pb,
	    CORBA::Long reqID, const char * volatileclientIOR);

  virtual CORBA::Long
  ping();


private:
  
  /** Reference of the parent */
  Agent_var parent;
  /** ID of this agent amongst the children of its parent */
  ChildID childID;

  /* (Fully qualified) local host name */
  char localHostName[257];

#if HAVE_JXTA
  /* endoint of JXTA SeD*/
  const char* uuid;
#endif // HAVE_JXTA

  /* Listening port */
  // size_t --> unsigned int
  unsigned int port;

  /* Service table */
  ServiceTable* SrvT;

  /* Data Manager associated to this SeD */
  DataMgrImpl* dataMgr;

#if HAVE_FAST

  /** Use of FAST */
  // size_t --> unsigned int
  unsigned int fastUse;

  /* Fast calls mutex, this should be used until FAST becomes reentrant */
  omni_mutex fastMutex;

#endif // HAVE_FAST

#if HAVE_LOGSERVICE
  /**
   * The actual dietLogComponent of this SeD. If it contains NULL,
   * no monitoring information must be gathered, so it must be checked
   * each time before it is used.
   */
  DietLogComponent* dietLogComponent;
#endif


  /**************************************************************************/
  /* Private methods                                                        */
  /**************************************************************************/



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
