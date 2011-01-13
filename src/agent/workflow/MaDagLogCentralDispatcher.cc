/****************************************************************************/
/* The MA DAG LogCentral Dispatcher                                         */
/*                                                                          */
/* Author(s):                                                               */
/*   - Benjamin Isnard (Benjamin.Isnard@ens-lyon.fr)                        */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.3  2011/01/13 23:01:09  ecaron
 * Add header for MaDag LogCentral Dispatcher
 *
 ****************************************************************************/

#include "MaDagLogCentralDispatcher.hh"
#include "debug.hh"

#ifdef USE_LOG_SERVICE
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
#endif
