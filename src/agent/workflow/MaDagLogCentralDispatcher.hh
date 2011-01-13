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
using namespace events;
using namespace madag;

class MaDagLogCentralDispatcher : public EventDispatcher {
  public:
#ifdef USE_LOG_SERVICE
    MaDagLogCentralDispatcher(DietLogComponent* LC);
    
    void onMultiWfSchedulerCreation(const EventFrom<MultiWfScheduler, EventStandardMsg<MultiWfScheduler, MultiWfScheduler::CONSTR> >* event);
    
    void onDagNodeReady(const EventFrom<DagNode, EventStandardMsg<DagNode, DagNode::READY> >* event);
//     void onDagNodeStart(const EventFrom<DagNodeLauncher, EventStandardMsg<DagNodeLauncher, DagNode::START> >* event);
//     void onDagNodeFinish(const EventFrom<DagNodeLauncher, EventStandardMsg<DagNodeLauncher, DagNode::FINISH> >* event);
//     void onDagNodeFailed(const EventFrom<DagNodeLauncher, EventStandardMsg<DagNodeLauncher, DagNode::FAILED> >* event);

  private:
    DietLogComponent*	myLC;
#endif
};


#endif
