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
 * Revision 1.1  2008/07/17 13:33:09  bisnard
 * New multi-wf heuristic SRPT
 *
 */

#ifndef _MULTIWFSRPT_HH_
#define _MULTIWFSRPT_HH_

#include "workflow/Dag.hh"
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
        handlerNodeDone(Node * node);

    /**
     * set node priority before inserting into execution queue
     */
    virtual void
        setExecPriority(Node * node);

    /**
     * set node priority before inserting back in the ready queue
     */
    virtual void
        setWaitingPriority(Node * node);
  };

}

#endif   /* not defined _MULTIWFSRPT_HH */

