/****************************************************************************/
/* The MA DAG HEFT scheduler                                                */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.7  2008/06/25 10:05:44  bisnard
 * - Waiting priority set when node is put back in waiting queue
 * - Node index in wf_response stored in Node class (new attribute submitIndex)
 * - HEFT scheduler uses SeD ref instead of hostname
 * - Estimation vector and ReqID passed to client when SeD chosen by MaDag
 * - New params in execNodeOnSeD to provide ReqId and estimation vector
 * to client for solve request
 *
 * Revision 1.6  2008/06/18 15:01:33  bisnard
 * use new attribute estDuration to store job duration for each node
 * rename method to avoid confusion (getCompTimeEst)
 * initialize dag scheduling time in multi-wf scheduler
 *
 * Revision 1.5  2008/05/05 13:54:17  bisnard
 * new computation time estimation get/set functions
 *
 * Revision 1.4  2008/04/30 07:32:24  bisnard
 * use relative timestamps for estimated and real completion time
 *
 * Revision 1.3  2008/04/28 11:54:52  bisnard
 * new methods setNodePriorities & setNodesEFT replacing schedule
 * nodes sort done in separate method in Dag class
 *
 * Revision 1.2  2008/04/21 14:31:45  bisnard
 * moved common multiwf routines from derived classes to MultiWfScheduler
 * use wf request identifer instead of dagid to reference client
 * use nodeQueue to manage multiwf scheduling
 * renamed WfParser as DagWfParser
 *
 * Revision 1.1  2008/04/10 09:17:10  bisnard
 * New version of the MaDag where workflow node execution is triggered by the MaDag agent and done by a new CORBA object CltWfMgr located in the client
 *
 ****************************************************************************/

#ifndef _HEFTSCHEDULER_HH_
#define _HEFTSCHEDULER_HH_

#include "WfScheduler.hh"

namespace madag {
  class HEFTScheduler : public WfScheduler {
  public:
    HEFTScheduler();

    virtual ~HEFTScheduler();

    /**
     * Set the priority of nodes using b-level algorithm
     */
    virtual void
    setNodesPriority(const wf_response_t * wf_response, Dag * dag);

    /**
     * Set the EFT of nodes using HEFT algorithm
     *
     * @param orderedNodes  vector of nodes ordered by decreasing priority
     * @param wf_response   the estimates given by the MA
     * @param dag           the dag
     * @param initTime      dag starting time in ms (relative)
     */
    virtual void
    setNodesEFT(std::vector<Node *>& orderedNodes,
                const wf_response_t * wf_response,
                Dag * dag,
                double initTime);

  private:

    /**
     * Get the value of estimated job duration from wf response
     * for a given service and a given server
     * @param wf_response the wf response structure
     * @param pbIndex the index of the problem (service)
     * @param srvIndex the index of the server
     */
    double
    getNodeDurationEst(const wf_response_t * wf_response,
                   unsigned int pbIndex,
                   unsigned int srvIndex);

    /**
     * Computes the average value of node workload across the Seds
     */
    void
    computeNodeWeights(const wf_response_t * wf_response, Dag * dag);

    /**
     * rank the node upward
     * @param n the top node to rank.
     */
    void
    rank(Node * n);

    /**
     * Stores all the nodes earliest finish times
     */
    map<string, double> AFT;

  };

}

#endif   /* not defined _HEFTSCHEDULER_HH */



