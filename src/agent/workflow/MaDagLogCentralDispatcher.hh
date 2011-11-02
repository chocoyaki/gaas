/**
* @file MaDagLogCentralDispatcher.hh
* 
* @brief  The MA DAG LogCentral Dispatcher 
* 
* @author - Benjamin Isnard (Benjamin.Isnard@ens-lyon.fr)
* 
* @section Licence
*   |LICENSE|                                                                
*/
/* $Id$
 * $Log$
 * Revision 1.3  2011/01/13 23:01:09  ecaron
 * Add header for MaDag LogCentral Dispatcher
 *
 ****************************************************************************/

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
#ifdef USE_LOG_SERVICE
#include "DietLogComponent.hh"
#endif

using events::EventDispatcher;
using events::EventFrom;
using events::EventStandardMsg;
using madag::MultiWfScheduler;


class MaDagLogCentralDispatcher : public EventDispatcher {
public:
#ifdef USE_LOG_SERVICE
  explicit MaDagLogCentralDispatcher(DietLogComponent* LC);

  void
  onMultiWfSchedulerCreation(
    const EventFrom<MultiWfScheduler,
                    EventStandardMsg<MultiWfScheduler,
                                     MultiWfScheduler::CONSTR> >* event);

  void
  onDagNodeReady(
    const EventFrom<DagNode,
                    EventStandardMsg<DagNode, DagNode::READY> >* event);

private:
  DietLogComponent* myLC;
#endif
};


#endif
