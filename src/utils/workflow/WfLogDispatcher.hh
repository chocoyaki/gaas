/**
* @file  WfLogDispatcher.hh
* 
* @brief Workflow Log Dispatcher
* 
* @author  Benjamin Isnard (benjamin.isnard@ens-lyon.fr)
* 
* @section Licence
*   |LICENCE|                                                                
*/


#ifndef WFLOGDISPATCHER_HH_
#define WFLOGDISPATCHER_HH_

#include "EventDispatcher.hh"
#include "EventTypes.hh"
#include "DagNodePort.hh"
#include "Dag.hh"
#include "FWorkflow.hh"
#include "WfPortAdapter.hh"
#include "WfLogService.hh"
#ifdef WIN32
#define DIET_API_LIB __declspec(dllexport)
#else
#define DIET_API_LIB
#endif

class DIET_API_LIB WfLogDispatcher : public events::EventDispatcher {
public:
  explicit WfLogDispatcher(std::ostream& output);
  void
  setWfLogService(WfLogService_var logService);

  // creation of object
  void
  onEventCreateWorkflow(
    const events::EventCreateObject<FWorkflow, FWorkflow>* event);

  void
  onEventCreateWfDag(const events::EventCreateObject<Dag, FWorkflow>* event);

  void
  onEventCreateDagNode(const events::EventCreateObject<DagNode, Dag>* event);

  void
  onEventCreateDagNodePort(
    const events::EventCreateObject<DagNodePort, DagNode>* event);

  void
  onEventCreateDagNodeLink(
    const events::EventCreateDirectedLink<DagNode, DagNode>* event);

  // specific event handler for a source class
  void
  onWorkflowInstanciate(
    const events::EventFrom<FWorkflow,
                            events::EventStandardMsg<FWorkflow,
                                                     FWorkflow::INST> >* event);
  void
  onWorkflowInstDone(
    const events::EventFrom<FWorkflow,
                            events::EventStandardMsg<FWorkflow,
                                                     FWorkflow::INSTDONE> >*
    event);

  void
  onWorkflowInstError(
    const events::EventFrom<FWorkflow,
                            events::EventStandardMsg<FWorkflow,
                                                     FWorkflow::INSTERROR> >*
    event);

  void
  onWorkflowCancelled(
    const events::EventFrom<FWorkflow,
                            events::EventStandardMsg<FWorkflow,
                                                     FWorkflow::CANCELLED> >*
    event);

  void
  onWorkflowCompleted(
    const events::EventFrom<FWorkflow,
                            events::EventStandardMsg<FWorkflow,
                                                     FWorkflow::COMPLETED> >*
    event);

  void
  onDagChangeId(const events::EventFrom<Dag,
                                        events::EventStandardMsg<Dag,
                                                                 Dag::MODID > >*
                event);

  void
  onDagIsEmpty(
    const events::EventFrom<Dag,
                            events::EventStandardMsg<Dag,
                                                     Dag::EMPTY > >* event);

  void
  onDagStateChange(
    const events::EventFrom<Dag,
                            events::EventStandardMsg<Dag,
                                                     Dag::STATE > >* event);

  void
  onDagNodeReady(
    const events::EventFrom<DagNode,
                            events::EventStandardMsg<DagNode,
                                                     DagNode::READY> >* event);

  void
  onDagNodeStart(
    const events::EventFrom<DagNodeLauncher,
                            events::EventStandardMsg<DagNodeLauncher,
                                                     DagNode::START> >* event);

  void
  onDagNodeFinish(
    const events::EventFrom<DagNodeLauncher,
                            events::EventStandardMsg<DagNodeLauncher,
                                                     DagNode::FINISH> >* event);

  void
  onDagNodeFailed(
    const events::EventFrom<DagNodeLauncher,
                            events::EventStandardMsg<DagNodeLauncher,
                                                     DagNode::FAILED> >* event);

  void
  onInPortDepend(
    const events::EventFrom<DagNodeInPort,
                            events::EventStandardMsg<DagNodeInPort,
                                                     DagNodeInPort::DEPEND> >*
    event);

  void
  onDataElements(
    const events::EventFrom<WfDataIDAdapter,
                            events::EventStandardMsg<WfDataIDAdapter,
                                                     WfDataIDAdapter::ELTIDLIST> >*
    event);

  void
  onDagNodeSetDataID(
    const events::EventFrom<DagNodePort,
                            events::EventStandardMsg<DagNodePort,
                                                     DagNodePort::DATAID> >*
    event);

  void
  onSourceDataTree(
    const events::EventFrom<FSourceNode,
                            events::EventStandardMsg<FSourceNode,
                                                     FSourceNode::DATATREE> >*
    event);

  void
  onSinkData(
    const events::EventFrom<FSinkNode,
                            events::EventStandardMsg<FSinkNode,
                                                     FSinkNode::DATAID>  >*
    event);

private:
  bool
  isOnline();

  std::ostream& myOutput;
  WfLogService_var myWfLogSrv;
};


#endif
