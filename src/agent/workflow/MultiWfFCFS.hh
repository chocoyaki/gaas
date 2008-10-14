/****************************************************************************/
/* Another multi-workflow scheduler                                         */
/* This scheduler is based on First Come First Serve                        */
/*                                                                          */
/* Author(s):                                                               */
/* - Raphaël  Bolze  (raphael.bolze@ens-lyon.fr)                            */
/* - Benjamin ISNARD (benjamin.isnard@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.2  2008/10/14 13:24:49  bisnard
 * use new class structure for dags (DagNode,DagNodePort)
 *
 * Revision 1.1  2008/07/24 21:08:11  rbolze
 * New multi-wf heuristic FCFS (First Come First Serve)
 *
 *
 */

#ifndef _MULTIWFFCFC_HH_
#define _MULTIWFFCFS_HH_

#include "workflow/Dag.hh"
#include "MultiWfScheduler.hh"

using namespace std;

namespace madag {

  class MultiWfFCFS : public MultiWfScheduler {
  public:
    MultiWfFCFS(MaDag_impl* maDag);
    virtual ~MultiWfFCFS();

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

#endif   /* not defined _MULTIWFFCFS_HH */

