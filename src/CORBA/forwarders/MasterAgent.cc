/**
 * @file MasterAgent.cc
 *
 * @brief  DIET forwarder implementation - MA DAG forwarder implementation
 *
 * @author  Gael Le Mahec   (gael.le.mahec@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


#include "DIETForwarder.hh"
#include "ORBMgr.hh"
#include "debug.hh"

#include <string>
#include <iostream>

corba_response_t *
DIETForwarder::submit(const ::corba_pb_desc_t &pb_profile,
                      ::CORBA::ULong maxServers,
                      const char *objName) {
  TRACE_TEXT(TRACE_STRUCTURES, __FILE__
             << ": l." << __LINE__ << " (" << __FUNCTION__ << ")\n");
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    TRACE_TEXT(TRACE_MAIN_STEPS,
               "Forwarder remote call submit(pb_profile, "
               << maxServers << ", " << maxServers
               << ", " << objString << ")\n");
    return getPeer()->submit(pb_profile, maxServers, objString.c_str());
  }

  name = getName(objString);

  TRACE_TEXT(TRACE_MAIN_STEPS, "Forwarder local object call: \n");
  TRACE_TEXT(TRACE_MAIN_STEPS,
             "\t- Resolve " << name << " from this forwarder ("
                            << this->name << ")\n");

  MasterAgent_var agent =
    ORBMgr::getMgr()->resolve<MasterAgent, MasterAgent_var>(AGENTCTXT,
                                                            name,
                                                            this->name);
  return agent->submit(pb_profile, maxServers);
} // submit

::CORBA::Long
DIETForwarder::get_session_num(const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->get_session_num(objString.c_str());
  }

  name = getName(objString);

  MasterAgent_var agent =
    ORBMgr::getMgr()->resolve<MasterAgent, MasterAgent_var>(AGENTCTXT,
                                                            name,
                                                            this->name);
  return agent->get_session_num();
} // get_session_num

char *
DIETForwarder::get_data_id(const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->get_data_id(objString.c_str());
  }

  name = getName(objString);

  MasterAgent_var agent =
    ORBMgr::getMgr()->resolve<MasterAgent, MasterAgent_var>(AGENTCTXT,
                                                            name,
                                                            this->name);
  return agent->get_data_id();
} // get_data_id

::CORBA::ULong
DIETForwarder::dataLookUp(const char *id, const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->dataLookUp(id, objString.c_str());
  }

  name = getName(objString);

  MasterAgent_var agent =
    ORBMgr::getMgr()->resolve<MasterAgent, MasterAgent_var>(AGENTCTXT,
                                                            name,
                                                            this->name);
  return agent->dataLookUp(id);
} // dataLookUp

corba_data_desc_t *
DIETForwarder::get_data_arg(const char *argID, const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->get_data_arg(argID, objString.c_str());
  }

  name = getName(objString);

  MasterAgent_var agent =
    ORBMgr::getMgr()->resolve<MasterAgent, MasterAgent_var>(AGENTCTXT,
                                                            name,
                                                            this->name);
  return agent->get_data_arg(argID);
} // get_data_arg

::CORBA::Long
DIETForwarder::diet_free_pdata(const char *argID, const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->diet_free_pdata(argID, objString.c_str());
  }

  name = getName(objString);

  MasterAgent_var agent =
    ORBMgr::getMgr()->resolve<MasterAgent, MasterAgent_var>(AGENTCTXT,
                                                            name,
                                                            this->name);
  return agent->diet_free_pdata(argID);
} // diet_free_pdata

SeqCorbaProfileDesc_t *
DIETForwarder::getProfiles(::CORBA::Long &length, const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->getProfiles(length, objString.c_str());
  }

  name = getName(objString);

  MasterAgent_var agent =
    ORBMgr::getMgr()->resolve<MasterAgent, MasterAgent_var>(AGENTCTXT,
                                                            name,
                                                            this->name);
  return agent->getProfiles(length);
} // getProfiles

#ifdef HAVE_WORKFLOW
wf_response_t *
DIETForwarder::submit_pb_set(const ::corba_pb_desc_seq_t &seq_pb,
                             const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->submit_pb_set(seq_pb, objString.c_str());
  }

  name = getName(objString);

  MasterAgent_var agent =
    ORBMgr::getMgr()->resolve<MasterAgent, MasterAgent_var>(AGENTCTXT,
                                                            name,
                                                            this->name);
  return agent->submit_pb_set(seq_pb);
} // submit_pb_set

response_seq_t *
DIETForwarder::submit_pb_seq(const ::corba_pb_desc_seq_t &pb_seq,
                             ::CORBA::Long reqCount,
                             ::CORBA::Boolean &complete,
                             ::CORBA::Long &firstReqId,
                             ::CORBA::Long &seqReqId,
                             const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->submit_pb_seq(pb_seq, reqCount, complete, firstReqId,
                                    seqReqId, objString.c_str());
  }

  name = getName(objString);

  MasterAgent_var agent =
    ORBMgr::getMgr()->resolve<MasterAgent, MasterAgent_var>(AGENTCTXT,
                                                            name,
                                                            this->name);
  return agent->submit_pb_seq(pb_seq, reqCount, complete, firstReqId, seqReqId);
} // submit_pb_seq
#endif // ifdef HAVE_WORKFLOW

::CORBA::Long
DIETForwarder::insertData(const char *key,
                          const ::SeqString &values,
                          const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->insertData(key, values, objString.c_str());
  }

  name = getName(objString);

  MasterAgent_var agent =
    ORBMgr::getMgr()->resolve<MasterAgent, MasterAgent_var>(AGENTCTXT,
                                                            name,
                                                            this->name);
  return agent->insertData(key, values);
} // insertData

#ifdef HAVE_MULTI_MA
::CORBA::Boolean
DIETForwarder::handShake(const char *masterAgentName,
                         const char *myName,
                         const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->handShake(masterAgentName, myName, objString.c_str());
  }

  name = getName(objString);

  MasterAgent_var agent =
    ORBMgr::getMgr()->resolve<MasterAgent, MasterAgent_var>(AGENTCTXT,
                                                            name,
                                                            this->name);
  return agent->handShake(masterAgentName, myName);
} // handShake

char *
DIETForwarder::getBindName(const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->getBindName(objString.c_str());
  }

  name = getName(objString);

  MasterAgent_var agent =
    ORBMgr::getMgr()->resolve<MasterAgent, MasterAgent_var>(AGENTCTXT,
                                                            name,
                                                            this->name);
  return agent->getBindName();
} // getBindName

void
DIETForwarder::searchService(const char *masterAgentName,
                             const char *myName,
                             const ::corba_request_t &request,
                             const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->searchService(masterAgentName, myName,
                                    request, objString.c_str());
  }

  name = getName(objString);

  MasterAgent_var agent =
    ORBMgr::getMgr()->resolve<MasterAgent, MasterAgent_var>(AGENTCTXT,
                                                            name,
                                                            this->name);
  return agent->searchService(masterAgentName, myName, request);
} // searchService

void
DIETForwarder::stopFlooding(::CORBA::Long reqId,
                            const char *senderId,
                            const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->stopFlooding(reqId, senderId, objString.c_str());
  }

  name = getName(objString);

  MasterAgent_var agent =
    ORBMgr::getMgr()->resolve<MasterAgent, MasterAgent_var>(AGENTCTXT,
                                                            name,
                                                            this->name);
  return agent->stopFlooding(reqId, senderId);
} // stopFlooding

void
DIETForwarder::serviceNotFound(::CORBA::Long reqId,
                               const char *senderId,
                               const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->serviceNotFound(reqId, senderId, objString.c_str());
  }

  name = getName(objString);

  MasterAgent_var agent =
    ORBMgr::getMgr()->resolve<MasterAgent, MasterAgent_var>(AGENTCTXT,
                                                            name,
                                                            this->name);
  return agent->serviceNotFound(reqId, senderId);
} // serviceNotFound

void
DIETForwarder::newFlood(::CORBA::Long reqId,
                        const char *senderId,
                        const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->newFlood(reqId, senderId, objString.c_str());
  }

  name = getName(objString);

  MasterAgent_var agent =
    ORBMgr::getMgr()->resolve<MasterAgent, MasterAgent_var>(AGENTCTXT,
                                                            name,
                                                            this->name);
  return agent->newFlood(reqId, senderId);
} // newFlood

void
DIETForwarder::floodedArea(::CORBA::Long reqId,
                           const char *senderId,
                           const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->floodedArea(reqId, senderId, objString.c_str());
  }

  name = getName(objString);

  MasterAgent_var agent =
    ORBMgr::getMgr()->resolve<MasterAgent, MasterAgent_var>(AGENTCTXT,
                                                            name,
                                                            this->name);
  return agent->floodedArea(reqId, senderId);
} // floodedArea

void
DIETForwarder::alreadyContacted(::CORBA::Long reqId,
                                const char *senderId,
                                const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->alreadyContacted(reqId, senderId, objString.c_str());
  }

  name = getName(objString);

  MasterAgent_var agent =
    ORBMgr::getMgr()->resolve<MasterAgent, MasterAgent_var>(AGENTCTXT,
                                                            name,
                                                            this->name);
  return agent->alreadyContacted(reqId, senderId);
} // alreadyContacted

void
DIETForwarder::serviceFound(::CORBA::Long reqId,
                            const ::corba_response_t &decision,
                            const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->serviceFound(reqId, decision, objString.c_str());
  }

  name = getName(objString);

  MasterAgent_var agent =
    ORBMgr::getMgr()->resolve<MasterAgent, MasterAgent_var>(AGENTCTXT,
                                                            name,
                                                            this->name);
  return agent->serviceFound(reqId, decision);
} // serviceFound
#endif // ifdef HAVE_MULTI_MA
