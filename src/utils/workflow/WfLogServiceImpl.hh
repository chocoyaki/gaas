/**
* @file  WfLogServiceImpl.hh
* 
* @brief Workflow LogService 
* 
* @author  Gaël Le Mahec (gael.le.mahec@ens-lyon.fr)
* 
* @section Licence
*   |LICENSE|                                                                
*/


#ifndef WFLOGSERVICEIMPL_HH
#define WFLOGSERVICEIMPL_HH

#include "WfLogService.hh"
#include "Forwarder.hh"
#include "WfLogServiceFwdr.hh"

class WfLogServiceFwdrImpl : public POA_WfLogServiceFwdr,
                             public PortableServer::RefCountServantBase {
public:
  WfLogServiceFwdrImpl(Forwarder_ptr fwdr, const char* objName);

  virtual void
  initWorkflow(const char* wfId, const char* name, const char* parentWfId);

  virtual void
  updateWorkflow(const char* wfId, const char* wfState, const char* data);

  virtual void
  createDag(const char* dagId, const char* wfId);

  virtual void
  updateDag(const char* dagId, const char* wfId,
            const char* dagState, const char* data);

  virtual void
  createDagNode(const char* dagNodeId, const char* dagId, const char* wfId);

  virtual void
  createDagNodePort(const char* dagNodePortId, const char* portDirection,
                    const char* dagNodeId, const char* wfId);

  virtual void
  setInPortDependencies(const char* dagNodePortId, const char* dagNodeId,
                        const char* wfId, const char* dependencies);

  virtual void
  createDagNodeLink(const char* srcNodeId, const char* srcWfId,
                    const char* destNodeId, const char* destWfId);

  virtual void
  createDagNodeData(const char* dagNodeId, const char* wfId,
                    const char* dagNodePortId, const char* dataId);

  virtual void
  createDataElements(const char* dataId, const char* elementIdList);

  virtual void
  createSourceDataTree(const char* sourceId, const char* wfId,
                       const char* dataIdTree);

  virtual void
  createSinkData(const char* sinkId, const char* wfId, const char* dataId);

  virtual void
  nodeIsReady(const char* dagNodeId, const char* wfId);

  virtual void
  nodeIsStarting(const char* dagNodeId, const char* wfId,
                 const char* pbName, const char* hostname);

  virtual void
  nodeIsRunning(const char* dagNodeId, const char* wfId);

  virtual void
  nodeIsDone(const char* dagNodeId, const char* wfId);

  virtual void
  nodeIsFailed(const char* dagNodeId, const char* wfId);

protected:
  Forwarder_ptr forwarder;
  char* objName;
};
#endif
