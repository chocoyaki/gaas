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


#include "workflow/Dag.hh"
#include "MultiWfScheduler.hh"
#include "debug.hh"

using namespace std;

namespace madag {

/****************************************************************************/
/*                              Multi-HEFT                                  */
/****************************************************************************/

  class MultiWfHEFT : public MultiWfScheduler {

  public:

    MultiWfHEFT(MaDag_impl* maDag);
    virtual ~MultiWfHEFT();

  protected:
    /**
     * Updates scheduler when a node has been executed
     */
    virtual void
        handlerNodeDone(Node * node);

  }; // end class MultiWfHEFT

/****************************************************************************/
/*                              Aging HEFT                                  */
/****************************************************************************/

  class MultiWfAgingHEFT : public MultiWfScheduler {

      public:

    MultiWfAgingHEFT(MaDag_impl* maDag);
    virtual ~MultiWfAgingHEFT();

  protected:

    /**
     * internal dag scheduling
     */
    virtual void
        intraDagSchedule(Dag * dag, MasterAgent_var MA)
        throw (NodeException);

    /**
     * Updates scheduler when a node has been executed
     */
    virtual void
        handlerNodeDone(Node * node);

    /**
     * set node priority before inserting into execution queue
     * (called by run method)
     * @param node   the node to insert
     */
    virtual void
        setExecPriority(Node * node);

    /**
     * Save the state of dags
     */
    map<Dag*, DagState> dagsState;

    /**
     * Store the nodes HEFT priority
     */
    map<Node*,double> nodesHEFTPrio;


  }; // end class MultiWfAgingHEFT

} // end namespace madag

#endif // _MULTIWFHEFT_HH_
