/**
* @file  CltWfLogCentralDispatcher.hh
* 
* @brief   Control Workflow LogCentral Dispatcher  
* 
* @author  - Benjamin ISNARD (benjamin.isnard@ens-lyon.fr)
* 
* @section Licence
*   |LICENSE|                                                                
*/
/* $Id$
 * $Log$
 * Revision 1.3  2011/01/13 23:38:19  ecaron
 * Add missing header
 *
 ****************************************************************************/

#ifndef CLTWFLOGCENTRALDISPATCHER_HH_
#define CLTWFLOGCENTRALDISPATCHER_HH_

#include "EventDispatcher.hh"
#include "EventTypes.hh"
#include "DagNodePort.hh"
#include "DagNode.hh"
#include "Dag.hh"
#include "FWorkflow.hh"
#include "DagNodeLauncher.hh"
#ifdef USE_LOG_SERVICE
#include "DietLogComponent.hh"
#endif

class CltWfLogCentralDispatcher : public events::EventDispatcher {
public:
#ifdef USE_LOG_SERVICE
  explicit CltWfLogCentralDispatcher(DietLogComponent* LC);

  void
  onDagNodeReady(
    const events::EventFrom<DagNode,
    events::EventStandardMsg<DagNode, DagNode::READY> >*
    event);

  void
  onDagNodeStart(
    const events::EventFrom<DagNodeLauncher,
    events::EventStandardMsg<DagNodeLauncher,
    DagNode::START> >*
    event);

  void
  onDagNodeFinish(
    const events::EventFrom<DagNodeLauncher,
    events::EventStandardMsg<DagNodeLauncher,
    DagNode::FINISH> >*
    event);

  void
  onDagNodeFailed(
    const events::EventFrom<DagNodeLauncher,
    events::EventStandardMsg<DagNodeLauncher,
    DagNode::FAILED> >*
    event);

private:
  DietLogComponent*   myLC;
#endif
};

#endif
