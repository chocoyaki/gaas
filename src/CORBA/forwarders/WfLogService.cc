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
 * Revision 1.4  2010/07/20 08:56:23  bisnard
 * Updated WfLogService IDL
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

void DIETForwarder::createDag(const char* dagId, const char* wfId, const char* objName)
{
  string objString(objName);
	string name;

	if (!remoteCall(objString)) {
		return peer->createDag(dagId, wfId, objName);
	}
	
	name = getName(objString);
	
	WfLogService_var wfls =
		ORBMgr::getMgr()->resolve<WfLogService, WfLogService_var>(WFLOGCTXT, name,this->name);
	return wfls->createDag(dagId, wfId);
}

void DIETForwarder::createDagNode(const char* dagNodeId, const char* dagId, const char* wfId, const char* objName)
{
  string objString(objName);
	string name;

	if (!remoteCall(objString)) {
		return peer->createDagNode(dagNodeId, dagId, wfId, objName);
	}
	
	name = getName(objString);
	
	WfLogService_var wfls =
		ORBMgr::getMgr()->resolve<WfLogService, WfLogService_var>(WFLOGCTXT, name,this->name);
	return wfls->createDagNode(dagNodeId, dagId, wfId);
}

void DIETForwarder::createDagNodeData(const char* dagNodeId, const char* wfId, const char* dagNodePortId, const char* dataId, const char* objName)
{
  string objString(objName);
	string name;

	if (!remoteCall(objString)) {
		return peer->createDagNodeData(dagNodeId, wfId, dagNodePortId, dataId, objName);
	}
	
	name = getName(objString);
	
	WfLogService_var wfls =
		ORBMgr::getMgr()->resolve<WfLogService, WfLogService_var>(WFLOGCTXT, name,this->name);
	return wfls->createDagNodeData(dagNodeId, wfId, dagNodePortId, dataId);
}

void DIETForwarder::createDagNodeLink(const char* srcNodeId, const char* srcWfId, const char* destNodeId, const char* destWfId, const char* objName)
{
  string objString(objName);
	string name;

	if (!remoteCall(objString)) {
		return peer->createDagNodeLink(srcNodeId, srcWfId, destNodeId, destWfId, objName);
	}
	
	name = getName(objString);
	
	WfLogService_var wfls =
		ORBMgr::getMgr()->resolve<WfLogService, WfLogService_var>(WFLOGCTXT, name,this->name);
	return wfls->createDagNodeLink(srcNodeId, srcWfId, destNodeId, destWfId);
}

void DIETForwarder::createDagNodePort(const char* dagNodePortId, const char* portDirection, const char* dagNodeId, const char* wfId, const char* objName)
{
  string objString(objName);
	string name;

	if (!remoteCall(objString)) {
		return peer->createDagNodePort(dagNodePortId, portDirection, dagNodeId, wfId, objName);
	}
	
	name = getName(objString);
	
	WfLogService_var wfls =
		ORBMgr::getMgr()->resolve<WfLogService, WfLogService_var>(WFLOGCTXT, name,this->name);
	return wfls->createDagNodePort(dagNodePortId, portDirection, dagNodeId, wfId);
}

void DIETForwarder::createDataElements(const char* dataId, const char* elementIdList, const char* objName)
{
  string objString(objName);
	string name;

	if (!remoteCall(objString)) {
		return peer->createDataElements(dataId, elementIdList, objName);
	}
	
	name = getName(objString);
	
	WfLogService_var wfls =
		ORBMgr::getMgr()->resolve<WfLogService, WfLogService_var>(WFLOGCTXT, name,this->name);
	return wfls->createDataElements(dataId, elementIdList);
}

void DIETForwarder::createSinkData(const char* sinkId, const char* wfId, const char* dataId, const char* objName)
{
  string objString(objName);
	string name;

	if (!remoteCall(objString)) {
		return peer->createSinkData(sinkId, wfId, dataId, objName);
	}
	
	name = getName(objString);
	
	WfLogService_var wfls =
		ORBMgr::getMgr()->resolve<WfLogService, WfLogService_var>(WFLOGCTXT, name,this->name);
	return wfls->createSinkData(sinkId, wfId, dataId);
}

void DIETForwarder::createSourceDataTree(const char* sourceId, const char* wfId, const char* dataIdTree, const char* objName)
{
  string objString(objName);
	string name;

	if (!remoteCall(objString)) {
		return peer->createSourceDataTree(sourceId, wfId, dataIdTree, objName);
	}
	
	name = getName(objString);
	
	WfLogService_var wfls =
		ORBMgr::getMgr()->resolve<WfLogService, WfLogService_var>(WFLOGCTXT, name,this->name);
	return wfls->createSourceDataTree(sourceId, wfId, dataIdTree);
}

void DIETForwarder::initWorkflow(const char* wfId, const char* wfName, const char* parentWfId, const char* objName)
{
  string objString(objName);
	string name;

	if (!remoteCall(objString)) {
		return peer->initWorkflow(wfId, wfName, parentWfId, objName);
	}
	
	name = getName(objString);
	
	WfLogService_var wfls =
		ORBMgr::getMgr()->resolve<WfLogService, WfLogService_var>(WFLOGCTXT, name,this->name);
	return wfls->initWorkflow(wfId, wfName, parentWfId);
}

void DIETForwarder::setInPortDependencies(const char* dagNodePortId, const char* dagNodeId, const char* wfId, const char* dependencies, const char* objName)
{
  string objString(objName);
	string name;

	if (!remoteCall(objString)) {
		return peer->setInPortDependencies(dagNodePortId, dagNodeId, wfId, dependencies, objName);
	}
	
	name = getName(objString);
	
	WfLogService_var wfls =
		ORBMgr::getMgr()->resolve<WfLogService, WfLogService_var>(WFLOGCTXT, name,this->name);
	return wfls->setInPortDependencies(dagNodePortId, dagNodeId, wfId, dependencies);
}

void DIETForwarder::updateDag(const char* dagId, const char* wfId, const char* dagState, const char* data, const char* objName)
{
  string objString(objName);
	string name;

	if (!remoteCall(objString)) {
		return peer->updateDag(dagId, wfId, dagState, data, objName);
	}
	
	name = getName(objString);
	
	WfLogService_var wfls =
		ORBMgr::getMgr()->resolve<WfLogService, WfLogService_var>(WFLOGCTXT, name,this->name);
	return wfls->updateDag(dagId, wfId, dagState, data);
}

void DIETForwarder::updateWorkflow(const char* wfId, const char* wfState, const char* data, const char* objName)
{
  string objString(objName);
	string name;

	if (!remoteCall(objString)) {
		return peer->updateWorkflow(wfId, wfState, data, objName);
	}
	
	name = getName(objString);
	
	WfLogService_var wfls =
		ORBMgr::getMgr()->resolve<WfLogService, WfLogService_var>(WFLOGCTXT, name,this->name);
	return wfls->updateWorkflow(wfId, wfState, data);
}

void DIETForwarder::nodeIsDone(const char* node_id, const char* wfId, const char* objName)
{
	string objString(objName);
	string name;

	if (!remoteCall(objString)) {
		return peer->nodeIsDone(node_id, wfId, objString.c_str());
	}
	
	name = getName(objString);
	
	WfLogService_var wfls =
		ORBMgr::getMgr()->resolve<WfLogService, WfLogService_var>(WFLOGCTXT, name,this->name);
	return wfls->nodeIsDone(node_id, wfId);
}

void DIETForwarder::nodeIsFailed(const char* node_id, const char* wfId, const char* objName)
{
	string objString(objName);
	string name;

	if (!remoteCall(objString)) {
		return peer->nodeIsFailed(node_id, wfId, objString.c_str());
	}
	
	name = getName(objString);
	
	WfLogService_var wfls =
		ORBMgr::getMgr()->resolve<WfLogService, WfLogService_var>(WFLOGCTXT, name,this->name);
	return wfls->nodeIsFailed(node_id, wfId);
}

void DIETForwarder::nodeIsReady(const char* node_id, const char* wfId, const char* objName)
{
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return peer->nodeIsReady(node_id, wfId, objString.c_str());
	}
	
	name = getName(objString);
		
	WfLogService_var wfls =
	ORBMgr::getMgr()->resolve<WfLogService, WfLogService_var>(WFLOGCTXT, name, this->name);
	return wfls->nodeIsReady(node_id, wfId);	
}

void DIETForwarder::nodeIsRunning(const char* node_id, const char* wfId, const char* objName)
{
	string objString(objName);
	string name;

	if (!remoteCall(objString)) {
		return peer->nodeIsRunning(node_id, wfId, objString.c_str());
	}
	
	name = getName(objString);
	
	WfLogService_var wfls =
		ORBMgr::getMgr()->resolve<WfLogService, WfLogService_var>(WFLOGCTXT, name, this->name);
	return wfls->nodeIsRunning(node_id, wfId);
	
}

void DIETForwarder::nodeIsStarting(const char* node_id, const char* wfId, const char* pbName, const char* hostname, const char* objName)
{
	string objString(objName);
	string name;
	
	if (!remoteCall(objString)) {
		return peer->nodeIsStarting(node_id, wfId, pbName, hostname, objString.c_str());
	}
	
	name = getName(objString);
	
	WfLogService_var wfls =
		ORBMgr::getMgr()->resolve<WfLogService, WfLogService_var>(WFLOGCTXT, name, this->name);
	return wfls->nodeIsStarting(node_id, wfId, pbName, hostname);
}
