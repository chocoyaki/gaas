/****************************************************************************/
/* DIET forwarder implementation - Workflow Logservice forwarder implem.    */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Gael Le Mahec   (gael.le.mahec@ens-lyon.fr)                         */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
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

void DIETForwarder::setWf(const char* workflow_description,
													const char* objName)
{
	string objString(objName);
	string name;

	if (!remoteCall(objString)) {
		return peer->setWf(workflow_description, objString.c_str());
	}
	
	name = getName(objString);
	
	WfLogService_var wfls =
	  ORBMgr::getMgr()->resolve<WfLogService, WfLogService_var>(WFLOGCTXT, name,
																															this->name);
	return wfls->setWf(workflow_description);
}

void DIETForwarder::nodeIsDone(const char* node_id,
															 const char* objName)
{
	string objString(objName);
	string name;

	if (!remoteCall(objString)) {
		return peer->nodeIsDone(node_id, objString.c_str());
	}
	
	name = getName(objString);
	
	WfLogService_var wfls =
		ORBMgr::getMgr()->resolve<WfLogService, WfLogService_var>(WFLOGCTXT, name,
																															this->name);
	return wfls->nodeIsDone(node_id);
}

void DIETForwarder::nodeIsRunning(const char* node_id,
																	const char* hostname,
																	const char* objName)
{
	string objString(objName);
	string name;

	if (!remoteCall(objString)) {
		return peer->nodeIsRunning(node_id, hostname, objString.c_str());
	}
	
	name = getName(objString);
	
	WfLogService_var wfls =
		ORBMgr::getMgr()->resolve<WfLogService, WfLogService_var>(WFLOGCTXT, name,
																															this->name);
	return wfls->nodeIsRunning(node_id, hostname);
	
}

void DIETForwarder::nodeIsStarting(const char* node_id,
																	 const char* objName)
{
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return peer->nodeIsStarting(node_id, objString.c_str());
	}
	
	name = getName(objString);
	
	WfLogService_var wfls =
		ORBMgr::getMgr()->resolve<WfLogService, WfLogService_var>(WFLOGCTXT, name,
																															this->name);
	return wfls->nodeIsStarting(node_id);
}

void DIETForwarder::nodeIsWaiting(const char* node_id,
																	const char* objName)
{
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return peer->nodeIsWaiting(node_id, objString.c_str());
	}
	
	name = getName(objString);
		
	WfLogService_var wfls =
	ORBMgr::getMgr()->resolve<WfLogService, WfLogService_var>(WFLOGCTXT, name,
																														this->name);
	return wfls->nodeIsWaiting(node_id);	
}
