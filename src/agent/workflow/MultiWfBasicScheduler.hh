/****************************************************************************/
/* The Workflow Meta-scheduler                                              */
/* This scheduler is used when multi-workflow support is enabled            */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/* - Benjamin ISNARD (benjamin.isnard@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.6  2008/10/14 13:24:49  bisnard
 * use new class structure for dags (DagNode, DagNodePort)
 *
 * Revision 1.5  2008/04/30 07:37:01  bisnard
 * use relative timestamps for estimated and real completion time
 * make MultiWfScheduler abstract and add HEFT MultiWf scheduler
 *
 * Revision 1.4  2008/04/21 14:31:45  bisnard
 * moved common multiwf routines from derived classes to MultiWfScheduler
 * use wf request identifer instead of dagid to reference client
 * use nodeQueue to manage multiwf scheduling
 * renamed WfParser as DagWfParser
 *
 * Revision 1.3  2008/04/14 13:44:29  bisnard
 * - Parameter 'used' obsoleted in MultiWfScheduler::submit_wf & submit_pb_set
 *
 * Revision 1.2  2008/04/14 09:10:37  bisnard
 *  - Workflow rescheduling (CltReoMan) no longer used with MaDag v2
 *  - AbstractWfSched and derived classes no longer used with MaDag v2
 *
 * Revision 1.1  2008/04/10 09:13:29  bisnard
 * New version of the MaDag where workflow node execution is triggered by the MaDag agent and done by a new CORBA object CltWfMgr located in the client
 *
 ****************************************************************************/

#ifndef _MULTIWFBASICSCHEDULER_HH_
#define _MULTIWFBASICSCHEDULER_HH_

#include "MultiWfScheduler.hh"

namespace madag {

class MultiWfBasicScheduler : public MultiWfScheduler {
public:
  explicit MultiWfBasicScheduler(MaDag_impl* maDag);

  virtual ~MultiWfBasicScheduler();

  /**
   * Execution method
   */
  virtual void *
  run();

  /**
   * Updates scheduler when a node has been executed
   * (does nothing)
   */
  virtual void
  handlerNodeDone(DagNode * node);

protected:
  /**
   * create simple ordered NodeQueue for ready nodes
   * (does not use priority of nodes)
   */
  virtual OrderedNodeQueue *
  createNodeQueue(Dag * dag);

  /**
   * delete the node queue created in createNodeQueue
   */
  virtual void
  deleteNodeQueue(OrderedNodeQueue * nodeQ);
};
}

#endif   /* not defined _MULTIWFBASICSCHEDULER_HH */



