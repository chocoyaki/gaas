/****************************************************************************/
/* Workflows LogService                                                     */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Gaël Le Mahec (gael.le.mahec@ens-lyon.fr)                           */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.4  2010/07/20 14:31:46  glemahec
 * Forwarder robustness + bug corrections
 *
 * Revision 1.3  2010/07/20 09:20:11  bisnard
 * integration with eclipse gui and with dietForwarder
 *
 * Revision 1.2  2010/07/14 23:45:29  bdepardo
 * Header corrections
 *
 *
 ****************************************************************************/

#ifndef WFLOGSERVICEIMPL_HH
#define WFLOGSERVICEIMPL_HH

#include "WfLogService.hh"

#include "Forwarder.hh"
#include "WfLogServiceFwdr.hh"

class WfLogServiceFwdrImpl : public POA_WfLogServiceFwdr,
			     public PortableServer::RefCountServantBase
{
protected:
  Forwarder_ptr forwarder;
  char* objName;
  
public:
  WfLogServiceFwdrImpl(Forwarder_ptr fwdr, const char* objName);
  
  virtual void
  initWorkflow(const char* wfId, const char* name, const char* parentWfId);

  virtual void
  updateWorkflow(const char* wfId, const char* wfState, const char* data);

  virtual void
  createDag(const char* dagId, const char* wfId);

  virtual void
  updateDag(const char* dagId, const char* wfId, const char* dagState, const char* data);
  
  virtual void
  createDagNode(const char* dagNodeId, const char* dagId, const char* wfId);

  virtual void
  createDagNodePort(const char* dagNodePortId, const char* portDirection,
                    const char* dagNodeId, const char* wfId);

  virtual void
  setInPortDependencies(const char* dagNodePortId, const char* dagNodeId, const char* wfId,
                        const char* dependencies);
  
  virtual void
  createDagNodeLink(const char* srcNodeId, const char* srcWfId,
                    const char* destNodeId, const char* destWfId);
                    
  virtual void
  createDagNodeData(const char* dagNodeId, const char* wfId,
                    const char* dagNodePortId, const char* dataId);

  virtual void
  createDataElements(const char* dataId, const char* elementIdList);

  virtual void
  createSourceDataTree(const char* sourceId, const char* wfId, const char* dataIdTree);

  virtual void
  createSinkData(const char* sinkId, const char* wfId, const char* dataId);

  virtual void
  nodeIsReady(const char* dagNodeId, const char* wfId);

  virtual void
  nodeIsStarting(const char* dagNodeId, const char* wfId, const char* pbName, const char* hostname);
                    
  virtual void
  nodeIsRunning(const char* dagNodeId, const char* wfId);

  virtual void
  nodeIsDone(const char* dagNodeId, const char* wfId);

  virtual void
  nodeIsFailed(const char* dagNodeId, const char* wfId);
};
#endif
