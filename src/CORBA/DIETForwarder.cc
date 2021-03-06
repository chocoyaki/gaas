/**
 * @file DIETForwarder.cc
 *
 * @brief   DIET forwarder implementation
 *
 * @author  Gaël Le Mahec (gael.le.mahec@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */

#include "DIETForwarder.hh"

#include "Forwarder.hh"
#include "ORBMgr.hh"
#include "common_types.hh"

#include "AgentImpl.hh"
#include "CallbackImpl.hh"
#include "LocalAgentImpl.hh"
#include "MasterAgentImpl.hh"
#include "SeDImpl.hh"

#ifdef USE_LOG_SERVICE
#include "DietLogComponent.hh"
#endif

#include "DagdaImpl.hh"

#ifdef HAVE_WORKFLOW
#include "CltWfMgr.hh"
#include "MaDag_impl.hh"
#include "WfLogServiceImpl.hh"
#endif

#include "debug.hh"

#include <stdexcept>
#include <string>
#include <cstring>
#include <list>

#include "OSIndependance.hh" // For gethostname()

#ifdef MAXHOSTNAMELEN
#define MAX_HOSTNAME_LENGTH MAXHOSTNAMELEN
#else
#define MAX_HOSTNAME_LENGTH 255
#endif


/**
 * WARNING: tricky function
 * The function return wheter the object designed by objName is local or remote
 * If the object is local, the prefix 'remote' will be added
 * so that when passed to a remote omniname, it is known as a remote object
 * When remote object, removing the remote prefix to be able to use it locally
 */
bool
DIETForwarder::remoteCall(std::string &objName) {
  // Fixme when cleaning the code, looks for a string
  // that is prefixed by remote:
  if (objName.find("remote:") != 0) {
    /* Local network call: need to be forwarded to
     * the peer forwarder. Add the prefix.
     */
    objName = "remote:" + objName;
    return false;
  }
  /* Remote network call. Remove the prefix. */
  objName = objName.substr(strlen("remote:"));
  return true;
} // remoteCall


::CORBA::Object_ptr
DIETForwarder::getObjectCache(const std::string &name) {
  std::map<std::string, ::CORBA::Object_ptr>::iterator it;

  cachesMutex.lock();
  if ((it = objectCache.find(name)) != objectCache.end()) {
    cachesMutex.unlock();
    return CORBA::Object::_duplicate(it->second);
  }
  cachesMutex.unlock();
  return ::CORBA::Object::_nil();
} // getObjectCache

DIETForwarder::DIETForwarder(const std::string &name) {
  char buffer[MAX_HOSTNAME_LENGTH + 1];
  gethostname(buffer, MAX_HOSTNAME_LENGTH);

  this->name = name;
  this->host = buffer;

  // Wait for the peer init. The unlock will be done on setPeer().
  peerMutex.lock();
}

/* DIET object factory methods. */
Agent_ptr
DIETForwarder::getAgent(const char *name) {
  std::string nm(name);
  ::CORBA::Object_ptr object;

  if (nm.find('/') == std::string::npos) {
    nm = std::string(AGENTCTXT) + "/" + nm;
  }
  object = getObjectCache(nm);
  if (!CORBA::is_nil(object)) {
    if (object->_non_existent()) {
      removeObjectFromCache(name);
    } else {
      return Agent::_duplicate(Agent::_narrow(object));
    }
  }

  AgentFwdrImpl *agent = new AgentFwdrImpl(this->_this(), nm.c_str());

  ORBMgr::getMgr()->activate(agent);

  cachesMutex.lock();
  servants[nm] = agent;
  objectCache[nm] = CORBA::Object::_duplicate(agent->_this());
  cachesMutex.unlock();

  return Agent::_duplicate(agent->_this());
} // getAgent

Callback_ptr
DIETForwarder::getCallback(const char *name) {
  std::string nm(name);
  ::CORBA::Object_ptr object;

  if (nm.find('/') == std::string::npos) {
    nm = std::string(CLIENTCTXT) + "/" + nm;
  }
  object = getObjectCache(nm);
  if (!CORBA::is_nil(object)) {
    if (object->_non_existent()) {
      removeObjectFromCache(name);
    } else {
      return Callback::_duplicate(Callback::_narrow(object));
    }
  }

  CallbackFwdrImpl *callback = new CallbackFwdrImpl(this->_this(), nm.c_str());

  ORBMgr::getMgr()->activate(callback);

  cachesMutex.lock();
  servants[nm] = callback;
  objectCache[nm] = CORBA::Object::_duplicate(callback->_this());
  cachesMutex.unlock();

  return Callback::_duplicate(callback->_this());
} // getCallback

LocalAgent_ptr
DIETForwarder::getLocalAgent(const char *name) {
  std::string nm(name);
  ::CORBA::Object_ptr object;
  if (nm.find('/') == std::string::npos) {
    nm = std::string(AGENTCTXT) + "/" + nm;
  }
  object = getObjectCache(nm);
  if (!CORBA::is_nil(object)) {
    if (object->_non_existent()) {
      removeObjectFromCache(name);
    } else {
      return LocalAgent::_duplicate(LocalAgent::_narrow(object));
    }
  }
  LocalAgentFwdrImpl *agent = new LocalAgentFwdrImpl(this->_this(), nm.c_str());

  ORBMgr::getMgr()->activate(agent);

  cachesMutex.lock();
  servants[nm] = agent;
  objectCache[nm] = CORBA::Object::_duplicate(agent->_this());
  cachesMutex.unlock();

  return LocalAgent::_duplicate(agent->_this());
} // getLocalAgent

MasterAgent_ptr
DIETForwarder::getMasterAgent(const char *name) {
  std::string nm(name);
  ::CORBA::Object_ptr object;

  if (nm.find('/') == std::string::npos) {
    nm = std::string(AGENTCTXT) + "/" + nm;
  }
  object = getObjectCache(nm);
  if (!CORBA::is_nil(object)) {
    if (object->_non_existent()) {
      removeObjectFromCache(name);
    } else {
      return MasterAgent::_duplicate(MasterAgent::_narrow(object));
    }
  }
  MasterAgentFwdrImpl *agent =
    new MasterAgentFwdrImpl(this->_this(), nm.c_str());
  ORBMgr::getMgr()->activate(agent);

  cachesMutex.lock();
  servants[nm] = agent;
  objectCache[nm] = CORBA::Object::_duplicate(agent->_this());
  cachesMutex.unlock();

  return MasterAgent::_duplicate(agent->_this());
} // getMasterAgent

SeD_ptr
DIETForwarder::getSeD(const char *name) {
  std::string nm(name);
  ::CORBA::Object_ptr object;

  if (nm.find('/') ==
      std::string::npos) {
    nm = std::string(SEDCTXT) + "/" + nm;
  }
  object = getObjectCache(nm);
  if (!CORBA::is_nil(object)) {
    if (object->_non_existent()) {
      removeObjectFromCache(name);
    } else {
      return SeD::_duplicate(SeD::_narrow(object));
    }
  }

  SeDFwdrImpl *sed = new SeDFwdrImpl(this->_this(), nm.c_str());
  ORBMgr::getMgr()->activate(sed);

  cachesMutex.lock();
  objectCache[nm] = CORBA::Object::_duplicate(sed->_this());
  servants[nm] = sed;
  cachesMutex.unlock();

  return SeD::_duplicate(sed->_this());
} // getSeD

Dagda_ptr
DIETForwarder::getDagda(const char *name) {
  std::string nm(name);
  ::CORBA::Object_ptr object;

  if (nm.find('/') == std::string::npos) {
    nm = std::string(DAGDACTXT) + "/" + nm;
  }
  object = getObjectCache(nm);
  if (!CORBA::is_nil(object)) {
    if (object->_non_existent()) {
      removeObjectFromCache(name);
    } else {
      return Dagda::_duplicate(Dagda::_narrow(object));
    }
  }
  DagdaFwdrImpl *dagda = new DagdaFwdrImpl(this->_this(), nm.c_str());
  ORBMgr::getMgr()->activate(dagda);

  cachesMutex.lock();
  servants[nm] = dagda;
  objectCache[nm] = CORBA::Object::_duplicate(dagda->_this());
  cachesMutex.unlock();

  return Dagda::_duplicate(dagda->_this());
} // getDagda

#ifdef HAVE_WORKFLOW
CltMan_ptr
DIETForwarder::getCltMan(const char *name) {
  std::string nm(name);
  ::CORBA::Object_ptr object;

  if (nm.find('/') == std::string::npos) {
    nm = std::string(WFMGRCTXT) + "/" + nm;
  }
  object = getObjectCache(nm);
  if (!CORBA::is_nil(object)) {
    if (object->_non_existent()) {
      removeObjectFromCache(name);
    } else {
      return CltManFwdr::_duplicate(CltManFwdr::_narrow(object));
    }
  }

  CltWfMgrFwdr *mgr = new CltWfMgrFwdr(this->_this(), nm.c_str());
  ORBMgr::getMgr()->activate(mgr);

  cachesMutex.lock();
  servants[nm] = mgr;
  objectCache[nm] = CORBA::Object::_duplicate(mgr->_this());
  cachesMutex.unlock();

  return CltManFwdr::_duplicate(mgr->_this());
} // getCltMan

MaDag_ptr
DIETForwarder::getMaDag(const char *name) {
  std::string nm(name);
  ::CORBA::Object_ptr object;

  if (nm.find('/') == std::string::npos) {
    nm = std::string(AGENTCTXT) + "/" + nm;
  }
  object = getObjectCache(nm);
  if (!CORBA::is_nil(object)) {
    if (object->_non_existent()) {
      removeObjectFromCache(name);
    } else {
      return MaDag::_duplicate(MaDag::_narrow(object));
    }
  }

  MaDagFwdrImpl *madag = new MaDagFwdrImpl(this->_this(), nm.c_str());
  ORBMgr::getMgr()->activate(madag);

  cachesMutex.lock();
  servants[nm] = madag;
  objectCache[nm] = CORBA::Object::_duplicate(madag->_this());
  cachesMutex.unlock();

  return MaDag::_duplicate(madag->_this());
} // getMaDag

WfLogService_ptr
DIETForwarder::getWfLogService(const char *name) {
  std::string nm(name);
  ::CORBA::Object_ptr object;

  if (nm.find('/') == std::string::npos) {
    nm = std::string(WFLOGCTXT) + "/" + nm;
  }
  object = getObjectCache(nm);
  if (!CORBA::is_nil(object)) {
    if (object->_non_existent()) {
      removeObjectFromCache(name);
    } else {
      return WfLogService::_duplicate(WfLogService::_narrow(object));
    }
  }

  WfLogServiceFwdrImpl *wfl =
    new WfLogServiceFwdrImpl(this->_this(), nm.c_str());
  ORBMgr::getMgr()->activate(wfl);

  cachesMutex.lock();
  objectCache[nm] = CORBA::Object::_duplicate(wfl->_this());
  servants[nm] = wfl;
  cachesMutex.unlock();

  return WfLogService::_duplicate(wfl->_this());
} // getWfLogService

#endif // ifdef HAVE_WORKFLOW

/* Common methods implementations. */
::CORBA::Long
DIETForwarder::ping(const char *objName) {
  std::string objString(objName);
  std::string name;
  std::string ctxt;

  if (!remoteCall(objString)) {
    return getPeer()->ping(objString.c_str());
  }

  name = getName(objString);
  ctxt = getCtxt(objString);

  if (ctxt == std::string(AGENTCTXT)) {
    Agent_var agent =
      ORBMgr::getMgr()->resolve<Agent, Agent_var>(AGENTCTXT, name, this->name);
    return agent->ping();
  }
  if (ctxt == std::string(CLIENTCTXT)) {
    Callback_var cb =
      ORBMgr::getMgr()->resolve<Callback, Callback_var>(CLIENTCTXT, name,
                                                        this->name);
    return cb->ping();
  }
#ifdef HAVE_WORKFLOW
  if (ctxt == std::string(WFMGRCTXT)) {
    CltMan_var clt =
      ORBMgr::getMgr()->resolve<CltMan, CltMan_var>(WFMGRCTXT, name,
                                                    this->name);
    return clt->ping();
  }
  if (ctxt == std::string(MADAGCTXT)) {
    MaDag_var madag =
      ORBMgr::getMgr()->resolve<MaDag, MaDag_var>(MADAGCTXT, name, this->name);
    return madag->ping();
  }
#endif // ifdef HAVE_WORKFLOW
  if (ctxt == std::string(SEDCTXT)) {
    SeD_var sed = ORBMgr::getMgr()->resolve<SeD, SeD_var>(SEDCTXT, name,
                                                          this->name);
    return sed->ping();
  }
  throw BadNameException(objString.c_str(), __FUNCTION__, getName());
} // ping

void
DIETForwarder::getRequest(const ::corba_request_t &req, const char *objName) {
  std::string objString(objName);
  std::string name;
  std::string ctxt;

  if (!remoteCall(objString)) {
    return getPeer()->getRequest(req, objString.c_str());
  }

  name = getName(objString);
  ctxt = getCtxt(objString);

  if (ctxt == std::string(AGENTCTXT)) {
    LocalAgent_var agent =
      ORBMgr::getMgr()->resolve<LocalAgent, LocalAgent_var>(ctxt, name,
                                                            this->name);
    return agent->getRequest(req);
  }
  if (ctxt == std::string(SEDCTXT)) {
    SeD_var sed =
      ORBMgr::getMgr()->resolve<SeD, SeD_var>(ctxt, name, this->name);
    return sed->getRequest(req);
  }
} // getRequest

char *
DIETForwarder::getHostname(const char *objName) {
  std::string objString(objName);
  std::string name;
  std::string ctxt;

  if (!remoteCall(objString)) {
    return getPeer()->getHostname(objString.c_str());
  }

  name = objString;

  if (name.find('/') == std::string::npos) {
    throw BadNameException(name.c_str(), __FUNCTION__, getName());
  }

  ctxt = getCtxt(objString);
  name = getName(objString);

  if (ctxt == std::string(AGENTCTXT)) {
    Agent_var agent =
      ORBMgr::getMgr()->resolve<Agent, Agent_var>(ctxt, name, this->name);
    return agent->getHostname();
  }

  if (ctxt == std::string(DAGDACTXT)) {
    Dagda_var dagda = ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(ctxt, name,
                                                                  this->name);
    return dagda->getHostname();
  }

  throw BadNameException(objString.c_str(), __FUNCTION__, getName());
} // getHostname

::CORBA::Long
DIETForwarder::bindParent(const char *parentName, const char *objName) {
  std::string objString(objName);
  std::string name;
  std::string ctxt;

  if (!remoteCall(objString)) {
    return getPeer()->bindParent(parentName, objString.c_str());
  }

  name = objString;

  if (name.find('/') == std::string::npos) {
    throw BadNameException(name.c_str(), __FUNCTION__, getName());
  }

  ctxt = getCtxt(objString);
  name = getName(objString);

  if (ctxt == std::string(AGENTCTXT)) {
    LocalAgent_var agent =
      ORBMgr::getMgr()->resolve<LocalAgent, LocalAgent_var>(ctxt, name,
                                                            this->name);
    return agent->bindParent(parentName);
  }
  if (ctxt == std::string(SEDCTXT)) {
    SeD_var sed =
      ORBMgr::getMgr()->resolve<SeD, SeD_var>(ctxt, name, this->name);
    return sed->bindParent(parentName);
  }
  throw BadNameException(objString.c_str(), __FUNCTION__, getName());
} // bindParent

::CORBA::Long
DIETForwarder::disconnect(const char *objName) {
  std::string objString(objName);
  std::string name;
  std::string ctxt;

  if (!remoteCall(objString)) {
    return getPeer()->disconnect(objString.c_str());
  }

  name = objString;

  if (name.find('/') == std::string::npos) {
    throw BadNameException(name.c_str(), __FUNCTION__, getName());
  }

  ctxt = getCtxt(objString);
  name = getName(objString);

  if (ctxt == std::string(AGENTCTXT)) {
    LocalAgent_var agent =
      ORBMgr::getMgr()->resolve<LocalAgent, LocalAgent_var>(ctxt, name,
                                                            this->name);
    return agent->disconnect();
  }
  if (ctxt == std::string(SEDCTXT)) {
    SeD_var sed = ORBMgr::getMgr()->resolve<SeD, SeD_var>(ctxt, name,
                                                          this->name);
    return sed->disconnect();
  }
  throw BadNameException(objString.c_str(), __FUNCTION__, getName());
} // disconnect

::CORBA::Long
DIETForwarder::removeElement(::CORBA::Boolean recursive, const char *objName) {
  std::string objString(objName);
  std::string name;
  std::string ctxt;

  if (!remoteCall(objString)) {
    return getPeer()->removeElement(recursive, objString.c_str());
  }

  name = objString;

  if (name.find('/') == std::string::npos) {
    throw BadNameException(name.c_str(), __FUNCTION__, getName());
  }

  ctxt = getCtxt(objString);
  name = getName(objString);

  if (ctxt == std::string(AGENTCTXT)) {
    Agent_var agent = ORBMgr::getMgr()->resolve<Agent, Agent_var>(ctxt, name,
                                                                  this->name);
    return agent->removeElement(recursive);
  }
  if (ctxt == std::string(SEDCTXT)) {
    SeD_var sed =
      ORBMgr::getMgr()->resolve<SeD, SeD_var>(ctxt, name, this->name);
    return sed->removeElement();
  }
  throw BadNameException(objString.c_str(), __FUNCTION__, getName());
} // removeElement


void
DIETForwarder::bind(const char *objName, const char *ior) {
  /* To avoid crashes when the peer forwarder is not ready: */
  /* If the peer was not initialized, the following call is blocking. */
  peerMutex.lock();
  peerMutex.unlock();

  std::string objString(objName);
  std::string name;
  std::string ctxt;

  if (!remoteCall(objString)) {
    TRACE_TEXT(TRACE_MAIN_STEPS,
               "Forward bind to peer (" << objName << ")\n");
    return getPeer()->bind(objString.c_str(), ior);
  }
  ctxt = getCtxt(objString);
  name = getName(objString);
  TRACE_TEXT(TRACE_MAIN_STEPS,
             "Bind locally (" << objString << ")\n");
  if (ctxt == LOCALAGENT) {
    ctxt = AGENTCTXT;
    /* Specific case for local agent.
       It is added into the cache to avoid resolution
       problems later.
     */
    LocalAgent_ptr agent = getLocalAgent(name.c_str());
    cachesMutex.lock();
    objectCache[ctxt + "/" + name] = CORBA::Object::_duplicate(agent);
    cachesMutex.unlock();
  } else {
    if (ctxt == MASTERAGENT) {
      ctxt = AGENTCTXT;
      /* Specific case for master agent.
         It is added into the cache to avoid resolution
         problems later.
       */
      MasterAgent_ptr agent = getMasterAgent(name.c_str());
      cachesMutex.lock();
      objectCache[ctxt + "/" + name] = CORBA::Object::_duplicate(agent);
      cachesMutex.unlock();
    }
  }
  /* NEW: Tag the object with the forwarder name. */
  std::string newIOR =
    ORBMgr::convertIOR(ior, std::string("@") + getName(), 0);

  ORBMgr::getMgr()->bind(ctxt, name, newIOR, true);
  // Broadcast the binding to all forwarders.
  ORBMgr::getMgr()->fwdsBind(ctxt, name, newIOR, this->name);
  TRACE_TEXT(TRACE_MAIN_STEPS,
             "Binded! (" << ctxt << "/" << name << ")\n");
} // bind

/* Return the local bindings. Result is a set of couple
 * (object name, object ior)
 */
SeqString *
DIETForwarder::getBindings(const char *ctxt) {
  std::list<std::string> objects;
  std::list<std::string>::iterator it;
  SeqString *result = new SeqString();
  unsigned int cmpt = 0;

  objects = ORBMgr::getMgr()->list(ctxt);
  result->length(objects.size() * 2);

  for (it = objects.begin(); it != objects.end(); ++it) {
    try {
      CORBA::Object_ptr obj =
        ORBMgr::getMgr()->resolveObject(ctxt, it->c_str(), "no-Forwarder");
      (*result)[cmpt++] = it->c_str();
      (*result)[cmpt++] = ORBMgr::getMgr()->getIOR(obj).c_str();
    } catch (const std::runtime_error &err) {
      continue;
    }
  }
  result->length(cmpt);
  return result;
} // getBindings

void
DIETForwarder::unbind(const char *objName) {
  std::string objString(objName);
  std::string name;
  std::string ctxt;

  if (!remoteCall(objString)) {
    return getPeer()->unbind(objString.c_str());
  }

  name = objString;

  if (name.find('/') == std::string::npos) {
    return;
  }

  ctxt = getCtxt(objString);
  name = getName(objString);

  removeObjectFromCache(name);

  ORBMgr::getMgr()->unbind(ctxt, name);
  // Broadcast the unbinding to all forwarders.
  ORBMgr::getMgr()->fwdsUnbind(ctxt, name, this->name);
} // unbind

void
DIETForwarder::removeObjectFromCache(const std::string &name) {
  std::map<std::string, ::CORBA::Object_ptr>::iterator it;
  std::map<std::string, PortableServer::ServantBase *>::iterator jt;
  /* If the object is in the servant cache. */
  cachesMutex.lock();
  if ((jt = servants.find(name)) != servants.end()) {
    /* - Deactivate object. */
    try {
      ORBMgr::getMgr()->deactivate(jt->second);
    } catch (...) {
      /* There was a problem with the servant. But we want
       * to remove it...
       */
    }
    /* - Remove activated servants. */
    servants.erase(jt);
  }
  /* Remove the object from the cache. */
  if ((it = objectCache.find(name)) != objectCache.end()) {
    objectCache.erase(it);
  }
  cachesMutex.unlock();
} // removeObjectFromCache

/* Remove non existing objects from the caches. */
void
DIETForwarder::cleanCaches() {
  std::map<std::string, ::CORBA::Object_ptr>::iterator it;
  std::list<std::string> invalidObjects;
  std::list<std::string>::const_iterator jt;

  cachesMutex.lock();
  for (it = objectCache.begin(); it != objectCache.end(); ++it) {
    try {
      Forwarder_var object = Forwarder::_narrow(it->second);
      object->getName();
    } catch (const CORBA::TRANSIENT &err) {
      invalidObjects.push_back(it->first);
    }
  }
  cachesMutex.unlock();
  for (jt = invalidObjects.begin(); jt != invalidObjects.end(); ++jt)
    removeObjectFromCache(*jt);
} // cleanCaches

void
DIETForwarder::connectPeer(const char *ior, const char *host,
                           const ::CORBA::Long port) {
  std::string converted = ORBMgr::convertIOR(ior, host, port);
  setPeer(ORBMgr::getMgr()->resolve<Forwarder, Forwarder_ptr>(converted));
}

void
DIETForwarder::setPeer(Forwarder_ptr peer) {
  this->peer = Forwarder::_duplicate(Forwarder::_narrow(peer));
  // Peer was init. The lock was done on the constructor.
  peerMutex.unlock();
}

Forwarder_var
DIETForwarder::getPeer() {
  // Wait for setPeer
  peerMutex.lock();
  peerMutex.unlock();
  try {
    // Check if peer is still alive
    peer->getName();
  } catch (const CORBA::COMM_FAILURE &err) {
    // Lock peerMutex, then wait for setPeer
    // (setPeer() unlock the mutex
    peerMutex.lock();
    peerMutex.lock();
    peerMutex.unlock();
  } catch (const CORBA::TRANSIENT &err) {
    // Lock peerMutex, then wait for setPeer
    // (setPeer() unlock the mutex
    peerMutex.lock();
    peerMutex.lock();
    peerMutex.unlock();
  }
  return peer;
} // getPeer


char *
DIETForwarder::getIOR() {
  return CORBA::string_dup(ORBMgr::getMgr()->getIOR(_this()).c_str());
}

char *
DIETForwarder::getName() {
  return CORBA::string_dup(name.c_str());
}

char *
DIETForwarder::getPeerName() {
  return CORBA::string_dup(getPeer()->getName());
}

char *
DIETForwarder::getHost() {
  return CORBA::string_dup(host.c_str());
}

char *
DIETForwarder::getPeerHost() {
  return CORBA::string_dup(getPeer()->getHost());
}

SeqString *
DIETForwarder::routeTree() {
  SeqString *result = new SeqString();
  return result;
}


std::list<std::string>
DIETForwarder::otherForwarders() const {
  ORBMgr *mgr = ORBMgr::getMgr();
  std::list<std::string> result = mgr->list(FWRDCTXT);

  result.remove(name);
  return result;
}

std::string
DIETForwarder::getName(const std::string &namectxt) {
  size_t pos = namectxt.find('/');
  if (pos == std::string::npos) {
    return namectxt;
  }
  return namectxt.substr(pos + 1);
}

std::string
DIETForwarder::getCtxt(const std::string &namectxt) {
  size_t pos = namectxt.find('/');
  if (pos == std::string::npos) {
    return "";
  }
  return namectxt.substr(0, pos);
}
