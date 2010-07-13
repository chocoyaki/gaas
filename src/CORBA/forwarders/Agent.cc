/****************************************************************************/
/* DIET forwarder implementation - Agent forwarder implementation           */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Gael Le Mahec   (gael.le.mahec@ens-lyon.fr)                         */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$ */
/* $Log$
/* Revision 1.2  2010/07/13 15:24:13  glemahec
/* Warnings corrections and some robustness improvements
/* */
/* Revision 1.1  2010/07/12 16:08:56  glemahec */
/* DIET 2.5 beta 1 - Forwarder implementations */
/* */

#include "DIETForwarder.hh"
#include "ORBMgr.hh"
#include <string>
#include <iostream>

using namespace std;

::CORBA::Long DIETForwarder::agentSubscribe(const char* agentName,
																						const char* hostname,
																						const ::SeqCorbaProfileDesc_t& services,
																						const char* objName)
{
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return peer->agentSubscribe(agentName, hostname, services, objString.c_str());
	}
	
	name = getName(objString);

	Agent_var agent = ORBMgr::getMgr()->resolve<Agent, Agent_var>(AGENTCTXT, name,
																																this->name);
	return agent->agentSubscribe(agentName, hostname, services);
}

::CORBA::Long DIETForwarder::serverSubscribe(const char* seDName,
																						 const char* hostname,
#ifdef HAVE_JXTA
																						 const char* uuid,
#endif
																						 const ::SeqCorbaProfileDesc_t& services,
																						 const char* objName)
{
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return peer->serverSubscribe(seDName, hostname,
#ifdef HAVE_JXTA
																 uuid,
#endif
																 services, objString.c_str());
	}
	
	name = getName(objString);
	
	Agent_var agent = ORBMgr::getMgr()->resolve<Agent, Agent_var>(AGENTCTXT, name,
																																this->name);
	cout << "Local call of " << __FUNCTION__;
	cout << "(" << seDName << ", " << hostname << "services)" << endl;
	return agent->serverSubscribe(seDName, hostname,
#ifdef HAVE_JXTA
																uuid,
#endif
																services);
}

#ifdef HAVE_DYNAMICS
::CORBA::Long DIETForwarder::childUnsubscribe(::CORBA::ULong childID,
																							const ::SeqCorbaProfileDesc_t& services,
																							const char* objName)
{
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return peer->childUnsubscribe(childID, services, objString.c_str());
	}
	
	name = getName(objString);
		
	Agent_var agent = ORBMgr::getMgr()->resolve<Agent, Agent_var>(AGENTCTXT, name,
																																this->name);
	return agent->childUnsubscribe(childID, services);
}
#endif

::CORBA::Long DIETForwarder::childRemoveService(::CORBA::ULong childID,
																								const ::corba_profile_desc_t& profile,
																								const char* objName)
{
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return peer->childRemoveService(childID, profile, objString.c_str());
	}
	
	name = getName(objString);
		
	Agent_var agent = ORBMgr::getMgr()->resolve<Agent, Agent_var>(AGENTCTXT, name,
																																this->name);
	return agent->childRemoveService(childID, profile);
}

::CORBA::Long DIETForwarder::addServices(::CORBA::ULong myID,
																				 const ::SeqCorbaProfileDesc_t& services,
																				 const char* objName)
{
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return peer->addServices(myID, services, objString.c_str());
	}
	
	name = getName(objString);
		
	Agent_var agent = ORBMgr::getMgr()->resolve<Agent, Agent_var>(AGENTCTXT, name,
																																this->name);
	return agent->addServices(myID, services);
}

void DIETForwarder::getResponse(const ::corba_response_t& resp,
																const char* objName)
{
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return peer->getResponse(resp, objString.c_str());
	}
	
	name = getName(objString);
		
	Agent_var agent = ORBMgr::getMgr()->resolve<Agent, Agent_var>(AGENTCTXT, name,
																																this->name);
	return agent->getResponse(resp);
}

#ifdef HAVE_DAGDA
char* DIETForwarder::getDataManager(const char* objName) {
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return peer->getDataManager(objString.c_str());
	}
	
	name = getName(objString);
		
	Agent_var agent = ORBMgr::getMgr()->resolve<Agent, Agent_var>(AGENTCTXT, name,
																																this->name);
	return agent->getDataManager();
}
#endif
