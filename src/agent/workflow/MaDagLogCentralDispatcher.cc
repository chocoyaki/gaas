#include "MaDagLogCentralDispatcher.hh"
#include "debug.hh"

MaDagLogCentralDispatcher::MaDagLogCentralDispatcher(DietLogComponent* LC): myLC(LC)
{
  if (myLC == NULL) {
    INTERNAL_ERROR("Invalid MaDagLogCentralDispatcher constructor parameter", 1);
  }
  // register event handlers
  registerEventFunc(this, &MaDagLogCentralDispatcher::onMultiWfSchedulerCreation);
  registerEventFunc(this, &MaDagLogCentralDispatcher::onDagNodeReady);
}

void 
MaDagLogCentralDispatcher::onMultiWfSchedulerCreation(const events::EventFrom< MultiWfScheduler, events::EventStandardMsg< MultiWfScheduler, madag::MultiWfScheduler::CONSTR > >* event)
{
  myLC->maDagSchedulerType(event->getData().c_str());
}

void 
MaDagLogCentralDispatcher::onDagNodeReady(const events::EventFrom< DagNode, events::EventStandardMsg< DagNode, DagNode::READY > >* event)
{
  myLC->logWfNodeReady(event->getSource()->getDag()->getId().c_str(),
		       event->getSource()->getId().c_str());
}
