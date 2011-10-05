/****************************************************************************/
/* Another multi-workflow scheduler                                         */
/* This scheduler is based on Shortest Remaining Processing Time            */
/*                                                                          */
/* Author(s):                                                               */
/* - Benjamin ISNARD (benjamin.isnard@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.3  2009/01/16 13:41:22  bisnard
 * added common base class DagScheduler to simplify dag events handling
 * improved exception management
 *
 * Revision 1.2  2008/10/14 13:24:49  bisnard
 * use new class structure for dags (DagNode,DagNodePort)
 *
 * Revision 1.1  2008/07/17 13:33:09  bisnard
 * New multi-wf heuristic SRPT
 *
 */

#ifndef _MULTIWFSRPT_HH_
#define _MULTIWFSRPT_HH_

#include "MultiWfScheduler.hh"

using namespace std;

namespace madag {

class MultiWfSRPT : public MultiWfScheduler {
public:
  MultiWfSRPT(MaDag_impl* maDag);
  virtual ~MultiWfSRPT();

protected:

  /**
   * Updates scheduler when a node has been executed
   */
  virtual void
  handlerNodeDone(DagNode * node);

  /**
   * set node priority before inserting into execution queue
   */
  virtual void
  setExecPriority(DagNode * node);

  /**
   * set node priority before inserting back in the ready queue
   */
  virtual void
  setWaitingPriority(DagNode * node);
};

}

#endif   /* not defined _MULTIWFSRPT_HH */

