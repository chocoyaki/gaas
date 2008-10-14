
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
#include "Node.hh"
#include "NodeQueue.hh"
#include "Thread.hh"

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

/****************************************************************************/
/*                                                                          */
/*                            class DagNode                                 */
/*                                                                          */
/****************************************************************************/

class Dag;

class DagNode : public Node  {
  /*******************************/
  /* friend classes              */
  friend class RunnableNode;
  /*******************************/

public:

  /**
   * The Dag Node default constructor
   * @param dag        the parent dag
   * @param id         the node id
   * @param pb_name    the node service name
   */
  DagNode(Dag *dag, string id, string pb_name);

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
   * Get the node container
   */
  NodeSet*
  getNodeSet();

  /**
   * get the node Dag reference
   */
  Dag *
  getDag();

  /**
   * create a new port
   */
  WfPort *
  newPort(string portId, uint ind,
          WfPort::WfPortType type, WfCst::WfDataType dataType,
          uint depth);

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
   * @return true if success, false on failure
   */
  bool
  initProfileSubmit();

  /**
   * create the diet profile associated to the node (CLIENT) *
   * @return true if success, false on failure
   */
  bool
  initProfileExec();

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
  displayResults();

  /**
   * set the SeD reference associated to the Node
   * @param sed the SeD reference
   * @param reqID the request ID (of previous submit request)
   * @param ev  the Estimation vector for this SeD (required to call diet_call_common)
   */
  void
  setSeD(const SeD_var& sed, const unsigned long reqID, corba_estimation_t& ev);

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
  /* Execution status (MaDag-side)  */
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
  setAsDone(double compTime);

  /**
   * Set the node status as failed
   */
  void
  setAsFailed();

  /**********************************/
  /* Execution status (client-side) */
  /**********************************/

  /**
   * start the node execution *
   * @param join
   */
  void
  start(bool join = false);

  /**
   * called when the node execution is done *
   */
  void
  done();

  /**
   * called when a next node is done *
   */
  void
  nextIsDone();

private:

  void
  setPrev(int index, Node * node);

  /*********************************************************************/
  /* private members                                                   */
  /*********************************************************************/

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


};

#endif // _DAGNODE_HH_
