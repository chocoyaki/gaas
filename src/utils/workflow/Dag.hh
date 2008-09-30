/****************************************************************************/
/* Dag description                                                          */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.13  2008/09/30 15:32:53  bisnard
 * - using simple port id instead of composite ones
 * - dag nodes linking refactoring
 * - prevNodes and nextNodes data structures modified
 * - prevNodes initialization by Node::setNodePredecessors
 *
 * Revision 1.12  2008/09/19 13:11:07  bisnard
 * - added support for containers split/merge in workflows
 * - added support for multiple port references
 * - profile for node execution initialized by port (instead of node)
 * - ports linking managed by ports (instead of dag)
 *
 * Revision 1.11  2008/07/17 12:19:18  bisnard
 * Added dag cancellation method
 *
 * Revision 1.10  2008/07/11 07:56:01  bisnard
 * provide list of failed nodes in case of cancelled dag
 *
 * Revision 1.9  2008/06/25 10:07:12  bisnard
 * - removed debug messages
 * - Node index in wf_response stored in Node class (new attribute submitIndex)
 *
 * Revision 1.8  2008/06/19 10:17:13  bisnard
 * new attribute to store DAG start time
 *
 * Revision 1.7  2008/06/02 08:35:39  bisnard
 * Avoid MaDag crash in case of client-SeD comm failure
 *
 * Revision 1.6  2008/06/01 09:18:43  rbolze
 * remove myreqID attribute from the RunnableNode
 * add getReqID() method which return the reqID stored in the diet_profile_t
 * add 2 methods: showDietReqID and getAllDietReqID
 * this is use to send feedback through logservice
 *
 * Revision 1.5  2008/05/16 12:32:10  bisnard
 * API function to retrieve all workflow results
 *
 * Revision 1.4  2008/04/28 12:15:00  bisnard
 * new NodeQueue implementation for FOFT
 * nodes sorting done by Dag instead of scheduler
 * method to propagate delay at execution (FOFT)
 *
 * Revision 1.3  2008/04/21 14:36:59  bisnard
 * use nodeQueue to manage multiwf scheduling
 * use wf request identifer instead of dagid to reference client
 * renamed WfParser as DagWfParser
 *
 * Revision 1.2  2008/04/14 09:10:40  bisnard
 *  - Workflow rescheduling (CltReoMan) no longer used with MaDag v2
 *  - AbstractWfSched and derived classes no longer used with MaDag v2
 *
 * Revision 1.1  2008/04/10 08:38:50  bisnard
 * New version of the MaDag where workflow node execution is triggered by the MaDag agent and done by a new CORBA object CltWfMgr located in the client
 *
 ****************************************************************************/

#ifndef _DAG_HH_
#define _DAG_HH_

#include <string>
#include <map>
#include <list>

#include "response.hh"

#include "Node.hh"
#include <sys/time.h>
#include <time.h>

class Dag {
public:
  /*********************************************************************/
  /* public methods                                                    */
  /*********************************************************************/

  /**
   * Dag constructor
   */
  Dag();

  /**
   * Dag destructor
   */
  ~Dag();

  /**
   * Set the dag id
   */
  void
  setId(const std::string id);

  /**
   * Get the dag id
   */
  std::string
  getId();

  /**
   * Add a node to the dag
   *
   * @param nodeId  node id (not the complete id)
   * @param node    node ref
   */
  void
  addNode (string nodeId, Node * node);

  /**
   * Get the node with given identifier
   *
   * @param node_id node identifier
   */
  Node *
  getNode(std::string node_id);

  /**
   * check the precedence between node
   * this function check only the precedence between node, it doesn't
   * link the ports
   */
  bool
  checkPrec();

  /**
   * return a string representation of the Dag *
   */
  std::string
  toString();

  /**
   * return the size of the Dag (the nodes number and not the dag length)
   */
  unsigned int
  size();

  /**
   * return an iterator on the first node
   * (according to the std::map and not to the dag structure)
   */
  std::map <std::string, Node *>::iterator
  begin();

  /**
   * return an iterator on the last node *
   * (according to the std::map and not to the dag structure) *
   */
  std::map <std::string, Node *>::iterator
  end();

  /**
   * link all ports of the dag
   */
  void
  linkAllPorts();

  /**
   * link the port of the node @n
   */
  void
  linkNodePorts(Node * n);

  /**
   * check if the dag execution is ongoing
   */
  bool
  isRunning();

  /**
   * check if the dag execution is completed *
   */
  bool
  isDone();

  /**
   * check if the dag execution is cancelled *
   */
  bool
  isCancelled();

  /**
   * get a scalar result of the workflow
   *
   * @param id    the port identifier (complete id 'node id'#'port id')
   * @param value the data to store the returned scalar value
   */
  int
  get_scalar_output(const char * id, void** value);

  /**
   * get a string result of the workflow
   *
   * @param id    the port identifier
   * @param value the data to store the returned string value
   */
  int
  get_string_output(const char * id, char** value);


  /**
   * Get a file result of the workflow
   *
   * @param id    the output port id (prefixed with the node id)
   * @param size  the size of the returned file
   * @param path  the file name
   */
  int
  get_file_output (const char * id,
		   size_t* size, char** path);

  /**
   * Get a matrix result of the workflow
   *
   * @param id      the output port id (prefixed with the node id)
   * @param value   the matrix reference
   * @param nb_rows the matrix rows count
   * @param nb_cols the matrix columns count
   * @param order   the matrix order
   */
  int
  get_matrix_output (const char * id, void** value,
		     size_t* nb_rows, size_t *nb_cols,
		     diet_matrix_order_t* order);

  /**
   * Get all the results and display them. This function doesn't returned
   * the value.
   */
  int
  get_all_results();

  /**
   * Delete all results of the workflow (includes intermediary and final
   * results)
   */
  void
  deleteAllResults();

  /**
   * Move a node to the trash vector (called when rescheduling)
   * @deprecated
   * @param n the node to remove reference
   */
//   void
//   moveToTrash(Node * n);

  /**
   * Get all dag nodes sorted by priority
   *
   * @return ref to a vector of nodes (to be deleted by caller)
   */
  std::vector<Node*>&
  getNodesByPriority();

  /**
   * Get the input nodes
   */
  std::vector<Node *>
  getInputNodes();

  /**
   * @deprecated
   * Get the output nodes
   */
//   std::vector<Node *>
//   getOutputNodes();

  /**
   * Set all input nodes as ready
   * (used to notify node queue)
   */
  void
  setInputNodesReady();

  /**
   * @deprecated
   * Get all profiles in the dag
   */
//   std::vector<diet_profile_t *> *
//   getAllProfiles();

  /**
   * @deprecated
   * set the dag as a temporary object
   * Used to not delete the nodes of the dag when destructing the dag
   */
//   void
//   setAsTemp(bool b = false);

  /**
   * get the estimated makespan of the DAG
   * @deprecated
   */
//   double
//   getEstMakespan();

  /**
   * get the estimated earliest finish time of the DAG
   */
  double
  getEFT();

  /**
   * set the start time of the DAG
   */
  void
  setStartTime(double time);

  /**
   * get the start time of the DAG
   */
  double
  getStartTime();

  /**
   * get the estimated delay of the DAG
   */
  double
  getEstDelay();

  /**
   * set the estimated delay of the DAG
   * (updated by an exit node)
   */
  void
  setEstDelay(double delay);

  /**
   * recursively updates the estimated delay starting from a node
   * and applying it to successors
   */
  bool
  updateDelayRec(Node * node, double newDelay);

  /**
   * notify dag of node execution failure
   */
  void
  setNodeFailure(std::string nodeId);

  /**
   * get the list of failed nodes
   */
  const std::list<string>&
  getNodeFailureList();

  /**
   * set the dag as cancelled
   */
  void
  setAsCancelled();

  void
  showDietReqID();

  vector<diet_reqID_t>
  getAllDietReqID();

private:
  /*********************************************************************/
  /* private fields                                                    */
  /*********************************************************************/

  /**
   * The dag id
   */
  std::string myId;

  /**
   * Workflow nodes *
   */
  std::map <std::string, Node *>   nodes;

  /**
   * start time
   */
  double startTime;

  /**
   * estimated delay
   */
  double estDelay;

  /**
   * Trash nodes vector
   */
  std::vector<Node *> trash;

  /**
   * Failed nodes list
   */
  std::list<string> failedNodes;

  /**
   * Temporary dag flag. Used to not delete the nodes of the dag
   */
  bool tmpDag;

  /**
   * Cancelled flag (used when a node execution fails)
   */
  bool cancelled;

  /**
    * Critical section of the dag
   */
  omni_mutex myLock;

  /**
   * Recursive method for updateDelayRec
   */
  bool
  _updateDelayRec(Node * node, double newDelay);
};


bool operator == (diet_profile_t& a,   diet_profile_t& b);

#endif   /* not defined _DAG_HH. */



