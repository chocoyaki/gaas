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
 * Revision 1.27  2011/05/13 06:42:22  bdepardo
 * Const methods whenever possible
 *
 * Revision 1.26  2011/02/15 16:18:24  bdepardo
 * Go back to the old signal handle with semaphores. I did not find any better
 * idea.
 *
 * Revision 1.25  2010/11/10 02:41:23  kcoulomb
 * Small modifications to use the log service (LogService divided in 2 separated contexts, one for components and one for tools)
 *
 * Revision 1.24  2010/11/02 05:53:18  bdepardo
 * Correct a bug preventing the SeDs from connecting to an LA through the
 * forwarders: when resolving an agent we first try to resolve MA or LA
 * depending on the context, but if this fails, then we fall back to resolving
 * a general agent.
 *
 * Revision 1.23  2010/07/27 10:24:32  glemahec
 * Improve robustness & general performance
 *
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

#include <omnithread.h>

#include "Forwarder.hh"

#define DAGDACTXT   "Dagda"
#define LOGCOMPCTXT "LogServiceC"
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
public:  
  /* Constructors. */
  ORBMgr(int argc, char* argv[]);
  ORBMgr(CORBA::ORB_ptr ORB);
  ORBMgr(CORBA::ORB_ptr ORB, PortableServer::POA_var POA);
  
  /* Destructor. */
  ~ORBMgr();

  /* Bind the object using its ctxt/name */
  void bind(const std::string& ctxt, const std::string& name,
            CORBA::Object_ptr object, const bool rebind = false) const;
  /* Bind an object using its IOR. */
  void bind(const std::string& ctxt, const std::string& name,
            const std::string& IOR, const bool rebind = false) const;
  /* Rebind objects. */
  void rebind(const std::string& ctxt, const std::string& name,
              CORBA::Object_ptr object) const;
  void rebind(const std::string& ctxt, const std::string& name,
              const std::string& IOR) const;
  /* Unbind an object. */
  void unbind(const std::string& ctxt, const std::string& name) const;
	
  /* Forwarders binding. */
  void fwdsBind(const std::string& ctxt, const std::string& name,
		const std::string& ior, const std::string& fwName = "") const;
  /* Forwarders unbinding. */
  void fwdsUnbind(const std::string& ctxt, const std::string& name,
		  const std::string& fwName = "") const;
	
  /* Resolve an object using its IOR or ctxt/name. */
  CORBA::Object_ptr resolveObject(const std::string& IOR) const;
  CORBA::Object_ptr resolveObject(const std::string& ctxt, const std::string& name,
                                  const std::string& fwdName = "") const;
  /* Resolve objects without object caching or invoking forwarders. */
  CORBA::Object_ptr simpleResolve(const std::string& ctxt, const std::string& name) const;
	
  /* Get the list of the objects id binded in the omniNames server for a given context. */
  std::list<std::string> list(CosNaming::NamingContext_var& ctxt) const;
  std::list<std::string> list(const std::string& ctxtName) const;
  /* Get the list of declared CORBA contexts. */
  std::list<std::string> contextList() const;
  /* Return true if the object is local, false if it is reachable through a forwarder. */
  bool isLocal(const std::string& ctxt, const std::string& name) const;
  /* Return the name of the forwarder that manage the object. */
  std::string forwarderName(const std::string& ctxt, const std::string& name) const;
	
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
  void shutdown(bool waitForCompletion);

  static void init(int argc, char* argv[]);
	
  static ORBMgr* getMgr();
  
  std::list<std::string> forwarderObjects(const std::string& fwdName,
                                          const std::string& ctxt) const;
  std::list<std::string> localObjects(const std::string& ctxt) const;
	
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
	
  /* Object cache management functions. */
  void resetCache() const;
  void removeObjectFromCache(const std::string& name) const;
  void removeObjectFromCache(const std::string& ctxt,
			     const std::string& name) const;
  void cleanCache() const;
  static void hexStringToBuffer(const char* ptr, const size_t size,
				cdrMemoryStream& buffer);

private:
  /* The omniORB Object Request Broker for this manager. */
  CORBA::ORB_ptr ORB;
  /* The Portable Object Adaptor. */
  PortableServer::POA_var POA;
  
  /* Is the ORB down? */
  bool down;
  

  /* CORBA initialization. */
  void init(CORBA::ORB_ptr ORB);

  /* Object cache to avoid to contact OmniNames too many times. */
  mutable std::map<std::string, CORBA::Object_ptr> cache;
  /* Cache mutex. */
  mutable omni_mutex cacheMutex;

  /* The manager instance. */
  static ORBMgr* theMgr;
  

  static void
  sigIntHandler(int sig);
#ifndef __cygwin__
  static omni_mutex waitLock;
#else
  static sem_t waitLock;
#endif

};


#endif
