/**
 * @file  CltWfLogCentralDispatcher.cc
 *
 * @brief   Control Workflow LogCentral Dispatcher
 *
 * @author  Benjamin ISNARD (benjamin.isnard@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


#include "CltWfLogCentralDispatcher.hh"
#include "debug.hh"

#ifdef USE_LOG_SERVICE
CltWfLogCentralDispatcher::CltWfLogCentralDispatcher(DietLogComponent *LC):
  myLC(LC) {
  if (myLC == NULL) {
    INTERNAL_ERROR("Invalid WfLogCentralDispatcher constructor parameter", 1);
  }
  // register event handlers
  registerEventFunc(this, &CltWfLogCentralDispatcher::onDagNodeStart);
  registerEventFunc(this, &CltWfLogCentralDispatcher::onDagNodeFinish);
  registerEventFunc(this, &CltWfLogCentralDispatcher::onDagNodeFailed);
}

void
CltWfLogCentralDispatcher::onDagNodeStart(
  const events::EventFrom< DagNodeLauncher,
                           events::EventStandardMsg< DagNodeLauncher,
                                                     DagNode::START > > *event)
{
  if (event->getSource()->isSeDDefined()) {
    myLC->logWfNodeStart(event->getSource()->getNode()->getDag()->getId().c_str(),
                         event->getSource()->getNode()->getId().c_str(),
                         event->getSource()->getSeDName().c_str(),
                         event->getSource()->getNode()->getPbName().c_str(),
                         event->getSource()->getReqId()
                         );
  } else {
    myLC->logWfNodeStart(event->getSource()->getNode()->getDag()->getId().c_str(),
                         event->getSource()->getNode()->getId().c_str()
                         );
  }
} // onDagNodeStart

void
CltWfLogCentralDispatcher::onDagNodeFinish(
  const events::EventFrom< DagNodeLauncher,
                           events::EventStandardMsg< DagNodeLauncher,
                                                     DagNode::FINISH > > *event)
{
  myLC->logWfNodeFinish(event->getSource()->getNode()->getDag()->getId().c_str(),
                        event->getSource()->getNode()->getId().c_str()
                        );
}

void
CltWfLogCentralDispatcher::onDagNodeFailed(
  const events::EventFrom< DagNodeLauncher,
                           events::EventStandardMsg< DagNodeLauncher,
                                                     DagNode::FAILED > > *event)
{
  myLC->logWfNodeFailed(event->getSource()->getNode()->getDag()->getId().c_str(),
                        event->getSource()->getNode()->getId().c_str()
                        );
}

#endif // ifdef USE_LOG_SERVICE
