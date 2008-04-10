/****************************************************************************/
/* Meta Dag class                                                           */
/* This class is used in multi-workflow support to manipulate more than     */
/* than one DAG                                                             */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.1  2008/04/10 09:13:29  bisnard
 * New version of the MaDag where workflow node execution is triggered by the MaDag agent and done by a new CORBA object CltWfMgr located in the client
 *
 ****************************************************************************/

#ifndef MULTIDAG_HH
#define MULTIDAG_HH

#include "workflow/Dag.hh"
#include <map>
#include <vector>
#include <string>


namespace madag {

  class MultiDag {
  public:
    MultiDag();

    ~MultiDag();

    /**
     * Add a dag to the meta-dag
     * @param dag  The dag to add
     */
    void
    addDag(Dag * dag);

    /**
     * Update an already present dag in the meta-dag
     * @param id the dag id to update
     */
    void 
    updateDag(const std::string id);

    /**
     * Update an already present dag in the meta-dag
     * @param id the dag id to update
     * @param doneNodes vector of already done nodes
     */
    void 
    updateDag(const std::string id, std::vector<Node*> doneNodes);
  

    /**
     * Remove a dag from the meta-dag
     * @param id the dag id
     */
    void
    removeDag(const std::string id);  

    /**
     * Regenerate the meta-dag
     */
    void
    rebuild();

    /**
     * Get the XML representation of the meta-dag
     */
    std::string
    toXML();

    /**
     * return a Dag object representing the meta-dag
     * Don't forget to free the memory after the use of the returned dag
     * TODO rename as getMultiDag to avoid confusion (bi)
     */
    Dag *
    getDag();
  
    /**
     * set the node state as done
     */
    void
    setNodeAsDone(const char * dagId,const char * nodeId, 
                  bool state = true);

    /**
     * get the dags that compose the meta-dag
     */
    std::vector<Dag *>
    getAllDags();

    /**
     * return the node with identifier id
     * @param id node complete id
     */
    Node *
    getNode(const std::string id);

    /**
     * return the dag with identifier id
     * @param id dag identifier
     */
    Dag *
    getDag(const std::string id);

    /**
     * return the number of the DAGs in the meta-dag
     */
    unsigned int 
    getLength();

    /**
     * Return a vector of ready nodes
     */
    std::vector<Node *>
    getReadyNodes();

    /**
     * Map scheduling 
     */
    void
    mapSeDs(wf_node_sched_seq_t& sched_seq);

  private:
    /**
     * All dags that compose the meta-dag
     * The key is the dag identifier
     */
    map<std::string, Dag*> myDags;

    /**
     * Meta-dag nodes 
     * The key is the node id
     */
    map <std::string, Node *>   myNodes;

    /**
     * The nodes state (true if node is done, false otherwise)
     */
    map <Node *, bool> nodesState;

    /**
     * The (fictive) input node 
     */
    Node * input;

    /**
     * The (fictive) output node
     */
    Node * output;

    /**
     * Lock to prevent concurrent access
     */
    omni_mutex myMutex ;

    /**
     * Links all the dags of the meta-dag
     *   - links the meta-dag input with dags inputs
     *   - links the dags outputs to the meta-dag output
     */
    void
    linkDags();

    /**
     * Create the links between meta-dag input/output and the dag input/output
     * @param dag the dag to link
     */
    void
    linkDag(Dag * dag);
  };

}
#endif // MULTIDAG_HH
