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

class Dag {
public:
  /*********************************************************************/
  /* public methods                                                    */
  /*********************************************************************/

  Dag();

  ~Dag();

  /**
   * add a node to the dag
   */
  void
  addNode (string nodeId, Node * node);

  /**
   * check the precedence between node *
   * this function check only the precedence between node, it doesn't *
   * link the ports *
   */
  bool
  checkPrec();

  /**
   * return a string representation of the Dag *
   */
  string
  toString();
  
  /*********************************************************************/
  /* the size, begin end end methods are only for testing & debugging  */
  /*********************************************************************/

  /**
   * return the size of the Dag (the nodes number)
   */
  unsigned int 
  size();

  /**
   * return an iterator on the first node *
   * (according to the std::map and not to the dag structure) *
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
   * link all ports of the dag *
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
   */
  void
  setSchedResponse(wf_node_sched_seq_t * response);

  /**
   * get a scalar result of the workflow *
   */
  int 
  get_scalar_output(const char * id,
		    void** value);

  /**
   * get a string result of the workflow *
   */
  int 
  get_string_output(const char * id,
		    char** value);


  /**
   * tag the dag *
   * the input node will receive a tag equal to zero *
   * so of the level 1 will have a tag equal to 1 and so on *
   * the output nodes will have a tag equal to the length of the dag *
   */
  void
  setTags();

private:
  /*********************************************************************/
  /* private fields                                                    */
  /*********************************************************************/

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

};


#endif   /* not defined _DAG_HH. */



