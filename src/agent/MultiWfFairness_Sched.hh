/****************************************************************************/
/* Another multi-workflow scheduler                                         */
/* This scheduler is based on Fairness On Finish Time algorithm             */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.4  2007/04/20 09:43:27  ycaniou
 * Cosmetic changements for less warnings when compiling with doc.
 * Still some errors to correct (parameters to detail) that I cannot do.
 *
 * Revision 1.3  2006/11/27 09:53:00  aamar
 * Correct headers of source files used in workflow support.
 *
 ****************************************************************************/

#ifndef _MULTIWFFAIRNESS_SCHED_HH_
#define _MULTIWFFAIRNESS_SCHED_HH_

#include <map>
#include <vector>

#include "Dag.hh"
#include "AbsWfMetaSched.hh"


using namespace std;

class DagState;
class NodeState;

class MultiWfFairness_Sched : public AbsWfMetaSched {
public:
  MultiWfFairness_Sched();
  
  virtual ~MultiWfFairness_Sched();

  /**
   * set the scheduler used by the MA DAG
   * @param sched the base scheduler to be used
   */
  virtual void 
  setSched(MaDag_sched * sched);

  /**
   * Workflow submission function. 
   * @param wf_desc workflow string description
   * @param dag_id the dag ID
   */
  virtual wf_sched_response_t * 
  submit_wf (const corba_wf_desc_t& wf_desc, int dag_id,
	     MasterAgent_var parent,
	     const bool used);

  /**
   * Remove a client from myClients map
   * @param dag id
   */
  virtual void
  removeClient(const string dag);

  /**
   * Set the node state as done
   * @param dagId the DAG identifier
   * @param nodeId the node identifier in the DAG
   */
  virtual void 
  setNodeAsDone(const char* dagId, const char* nodeId);

protected:
  /**
   * The scheduling of each dag using a scheduling algorithm 'R'
   */
  map<Dag*, wf_node_sched_seq_t> sOwn;

  /**
   * The scheduling using the multiworkflow algorithm
   */
  map<Node*, wf_node_sched_t> sMulti;

  /**
   * Save the state of dags
   * See the class DagState
   */
  map<Dag*, DagState> dagsState;

  /**
   * Save the state of nodes
   * See the class NodeState
   */
  map<Node*, NodeState> nodesState;

  /**
   * Vector of unexecuted dags
   * must be sorted
   */
  vector<Dag *> U;

  /**
   * Init the scheduling
   * 1. schedule each dag
   * 2. mark each dag as unexecuted, initialize the slowdown
   */
  virtual 
  void init();

  /**
   * The main scheduling function
   */
  void
  fairnessOnFinishTime(const wf_response_t * wf_response);

  Node *
  get1stReadyNode(vector<Node*>& v);


  void
  sortU();
};

class DagState {
public:
  DagState();

  /**
   * true if the dag is marked as executed otherwise false
   *
   */
  bool executed;

  /**
   * the makespan of the DAG scheduled alone
   */
  double makespan;

  /**
   * Dag slowdown
   */
  double slowdown;
  
  /**
   * The number of executed (or scheduled) nodes
   */
  unsigned int executedNodes;
};

class NodeState {
public:
  NodeState();

  /**
   * true if the dag is marked as executed otherwise false
   */
  bool executed;

  /**
   * Node finish time based on the multi-workflow algorithm
   */
  double multiFT;

  /**
   * Node finish time when the dag is scheduled alone
   */
  double ownFT;
};

#endif   /* not defined _MULTIWFFAIRNESS_SCHED_HH */



