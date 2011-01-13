/****************************************************************************/
/* Workflow Log Dispatcher                                                  */
/*                                                                          */
/* Author(s):                                                               */
/*   - Benjamin Isnard (benjamin.isnard@ens-lyon.fr)                        */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.2  2011/01/13 23:36:12  ecaron
 * Add missing header
 *
 ****************************************************************************/

#ifndef WFLOGDISPATCHER_HH_
#define WFLOGDISPATCHER_HH_

#include "EventDispatcher.hh"
#include "EventTypes.hh"
#include "DagNodePort.hh"
#include "Dag.hh"
#include "FWorkflow.hh"
#include "WfPortAdapter.hh"
#include "WfLogService.hh"

using namespace events;

class WfLogDispatcher : public EventDispatcher {
  public:
    WfLogDispatcher(std::ostream& output);
    void setWfLogService(WfLogService_var logService);
    
    // creation of object
    void onEventCreateWorkflow(const EventCreateObject<FWorkflow,FWorkflow>* event);
    void onEventCreateWfDag(const EventCreateObject<Dag,FWorkflow>* event);
    void onEventCreateDagNode(const EventCreateObject<DagNode,Dag>* event);
    void onEventCreateDagNodePort(const EventCreateObject<DagNodePort,DagNode>* event);
    void onEventCreateDagNodeLink(const EventCreateDirectedLink<DagNode,DagNode>* event);
    
    // specific event handler for a source class
    void onWorkflowInstanciate(const EventFrom< FWorkflow, EventStandardMsg<FWorkflow, FWorkflow::INST> >* event);
    void onWorkflowInstDone(const EventFrom< FWorkflow, EventStandardMsg<FWorkflow, FWorkflow::INSTDONE> >* event);
    void onWorkflowInstError(const EventFrom< FWorkflow, EventStandardMsg<FWorkflow, FWorkflow::INSTERROR> >* event);
    void onWorkflowCancelled(const EventFrom< FWorkflow, EventStandardMsg<FWorkflow, FWorkflow::CANCELLED> >* event);
    void onWorkflowCompleted(const EventFrom< FWorkflow, EventStandardMsg<FWorkflow, FWorkflow::COMPLETED> >* event);
    void onDagChangeId(const EventFrom< Dag,EventStandardMsg<Dag, Dag::MODID > >* event);
    void onDagIsEmpty(const EventFrom< Dag,EventStandardMsg<Dag, Dag::EMPTY > >* event);
    void onDagStateChange(const EventFrom< Dag, EventStandardMsg<Dag, Dag::STATE > >* event);
    void onDagNodeReady(const EventFrom<DagNode, EventStandardMsg<DagNode, DagNode::READY> >* event);
    void onDagNodeStart(const EventFrom<DagNodeLauncher, EventStandardMsg<DagNodeLauncher, DagNode::START> >* event);
    void onDagNodeFinish(const EventFrom<DagNodeLauncher, EventStandardMsg<DagNodeLauncher, DagNode::FINISH> >* event);
    void onDagNodeFailed(const EventFrom<DagNodeLauncher, EventStandardMsg<DagNodeLauncher, DagNode::FAILED> >* event);
    void onInPortDepend(const EventFrom<DagNodeInPort, EventStandardMsg<DagNodeInPort, DagNodeInPort::DEPEND> >* event);
    void onDataElements(const EventFrom<WfDataIDAdapter, EventStandardMsg<WfDataIDAdapter, WfDataIDAdapter::ELTIDLIST> >* event);
    void onDagNodeSetDataID(const EventFrom<DagNodePort, EventStandardMsg<DagNodePort, DagNodePort::DATAID> >* event);
    void onSourceDataTree(const EventFrom<FSourceNode, EventStandardMsg<FSourceNode, FSourceNode::DATATREE> >* event);
    void onSinkData(const EventFrom<FSinkNode, EventStandardMsg<FSinkNode, FSinkNode::DATAID>  >* event);
    
  private:
    bool isOnline();
    std::ostream&	myOutput;
    WfLogService_var 	myWfLogSrv;
};


#endif
