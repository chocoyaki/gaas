/****************************************************************************/
/* DIET forwarder implementation                                            */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Gael Le Mahec   (gael.le.mahec@ens-lyon.fr)                         */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$ */
/* $Log$
/* Revision 1.1  2010/07/12 16:11:03  glemahec
/* DIET 2.5 beta 1 - New ORB manager; dietForwarder application
/* */

#include "DIETForwarder.hh"

#include "Forwarder.hh"
#include "ORBMgr.hh"

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


::CORBA::Object_ptr DIETForwarder::getObjectCache(const std::string& name)
{
	map<string, ::CORBA::Object_ptr>::iterator it;
	
	if ((it=objectCache.find(name))!=objectCache.end()) {
		return CORBA::Object::_duplicate(it->second);
	}
	return ::CORBA::Object::_nil();	
}

DIETForwarder::DIETForwarder(const string& name, const string& cfgPath) :
	netCfg(cfgPath)
{
	this->name = name;
}

/* DIET object factory methods. */
Agent_ptr DIETForwarder::getAgent(const char* name)
{
	size_t pos;
	string nm(name);
	::CORBA::Object_ptr object;
	
	if ((pos=nm.find('/'))==string::npos) {
		nm = string(AGENTCTXT)+"/"+nm;
	}
	object = getObjectCache(nm);
	if (!CORBA::is_nil(object)) {
		return Agent::_duplicate(Agent::_narrow(object));
	}
	
	AgentFwdrImpl * agent = new AgentFwdrImpl(this->_this(), nm.c_str());
	ORBMgr::getMgr()->activate(agent);
	objectCache[nm] = CORBA::Object::_duplicate(agent->_this());
	
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
	if (!CORBA::is_nil(object))
		return Callback::_duplicate(Callback::_narrow(object));
	
	CallbackFwdrImpl * callback = new CallbackFwdrImpl(this->_this(), nm.c_str());
	ORBMgr::getMgr()->activate(callback);
	objectCache[nm] = CORBA::Object::_duplicate(callback->_this());
	
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
		return LocalAgent::_duplicate(LocalAgent::_narrow(object));
	}
	LocalAgentFwdrImpl * agent = new LocalAgentFwdrImpl(this->_this(), nm.c_str());
	ORBMgr::getMgr()->activate(agent);
	objectCache[nm] = CORBA::Object::_duplicate(agent->_this());
	
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
	if (!CORBA::is_nil(object))
		return ComponentConfigurator::_duplicate(ComponentConfigurator::_narrow(object));
	
	DietLogComponentFwdr * comp = new DietLogComponentFwdr(this->_this(), nm.c_str());
	ORBMgr::getMgr()->activate(comp);
	objectCache[name] = CORBA::Object::_duplicate(comp->_this());
	
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
	if (!CORBA::is_nil(object))
		return MasterAgent::_duplicate(MasterAgent::_narrow(object));
	
	MasterAgentFwdrImpl * agent = new MasterAgentFwdrImpl(this->_this(), nm.c_str());
	ORBMgr::getMgr()->activate(agent);
	objectCache[nm] = CORBA::Object::_duplicate(agent->_this());
	
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
	if (!CORBA::is_nil(object))
		return SeD::_duplicate(SeD::_narrow(object));
	
	SeDFwdrImpl * sed = new SeDFwdrImpl(this->_this(), nm.c_str());
	ORBMgr::getMgr()->activate(sed);
	objectCache[nm] = CORBA::Object::_duplicate(sed->_this());
	
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
		return Dagda::_duplicate(Dagda::_narrow(object));
	}
	DagdaFwdrImpl * dagda = new DagdaFwdrImpl(this->_this(), nm.c_str());
	ORBMgr::getMgr()->activate(dagda);
	objectCache[nm] = CORBA::Object::_duplicate(dagda->_this());
	
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
	if (!CORBA::is_nil(object))
		return DataMgr::_duplicate(DataMgr::_narrow(object));
	
	DataMgrFwdrImpl * dtm = new DataMgrFwdrImpl(this->_this(), nm.c_str());
	ORBMgr::getMgr()->activate(dtm);
	objectCache[nm] = CORBA::Object::_duplicate(dtm->_this());
	
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
	if (!CORBA::is_nil(object))
		return LocMgr::_duplicate(LocMgr::_narrow(object));
	
	LocMgrFwdrImpl * dtm = new LocMgrFwdrImpl(this->_this(), nm.c_str());
	ORBMgr::getMgr()->activate(dtm);
	objectCache[nm] = CORBA::Object::_duplicate(dtm->_this());
	
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
	if (!CORBA::is_nil(object))
		return CltManFwdr::_duplicate(CltManFwdr::_narrow(object));
	
	CltWfMgrFwdr * mgr = new CltWfMgrFwdr(this->_this(), nm.c_str());
	ORBMgr::getMgr()->activate(mgr);
	objectCache[nm] = CORBA::Object::_duplicate(mgr->_this());
	
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
	if (!CORBA::is_nil(object))
		return MaDag::_duplicate(MaDag::_narrow(object));
	
	MaDagFwdrImpl * madag = new MaDagFwdrImpl(this->_this(), nm.c_str());
	ORBMgr::getMgr()->activate(madag);
	objectCache[nm] = CORBA::Object::_duplicate(madag->_this());
	
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
	if (!CORBA::is_nil(object))
		return WfLogService::_duplicate(WfLogService::_narrow(object));
	
	WfLogServiceFwdrImpl * wfl = new WfLogServiceFwdrImpl(this->_this(), nm.c_str());
	ORBMgr::getMgr()->activate(wfl);
	objectCache[nm] = CORBA::Object::_duplicate(wfl->_this());
	
	return WfLogService::_duplicate(wfl->_this());
}

#endif

/* Common methods implementations. */
::CORBA::Long DIETForwarder::ping(const char* objName)
{
	/* ping() is a common function to different idl classes. */
	/* Temp: always return 0. To be changer later... */
	//return 0;
	/* -------- */
	string objString(objName);
	string name;
	string ctxt;
	
	if (!remoteCall(objString)) {
		return peer->ping(objString.c_str());
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
	if (ctxt==string(WFLOGCTXT)) {
		WfLogService_var wfl =
			ORBMgr::getMgr()->resolve<WfLogService, WfLogService_var>(WFLOGCTXT, name,
																																this->name);
		wfl->ping();
		return 0;
	}
#endif
	if (ctxt==string(SEDCTXT)) {
		SeD_var sed = ORBMgr::getMgr()->resolve<SeD, SeD_var>(SEDCTXT, name,
																													this->name);
		return sed->ping();
	}
	throw Forwarder::BadNameException(objString.c_str());
}

void DIETForwarder::getRequest(const ::corba_request_t& req, const char* objName)
{
	string objString(objName);
	string name;
	string ctxt;
	size_t pos;
	
	if (!remoteCall(objString)) {
		return peer->getRequest(req, objString.c_str());
	}
	
	name = objString;

	if ((pos=name.find('/'))==string::npos)
		throw Forwarder::BadNameException(name.c_str());
	ctxt = name.substr(0, pos);
	name = name.substr(pos+1);
	
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
	throw Forwarder::BadNameException(objString.c_str());
}

char* DIETForwarder::getHostname(const char* objName)
{
	string objString(objName);
	string name;
	string ctxt;
	size_t pos;
	
	if (!remoteCall(objString)) {
		return peer->getHostname(objString.c_str());
	}
	
	name = objString;
	
	if ((pos=name.find('/'))==string::npos)
		throw Forwarder::BadNameException(name.c_str());
	ctxt = name.substr(0, pos);
	name = name.substr(pos+1);
	
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
	throw Forwarder::BadNameException(objString.c_str());
}

#ifdef HAVE_DYNAMICS
::CORBA::Long DIETForwarder::bindParent(const char* parentName, const char* objName)
{
	string objString(objName);
	string name;
	string ctxt;
	size_t pos;
	
	if (!remoteCall(objString)) {
		return peer->bindParent(parentName, objString.c_str());
	}
	
	name = objString;
	
	if ((pos=name.find('/'))==string::npos)
		throw Forwarder::BadNameException(name.c_str());
	ctxt = name.substr(0, pos);
	name = name.substr(pos+1);
	
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
	throw Forwarder::BadNameException(objString.c_str());
}

::CORBA::Long DIETForwarder::disconnect(const char* objName)
{
	string objString(objName);
	string name;
	string ctxt;
	size_t pos;
	
	if (!remoteCall(objString)) {
		return peer->disconnect(objString.c_str());
	}
	
	name = objString;
	
	if ((pos=name.find('/'))==string::npos)
		throw Forwarder::BadNameException(name.c_str());
	ctxt = name.substr(0, pos);
	name = name.substr(pos+1);
	
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
	throw Forwarder::BadNameException(objString.c_str());
}

::CORBA::Long DIETForwarder::removeElement(::CORBA::Boolean recursive, const char* objName)
{
	string objString(objName);
	string name;
	string ctxt;
	size_t pos;
	
	if (!remoteCall(objString)) {
		return peer->removeElement(recursive, objString.c_str());
	}
	
	name = objString;
	
	if ((pos=name.find('/'))==string::npos)
		throw Forwarder::BadNameException(name.c_str());
	ctxt = name.substr(0, pos);
	name = name.substr(pos+1);
	
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
	throw Forwarder::BadNameException(objString.c_str());
}
#endif

#ifndef HAVE_DAGDA
char* DIETForwarder::whereData(const char* argID,
															 const char* objName)
{
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return peer->whereData(argID, objString.c_str());
	}
	
	name = getName(objString);
	if (getCtxt(objString)==LOCMGRCTXT) {
		LocMgr_var mgr = ORBMgr::getMgr()->resolve<LocMgr, LocMgr_var>(LOCMGRCTXT, name,
																																	 this->name);
		return mgr->whereData(argID);
	} else {
		DataMgr_var mgr = ORBMgr::getMgr()->resolve<DataMgr, DataMgr_var>(LOCMGRCTXT, name,
																																			this->name);
		return mgr->whereData(argID);
	}
}

char* DIETForwarder::setMyName(const char* objName) {
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return peer->setMyName(objString.c_str());
	}
	
	name = getName(objString);
	if (getCtxt(objString)==LOCMGRCTXT) {
		LocMgr_var mgr = ORBMgr::getMgr()->resolve<LocMgr, LocMgr_var>(LOCMGRCTXT, name,
																																	 this->name);
		return mgr->setMyName();
	} else {
		DataMgr_var mgr = ORBMgr::getMgr()->resolve<DataMgr, DataMgr_var>(LOCMGRCTXT, name,
																																			this->name);
		return mgr->setMyName();
	}

}

char* DIETForwarder::whichSeDOwner(const char* argID,
																	 const char* objName)
{
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return peer->whichSeDOwner(argID, objString.c_str());
	}
	
	name = getName(objString);
	
	if (getCtxt(objString)==LOCMGRCTXT) {
		LocMgr_var mgr = ORBMgr::getMgr()->resolve<LocMgr, LocMgr_var>(LOCMGRCTXT, name,
																																	 this->name);
		
		return mgr->whichSeDOwner(argID);
	} else {
		DataMgr_var mgr = ORBMgr::getMgr()->resolve<DataMgr, DataMgr_var>(LOCMGRCTXT, name,
																																			this->name);
		return mgr->whichSeDOwner(argID);
	}

}

void DIETForwarder::printList(const char* objName) {
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return peer->printList(objString.c_str());
	}
	
	name = getName(objString);
	
	if (getCtxt(objString)==LOCMGRCTXT) {
		LocMgr_var mgr = ORBMgr::getMgr()->resolve<LocMgr, LocMgr_var>(LOCMGRCTXT, name,
																																	 this->name);
		return mgr->printList();
	} else {
		DataMgr_var mgr = ORBMgr::getMgr()->resolve<DataMgr, DataMgr_var>(LOCMGRCTXT, name,
																																			this->name);
		return mgr->printList();
	}
}
#endif

void DIETForwarder::bind(const char* objName, const char* ior) {
	string objString(objName);
	string name;
	string ctxt;
	size_t pos;

	if (!remoteCall(objString)) {
		return peer->bind(objString.c_str(), ior);
	}
	name = objString;
	
	if ((pos=name.find('/'))==string::npos)
		throw Forwarder::BadNameException(name.c_str());
	ctxt = name.substr(0, pos);
	name = name.substr(pos+1);
	if (ctxt==LOCALAGENT) {
		ctxt = AGENTCTXT;
		/* Specific case for local agent.
		 It is added into the cache to avoid resolution
		 problems later.
		*/
		LocalAgent_ptr agent = getLocalAgent(name.c_str());
		objectCache[ctxt+"/"+name] = CORBA::Object::_duplicate(agent);
		
	}
	if (ctxt==MASTERAGENT) {
		ctxt = AGENTCTXT;
		/* Specific case for master agent.
		 It is added into the cache to avoid resolution
		 problems later.
		 */
		MasterAgent_ptr agent = getMasterAgent(name.c_str());
		objectCache[ctxt+"/"+name] = CORBA::Object::_duplicate(agent);
		
	}
	ORBMgr::getMgr()->bind(ctxt, name, ior, true);
	// Broadcast the binding to all forwarders.
	ORBMgr::getMgr()->fwdsBind(ctxt, name, ior, this->name);
}

void DIETForwarder::unbind(const char* objName) {
	string objString(objName);
	string name;
	string ctxt;
	size_t pos;
	
	if (!remoteCall(objString)) {
		return peer->unbind(objString.c_str());
	}
	
	name = objString;
	
	if ((pos=name.find('/'))==string::npos)
		throw Forwarder::BadNameException(name.c_str());
	ctxt = name.substr(0, pos);
	name = name.substr(pos+1);
	
	ORBMgr::getMgr()->unbind(ctxt, name);
	// Broadcast the unbinding to all forwarders.
	ORBMgr::getMgr()->fwdsUnbind(ctxt, name, this->name);
}

void DIETForwarder::connectPeer(const char* ior, const char* host,
																const ::CORBA::Long port)
{
	string converted = ORBMgr::convertIOR(ior, host, port);
	peer = ORBMgr::getMgr()->resolve<Forwarder, Forwarder_ptr>(converted);
}

void DIETForwarder::setPeer(Forwarder_ptr peer) {
	this->peer = Forwarder::_duplicate(Forwarder::_narrow(peer));
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
