/**
 * @file MaDagLogCentralDispatcher.cc
 *
 * @brief  The MA DAG LogCentral Dispatcher
 *
 * @author  Benjamin Isnard (Benjamin.Isnard@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENCE|
 */

#include "MaDagLogCentralDispatcher.hh"
#include "debug.hh"

#ifdef USE_LOG_SERVICE
MaDagLogCentralDispatcher::MaDagLogCentralDispatcher(DietLogComponent *LC)
  : myLC(LC) {
  if (myLC == NULL) {
    INTERNAL_ERROR("Invalid MaDagLogCentralDispatcher constructor parameter", 1);
  }
  // register event handlers
  registerEventFunc(this,
                    &MaDagLogCentralDispatcher::onMultiWfSchedulerCreation);
  registerEventFunc(this, &MaDagLogCentralDispatcher::onDagNodeReady);
}

void
MaDagLogCentralDispatcher::onMultiWfSchedulerCreation(
  const EventFrom<MultiWfScheduler,
                  EventStandardMsg<MultiWfScheduler,
                                   MultiWfScheduler::CONSTR > > *event) {
  myLC->maDagSchedulerType(event->getData().c_str());
}

void
MaDagLogCentralDispatcher::onDagNodeReady(
  const EventFrom<DagNode,
                  EventStandardMsg<DagNode,
                                   DagNode::READY > > *event) {
  myLC->logWfNodeReady(event->getSource()->getDag()->getId().c_str(),
                       event->getSource()->getId().c_str());
}
#endif // ifdef USE_LOG_SERVICE
