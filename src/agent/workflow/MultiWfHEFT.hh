/****************************************************************************/
/* Another multi-workflow scheduler                                         */
/* This scheduler is based on HEFT algorithm                                */
/*                                                                          */
/* Author(s):                                                               */
/* - Benjamin ISNARD (benjamin.isnard@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/

/* $Id$
 * $Log$
 * Revision 1.10  2010/03/31 21:15:39  bdepardo
 * Changed C headers into C++ headers
 *
 * Revision 1.9  2009/02/06 14:50:35  bisnard
 * setup exceptions
 *
 * Revision 1.8  2009/01/16 13:41:22  bisnard
 * added common base class DagScheduler to simplify dag events handling
 * improved exception management
 *
 * Revision 1.7  2008/10/14 13:24:49  bisnard
 * use new class structure for dags (DagNode, DagNodePort)
 *
 * Revision 1.6  2008/07/24 21:08:11  rbolze
 * New multi-wf heuristic FCFS (First Come First Serve)
 *
 * Revision 1.5  2008/06/25 10:05:44  bisnard
 * - Waiting priority set when node is put back in waiting queue
 * - Node index in wf_response stored in Node class (new attribute submitIndex)
 * - HEFT scheduler uses SeD ref instead of hostname
 * - Estimation vector and ReqID passed to client when SeD chosen by MaDag
 * - New params in execNodeOnSeD to provide ReqId and estimation vector
 * to client for solve request
 *
 * Revision 1.4  2008/06/19 10:18:54  bisnard
 * new heuristic AgingHEFT for multi-workflow scheduling
 *
 * Revision 1.3  2008/06/18 15:06:51  bisnard
 * set NodeRun class as friend to protect handler methods
 *
 * Revision 1.2  2008/06/01 14:06:57  rbolze
 * replace most ot the cout by adapted function from debug.cc
 * there are some left ...
 *
 * Revision 1.1  2008/04/30 07:37:01  bisnard
 * use relative timestamps for estimated and real completion time
 * make MultiWfScheduler abstract and add HEFT MultiWf scheduler
 *
 */

#ifndef _MULTIWFHEFT_HH_
#define _MULTIWFHEFT_HH_

#include <cmath>
#include <map>
#include "debug.hh"
#include "MultiWfScheduler.hh"



namespace madag {
/****************************************************************************/
/*                              Multi-HEFT                                  */
/****************************************************************************/

class MultiWfHEFT : public MultiWfScheduler {
public:
  explicit MultiWfHEFT(MaDag_impl* maDag);

  virtual ~MultiWfHEFT();

protected:
  /**
   * Updates scheduler when a node has been executed
   */
  virtual void
  handlerNodeDone(DagNode * node);
};  // end class MultiWfHEFT

/****************************************************************************/
/*                              Aging HEFT                                  */
/****************************************************************************/

class MultiWfAgingHEFT : public MultiWfScheduler {
public:
  explicit MultiWfAgingHEFT(MaDag_impl* maDag);
  virtual ~MultiWfAgingHEFT();

protected:
  /**
   * internal dag scheduling
   */
  virtual void
  intraDagSchedule(Dag * dag, MasterAgent_var MA)
    throw(MaDag::ServiceNotFound, MaDag::CommProblem);

  /**
   * Updates scheduler when a node has been executed
   */
  virtual void
  handlerNodeDone(DagNode * node);

  /**
   * set node priority before inserting into execution queue
   * (called by run method)
   * @param node   the node to insert
   */
  virtual void
  setExecPriority(DagNode * node);

  /**
   * set node priority before inserting back in the ready queue
   */
  virtual void
  setWaitingPriority(DagNode * node);

  /**
   * Save the state of dags
   */
  std::map<Dag*, DagState> dagsState;
};  // end class MultiWfAgingHEFT
}  // end namespace madag

#endif  // _MULTIWFHEFT_HH_
