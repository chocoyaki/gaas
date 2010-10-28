/****************************************************************************/
/* DIET forwarder implementation                                            */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Gael Le Mahec   (gael.le.mahec@ens-lyon.fr)                         */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.10  2010/10/28 06:53:49  bdepardo
 * Bind messages are more explicit
 *
 * Revision 1.9  2010/07/27 16:16:48  glemahec
 * Forwarders robustness
 *
 * Revision 1.8  2010/07/27 13:25:01  glemahec
 * Forwarders robustness improvements
 *
 * Revision 1.7  2010/07/27 10:24:31  glemahec
 * Improve robustness & general performance
 *
 * Revision 1.6  2010/07/20 14:31:45  glemahec
 * Forwarder robustness + bug corrections
 *
 * Revision 1.5  2010/07/20 08:56:22  bisnard
 * Updated WfLogService IDL
 *
 * Revision 1.4  2010/07/14 23:45:30  bdepardo
 * Header corrections
 *
 * Revision 1.3  2010/07/14 22:43:26  glemahec
 * Remove WAITPEERINIT macro
 *
 * Revision 1.2  2010/07/13 15:24:13  glemahec
 * Warnings corrections and some robustness improvements
 *
 * Revision 1.1  2010/07/12 16:11:03  glemahec
 * DIET 2.5 beta 1 - New ORB manager; dietForwarder application
 ****************************************************************************/

#include "DIETForwarder.hh"

#include "Forwarder.hh"
#include "ORBMgr.hh"
#include "common_types.hh"

#include "AgentImpl.hh"
#include "CallbackImpl.hh"
#include "LocalAgentImpl.hh"
#include "DietLogComponent.hh"
#include "MasterAgentImpl.hh"
#include "SeDImpl.hh"
#ifndef HAVE_DAGDA
#include "DataMgrImpl.hh"
#include "LocMgrImpl.hh"
#else
#include "DagdaImpl.hh"
#endif

#ifdef HAVE_WORKFLOW
#include "CltWfMgr.hh"
#include "MaDag_impl.hh"
#include "WfLogServiceImpl.hh"
#endif

#include <stdexcept>
#include <string>
#include <cstring>
#include <list>

using namespace std;

bool DIETForwarder::remoteCall(string& objName) {
  if (objName.find("remote:")!=0) {
    /* Local network call: need to be forwarded to
     * the peer forwarder. Add the prefix.
     */
    objName = "remote:"+objName;
    return false;
  }
  /* Remote network call. Remove the prefix. */
  objName = objName.substr(strlen("remote:"));
  return true;
}


::CORBA::Object_ptr DIETForwarder::getObjectCache(const std::string& name) {
  map<string, ::CORBA::Object_ptr>::iterator it;
	
  cachesMutex.lock();
  if ((it=objectCache.find(name))!=objectCache.end()) {
    cachesMutex.unlock();
    return CORBA::Object::_duplicate(it->second);
  }
  cachesMutex.unlock();
  return ::CORBA::Object::_nil();	
}

DIETForwarder::DIETForwarder(const string& name, const string& cfgPath) :
  netCfg(cfgPath)
{
  this->name = name;
  // Wait for the peer init. The unlock will be done on setPeer().
  peerMutex.lock(); 
}

/* DIET object factory methods. */
Agent_ptr DIETForwarder::getAgent(const char* name) {
  size_t pos;
  string nm(name);
  ::CORBA::Object_ptr object;
	
  if ((pos=nm.find('/'))==string::npos) {
    nm = string(AGENTCTXT)+"/"+nm;
  }
  object = getObjectCache(nm);
  if (!CORBA::is_nil(object)) {
    if (object->_non_existent()) {
      removeObjectFromCache(name);
    } else
      return Agent::_duplicate(Agent::_narrow(object));
  }
	
  AgentFwdrImpl * agent = new AgentFwdrImpl(this->_this(), nm.c_str());
	
  ORBMgr::getMgr()->activate(agent);
	
  cachesMutex.lock();
  servants[nm] = agent;
  objectCache[nm] = CORBA::Object::_duplicate(agent->_this());
  cachesMutex.unlock();
	
  return Agent::_duplicate(agent->_this());
}

Callback_ptr DIETForwarder::getCallback(const char* name)
{
  size_t pos;
  string nm(name);
  ::CORBA::Object_ptr object;
		
  if ((pos=nm.find('/'))==string::npos) {
    nm = string(CLIENTCTXT)+"/"+nm;
  }
  object = getObjectCache(nm);
  if (!CORBA::is_nil(object)) {
    if (object->_non_existent()) {
      removeObjectFromCache(name);
    } else
      return Callback::_duplicate(Callback::_narrow(object));
  }
	
  CallbackFwdrImpl * callback = new CallbackFwdrImpl(this->_this(), nm.c_str());

  ORBMgr::getMgr()->activate(callback);
	
  cachesMutex.lock();
  servants[nm] = callback;
  objectCache[nm] = CORBA::Object::_duplicate(callback->_this());
  cachesMutex.unlock();
	
  return Callback::_duplicate(callback->_this());
}

LocalAgent_ptr DIETForwarder::getLocalAgent(const char* name)
{
  size_t pos;
  string nm(name);
  ::CORBA::Object_ptr object;
  if ((pos=nm.find('/'))==string::npos) {
    nm = string(AGENTCTXT)+"/"+nm;
  }
  object = getObjectCache(nm);
  if (!CORBA::is_nil(object)) {
    if (object->_non_existent()) {
      removeObjectFromCache(name);
    } else
      return LocalAgent::_duplicate(LocalAgent::_narrow(object));
  }
  LocalAgentFwdrImpl * agent = new LocalAgentFwdrImpl(this->_this(), nm.c_str());

  ORBMgr::getMgr()->activate(agent);
	
  cachesMutex.lock();
  servants[nm] = agent;
  objectCache[nm] = CORBA::Object::_duplicate(agent->_this());
  cachesMutex.unlock();
	
  return LocalAgent::_duplicate(agent->_this());
}

/*DietLogComponent_ptr DIETForwarder::getLogComponent(const char* name)
  {
  size_t pos;
  string nm(name);
  ::CORBA::Object_ptr object;
	
  if ((pos=nm.find('/'))==string::npos) {
  nm = string(AGENTCTXT)+"/"+nm;
  }
  object = getObjectCache(nm);
  if (!CORBA::is_nil(object)) {
  if (object->_non_existent()) {
  removeObjectFromCache(name);
  } else
  return ComponentConfigurator::_duplicate(ComponentConfigurator::_narrow(object));
  }
	
  DietLogComponentFwdr * comp = new DietLogComponentFwdr(this->_this(), nm.c_str());
  ORBMgr::getMgr()->activate(comp);
 
  cachesMutex.lock();
  servants[nm] = comp;
  objectCache[name] = CORBA::Object::_duplicate(comp->_this());
  cachesMutex.unlock();

  return ComponentConfigurator::_duplicate(comp->_this());
	
  }*/

/*LogCentralComponent_ptr DIETForwarder::getLogCentralComponent(const char* name)
  {
	
  }*/

MasterAgent_ptr DIETForwarder::getMasterAgent(const char* name)
{
  size_t pos;
  string nm(name);
  ::CORBA::Object_ptr object;
		
  if ((pos=nm.find('/'))==string::npos) {
    nm = string(AGENTCTXT)+"/"+nm;
  }
  object = getObjectCache(nm);
  if (!CORBA::is_nil(object)) {
    if (object->_non_existent()) {
      removeObjectFromCache(name);
    } else
      return MasterAgent::_duplicate(MasterAgent::_narrow(object));
  }
  MasterAgentFwdrImpl * agent = new MasterAgentFwdrImpl(this->_this(), nm.c_str());
  ORBMgr::getMgr()->activate(agent);
	
  cachesMutex.lock();
  servants[nm] = agent;
  objectCache[nm] = CORBA::Object::_duplicate(agent->_this());
  cachesMutex.unlock();
	
  return MasterAgent::_duplicate(agent->_this());
}

SeD_ptr DIETForwarder::getSeD(const char* name)
{
  size_t pos;
  string nm(name);
  ::CORBA::Object_ptr object;
		
  if ((pos=nm.find('/'))==string::npos) {
    nm = string(SEDCTXT)+"/"+nm;
  }
  object = getObjectCache(nm);
  if (!CORBA::is_nil(object)) {
    if (object->_non_existent()) {
      removeObjectFromCache(name);
    } else
      return SeD::_duplicate(SeD::_narrow(object));
  }
	
  SeDFwdrImpl * sed = new SeDFwdrImpl(this->_this(), nm.c_str());
  ORBMgr::getMgr()->activate(sed);
	
  cachesMutex.lock();
  objectCache[nm] = CORBA::Object::_duplicate(sed->_this());
  servants[nm] = sed;
  cachesMutex.unlock();

  return SeD::_duplicate(sed->_this());
}

#ifdef HAVE_DAGDA
Dagda_ptr DIETForwarder::getDagda(const char* name)
{
  size_t pos;
  string nm(name);
  ::CORBA::Object_ptr object;
		
  if ((pos=nm.find('/'))==string::npos) {
    nm = string(DAGDACTXT)+"/"+nm;
  }
  object = getObjectCache(nm);
  if (!CORBA::is_nil(object)) {
    if (object->_non_existent()) {
      removeObjectFromCache(name);
    } else
      return Dagda::_duplicate(Dagda::_narrow(object));
  }
  DagdaFwdrImpl * dagda = new DagdaFwdrImpl(this->_this(), nm.c_str());
  ORBMgr::getMgr()->activate(dagda);
	
  cachesMutex.lock();
  servants[nm] = dagda;
  objectCache[nm] = CORBA::Object::_duplicate(dagda->_this());
  cachesMutex.unlock();
	
  return Dagda::_duplicate(dagda->_this());
}

#else
DataMgr_ptr DIETForwarder::getDataMgr(const char* name)
{
  size_t pos;
  string nm(name);
  ::CORBA::Object_ptr object;
	
  if ((pos=nm.find('/'))==string::npos) {
    nm = string(DATAMGRCTXT)+"/"+nm;
  }
  object = getObjectCache(nm);
  if (!CORBA::is_nil(object)) {
    if (object->_non_existent()) {
      removeObjectFromCache(name);
    } else
      return DataMgr::_duplicate(DataMgr::_narrow(object));
  }
	
  DataMgrFwdrImpl * dtm = new DataMgrFwdrImpl(this->_this(), nm.c_str());
  ORBMgr::getMgr()->activate(dtm);
	
  cachesMutex.lock();
  servants[nm] = dtm;
  objectCache[nm] = CORBA::Object::_duplicate(dtm->_this());
  cachesMutex.unlock();
	
  return DataMgr::_duplicate(dtm->_this());
}

LocMgr_ptr DIETForwarder::getLocMgr(const char* name)
{
  size_t pos;
  string nm(name);
  ::CORBA::Object_ptr object;
	
  if ((pos=nm.find('/'))==string::npos) {
    nm = string(LOCMGRCTXT)+"/"+nm;
  }
  object = getObjectCache(nm);
  if (!CORBA::is_nil(object)) {
    if (object->_non_existent()) {
      removeObjectFromCache(name);
    } else
      return LocMgr::_duplicate(LocMgr::_narrow(object));
  }
	
  LocMgrFwdrImpl * dtm = new LocMgrFwdrImpl(this->_this(), nm.c_str());
  ORBMgr::getMgr()->activate(dtm);
	
  cachesMutex.lock();
  servants[nm] = dtm;
  objectCache[nm] = CORBA::Object::_duplicate(dtm->_this());
  cachesMutex.unlock();
	
  return LocMgr::_duplicate(dtm->_this());
}

#endif
#ifdef HAVE_WORKFLOW
CltMan_ptr DIETForwarder::getCltMan(const char* name)
{
  size_t pos;
  string nm(name);
  ::CORBA::Object_ptr object;
	
  if ((pos=nm.find('/'))==string::npos) {
    nm = string(WFMGRCTXT)+"/"+nm;
  }
  object = getObjectCache(nm);
  if (!CORBA::is_nil(object)) {
    if (object->_non_existent()) {
      removeObjectFromCache(name);
    } else
      return CltManFwdr::_duplicate(CltManFwdr::_narrow(object));
  }
	
  CltWfMgrFwdr * mgr = new CltWfMgrFwdr(this->_this(), nm.c_str());
  ORBMgr::getMgr()->activate(mgr);
	
  cachesMutex.lock();
  servants[nm] = mgr;
  objectCache[nm] = CORBA::Object::_duplicate(mgr->_this());
  cachesMutex.unlock();
	
  return CltManFwdr::_duplicate(mgr->_this());
}

MaDag_ptr DIETForwarder::getMaDag(const char* name)
{
  size_t pos;
  string nm(name);
  ::CORBA::Object_ptr object;
	
  if ((pos=nm.find('/'))==string::npos) {
    nm = string(AGENTCTXT)+"/"+nm;
  }
  object = getObjectCache(nm);
  if (!CORBA::is_nil(object)) {
    if (object->_non_existent()) {
      removeObjectFromCache(name);
    } else
      return MaDag::_duplicate(MaDag::_narrow(object));
  }
	
  MaDagFwdrImpl * madag = new MaDagFwdrImpl(this->_this(), nm.c_str());
  ORBMgr::getMgr()->activate(madag);
	
  cachesMutex.lock();
  servants[nm] = madag;
  objectCache[nm] = CORBA::Object::_duplicate(madag->_this());
  cachesMutex.unlock();
	
  return MaDag::_duplicate(madag->_this());
}

WfLogService_ptr DIETForwarder::getWfLogService(const char* name)
{
  size_t pos;
  string nm(name);
  ::CORBA::Object_ptr object;
	
  if ((pos=nm.find('/'))==string::npos) {
    nm = string(WFLOGCTXT)+"/"+nm;
  }
  object = getObjectCache(nm);
  if (!CORBA::is_nil(object)) {
    if (object->_non_existent()) {
      removeObjectFromCache(name);
    } else
      return WfLogService::_duplicate(WfLogService::_narrow(object));
  }
	
  WfLogServiceFwdrImpl * wfl = new WfLogServiceFwdrImpl(this->_this(), nm.c_str());
  ORBMgr::getMgr()->activate(wfl);
	
  cachesMutex.lock();
  objectCache[nm] = CORBA::Object::_duplicate(wfl->_this());
  servants[nm] = wfl;
  cachesMutex.unlock();
	
  return WfLogService::_duplicate(wfl->_this());
}

#endif

/* Common methods implementations. */
::CORBA::Long DIETForwarder::ping(const char* objName)
{
  string objString(objName);
  string name;
  string ctxt;
	
  if (!remoteCall(objString)) {
    return getPeer()->ping(objString.c_str());
  }
	
  name = getName(objString);
  ctxt = getCtxt(objString);
	
  if (ctxt==string(AGENTCTXT)) {
    Agent_var agent = ORBMgr::getMgr()->resolve<Agent, Agent_var>(AGENTCTXT, name,
								  this->name);
    return agent->ping();
  }
  if (ctxt==string(CLIENTCTXT)) {
    Callback_var cb = ORBMgr::getMgr()->resolve<Callback, Callback_var>(CLIENTCTXT, name,
									this->name);
    return cb->ping();
  }
#ifdef HAVE_WORKFLOW
  if (ctxt==string(WFMGRCTXT)) {
    CltMan_var clt = ORBMgr::getMgr()->resolve<CltMan, CltMan_var>(WFMGRCTXT, name,
								   this->name);
    return clt->ping();
  }
  if (ctxt==string(MADAGCTXT)) {
    MaDag_var madag = ORBMgr::getMgr()->resolve<MaDag, MaDag_var>(MADAGCTXT, name,
								  this->name);
    return madag->ping();
  }
#endif
  if (ctxt==string(SEDCTXT)) {
    SeD_var sed = ORBMgr::getMgr()->resolve<SeD, SeD_var>(SEDCTXT, name,
							  this->name);
    return sed->ping();
  }
  throw BadNameException(objString.c_str(), __FUNCTION__, getName());
}

void DIETForwarder::getRequest(const ::corba_request_t& req, const char* objName)
{
  string objString(objName);
  string name;
  string ctxt;
	
  if (!remoteCall(objString)) {
    return getPeer()->getRequest(req, objString.c_str());
  }
	
  name = getName(objString);
  ctxt = getCtxt(objString);
	
  if (ctxt==string(AGENTCTXT)) {
    LocalAgent_var agent = ORBMgr::getMgr()->resolve<LocalAgent, LocalAgent_var>(ctxt, name,
										 this->name);
    return agent->getRequest(req);
  }
  if (ctxt==string(SEDCTXT)) {
    SeD_var sed = ORBMgr::getMgr()->resolve<SeD, SeD_var>(ctxt, name,
							  this->name);
    return sed->getRequest(req);
  }
  /* oneway CORBA function. Can't throw anything. */
  //throw BadNameException(objString.c_str(), __FUNCTION__, getName());
}

char* DIETForwarder::getHostname(const char* objName)
{
  string objString(objName);
  string name;
  string ctxt;
	
  if (!remoteCall(objString)) {
    return getPeer()->getHostname(objString.c_str());
  }
	
  name = objString;
	
  if (name.find('/')==string::npos)
    throw BadNameException(name.c_str(), __FUNCTION__, getName());

  ctxt = getCtxt(objString);
  name = getName(objString);
	
  if (ctxt==string(AGENTCTXT)) {
    Agent_var agent = ORBMgr::getMgr()->resolve<Agent, Agent_var>(ctxt, name,
								  this->name);
    return agent->getHostname();
  }
#ifdef HAVE_DAGDA
  if (ctxt==string(DAGDACTXT)) {
    Dagda_var dagda = ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(ctxt, name,
								  this->name);
    return dagda->getHostname();
  }
#endif
  throw BadNameException(objString.c_str(), __FUNCTION__, getName());
}

#ifdef HAVE_DYNAMICS
::CORBA::Long DIETForwarder::bindParent(const char* parentName, const char* objName)
{
  string objString(objName);
  string name;
  string ctxt;
	
  if (!remoteCall(objString)) {
    return getPeer()->bindParent(parentName, objString.c_str());
  }
	
  name = objString;
	
  if (name.find('/')==string::npos)
    throw BadNameException(name.c_str(), __FUNCTION__, getName());

  ctxt = getCtxt(objString);
  name = getName(objString);
	
  if (ctxt==string(AGENTCTXT)) {
    LocalAgent_var agent = ORBMgr::getMgr()->resolve<LocalAgent, LocalAgent_var>(ctxt, name,
										 this->name);
    return agent->bindParent(parentName);
  }
  if (ctxt==string(SEDCTXT)) {
    SeD_var sed = ORBMgr::getMgr()->resolve<SeD, SeD_var>(ctxt, name,
							  this->name);
    return sed->bindParent(parentName);
  }
  throw BadNameException(objString.c_str(), __FUNCTION__, getName());
}

::CORBA::Long DIETForwarder::disconnect(const char* objName)
{
  string objString(objName);
  string name;
  string ctxt;
	
  if (!remoteCall(objString)) {
    return getPeer()->disconnect(objString.c_str());
  }
	
  name = objString;
	
  if (name.find('/')==string::npos)
    throw BadNameException(name.c_str(), __FUNCTION__, getName());

  ctxt = getCtxt(objString);
  name = getName(objString);
	
  if (ctxt==string(AGENTCTXT)) {
    LocalAgent_var agent = ORBMgr::getMgr()->resolve<LocalAgent, LocalAgent_var>(ctxt, name,
										 this->name);
    return agent->disconnect();
  }
  if (ctxt==string(SEDCTXT)) {
    SeD_var sed = ORBMgr::getMgr()->resolve<SeD, SeD_var>(ctxt, name,
							  this->name);
    return sed->disconnect();
  }
  throw BadNameException(objString.c_str(), __FUNCTION__, getName());
}

::CORBA::Long DIETForwarder::removeElement(::CORBA::Boolean recursive, const char* objName)
{
  string objString(objName);
  string name;
  string ctxt;
	
  if (!remoteCall(objString)) {
    return getPeer()->removeElement(recursive, objString.c_str());
  }
	
  name = objString;
	
  if (name.find('/')==string::npos)
    throw BadNameException(name.c_str(), __FUNCTION__, getName());

  ctxt = getCtxt(objString);
  name = getName(objString);
	
  if (ctxt==string(AGENTCTXT)) {
    Agent_var agent = ORBMgr::getMgr()->resolve<Agent, Agent_var>(ctxt, name,
								  this->name);
    return agent->removeElement(recursive);
  }
  if (ctxt==string(SEDCTXT)) {
    SeD_var sed = ORBMgr::getMgr()->resolve<SeD, SeD_var>(ctxt, name,
							  this->name);
    return sed->removeElement();
  }
  throw BadNameException(objString.c_str(), __FUNCTION__, getName());
}
#endif

#ifndef HAVE_DAGDA
char* DIETForwarder::whereData(const char* argID,
			       const char* objName)
{
  string objString(objName);
  string name;
  string ctxt;
	
  if (!remoteCall(objString)) {
    return getPeer()->whereData(argID, objString.c_str());
  }
	
  ctxt = getCtxt(objString);
  name = getName(objString);
	
  if (ctxt==LOCMGRCTXT) {
    LocMgr_var mgr = ORBMgr::getMgr()->resolve<LocMgr, LocMgr_var>(LOCMGRCTXT, name,
								   this->name);
    return mgr->whereData(argID);
  }
  if (ctxt==DATAMGRCTXT) {
    DataMgr_var mgr = ORBMgr::getMgr()->resolve<DataMgr, DataMgr_var>(DATAMGRCTXT, name,
								      this->name);
    return mgr->whereData(argID);
  }
  throw BadNameException(objString.c_str(), __FUNCTION__, getName());
}

char* DIETForwarder::setMyName(const char* objName) {
  string objString(objName);
  string name;
  string ctxt;
	
  if (!remoteCall(objString)) {
    return getPeer()->setMyName(objString.c_str());
  }
	
  ctxt = getCtxt(objString);
  name = getName(objString);
	
  if (ctxt==LOCMGRCTXT) {
    LocMgr_var mgr = ORBMgr::getMgr()->resolve<LocMgr, LocMgr_var>(LOCMGRCTXT, name,
								   this->name);
    return mgr->setMyName();
  }
  if (ctxt==DATAMGRCTXT) {
    DataMgr_var mgr = ORBMgr::getMgr()->resolve<DataMgr, DataMgr_var>(DATAMGRCTXT, name,
								      this->name);
    return mgr->setMyName();
  }
  throw BadNameException(objString.c_str(), __FUNCTION__, getName());
}

char* DIETForwarder::whichSeDOwner(const char* argID,
				   const char* objName)
{
  string objString(objName);
  string name;
  string ctxt;
	
  if (!remoteCall(objString)) {
    return getPeer()->whichSeDOwner(argID, objString.c_str());
  }
	
  ctxt = getCtxt(objString);
  name = getName(objString);
	
  if (ctxt==LOCMGRCTXT) {
    LocMgr_var mgr = ORBMgr::getMgr()->resolve<LocMgr, LocMgr_var>(LOCMGRCTXT, name,
								   this->name);
    return mgr->whichSeDOwner(argID);
  }
  if (ctxt==DATAMGRCTXT) {
    DataMgr_var mgr = ORBMgr::getMgr()->resolve<DataMgr, DataMgr_var>(DATAMGRCTXT, name,
								      this->name);
    return mgr->whichSeDOwner(argID);
  }
  throw BadNameException(objString.c_str(), __FUNCTION__, getName());
}

void DIETForwarder::printList(const char* objName) {
  string objString(objName);
  string name;
  string ctxt;
	
  if (!remoteCall(objString)) {
    return getPeer()->printList(objString.c_str());
  }
	
  ctxt = getCtxt(objString);
  name = getName(objString);
	
  if (ctxt==LOCMGRCTXT) {
    LocMgr_var mgr = ORBMgr::getMgr()->resolve<LocMgr, LocMgr_var>(LOCMGRCTXT, name,
								   this->name);
    return mgr->printList();
  }
  if (ctxt==DATAMGRCTXT) {
    DataMgr_var mgr = ORBMgr::getMgr()->resolve<DataMgr, DataMgr_var>(DATAMGRCTXT, name,
								      this->name);
    return mgr->printList();
  }
  throw BadNameException(objString.c_str(), __FUNCTION__, getName());
}
#endif

void DIETForwarder::bind(const char* objName, const char* ior) {
  /* To avoid crashes when the peer forwarder is not ready: */
  /* If the peer was not initialized, the following call is blocking. */
  peerMutex.lock();
  peerMutex.unlock();

  string objString(objName);
  string name;
  string ctxt;

  if (!remoteCall(objString)) {
    cout << "Forward bind to peer (" << objName << ")" << endl;
    return getPeer()->bind(objString.c_str(), ior);
  }
  ctxt = getCtxt(objString);
  name = getName(objString);
  cout << "Bind locally (" << objString << ")" << endl;
  if (ctxt==LOCALAGENT) {
    ctxt = AGENTCTXT;
    /* Specific case for local agent.
       It is added into the cache to avoid resolution
       problems later.
    */
    LocalAgent_ptr agent = getLocalAgent(name.c_str());
    cachesMutex.lock();
    objectCache[ctxt+"/"+name] = CORBA::Object::_duplicate(agent);
    cachesMutex.unlock();
  } else {
    if (ctxt==MASTERAGENT) {
      ctxt = AGENTCTXT;
      /* Specific case for master agent.
	 It is added into the cache to avoid resolution
	 problems later.
      */
      MasterAgent_ptr agent = getMasterAgent(name.c_str());
      cachesMutex.lock();
      objectCache[ctxt+"/"+name] = CORBA::Object::_duplicate(agent);
      cachesMutex.unlock();
    }
  }
  ORBMgr::getMgr()->bind(ctxt, name, ior, true);
  // Broadcast the binding to all forwarders.
  ORBMgr::getMgr()->fwdsBind(ctxt, name, ior, this->name);
  cout << "Binded! (" << ctxt << "/" << name << ")" << endl;
}

/* Return the local bindings. Result is a set of couple
 * (object name, object ior)
 */
SeqString* DIETForwarder::getBindings(const char* ctxt) {
  list<string> objects;
  list<string>::iterator it;
  SeqString* result = new SeqString();
  unsigned int cmpt = 0;
	
  objects = ORBMgr::getMgr()->list(ctxt);
  result->length(objects.size()*2);

  for (it=objects.begin(); it!=objects.end(); ++it) {
    try {
      CORBA::Object_ptr obj = ORBMgr::getMgr()->resolveObject(ctxt, it->c_str(),
							      "no-Forwarder");
      (*result)[cmpt++]=it->c_str();
      (*result)[cmpt++]=ORBMgr::getMgr()->getIOR(obj).c_str();
    } catch (const runtime_error& err) {
      continue;
    }
  }
  result->length(cmpt);
  return result;
}

void DIETForwarder::unbind(const char* objName) {
  string objString(objName);
  string name;
  string ctxt;
	
  if (!remoteCall(objString)) {
    return getPeer()->unbind(objString.c_str());
  }
	
  name = objString;
	
  if (name.find('/')==string::npos)
    return;
	
  ctxt = getCtxt(objString);
  name = getName(objString);
	
  removeObjectFromCache(name);
	
  ORBMgr::getMgr()->unbind(ctxt, name);
  // Broadcast the unbinding to all forwarders.
  ORBMgr::getMgr()->fwdsUnbind(ctxt, name, this->name);
}

void DIETForwarder::removeObjectFromCache(const string& name) {
  map<string, ::CORBA::Object_ptr>::iterator it;
  map<string, PortableServer::ServantBase*>::iterator jt;
  /* If the object is in the servant cache. */
  cachesMutex.lock();
  if ((jt=servants.find(name))!=servants.end()) {
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
  if ((it=objectCache.find(name))!=objectCache.end())
    objectCache.erase(it);
  cachesMutex.unlock();
}

/* Remove non existing objects from the caches. */
void DIETForwarder::cleanCaches() {
  map<string, ::CORBA::Object_ptr>::iterator it;
  list<string> invalidObjects;
  list<string>::const_iterator jt;
	
  cachesMutex.lock();
  for (it=objectCache.begin(); it!=objectCache.end(); ++it) {
    try {
      Forwarder_var object = Forwarder::_narrow(it->second);
      object->getName();
    } catch (const CORBA::TRANSIENT& err) {
      invalidObjects.push_back(it->first);
    }
  }
  cachesMutex.unlock();
  for (jt=invalidObjects.begin(); jt!=invalidObjects.end(); ++jt)
    removeObjectFromCache(*jt);
}

void DIETForwarder::connectPeer(const char* ior, const char* host,
				const ::CORBA::Long port) {
  string converted = ORBMgr::convertIOR(ior, host, port);
  setPeer(ORBMgr::getMgr()->resolve<Forwarder, Forwarder_ptr>(converted));
}

void DIETForwarder::setPeer(Forwarder_ptr peer) {
  this->peer = Forwarder::_duplicate(Forwarder::_narrow(peer));
  // Peer was init. The lock was done on the constructor.
  peerMutex.unlock();
}

Forwarder_var DIETForwarder::getPeer() {
  // Wait for setPeer
  peerMutex.lock();
  peerMutex.unlock();
  try {
    // Check if peer is still alive
    peer->getName();
  } catch (const CORBA::COMM_FAILURE& err) {
    // Lock peerMutex, then wait for setPeer
    // (setPeer() unlock the mutex
    peerMutex.lock();
    peerMutex.lock();
    peerMutex.unlock();
  } catch (const CORBA::TRANSIENT& err) {
    // Lock peerMutex, then wait for setPeer
    // (setPeer() unlock the mutex
    peerMutex.lock();
    peerMutex.lock();
    peerMutex.unlock();
  }
  return peer;
}


char* DIETForwarder::getIOR() {
  return CORBA::string_dup(ORBMgr::getMgr()->getIOR(_this()).c_str());
}

char* DIETForwarder::getName() {
  return CORBA::string_dup(name.c_str());
}

SeqString* DIETForwarder::acceptList() {
  SeqString* result = new SeqString;
  list<string>::const_iterator it;
	
  result->length(netCfg.getAcceptList().size());

  unsigned int i=0;
  for (it=netCfg.getAcceptList().begin();
       it!=netCfg.getAcceptList().end();
       ++it, ++i)
    {
      (*result)[i] = it->c_str();
    }
	
  return result;
}

SeqString* DIETForwarder::rejectList() {
  SeqString* result = new SeqString;
  list<string>::const_iterator it;
	
  result->length(netCfg.getRejectList().size());
	
  unsigned int i=0;
  for (it=netCfg.getRejectList().begin();
       it!=netCfg.getRejectList().end();
       ++it, ++i)
    {
      (*result)[i] = it->c_str();
    }
	
  return result;
}

::CORBA::Boolean DIETForwarder::manage(const char* hostname) {
  //cout << "Is this forwarder managing " << hostname << "?" << endl;
  return netCfg.manage(hostname);
}

SeqString* DIETForwarder::routeTree() {
  SeqString* result = new SeqString();
  /*std::list<string> forwarders = ORBMgr::list(FWRDCTXT);
    std::list<string>::const_iterator it;

    result->length(1);
    (*result)[0]=CORBA::string_dup(name.c_str())
    for (it=forwarders.begin(); it!=forwarders.end(); ++it) {
    if (*it==name) continue;
		
    Forwarder_var fwd = resolve<Forwarder, Forwarder_var>(FWRDCTXT, *it,
    this->name);
    SeqString* tmp = fwd->routeTree();
    unsigned int curLgth = result->length();
    unsigned int tmpLgth = tmp->lgth();
		
    result->length(lgth+tmpLgth);
    for (unsigned int i=lgth; i<lgth+tmpLgth; ++i) {
    (*result)[i] = (*tmp)[i-lgth];
    }
		
    }*/
  return result;
}

string DIETForwarder::getName(const string& namectxt) {
  size_t pos = namectxt.find('/');
  if (pos==string::npos) return namectxt;
  return namectxt.substr(pos+1);
}

string DIETForwarder::getCtxt(const string& namectxt) {
  size_t pos = namectxt.find('/');
  if (pos==string::npos) return "";
  return namectxt.substr(0, pos);
}
