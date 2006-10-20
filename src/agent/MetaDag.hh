/****************************************************************************/
/* Meta Dag class                                                           */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id : 
 * $Log : 
 ****************************************************************************/

#ifndef METADAG_HH
#define METADAG_HH

#include "Dag.hh"
#include <map>
#include <vector>
#include <string>

using namespace std;


class MetaDag {
public:
  MetaDag();

  ~MetaDag();

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
  updateDag(const string id);

  /**
   * Update an already present dag in the meta-dag
   * @param id the dag id to update
   * @param doneNodes vector of already done nodes
   */
  void 
  updateDag(const string id, vector<Node*> doneNodes);
  

  /**
   * Remove a dag from the meta-dag
   * @param id the dag id
   */
  void
  removeDag(const string id);  

  /**
   * Regenerate the meta-dag
   */
  void
  rebuild();

  /**
   * Get the XML representation of the meta-dag
   */
  string
  toXML();

  /**
   * return a Dag object representing the meta-dag
   * Don't forget to free the memory after the use of the returned dag
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
  vector<Dag *>
  getAllDags();

  /**
   * return the node with identifier id
   * @param id node complete id
   */
  Node *
  getNode(const string id);

  /**
   * return the dag with identifier id
   * @param id dag identifier
   */
  Dag *
  getDag(const string id);

  /**
   * return the number of the DAGs in the meta-dag
   */
  unsigned int 
  getLength();

private:
  /**
   * All dags that compose the meta-dag
   * The key is the dag identifier
   */
  map<string, Dag*> myDags;

  /**
   * Meta-dag nodes 
   * The key is the node id
   */
  map <string, Node *>   myNodes;

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
   * @param the dag to link
   */
  void
  linkDag(Dag * dag);
};

#endif // METADAG_HH
