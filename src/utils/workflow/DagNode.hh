/****************************************************************************/
/* The node class used for dag execution                                    */
/* Manages the execution profile, the data, the execution status and time   */
/*                                                                          */
/* Author(s):                                                               */
/* - Benjamin ISNARD (benjamin.isnard@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.12  2009/07/09 07:18:38  bisnard
 * fixed bug with getWorkflow()
 *
 * Revision 1.11  2009/07/07 09:01:32  bisnard
 * new attribute myWf to replace dag-related workflow
 *
 * Revision 1.10  2009/05/15 11:03:41  bisnard
 * added new exception types
 *
 * Revision 1.9  2009/04/17 08:54:43  bisnard
 * renamed Node class as WfNode
 *
 * Revision 1.8  2009/02/20 10:23:54  bisnard
 * use estimation class to reduce the nb of submit requests
 *
 * Revision 1.7  2009/02/06 14:55:08  bisnard
 * setup exceptions
 *
 * Revision 1.6  2009/01/16 13:55:36  bisnard
 * changes in dag event handling methods
 *
 * Revision 1.5  2008/12/09 12:14:05  bisnard
 * added reference to FNode to handle pending instanciation
 *
 * Revision 1.4  2008/12/02 10:14:51  bisnard
 * modified nodes links mgmt to handle inter-dags links
 *
 * Revision 1.3  2008/10/22 09:29:00  bisnard
 * replaced uint by standard type
 *
 * Revision 1.2  2008/10/20 07:59:29  bisnard
 * file header creation
 *
 */

#ifndef _DAGNODE_HH_
#define _DAGNODE_HH_

#include <iostream>
#include <string>
#include <vector>

#include <sys/time.h>
#include <time.h>

// general DIET headers
#include "DIET_data.h"
#include "DIET_client.h"
#include "SeD.hh"
#include "common_types.hh"

// workflow related headers
#include "WfNode.hh"
#include "NodeQueue.hh"
#include "Thread.hh"
#include "DagScheduler.hh"

using namespace std;

/****************************************************************************/
/*                                                                          */
/*                         class RunnableNode                               */
/*                                                                          */
/****************************************************************************/

class DagNode;

/**
 * The RunnableNode class is used for node execution *
 */
class RunnableNode : public Thread {
public:
  /**
   * The default constructor
   * @param parent The node reference
   */
  RunnableNode(DagNode * parent);

private:
  /**
   * Node reference
   */
  DagNode * myParent;

  /**
   * Node execution method *
   * Allocates the profile and executes the call to the SeD *
   */
  void *
  run();

};

/*****************************************************************************/
/*                         CLASS WfDataException                             */
/*****************************************************************************/

class WfDataException {
  public:
    enum WfDataErrorType { eNOTFOUND,
                           eWRONGTYPE,
                           eINVALID_CONTAINER,
                           eID_UNDEF,
                           eVOID_DATA,
                           eINVALID_VALUE };
    WfDataException(WfDataErrorType t, const string& info)
      { this->why = t; this->info = info; }
    WfDataErrorType Type() { return this->why; }
    const string& Info() { return this->info; }
    string ErrorMsg();
  private:
    WfDataErrorType why;
    string info;
};

/****************************************************************************/
/*                                                                          */
/*                            class DagNode                                 */
/*                                                                          */
/****************************************************************************/

class Dag;
class FWorkflow;
class FProcNode;

class DagNode : public WfNode  {
  /*******************************/
  /* friend classes              */
  friend class RunnableNode;
  /*******************************/

public:

  /**
   * The Dag Node default constructor
   * @param id         the node id
   * @param dag        the parent dag
   * @param wf         the workflow that generated this dag node (optional)
   */
  DagNode(const string& id, Dag *dag, FWorkflow* wf = NULL);

  /**
   * DagNode destructor
   */
  virtual ~DagNode();

  /**
   * Get the node complete id (dagName-nodeName) *
   */
  string
  getCompleteId();

  /**
   * get the node Dag reference
   */
  Dag *
  getDag();

  /**
   * get the node Wf reference
   * (may return NULL if node does not belong to a functional workflow)
   */
  FWorkflow *
  getWorkflow();

  /**
   * set the node problem name (ie service name)
   */
  void
  setPbName(const string& pbName);

  /**
   * get the problem name
   */
  const string&
  getPbName();

  /**
   * set the functional node for which this node is an instance
   */
//   void
//   setFNode(FProcNode * fNode);

  /**
   * get the functional node
   */
//   FProcNode *
//   getFNode();

  /**
   * create a new port
   */
  WfPort *
  newPort(string portId, unsigned int ind,
          WfPort::WfPortType portType, WfCst::WfDataType dataType,
          unsigned int depth) throw (WfStructException);

  /**
   * Add a new predecessor
   * (may check some constraints before adding the predecessor effectively)
   * @param node  ptr to the predecessor
   * @param fullNodeId  contains the id of the predecessor eventually prefixed (if external)
   */
  virtual void
  addNodePredecessor(WfNode * node, const string& fullNodeId);

  /**
   * returns an XML  representation of a node
   */
  void
  toXML(ostream& output) const;

  /******************************/
  /* DIET Profile Mgmt          */
  /******************************/

  /**
   * Return the node profile
   */
  diet_profile_t *
  getProfile();

  /**
   * return the reqID of the node
   */
  diet_reqID_t
  getReqID();

  /**
   * Set the estimation class
   * (if dagNode belongs to an estimation class then
   * the scheduler will use the same estimation value as for any node
   * of the same class)
   */
  void
  setEstimationClass(const string& estimClassId);

  /**
   * Get the estimation class
   */
  const string&
  getEstimationClass();

  /**
   * set the submit index
   */
  void
  setSubmitIndex(int idx);

  /**
   * get the submit index
   */
  int
  getSubmitIndex();

  /**
   * creates a basic profile without parameters (common part)
   */
  void
  createProfile();

  /**
   * create the diet profile associated to the node (MADAG) *
   */
  void
  initProfileSubmit();

  /**
   * create the diet profile associated to the node (CLIENT) *
   */
  void
  initProfileExec() throw (WfDataException);

  /**
   * Store the persistent data IDs from profile to node
   */
  void
  storeProfileData();

  /**
   * free the node profile and all the output data
   */
  void
  freeProfileAndData();

  /**
   * Allocate a new char *
   * @param value the parameter value as a string
   */
  char *
  newChar  (const string value = "");
  /**
   * Allocate a new short *
   * @param value the parameter value as a string
   */
  short *
  newShort (const string value = "");

  /**
   * Allocate a new int  *
   * @param value the parameter value as a string
   */
  int *
  newInt   (const string value = "");

  /**
   * Allocate a new long *
   * @param value the parameter value as a string
   */
  long *
  newLong  (const string value = "");

  /**
   * Allocate a new string *
   * @param value the parameter value as a string
   */
  char *
  newString (const string value = "");

  /**
   * Allocate a new float  *
   * @param value the parameter value as a string
   */
  float *
  newFloat  (const string value = "");

  /**
   * Allocate a new double  *
   * @param value the parameter value as a string
   */
  double *
  newDouble (const string value = "");

  /**
   * Allocate a new filename  *
   * @param value the parameter value as a string
   */
  char *
  newFile   (const string value = "");

  /**
   * Display all results of the node
   */
  void
  displayResults(ostream& output);

  /**
   * set the SeD reference associated to the Node
   * @param sed the SeD reference
   * @param reqID the request ID (of previous submit request)
   * @param ev  the Estimation vector for this SeD (required to call diet_call_common)
   */
  void
  setSeD(const SeD_var& sed, const unsigned long reqID, corba_estimation_t& ev);

  /**
   * Get the chosen SeD
   */
  SeD_var&
  getSeD();

  /******************************/
  /* Scheduling                 */
  /******************************/

  /**
   * set the node priority *
   * @param priority the new priority of the node
   */
  void
  setPriority(double priority);

  /**
   * get the node priority *
   */
  double
  getPriority();

  /**
   * Set the NodeQueue when the node is inserted into it
   */
  void
  setNodeQueue(NodeQueue * nodeQ);

  /**
   * Remove node from the NodeQueue it belongs to (if applicable)
   * This is used in case of dag cancellation
   */
  void
  removeFromNodeQueue();

  /**
   * set the ref to the last nodeQueue occupied by the node
   */
  void
  setLastQueue(NodeQueue * queue);

  /**
   * get the ref to the last nodeQueue occupied by the node
   */
  NodeQueue *
  getLastQueue();

  /******************************/
  /* Timestamps (MaDag)         */
  /******************************/

  /**
   * set the estimated duration
   */
  void
  setEstDuration(double time);

  /**
   * get the estimated duration
   */
  double
  getEstDuration();

  /**
   * set the estimated completion time
   */
  void
  setEstCompTime(double time);

  /**
   * get the estimated completion time
   */
  double
  getEstCompTime();

  /**
   * set the real start time
   */
  void
  setRealStartTime(double time);

  /**
   * get the real start time
   */
  double
  getRealStartTime();

  /**
   * set the estimated delay
   */
  void
  setEstDelay(double delay);

  /**
   * get the estimated delay
   */
  double
  getEstDelay();

  /**
   * set the real completion time
   */
  void
  setRealCompTime(double time);

  /**
   * get the real completion time
   */
  double
  getRealCompTime();

  /**
   * get the real delay (positive) after execution
   * or 0 if no delay
   * or -1 if needed timestamps not set
   */
  double
  getRealDelay();

  /**********************************/
  /* Execution status               */
  /**********************************/

  /**
   * Return true if the node is ready for execution, false otherwise
   */
  bool
  isReady();

  /**
   * set the node as ready for execution
   */
  void
  setAsReady();

  /**
   * start the node execution (client side) *
   * @param join
   */
  void
  start(bool join = false);

  /**
   * Called when a previous node execution is done *
   */
  void
  prevNodeHasDone();

  /**
   * Another method to get if the node is ready for execution *
   */
  bool
  allPrevDone();

  /**
   * Test if the node is running *
   */
  bool
  isRunning();

  /**
   * Set node statuc as running *
   */
  void
  setAsRunning();

  /**
   * Test if the execution is done *
   */
  bool
  isDone();

  /**
   * Test if the execution failed *
   */
  bool
  hasFailed();

  /**
   * Set the node status as done
   * @param compTime  timestamp of node completion
   */
  void
  setAsDone(DagScheduler* scheduler = NULL);

  /**
   * Set the node status as failed
   * @param scheduler  workflow scheduler (for MADAG only)
   */
  void
  setAsFailed(DagScheduler* scheduler = NULL);


protected:

  void
  setPrev(int index, WfNode * node);

private:

  /*********************************************************************/
  /* private members                                                   */
  /*********************************************************************/

  /**
   * Node problem
   */
  string myPb;

  /**
   * NodeQueue ref (used to notify NodeQueue when state changes)
   */
  NodeQueue * myQueue;

  /**
   * NodeQueue ref (used to go back in the previous queue)
   */
  NodeQueue * lastQueue;

  /**
   * the number of previous nodes not finished *
   */
  int prevNodesTodoCount;

  /**
   * indicate if the task is done *
   */
  bool task_done;

  /**
   * indicate if execution of the task failed *
   */
  bool taskExecFailed;

  /**
   * Dag reference
   */
  Dag * myDag;

  /**
   * Workflow reference
   */
  FWorkflow * myWf;

  /**
   * Functional node
   */
//   FProcNode * myFNode;

  /**
   * problem profile *
   */
  diet_profile_t * profile;

  /**
   * node thread *
   */
  RunnableNode   * myRunnableNode;

  /*************************/
  /* problem parameters    */
  /*************************/
  vector<char*>   charParams;
  vector<short*>  shortParams;
  vector<int*>    intParams;
  vector<long*>   longParams;
  vector<char *>  stringParams;
  vector<float*>  floatParams;
  vector<double*> doubleParams;
  vector<char*>   fileParams;
  /*************************/

  /**
   * the node priority *
   */
  double priority;

    /**
   * node running status *
   */
  bool node_running;

  /**
   * Sed status (true if sed is defined)
   */
  bool SeDDefined;

  /**
   * chosen server *  (client-side)
   */
  SeD_var chosenServer;

  /**
   * estimation vector for chosen server * (client-side)
   */
  estVector_t estimVect;

  /**
   * request ID for chosen server (client-side)
   */
  unsigned long dietReqID;

  /**
   * number of immediate next nodes that have end their execution *
   */
  unsigned int nextDone;

  /**
   * Estimated duration (in ms)
   */
  double estDuration;

  /**
   * Estimated completion time (in ms)
   */
  double estCompTime;

  /**
   * Estimated delay (in ms)
   */
  double estDelay;

  /**
   * Real start time (in ms)
   */
  double realStartTime;

  /**
   * Real completion time (in ms)
   */
  double realCompTime;

  /**
   * Submit index (stores the index in MA response)
   */
  int submitIndex;

  /**
   * Estimation class (if dagNode belongs to an estimation class then
   * the scheduler will use the same estimation value as for any node
   * of the same class)
   */
  string estimationClass;

};

#endif // _DAGNODE_HH_
