#ifndef CLTWFLOGCENTRALDISPATCHER_HH_
#define CLTWFLOGCENTRALDISPATCHER_HH_

#include "EventDispatcher.hh"
#include "EventTypes.hh"
#include "DagNodePort.hh"
#include "DagNode.hh"
#include "Dag.hh"
#include "FWorkflow.hh"
#include "DagNodeLauncher.hh"
#include "DietLogComponent.hh"

using namespace events;

class CltWfLogCentralDispatcher : public EventDispatcher {
  public:
    CltWfLogCentralDispatcher(DietLogComponent* LC);
    
    void onDagNodeReady(const EventFrom<DagNode, EventStandardMsg<DagNode, DagNode::READY> >* event);
    void onDagNodeStart(const EventFrom<DagNodeLauncher, EventStandardMsg<DagNodeLauncher, DagNode::START> >* event);
    void onDagNodeFinish(const EventFrom<DagNodeLauncher, EventStandardMsg<DagNodeLauncher, DagNode::FINISH> >* event);
    void onDagNodeFailed(const EventFrom<DagNodeLauncher, EventStandardMsg<DagNodeLauncher, DagNode::FAILED> >* event);
    
  private:
    DietLogComponent*	myLC;
};


#endif