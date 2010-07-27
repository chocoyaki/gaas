/****************************************************************************/
/* DIET forwarder implementation - MA DAG forwarder implementation          */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Gael Le Mahec   (gael.le.mahec@ens-lyon.fr)                         */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
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

using namespace std;

::CORBA::Long DIETForwarder::processDagWf(const ::corba_wf_desc_t& dag_desc,
																					const char* cltMgrRef,
																					::CORBA::Long wfReqId,
																					const char* objName)
{
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return getPeer()->processDagWf(dag_desc, cltMgrRef, wfReqId, objString.c_str());
	}
	
	name = getName(objString);
		
	MaDag_var agent = ORBMgr::getMgr()->resolve<MaDag, MaDag_var>(MADAGCTXT, name,
																																this->name);
	return agent->processDagWf(dag_desc, cltMgrRef, wfReqId);
}

::CORBA::Long DIETForwarder::processMultiDagWf(const ::corba_wf_desc_t& dag_desc,
																							 const char* cltMgrRef,
																							 ::CORBA::Long wfReqId,
																							 ::CORBA::Boolean release,
																							 const char* objName)
{
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return getPeer()->processMultiDagWf(dag_desc, cltMgrRef, wfReqId, release,
																	 objString.c_str());
	}
	
	name = getName(objString);
		
	MaDag_var agent = ORBMgr::getMgr()->resolve<MaDag, MaDag_var>(MADAGCTXT, name,
																																this->name);
	return agent->processMultiDagWf(dag_desc, cltMgrRef, wfReqId, release);	
}

::CORBA::Long DIETForwarder::getWfReqId(const char* objName) {
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return getPeer()->getWfReqId(objString.c_str());
	}
	
	name = getName(objString);
	
	MaDag_var agent = ORBMgr::getMgr()->resolve<MaDag, MaDag_var>(MADAGCTXT, name,
																																this->name);
	return agent->getWfReqId();	
}

void DIETForwarder::releaseMultiDag(::CORBA::Long wfReqId,
																		const char* objName)
{
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return getPeer()->releaseMultiDag(wfReqId, objString.c_str());
	}
	
	name = getName(objString);
		
	MaDag_var agent = ORBMgr::getMgr()->resolve<MaDag, MaDag_var>(MADAGCTXT, name,
																																this->name);
	return agent->releaseMultiDag(wfReqId);	
}

void DIETForwarder::cancelDag(::CORBA::Long dagId,
															const char* objName)
{
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return getPeer()->cancelDag(dagId, objString.c_str());
	}
	
	name = getName(objString);
		
	MaDag_var agent = ORBMgr::getMgr()->resolve<MaDag, MaDag_var>(MADAGCTXT, name,
																																this->name);
	return agent->cancelDag(dagId);	
}

void DIETForwarder::setPlatformType(::MaDag::pfmType_t pfmType,
																		const char* objName)
{
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return getPeer()->setPlatformType(pfmType, objString.c_str());
	}
	
	name = getName(objString);
		
	MaDag_var agent = ORBMgr::getMgr()->resolve<MaDag, MaDag_var>(MADAGCTXT, name,
																																this->name);
	return agent->setPlatformType(pfmType);	
}
