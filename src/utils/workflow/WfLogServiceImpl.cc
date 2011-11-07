/**
* @file  WfLogServiceImpl.cc
* 
* @brief Workflow LogService 
* 
* @author  Gaël Le Mahec (gael.le.mahec@ens-lyon.fr)
* 
* @section Licence
*   |LICENCE|                                                                
*/

#include "WfLogServiceImpl.hh"

WfLogServiceFwdrImpl::WfLogServiceFwdrImpl(Forwarder_ptr fwdr,
                                           const char* objName) {
  this->forwarder = Forwarder::_duplicate(fwdr);
  this->objName = CORBA::string_dup(objName);
}

void
WfLogServiceFwdrImpl::createDag(const char* dagId, const char* wfId) {
  forwarder->createDag(dagId, wfId, objName);
}

void
WfLogServiceFwdrImpl::createDagNode(const char* dagNodeId, const char* dagId,
                                    const char* wfId) {
  forwarder->createDagNode(dagNodeId, dagId, wfId, objName);
}

void
WfLogServiceFwdrImpl::createDagNodeData(const char* dagNodeId,
                                        const char* wfId,
                                        const char* dagNodePortId,
                                        const char* dataId) {
  forwarder->createDagNodeData(dagNodeId, wfId,
                               dagNodePortId, dataId,
                               objName);
}

void
WfLogServiceFwdrImpl::createDagNodeLink(const char* srcNodeId,
                                        const char* srcWfId,
                                        const char* destNodeId,
                                        const char* destWfId) {
  forwarder->createDagNodeLink(srcNodeId, srcWfId,
                               destNodeId, destWfId, objName);
}

void
WfLogServiceFwdrImpl::createDagNodePort(const char* dagNodePortId,
                                        const char* portDirection,
                                        const char* dagNodeId,
                                        const char* wfId) {
  forwarder->createDagNodePort(dagNodePortId, portDirection,
                               dagNodeId, wfId, objName);
}

void
WfLogServiceFwdrImpl::createDataElements(const char* dataId,
                                         const char* elementIdList) {
  forwarder->createDataElements(dataId, elementIdList, objName);
}

void
WfLogServiceFwdrImpl::createSinkData(const char* sinkId, const char* wfId,
                                     const char* dataId) {
  forwarder->createSinkData(sinkId, wfId, dataId, objName);
}

void
WfLogServiceFwdrImpl::createSourceDataTree(const char* sourceId,
                                           const char* wfId,
                                           const char* dataIdTree) {
  forwarder->createSourceDataTree(sourceId, wfId, dataIdTree, objName);
}

void
WfLogServiceFwdrImpl::initWorkflow(const char* wfId, const char* name,
                                   const char* parentWfId) {
  forwarder->initWorkflow(wfId, name, parentWfId, objName);
}

void
WfLogServiceFwdrImpl::nodeIsDone(const char* dagNodeId, const char* wfId) {
  forwarder->nodeIsDone(dagNodeId, wfId, objName);
}

void
WfLogServiceFwdrImpl::nodeIsFailed(const char* dagNodeId, const char* wfId) {
  forwarder->nodeIsFailed(dagNodeId, wfId, objName);
}

void
WfLogServiceFwdrImpl::nodeIsReady(const char* dagNodeId, const char* wfId) {
  forwarder->nodeIsReady(dagNodeId, wfId, objName);
}

void
WfLogServiceFwdrImpl::nodeIsRunning(const char* dagNodeId, const char* wfId) {
  forwarder->nodeIsRunning(dagNodeId, wfId, objName);
}

void
WfLogServiceFwdrImpl::nodeIsStarting(const char* dagNodeId, const char* wfId,
                                     const char* pbName, const char* hostname) {
  forwarder->nodeIsStarting(dagNodeId, wfId, pbName, hostname, objName);
}

void
WfLogServiceFwdrImpl::setInPortDependencies(const char* dagNodePortId,
                                            const char* dagNodeId,
                                            const char* wfId,
                                            const char* dependencies) {
  forwarder->setInPortDependencies(dagNodePortId, dagNodeId,
                                   wfId, dependencies, objName);
}

void
WfLogServiceFwdrImpl::updateDag(const char* dagId, const char* wfId,
                                const char* dagState, const char* data) {
  forwarder->updateDag(dagId, wfId, dagState, data, objName);
}

void
WfLogServiceFwdrImpl::updateWorkflow(const char* wfId, const char* wfState,
                                     const char* data) {
  forwarder->updateWorkflow(wfId, wfState, data, objName);
}
