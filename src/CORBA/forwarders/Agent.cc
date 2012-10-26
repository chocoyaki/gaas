/**
 * @file Agent.cc
 *
 * @brief  DIET forwarder implementation - Agent forwarder implementation
 *
 * @author  Gael Le Mahec   (gael.le.mahec@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


#include "DIETForwarder.hh"
#include "ORBMgr.hh"
#include <string>
#include <iostream>
#include <stdio.h>

::CORBA::Long
DIETForwarder::agentSubscribe(const char *agentName,
                              const char *hostname,
                              const ::SeqCorbaProfileDesc_t &services,
                              const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->agentSubscribe(agentName, hostname,
                                     services, objString.c_str());
  }

  name = getName(objString);

  Agent_var agent =
    ORBMgr::getMgr()->resolve<Agent, Agent_var>(AGENTCTXT, name, this->name);
  return agent->agentSubscribe(agentName, hostname, services);
} // agentSubscribe

::CORBA::Long
DIETForwarder::serverSubscribe(const char *seDName, const char *hostname,
                               const ::SeqCorbaProfileDesc_t &services,
                               const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->serverSubscribe(seDName, hostname,
                                      services, objString.c_str());
  }
  name = getName(objString);

  Agent_var agent =
    ORBMgr::getMgr()->resolve<Agent, Agent_var>(AGENTCTXT, name, this->name);
  return agent->serverSubscribe(seDName, hostname, services);
} // serverSubscribe

::CORBA::Long
DIETForwarder::childUnsubscribe(::CORBA::ULong childID,
                                const ::SeqCorbaProfileDesc_t &services,
                                const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->childUnsubscribe(childID, services, objString.c_str());
  }

  name = getName(objString);

  Agent_var agent =
    ORBMgr::getMgr()->resolve<Agent, Agent_var>(AGENTCTXT, name, this->name);
  return agent->childUnsubscribe(childID, services);
} // childUnsubscribe

::CORBA::Long
DIETForwarder::childRemoveService(::CORBA::ULong childID,
                                  const ::corba_profile_desc_t &profile,
                                  const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->childRemoveService(childID, profile, objString.c_str());
  }

  name = getName(objString);

  Agent_var agent =
    ORBMgr::getMgr()->resolve<Agent, Agent_var>(AGENTCTXT, name, this->name);
  return agent->childRemoveService(childID, profile);
} // childRemoveService

::CORBA::Long
DIETForwarder::addServices(::CORBA::ULong myID,
                           const ::SeqCorbaProfileDesc_t &services,
                           const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->addServices(myID, services, objString.c_str());
  }

  name = getName(objString);

  Agent_var agent =
    ORBMgr::getMgr()->resolve<Agent, Agent_var>(AGENTCTXT, name, this->name);
  return agent->addServices(myID, services);
} // addServices

void
DIETForwarder::getResponse(const ::corba_response_t &resp,
                           const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->getResponse(resp, objString.c_str());
  }

  name = getName(objString);

  Agent_var agent =
    ORBMgr::getMgr()->resolve<Agent, Agent_var>(AGENTCTXT, name, this->name);
  return agent->getResponse(resp);
} // getResponse

char *
DIETForwarder::getDataManager(const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->getDataManager(objString.c_str());
  }

  name = getName(objString);

  Agent_var agent =
    ORBMgr::getMgr()->resolve<Agent, Agent_var>(AGENTCTXT, name, this->name);
  return agent->getDataManager();
} // getDataManager

SeqString *
DIETForwarder::searchData(const char *request,
                          const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->searchData(request, objString.c_str());
  }

  name = getName(objString);

  Agent_var agent =
    ORBMgr::getMgr()->resolve<Agent, Agent_var>(AGENTCTXT, name, this->name);
  return agent->searchData(request);
} // searchData
