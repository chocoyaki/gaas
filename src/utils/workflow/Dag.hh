/**
* @file Dag.hh
* 
* @brief  Dag description 
* 
* @author  Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)
*          Benjamin ISNARD (Benjamin.Isnard@ens-lyon.fr)
* 
* @section Licence
*   |LICENCE|                                                                
*/


#ifndef _DAG_HH_
#define _DAG_HH_

#include <ctime>
#include <iostream>
#include <list>
#include <map>
#include <string>
#include <vector>

#ifdef WIN32
#include <time.h>
#else
#include <sys/time.h>
#endif
#include "DIET_client.h"  // for diet_ReqID_t


#include "WfNode.hh"
#include "MasterAgent.hh"
#include "NodeSet.hh"
#ifdef WIN32
#define DIET_API_LIB __declspec(dllexport)
#else
#define DIET_API_LIB
#endif
class FWorkflow;
class DagNode;
class DagNodeOutPort;
class DagScheduler;

/*****************************************************************************/
/*                              CLASS Dag                                    */
/*****************************************************************************/

class DIET_API_LIB Dag : public NodeSet {
public:
  /*********************************************************************/
  /* constructors/destructor                                           */
  /*********************************************************************/

  /**
   * Dag constructor WITHOUT DAG execution agent
   * @param id  required non-empty identifier
   */
  explicit Dag(std::string id);

  /**
   * Dag constructor WITH DAG execution agent (DIET MasterAgent)
   * @param id  required non-empty identifier
   * @param MA  master agent (DIET platform MA used for dag execution)
   */
  Dag(std::string id, MasterAgent_var& MA);

  /**
   * Dag destructor
   */
  virtual ~Dag();

  /***************************************************/
  /*            NodeSet methods                      */
  /***************************************************/

  /**
   * Get the node with given identifier
   *
   * @param nodeId node identifier
   * @return pointer to node (does not return NULL)
   */
  virtual WfNode *
  getNode(const std::string& nodeId) throw(WfStructException);

  /**
   * check the precedence between node
   * this function check only the precedence between nodes, it doesn't
   * link the ports
   */
  virtual void
  checkPrec(NodeSet* contextNodeSet) throw(WfStructException);

  /***************************************************/
  /* event message types                             */
  /***************************************************/

  enum eventMsg_e {
    MODID,
    EMPTY,
    STATE
  };

  /***************************************************/
  /*               public methods                    */
  /***************************************************/

  /**
   * Set the dag id
   */
  void
  setId(const std::string& id);

  /**
   * Get the dag id
   */
  const std::string&
  getId() const;

  /**
   * Set the functional wf that created this dag
   */
  void
  setWorkflow(FWorkflow * wf);

  /**
   * Get the functional wf
   */
  FWorkflow *
  getWorkflow() const throw(WfStructException);

  /**
   * Set the execution agent
   */
  void
  setExecutionAgent(MasterAgent_var& MA);

  /**
   * Get the execution agent
   */
  MasterAgent_var&
  getExecutionAgent();

  /**
   * Create a new node of the dag
   * (allocates a new node and insert it in the dag)
   * @param id      node id (not the complete id)
   * @param wf      workflow ref (optional)
   * @return pointer to the node (does not return NULL)
   */
  DagNode*
  createDagNode(const std::string& id, FWorkflow* wf = NULL)
    throw(WfStructException);

  /**
   * return a dag's node
   * @return pointer to the node (does not return NULL)
   */
  DagNode *
  getDagNode(const std::string& nodeId) throw(WfStructException);

  /**
   * remove a node from the dag
   */
  void
  removeNode(const std::string& nodeId) throw(WfStructException);

  /**
   * return the size of the Dag (the nodes number and not the dag length)
   */
  unsigned int
  size();

  /**
   * return an iterator on the first node
   * (according to the map and not to the dag structure)
   */
  std::map <std::string, DagNode *>::iterator
  begin();

  /**
   * return an iterator on the last node *
   * (according to the map and not to the dag structure) *
   */
  std::map <std::string, DagNode *>::iterator
  end();

  /**
   * Returns a std::string description of the dag
   */
  std::string
  toString() const;

  /**
   * returns the XML description of the dag
   */
  void
  toXML(std::ostream& output) const;

  /**
   * link all ports of the dag
   */
  void
  linkAllPorts() throw(WfStructException);

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
  get_file_output(const char * id, size_t* size, char** path);

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
  get_matrix_output(const char * id, void** value,
                    size_t* nb_rows, size_t *nb_cols,
                    diet_matrix_order_t* order);

  /**
   * Get a container result of the workflow
   * @param id      the output port id (prefixed with the node id)
   * @param dataID  a ref to a string that will contain a copy of the container dataID
   */
  int
  get_container_output(const char * id, char** dataID);

  /**
   * Get all the results and display them. This function doesn't returned
   * the value.
   */
  void
  displayAllResults(std::ostream& output);

  /**
   * Delete all results of the workflow (includes intermediary and final
   * results)
   */
  void
  deleteAllResults();

  /**
   * Get all dag nodes sorted by priority
   *
   * @return ref to a std::vector of nodes (to be deleted by caller)
   */
  std::vector<DagNode*>&
  getNodesByPriority();

  /**
   * Get the input nodes
   */
  std::vector<DagNode*>
  getInputNodes();

  /**
   * Set all input nodes as ready
   * (used to notify node queue)
   */
  void
  setInputNodesReady(DagScheduler* scheduler);

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
   * set the finish time of the DAG
   */
  void
  setFinishTime(double time);

  /**
   * get the makespan of the DAG (finish - start)
   */
  double
  getMakespan();

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
  updateDelayRec(DagNode * node, double newDelay);

  /**
   * Notify dag of node end of execution (when successful)
   */
  void
  setNodeDone(DagNode* node, DagScheduler* scheduler = NULL);

  /**
   * notify dag of node execution failure
   */
  void
  setNodeFailure(std::string nodeId, DagScheduler* scheduler = NULL);

  /**
   * get the list of failed nodes
   */
  const std::list<std::string>&
  getNodeFailureList();

  /**
   * set the dag as cancelled
   */
  void
  setAsCancelled(DagScheduler* scheduler = NULL);

  void
  showDietReqID();

  std::vector<diet_reqID_t>
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
   * Workflow for which dag is an instance
   */
  FWorkflow * myWf;

  /**
   * Ref to DAG execution agent
   */
  MasterAgent_var  myExecAgent;

  /**
   * Dag nodes *
   */
  std::map<std::string, DagNode *> nodes;

  /**
   * start time
   */
  double startTime;

  /**
   * finish time
   */
  double finishTime;

  /**
   * estimated delay
   */
  double estDelay;

  /**
   * Failed nodes list
   */
  std::list<std::string> failedNodes;

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
  _updateDelayRec(DagNode * node, double newDelay);

  /**
   * Get port using composite id
   * (used for getting results for client API)
   */
  DagNodeOutPort *
  getOutputPort(const char* id) throw(WfStructException);
};

#endif   /* not defined _DAG_HH. */



