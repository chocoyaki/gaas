/****************************************************************************/
/* DIET forwarder implementation - Master Agent forwarder implementation    */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Gael Le Mahec   (gael.le.mahec@ens-lyon.fr)                         */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.5  2010/11/24 15:12:56  bdepardo
 * searchData is now available on all agents. SeDs are now able to retrieve
 * a DAGDA data from an alias specified by a client.
 * Currently a SeD cannot declare an alias.
 *
 * Revision 1.4  2010/07/27 16:16:49  glemahec
 * Forwarders robustness
 *
 * Revision 1.3  2010/07/14 23:45:30  bdepardo
 * Header corrections
 *
 * Revision 1.2  2010/07/13 15:24:13  glemahec
 * Warnings corrections and some robustness improvements
 *
 * Revision 1.1  2010/07/12 16:08:56  glemahec
 * DIET 2.5 beta 1 - Forwarder implementations
 ****************************************************************************/

#include "DIETForwarder.hh"
#include "ORBMgr.hh"
#include <string>
#include <iostream>

using namespace std;

corba_response_t* DIETForwarder::submit(const ::corba_pb_desc_t& pb_profile,
					::CORBA::ULong maxServers,
					const char* objName)
{
  cout << __FILE__ << ": l." << __LINE__ << " (" << __FUNCTION__ << ")" << endl;
  string objString(objName);
  string name;
	
  if (!remoteCall(objString)) {
    cout << "Forwarder remote call submit(pb_profile, " << maxServers << ", " << maxServers;
    cout << ", " << objString << ")" << endl;
    return getPeer()->submit(pb_profile, maxServers, objString.c_str());
  }
	
  name = getName(objString);
	
  cout << "Forwarder local object call: " << endl;
  cout << "\t- Resolve " << name << " from this forwarder (" << this->name << ")" << endl;
	
  MasterAgent_var agent = ORBMgr::getMgr()->resolve<MasterAgent, MasterAgent_var>(AGENTCTXT,
										  name,
										  this->name);
  return agent->submit(pb_profile, maxServers);
}

::CORBA::Long DIETForwarder::get_session_num(const char* objName) {
  string objString(objName);
  string name;
	
  if (!remoteCall(objString)) {
    return getPeer()->get_session_num(objString.c_str());
  }
	
  name = getName(objString);
		
  MasterAgent_var agent = ORBMgr::getMgr()->resolve<MasterAgent, MasterAgent_var>(AGENTCTXT,
										  name,
										  this->name);
  return agent->get_session_num();
}

char* DIETForwarder::get_data_id(const char* objName) {
  string objString(objName);
  string name;
	
  if (!remoteCall(objString)) {
    return getPeer()->get_data_id(objString.c_str());
  }
	
  name = getName(objString);
		
  MasterAgent_var agent = ORBMgr::getMgr()->resolve<MasterAgent, MasterAgent_var>(AGENTCTXT,
										  name,
										  this->name);
  return agent->get_data_id();
}

::CORBA::ULong DIETForwarder::dataLookUp(const char* id,
					 const char* objName)
{
  string objString(objName);
  string name;
	
  if (!remoteCall(objString)) {
    return getPeer()->dataLookUp(id, objString.c_str());
  }
	
  name = getName(objString);
	
  MasterAgent_var agent = ORBMgr::getMgr()->resolve<MasterAgent, MasterAgent_var>(AGENTCTXT,
										  name,
										  this->name);
  return agent->dataLookUp(id);
}

corba_data_desc_t* DIETForwarder::get_data_arg(const char* argID,
					       const char* objName)
{
  string objString(objName);
  string name;
	
  if (!remoteCall(objString)) {
    return getPeer()->get_data_arg(argID, objString.c_str());
  }
	
  name = getName(objString);
	
  MasterAgent_var agent = ORBMgr::getMgr()->resolve<MasterAgent, MasterAgent_var>(AGENTCTXT,
										  name,
										  this->name);
  return agent->get_data_arg(argID);
}

::CORBA::Long DIETForwarder::diet_free_pdata(const char* argID,
					     const char* objName)
{
  string objString(objName);
  string name;
	
  if (!remoteCall(objString)) {
    return getPeer()->diet_free_pdata(argID, objString.c_str());
  }
	
  name = getName(objString);
	
  MasterAgent_var agent = ORBMgr::getMgr()->resolve<MasterAgent, MasterAgent_var>(AGENTCTXT,
										  name,
										  this->name);
  return agent->diet_free_pdata(argID);
}

SeqCorbaProfileDesc_t* DIETForwarder::getProfiles(::CORBA::Long& length,
						  const char* objName)
{
  string objString(objName);
  string name;
	
  if (!remoteCall(objString)) {
    return getPeer()->getProfiles(length, objString.c_str());
  }
	
  name = getName(objString);
	
  MasterAgent_var agent = ORBMgr::getMgr()->resolve<MasterAgent, MasterAgent_var>(AGENTCTXT,
										  name,
										  this->name);
  return agent->getProfiles(length);
}

#ifdef HAVE_WORKFLOW
wf_response_t* DIETForwarder::submit_pb_set(const ::corba_pb_desc_seq_t& seq_pb,
					    const char* objName)
{
  string objString(objName);
  string name;
	
  if (!remoteCall(objString)) {
    return getPeer()->submit_pb_set(seq_pb, objString.c_str());
  }
	
  name = getName(objString);
	
  MasterAgent_var agent = ORBMgr::getMgr()->resolve<MasterAgent, MasterAgent_var>(AGENTCTXT,
										  name,
										  this->name);
  return agent->submit_pb_set(seq_pb);
}
response_seq_t* DIETForwarder::submit_pb_seq(const ::corba_pb_desc_seq_t& pb_seq,
					     ::CORBA::Long reqCount,
					     ::CORBA::Boolean& complete,
					     ::CORBA::Long& firstReqId,
					     ::CORBA::Long& seqReqId,
					     const char* objName)
{
  string objString(objName);
  string name;
	
  if (!remoteCall(objString)) {
    return getPeer()->submit_pb_seq(pb_seq, reqCount, complete, firstReqId,
				    seqReqId, objString.c_str());
  }
	
  name = getName(objString);
	
  MasterAgent_var agent = ORBMgr::getMgr()->resolve<MasterAgent, MasterAgent_var>(AGENTCTXT,
										  name,
										  this->name);
  return agent->submit_pb_seq(pb_seq, reqCount, complete, firstReqId, seqReqId);
}
#endif
#ifdef HAVE_DAGDA
::CORBA::Long DIETForwarder::insertData(const char* key,
					const ::SeqString& values,
					const char* objName)
{
  string objString(objName);
  string name;
	
  if (!remoteCall(objString)) {
    return getPeer()->insertData(key, values, objString.c_str());
  }
	
  name = getName(objString);
	
  MasterAgent_var agent = ORBMgr::getMgr()->resolve<MasterAgent, MasterAgent_var>(AGENTCTXT,
										  name,
										  this->name);
  return agent->insertData(key, values);
}
#endif
#ifdef HAVE_MULTI_MA
::CORBA::Boolean DIETForwarder::handShake(const char* masterAgentName,
					  const char* myName,
					  const char* objName)
{
  string objString(objName);
  string name;
	
  if (!remoteCall(objString)) {
    return getPeer()->handShake(masterAgentName, myName, objString.c_str());
  }
	
  name = getName(objString);
	
  MasterAgent_var agent = ORBMgr::getMgr()->resolve<MasterAgent, MasterAgent_var>(AGENTCTXT,
										  name,
										  this->name);
  return agent->handShake(masterAgentName, myName);
}

char* DIETForwarder::getBindName(const char* objName) {
  string objString(objName);
  string name;
	
  if (!remoteCall(objString)) {
    return getPeer()->getBindName(objString.c_str());
  }
	
  name = getName(objString);
	
  MasterAgent_var agent = ORBMgr::getMgr()->resolve<MasterAgent, MasterAgent_var>(AGENTCTXT,
										  name,
										  this->name);
  return agent->getBindName();
}

void DIETForwarder::searchService(const char* masterAgentName,
				  const char* myName,
				  const ::corba_request_t& request,
				  const char* objName)
{
  string objString(objName);
  string name;
	
  if (!remoteCall(objString)) {
    return getPeer()->searchService(masterAgentName, myName, request,
				    objString.c_str());
  }
	
  name = getName(objString);
	
  MasterAgent_var agent = ORBMgr::getMgr()->resolve<MasterAgent, MasterAgent_var>(AGENTCTXT,
										  name,
										  this->name);
  return agent->searchService(masterAgentName, myName, request);
}

void DIETForwarder::stopFlooding(::CORBA::Long reqId,
				 const char* senderId,
				 const char* objName)
{
  string objString(objName);
  string name;
	
  if (!remoteCall(objString)) {
    return getPeer()->stopFlooding(reqId, senderId, objString.c_str());
  }
	
  name = getName(objString);
	
  MasterAgent_var agent = ORBMgr::getMgr()->resolve<MasterAgent, MasterAgent_var>(AGENTCTXT,
										  name,
										  this->name);
  return agent->stopFlooding(reqId, senderId);
}

void DIETForwarder::serviceNotFound(::CORBA::Long reqId,
				    const char* senderId,
				    const char* objName)
{
  string objString(objName);
  string name;
	
  if (!remoteCall(objString)) {
    return getPeer()->serviceNotFound(reqId, senderId, objString.c_str());
  }
	
  name = getName(objString);
	
  MasterAgent_var agent = ORBMgr::getMgr()->resolve<MasterAgent, MasterAgent_var>(AGENTCTXT,
										  name,
										  this->name);
  return agent->serviceNotFound(reqId, senderId);
}

void DIETForwarder::newFlood(::CORBA::Long reqId,
			     const char* senderId,
			     const char* objName)
{
  string objString(objName);
  string name;
	
  if (!remoteCall(objString)) {
    return getPeer()->newFlood(reqId, senderId, objString.c_str());
  }
	
  name = getName(objString);
	
  MasterAgent_var agent = ORBMgr::getMgr()->resolve<MasterAgent, MasterAgent_var>(AGENTCTXT,
										  name,
										  this->name);
  return agent->newFlood(reqId, senderId);
}

void DIETForwarder::floodedArea(::CORBA::Long reqId,
				const char* senderId,
				const char* objName)
{
  string objString(objName);
  string name;
	
  if (!remoteCall(objString)) {
    return getPeer()->floodedArea(reqId, senderId, objString.c_str());
  }
	
  name = getName(objString);
	
  MasterAgent_var agent = ORBMgr::getMgr()->resolve<MasterAgent, MasterAgent_var>(AGENTCTXT,
										  name,
										  this->name);
  return agent->floodedArea(reqId, senderId);
}

void DIETForwarder::alreadyContacted(::CORBA::Long reqId,
				     const char* senderId,
				     const char* objName)
{
  string objString(objName);
  string name;
	
  if (!remoteCall(objString)) {
    return getPeer()->alreadyContacted(reqId, senderId, objString.c_str());
  }
	
  name = getName(objString);
	
  MasterAgent_var agent = ORBMgr::getMgr()->resolve<MasterAgent, MasterAgent_var>(AGENTCTXT,
										  name,
										  this->name);
  return agent->alreadyContacted(reqId, senderId);
}

void DIETForwarder::serviceFound(::CORBA::Long reqId,
				 const ::corba_response_t& decision,
				 const char* objName)
{
  string objString(objName);
  string name;
	
  if (!remoteCall(objString)) {
    return getPeer()->serviceFound(reqId, decision, objString.c_str());
  }
	
  name = getName(objString);
	
  MasterAgent_var agent = ORBMgr::getMgr()->resolve<MasterAgent, MasterAgent_var>(AGENTCTXT,
										  name,
										  this->name);
  return agent->serviceFound(reqId, decision);
}
#endif
