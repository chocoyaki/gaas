/****************************************************************************/
/* ORB manager v. 2.0 - CORBA management with DIET forwarders               */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Gaël Le Mahec (gael.le.mahec@ens-lyon.fr)                           */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.22  2010/07/14 23:45:30  bdepardo
 * Header corrections
 *
 * Revision 1.21  2010/07/12 16:11:04  glemahec
 * DIET 2.5 beta 1 - New ORB manager; dietForwarder application
 *
 ****************************************************************************/

#ifndef ORBMGR_HH
#define ORBMGR_HH
#include <string>
#include <map>
#include <list>

#include <omniORB4/CORBA.h>
#include <sys/types.h>

#include "Forwarder.hh"

#define DAGDACTXT   "Dagda"
#define LOGCOMPCTXT "LogService"
#define AGENTCTXT	  "dietAgent"
#define SEDCTXT			"dietSeD"
#define DATAMGRCTXT "dataMgrDTM"
#define LOCMGRCTXT	"locMgrDTM"
#define MADAGCTXT		"dietMADag"
#define WFMGRCTXT		"dietWfMgr"
#define CLIENTCTXT  "dietClient"
#define WFLOGCTXT		"WfLogService"
#define FWRDCTXT		"dietForwarder"
/* LOCALAGENT & MASTERAGENT are not context strings. */
#define LOCALAGENT  "localAgent" 
#define MASTERAGENT "masterAgent"


class ORBMgr {
private:
	/* The omniORB Object Request Broker for this manager. */
  CORBA::ORB_ptr ORB;
	/* The Portable Object Adaptor. */
  PortableServer::POA_var POA;
  
	/* CORBA initialization. */
  void init(CORBA::ORB_ptr ORB);

	/* Object cache to avoid to contact OmniNames too many times. */
  mutable std::map<std::string, CORBA::Object_ptr> cache;

	/* The manager instance. */
	static ORBMgr* theMgr;
public:
	/* Constructors. */
  ORBMgr(int argc, char* argv[]);
  ORBMgr(CORBA::ORB_ptr ORB);
  ORBMgr(CORBA::ORB_ptr ORB, PortableServer::POA_var POA);
  
	/* Destructor. */
  ~ORBMgr();

	/* Bind the object using its ctxt/name */
  void bind(const std::string& ctxt, const std::string& name,
            CORBA::Object_ptr object, const bool rebind = false);
	/* Bind an object using its IOR. */
  void bind(const std::string& ctxt, const std::string& name,
            const std::string& IOR, const bool rebind = false);
	/* Rebind objects. */
  void rebind(const std::string& ctxt, const std::string& name,
              CORBA::Object_ptr object);
  void rebind(const std::string& ctxt, const std::string& name,
              const std::string& IOR);
	/* Unbind an object. */
  void unbind(const std::string& ctxt, const std::string& name);
	
	/* Forwarders binding. */
	void fwdsBind(const std::string& ctxt, const std::string& name,
								const std::string& ior, const std::string& fwName = "");
  /* Forwarders unbinding. */
	void fwdsUnbind(const std::string& ctxt, const std::string& name,
									const std::string& fwName = "");
	
	/* Resolve an object using its IOR or ctxt/name. */
  CORBA::Object_ptr resolveObject(const std::string& IOR) const;
  CORBA::Object_ptr resolveObject(const std::string& ctxt, const std::string& name,
																	const std::string& fwdName = "") const;
	
	/* Get the list of the objects id binded in the omniNames server for a given context. */
	std::list<std::string> list(CosNaming::NamingContext_var& ctxt) const;
	std::list<std::string> list(const std::string& ctxtName) const;
	
	template <typename CORBA_object, typename CORBA_ptr>
	CORBA_ptr resolve(const std::string& ctxt, const std::string& name,
										const std::string& fwdName = "") const {
		return CORBA_object::_duplicate(CORBA_object::_narrow(resolveObject(ctxt, name, fwdName)));
	}
	template <typename CORBA_object, typename CORBA_ptr>
	CORBA_ptr resolve(const std::string& IOR) const {
		return CORBA_object::_duplicate(CORBA_object::_narrow(resolveObject(IOR)));
	}																
  
	/* Return the IOR of the passed object. */
	std::string getIOR(CORBA::Object_ptr object) const;
  std::string getIOR(const std::string& ctxt, const std::string& name) const;
	
	/* Activate an object. */
  void activate(PortableServer::ServantBase* object) const;
	/* Deactivate an object. */
	void deactivate(PortableServer::ServantBase* object) const;

	/* Wait for the request on activated objects. */
  void wait() const;
	
	static void init(int argc, char* argv[]);
	
	static ORBMgr* getMgr();
	
	/* IOR management functions. */
	static void makeIOR(const std::string& strIOR, IOP::IOR& ior);
	static void makeString(const IOP::IOR& ior, std::string& strIOR);
	static std::string getHost(IOP::IOR& ior);
	static std::string getHost(const std::string& strIOR);
	static unsigned int getPort(IOP::IOR& ior);
	static unsigned int getPort(const std::string& strIOR);
	static std::string getTypeID(IOP::IOR& ior);
	static std::string getTypeID(const std::string& strIOR);
	static std::string convertIOR(IOP::IOR& ior, const std::string& host,
																const unsigned int port);
	static std::string convertIOR(const std::string& ior, const std::string& host,
																const unsigned int port);
};


#endif
