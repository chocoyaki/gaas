/**
 * @file WfLogService.cc
 *
 * @brief  DIET forwarder implementation - Workflow Logservice forwarder implem
 *
 * @author Gael Le Mahec   (gael.le.mahec@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


#include "DIETForwarder.hh"
#include "ORBMgr.hh"
#include <string>

void
DIETForwarder::createDag(const char *dagId,
                         const char *wfId,
                         const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->createDag(dagId, wfId, objName);
  }

  name = getName(objString);

  WfLogService_var wfls =
    ORBMgr::getMgr()->resolve<WfLogService, WfLogService_var>(WFLOGCTXT,
                                                              name,
                                                              this->name);
  return wfls->createDag(dagId, wfId);
} // createDag

void
DIETForwarder::createDagNode(const char *dagNodeId, const char *dagId,
                             const char *wfId, const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->createDagNode(dagNodeId, dagId, wfId, objName);
  }

  name = getName(objString);

  WfLogService_var wfls =
    ORBMgr::getMgr()->resolve<WfLogService, WfLogService_var>(WFLOGCTXT,
                                                              name,
                                                              this->name);
  return wfls->createDagNode(dagNodeId, dagId, wfId);
} // createDagNode

void
DIETForwarder::createDagNodeData(const char *dagNodeId, const char *wfId,
                                 const char *dagNodePortId, const char *dataId,
                                 const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->createDagNodeData(dagNodeId, wfId, dagNodePortId,
                                        dataId, objName);
  }

  name = getName(objString);

  WfLogService_var wfls =
    ORBMgr::getMgr()->resolve<WfLogService, WfLogService_var>(WFLOGCTXT,
                                                              name,
                                                              this->name);
  return wfls->createDagNodeData(dagNodeId, wfId, dagNodePortId, dataId);
} // createDagNodeData

void
DIETForwarder::createDagNodeLink(const char *srcNodeId, const char *srcWfId,
                                 const char *destNodeId, const char *destWfId,
                                 const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->createDagNodeLink(srcNodeId, srcWfId, destNodeId,
                                        destWfId, objName);
  }

  name = getName(objString);

  WfLogService_var wfls =
    ORBMgr::getMgr()->resolve<WfLogService, WfLogService_var>(WFLOGCTXT,
                                                              name,
                                                              this->name);
  return wfls->createDagNodeLink(srcNodeId, srcWfId, destNodeId, destWfId);
} // createDagNodeLink

void
DIETForwarder::createDagNodePort(const char *dagNodePortId,
                                 const char *portDirection,
                                 const char *dagNodeId,
                                 const char *wfId,
                                 const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->createDagNodePort(dagNodePortId, portDirection,
                                        dagNodeId, wfId, objName);
  }

  name = getName(objString);

  WfLogService_var wfls =
    ORBMgr::getMgr()->resolve<WfLogService, WfLogService_var>(WFLOGCTXT,
                                                              name,
                                                              this->name);
  return wfls->createDagNodePort(dagNodePortId, portDirection, dagNodeId, wfId);
} // createDagNodePort

void
DIETForwarder::createDataElements(const char *dataId,
                                  const char *elementIdList,
                                  const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->createDataElements(dataId, elementIdList, objName);
  }

  name = getName(objString);

  WfLogService_var wfls =
    ORBMgr::getMgr()->resolve<WfLogService, WfLogService_var>(WFLOGCTXT,
                                                              name,
                                                              this->name);
  return wfls->createDataElements(dataId, elementIdList);
} // createDataElements

void
DIETForwarder::createSinkData(const char *sinkId, const char *wfId,
                              const char *dataId, const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->createSinkData(sinkId, wfId, dataId, objName);
  }

  name = getName(objString);

  WfLogService_var wfls =
    ORBMgr::getMgr()->resolve<WfLogService, WfLogService_var>(WFLOGCTXT,
                                                              name,
                                                              this->name);
  return wfls->createSinkData(sinkId, wfId, dataId);
} // createSinkData

void
DIETForwarder::createSourceDataTree(const char *sourceId,
                                    const char *wfId,
                                    const char *dataIdTree,
                                    const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->createSourceDataTree(sourceId, wfId,
                                           dataIdTree, objName);
  }

  name = getName(objString);

  WfLogService_var wfls =
    ORBMgr::getMgr()->resolve<WfLogService, WfLogService_var>(WFLOGCTXT,
                                                              name,
                                                              this->name);
  return wfls->createSourceDataTree(sourceId, wfId, dataIdTree);
} // createSourceDataTree

void
DIETForwarder::initWorkflow(const char *wfId, const char *wfName,
                            const char *parentWfId, const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->initWorkflow(wfId, wfName, parentWfId, objName);
  }

  name = getName(objString);

  WfLogService_var wfls =
    ORBMgr::getMgr()->resolve<WfLogService, WfLogService_var>(WFLOGCTXT,
                                                              name,
                                                              this->name);
  return wfls->initWorkflow(wfId, wfName, parentWfId);
} // initWorkflow

void
DIETForwarder::setInPortDependencies(const char *dagNodePortId,
                                     const char *dagNodeId,
                                     const char *wfId,
                                     const char *dependencies,
                                     const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->setInPortDependencies(dagNodePortId, dagNodeId,
                                            wfId, dependencies, objName);
  }

  name = getName(objString);

  WfLogService_var wfls =
    ORBMgr::getMgr()->resolve<WfLogService, WfLogService_var>(WFLOGCTXT,
                                                              name,
                                                              this->name);
  return wfls->setInPortDependencies(dagNodePortId, dagNodeId,
                                     wfId, dependencies);
} // setInPortDependencies

void
DIETForwarder::updateDag(const char *dagId, const char *wfId,
                         const char *dagState, const char *data,
                         const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->updateDag(dagId, wfId, dagState, data, objName);
  }

  name = getName(objString);

  WfLogService_var wfls =
    ORBMgr::getMgr()->resolve<WfLogService, WfLogService_var>(WFLOGCTXT,
                                                              name,
                                                              this->name);
  return wfls->updateDag(dagId, wfId, dagState, data);
} // updateDag

void
DIETForwarder::updateWorkflow(const char *wfId, const char *wfState,
                              const char *data, const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->updateWorkflow(wfId, wfState, data, objName);
  }

  name = getName(objString);

  WfLogService_var wfls =
    ORBMgr::getMgr()->resolve<WfLogService, WfLogService_var>(WFLOGCTXT,
                                                              name,
                                                              this->name);
  return wfls->updateWorkflow(wfId, wfState, data);
} // updateWorkflow

void
DIETForwarder::nodeIsDone(const char *node_id, const char *wfId,
                          const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->nodeIsDone(node_id, wfId, objString.c_str());
  }

  name = getName(objString);

  WfLogService_var wfls =
    ORBMgr::getMgr()->resolve<WfLogService, WfLogService_var>(WFLOGCTXT,
                                                              name,
                                                              this->name);
  return wfls->nodeIsDone(node_id, wfId);
} // nodeIsDone

void
DIETForwarder::nodeIsFailed(const char *node_id, const char *wfId,
                            const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->nodeIsFailed(node_id, wfId, objString.c_str());
  }

  name = getName(objString);

  WfLogService_var wfls =
    ORBMgr::getMgr()->resolve<WfLogService, WfLogService_var>(WFLOGCTXT,
                                                              name,
                                                              this->name);
  return wfls->nodeIsFailed(node_id, wfId);
} // nodeIsFailed

void
DIETForwarder::nodeIsReady(const char *node_id, const char *wfId,
                           const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->nodeIsReady(node_id, wfId, objString.c_str());
  }

  name = getName(objString);

  WfLogService_var wfls =
    ORBMgr::getMgr()->resolve<WfLogService, WfLogService_var>(WFLOGCTXT,
                                                              name,
                                                              this->name);
  return wfls->nodeIsReady(node_id, wfId);
} // nodeIsReady

void
DIETForwarder::nodeIsRunning(const char *node_id, const char *wfId,
                             const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->nodeIsRunning(node_id, wfId, objString.c_str());
  }

  name = getName(objString);

  WfLogService_var wfls =
    ORBMgr::getMgr()->resolve<WfLogService, WfLogService_var>(WFLOGCTXT,
                                                              name,
                                                              this->name);
  return wfls->nodeIsRunning(node_id, wfId);
} // nodeIsRunning

void
DIETForwarder::nodeIsStarting(const char *node_id, const char *wfId,
                              const char *pbName, const char *hostname,
                              const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->nodeIsStarting(node_id, wfId,
                                     pbName, hostname,
                                     objString.c_str());
  }

  name = getName(objString);

  WfLogService_var wfls =
    ORBMgr::getMgr()->resolve<WfLogService, WfLogService_var>(WFLOGCTXT,
                                                              name,
                                                              this->name);
  return wfls->nodeIsStarting(node_id, wfId, pbName, hostname);
} // nodeIsStarting
