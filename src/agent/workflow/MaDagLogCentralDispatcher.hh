#ifndef MADAGLOGCENTRALDISPATCHER_HH_
#define MADAGLOGCENTRALDISPATCHER_HH_

#include "EventDispatcher.hh"
#include "EventTypes.hh"
#include "MultiWfScheduler.hh"
#include "DagNodePort.hh"
#include "DagNode.hh"
#include "Dag.hh"
#include "FWorkflow.hh"
#include "DagNodeLauncher.hh"
#include "DietLogComponent.hh"

using namespace events;
using namespace madag;

class MaDagLogCentralDispatcher : public EventDispatcher {
  public:
    MaDagLogCentralDispatcher(DietLogComponent* LC);
    
    void onMultiWfSchedulerCreation(const EventFrom<MultiWfScheduler, EventStandardMsg<MultiWfScheduler, MultiWfScheduler::CONSTR> >* event);
    
    void onDagNodeReady(const EventFrom<DagNode, EventStandardMsg<DagNode, DagNode::READY> >* event);
//     void onDagNodeStart(const EventFrom<DagNodeLauncher, EventStandardMsg<DagNodeLauncher, DagNode::START> >* event);
//     void onDagNodeFinish(const EventFrom<DagNodeLauncher, EventStandardMsg<DagNodeLauncher, DagNode::FINISH> >* event);
//     void onDagNodeFailed(const EventFrom<DagNodeLauncher, EventStandardMsg<DagNodeLauncher, DagNode::FAILED> >* event);

  private:
    DietLogComponent*	myLC;
};


#endif
