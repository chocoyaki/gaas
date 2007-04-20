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
 * Revision 1.6  2007/04/20 09:43:28  ycaniou
 * Cosmetic changements for less warnings when compiling with doc.
 * Still some errors to correct (parameters to detail) that I cannot do.
 *
 * Revision 1.5  2007/04/17 13:34:54  ycaniou
 * Error in debug.tex header
 * Removes some warnings during doc generation
 *
 * Revision 1.4  2006/11/06 11:56:42  aamar
 * Workflow support:
 *    - Adding get_file_output and get_matrix_output (to get workflow file
 *      and matrix results)
 *
 * Revision 1.3  2006/10/20 08:40:02  aamar
 * Adding the following functions to the class:
 *    - isEnabled(); setEnable; and getRef
 * Implement the new (modified) function getRemainingDag
 *
 * Adding the following function to DAG class
 *   - getNodes; getId; setId; getInputNodes; getOutputNodes();
 *   getAllProfiles(); setAsTemp; getEstMakespan();
 *
 * Revision 1.2  2006/07/10 11:08:12  aamar
 * - Adding the toXML function that return the DAG XML
 * representation
 * - Adding reordering (rescheduling) management
 *
 * Revision 1.1  2006/04/14 13:46:07  aamar
 * Direct acyclic graph class (header).
 *
 ****************************************************************************/

#ifndef _DAG_HH_
#define _DAG_HH_

#include <string>
#include <map>

#include "response.hh"

#include "Node.hh"
#include "CltReoMan_impl.hh"
#include <sys/time.h>
#include <time.h>

class CltReoMan_impl;

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
   * Add a node to the dag
   *
   * @param nodeId the node to add identifier
   * @param node   the node to add reference
   */
  void
  addNode (string nodeId, Node * node);

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
  string
  toString();

  /**
   * return an XML representation of the DAG
   * if b = true, return the complete DAG representation
   * otherwise (b = false value by default) only the remaining DAG
   * (without done nodes)
   */
  string
  toXML(bool b= false);

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
   * execute the workflow *
   */
  void
  exec();
  
  /**
   * Get the ready nodes *
   */
  std::vector<Node *> 
  getReadyNodes();
  
  /**
   * check if the dag execution is completed *
   */
  bool 
  isDone();

  /**
   * set the workflow scheduling response *
   *
   * @param response sequence of results
   */
  void
  setSchedResponse(wf_node_sched_seq_t * response);

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
   * Tag the dag by level
   * The input node will receive a tag equal to zero *
   * so of the level 1 will have a tag equal to 1 and so on *
   * the output nodes will have a tag equal to the length of the dag *
   */
  void
  setTags();

  /**
   * Set the reordering parameters
   *
   * @param nb_sec is the number of seconds
   * @param nb_nodes is the number of nodes
   */
  void 
  set_reordering_delta(const long int nb_sec, 
		       const unsigned long int nb_nodes);

  /**
   * Check the scheduling. 
   * Test if the completion time of node is greater to the predicted
   * one
   */
  bool
  checkScheduling();

  /**
   * Set the client reordering manager
   *
   * @param crm Client reordering manager reference
   */
  void
  setCltReoMan(CltReoMan_impl * crm);

  /**
   * set the beginning time of execution
   *
   * @param tv the begining time
   */
  void
  setTheBeginning(struct timeval tv);

  /**
   * Move a node to the trash vector (called when rescheduling) 
   *
   * @param n the node to remove reference
   */
  void
  moveToTrash(Node * n);

  /**
   * Get the dag nodes as a vector of node reference
   */
  vector<Node*> 
  getNodes();

  /**
   * Get the dag id
   */
  std::string
  getId();

  /**
   * Set the dag id
   */
  void
  setId(const string id);

  /**
   * Get the input nodes
   */
  std::vector<Node *>
  getInputNodes();

  /**
   * Get the output nodes
   */
  std::vector<Node *>
  getOutputNodes();

  /**
   * Get all profiles in the dag
   */
  std::vector<diet_profile_t *>
  getAllProfiles();

  /**
   * set the dag as a temporary object
   * Used to not delete the nodes of the dag when destructing the dag
   */
  void 
  setAsTemp(bool b = false);

  /**
   * get the estimated makespan of the DAG
   */
  double
  getEstMakespan();

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
   * current node (not used) *
   */
  std::map <std::string, Node *>::iterator current_node;

  /**
   * Scheduling response *
   */
  wf_node_sched_seq_t * response;

  /**
   * Reordering parameter
   */
  long int
  nbSec;

  /**
   * Reordering parameter
   */
  unsigned long int 
  nbNodes;

  /**
   * Client Reordering Manager
   */
  CltReoMan_impl * myCltReoMan;

  /**
   * The time of execution beginning
   */
  struct timeval beginning;

  /**
   * Trash nodes vector
   */
  vector<Node *> trash;

  /**
   * Temporary dag flag. Used to not delete the nodes of the dag
   */
  bool tmpDag;
};


bool operator == (diet_profile_t& a,   diet_profile_t& b);

#endif   /* not defined _DAG_HH. */



