/****************************************************************************/
/* The MA DAG scheduler interface                                           */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.8  2009/01/16 13:41:22  bisnard
 * added common base class DagScheduler to simplify dag events handling
 * improved exception management
 *
 * Revision 1.7  2008/10/14 13:24:49  bisnard
 * use new class structure for dags (DagNode,DagNodePort)
 *
 * Revision 1.6  2008/06/25 10:05:44  bisnard
 * - Waiting priority set when node is put back in waiting queue
 * - Node index in wf_response stored in Node class (new attribute submitIndex)
 * - HEFT scheduler uses SeD ref instead of hostname
 * - Estimation vector and ReqID passed to client when SeD chosen by MaDag
 * - New params in execNodeOnSeD to provide ReqId and estimation vector
 * to client for solve request
 *
 * Revision 1.5  2008/06/18 15:03:39  bisnard
 * initialize dag scheduling time in multi-wf scheduler
 *
 * Revision 1.4  2008/04/30 07:36:21  bisnard
 * use relative timestamps for estimated and real completion time
 *
 * Revision 1.3  2008/04/28 11:54:53  bisnard
 * new methods setNodePriorities & setNodesEFT replacing schedule
 * nodes sort done in separate method in Dag class
 *
 * Revision 1.2  2008/04/21 14:31:45  bisnard
 * moved common multiwf routines from derived classes to MultiWfScheduler
 * use wf request identifer instead of dagid to reference client
 * use nodeQueue to manage multiwf scheduling
 * renamed WfParser as DagWfParser
 *
 * Revision 1.1  2008/04/10 08:16:15  bisnard
 * New version of the MaDag where workflow node execution is triggered by the MaDag agent and done by a new CORBA object CltWfMgr located in the client
 *
 ****************************************************************************/

#ifndef WFSCHEDULER_HH
#define WFSCHEDULER_HH

#include <vector>

#include "response.hh"

class Dag;
class DagNode;

namespace madag {
class WfScheduler {
public:
  /**
   * WfScheduler constructor
   */
  WfScheduler();

  /**
   * WfScheduler destructor
   */
  virtual ~WfScheduler();

  /**
   * Order the nodes using scheduling algorithm
   */
  virtual void
  setNodesPriority(const wf_response_t * wf_response, Dag * dag) = 0;

  /**
   * Computes the schedule using HEFT algorithm and updates node
   */
  virtual void
  setNodesEFT(std::vector<DagNode *>& orderedNodes,
              const wf_response_t * wf_response,
              Dag * dag,
              double initTime) = 0;

};

}

#endif // _WFMETASCHED_HH_
