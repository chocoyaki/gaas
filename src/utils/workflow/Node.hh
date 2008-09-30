/****************************************************************************/
/* Node description class                                                   */
/* This class contains the diet profile, the i/o ports and the execution    */
/* object                                                                   */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/* - Benjamin ISNARD (benjamin.isnard@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.15  2008/09/30 09:23:29  bisnard
 * removed diet profile initialization from DagWfParser and replaced by node methods initProfileSubmit and initProfileExec
 *
 * Revision 1.14  2008/09/19 13:11:07  bisnard
 * - added support for containers split/merge in workflows
 * - added support for multiple port references
 * - profile for node execution initialized by port (instead of node)
 * - ports linking managed by ports (instead of dag)
 *
 * Revision 1.13  2008/07/17 13:34:18  bisnard
 * new attribute RealStartTime and get/set for SRPT heuristic
 *
 * Revision 1.12  2008/06/25 10:07:12  bisnard
 * - removed debug messages
 * - Node index in wf_response stored in Node class (new attribute submitIndex)
 *
 * Revision 1.11  2008/06/18 15:00:32  bisnard
 * use new Node attribute estDuration to store job duration for each node
 *
 * Revision 1.10  2008/06/04 07:52:38  bisnard
 * SeD mapping done by MaDag just before node execution
 *
 * Revision 1.9  2008/06/03 12:14:29  bisnard
 * New lastQueue attribute to allow node go back to prev queue
 *
 * Revision 1.8  2008/06/02 08:35:39  bisnard
 * Avoid MaDag crash in case of client-SeD comm failure
 *
 * Revision 1.7  2008/06/01 09:16:57  rbolze
 * remove myreqID attribute from the RunnableNode
 * add getReqID() method which return the reqID stored in the diet_profile_t
 *
 * Revision 1.6  2008/05/16 12:33:32  bisnard
 * cleanup outputs of workflow node
 *
 * Revision 1.5  2008/04/30 07:28:56  bisnard
 * use relative timestamps for estimated and real completion time
 *
 * Revision 1.4  2008/04/28 12:06:03  bisnard
 * changed constructor for Node (new param wfReqId)
 * Node delay at execution: new attr & methods
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

#ifndef _NODE_HH_
#define _NODE_HH_

#include <iostream>
#include <string>
#include <map>

#include <sys/time.h>
#include <time.h>

// general DIET headers
#include "DIET_data.h"
#include "DIET_client.h"
#include "SeD.hh"
#include "common_types.hh"

// workflow related headers
#include "Thread.hh"
#include "WfUtils.hh"
#include "WfPort.hh"

class Node;
class Dag;
class NodeQueue;

using namespace std;

/**
 * The RunnableNode class is used for node execution *
 */
class RunnableNode : public Thread {
public:
  /**
   * The default constructor
   * @param parent The node reference
   */
  RunnableNode(Node * parent);

private:
  /**
   * Node reference
   */
  Node * myParent;

  /**
   * Node execution method *
   * Allocates the profile and executes the call to the SeD *
   */
  void *
  run();

};

class Node  {
  /*******************************/
  /* friend classes              */
  friend class RunnableNode;
  friend class Dag;
  friend class DagWfParser;
  /*******************************/

public:
  /*********************************************************************/
  /* public methods                                                    */
  /*********************************************************************/

  /**
   * The Node default constructor
   * @param wfReqId    the workflow request id
   * @param id         the node id
   * @param pb_name    the node service name
   */
  Node(int wfReqId, string id, string pb_name);

  /**
   * Node destructor
   */
  virtual ~Node();

  /**
   * To get the node id *
   */
  std::string
  getId();

  /**
   * Get the node complete id *
   */
  std::string
  getCompleteId();

  /**
   * Get the workflow request ID
   */
  int
  getWfReqId();

  /**
   * To get the node pb *
   */
  std::string
  getPb();

  /**
   * set the node tag value *
   * see the myTag attribute *
   * @param t the tag value
   */
//   void
//   setTag(unsigned int t);

  /**
   * Set the node ID
   * @param id the new id
   */
  void
  setId(std::string id);

  /**
   * set the parent Dag reference
   */
  void
  setDag(Dag * dag);

  /**
   * get the node Dag reference
   */
  Dag *
  getDag();

  /**
   * display the textual representation of a node  *
   */
  std::string
  toString();

  /******************************/
  /* Links with other nodes     */
  /******************************/

  /**
   * add a new previous node id and reference *
   * @param id previous node id
   * @param n previous node reference
   */
  void
  addPrec(string str, Node * n);

  void addPrecId(string str);

  /**
   * Get the previous node id by index *
   * @param n the requested previous node index
   */
  string
  getPrecId(unsigned int n);

  /**
   * add a new previous node *
   * This function change only the number of previous node *
   */
  void
  addPrevNode();

  /**
   * Add @param n new previous nodes *
   */
  void
  addPrevNode(int n);

  /**
   * return the number of previous nodes
   */
  unsigned int
  prevNb();

  /**
   * Add a next node reference *
   * @param n the next node reference
   */
  void
  addNext(Node * n);

  /**
   * Link input port to output port by id and setting references link *
   * @param in  the input port identifier
   * @param out the output port identifier
   */
  virtual void
  link_i2o(const string in, const string out);

  /**
   * Link output port to input port by id and setting references link *
   * @param out the output port identifier
   * @param in  the input port identifier
   */
  virtual void
  link_o2i(const string out, const string in);

  /**
   * Link inoutput port to input port by id and setting references link *
   * @param io  the inout port identifier
   * @param in  the input port identifier
   */
  virtual void
  link_io2i(const string io, const string in);

  /**
   * Link inoutput port to output port by id and setting references link *
   * @param io  the inout port identifier
   * @param out the output port identifier
   */
  virtual void
  link_io2o(const string io, const string out);

  /**
   * return true if the node is an input node *
   * only the nodes with no previous node are considered as dag input  *
   */
  bool
  isAnInput();

  /**
   * return true if the node is an output node *
   * only the nodes with no next node are considered as dag exit  *
   */
  bool
  isAnExit();

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
   * free the node profile and all the output data
   */
  void
  freeProfileAndData();


  /******************************/
  /* PORTS                      */
  /******************************/

  /**
   * create and add a new port to the node *
   * @param id        the port identifier
   * @param ind       the port index in diet profile
   * @param type      the port type (in, out, inout)
   * @param type      the data type (constant defined in WfCst class)
   * @param depth     the depth of the list structure (0 if no list)
   * @param v         the parameter value
   */
  WfPort *
  newPort(string id, uint ind, wf_port_t type, WfCst::WfDataType type, uint depth,
	       const string& v = string(""));

  /**
   * Get the input port references by id. If not found returns NULL
   * @param id the input port id
   */
  WfInPort*
  getInPort(string id);

  /**
   * Get the output port reference by id. If not found return NULL
   * @param id the requested output port id
   */
  WfOutPort*
  getOutPort(string id);

  /**
   * Get the input/output port reference by id. If not found return NULL
   * @param id the requested inout port id
   */
  WfInOutPort*
  getInOutPort(string id);

  /**
   * set the SeD reference  associated to the Node
   * @param sed the SeD reference
   * @param reqID the request ID (of previous submit request)
   * @param ev  the Estimation vector for this SeD (required to call diet_call_common)
   */
  void
  setSeD(const SeD_var& sed, const unsigned long reqID, corba_estimation_t& ev);

  /**
   * return the number of next nodes
   */
  unsigned int
  nextNodesCount();

  /**
   * return  next node with index n
   * @param n the next node index
   */
  Node *
  getNext(unsigned int n);

  /**
   * return the number of previous nodes
   */
  unsigned int
  prevNodesCount();

  /**
   * return  next node
   */
  Node *
  getPrev(unsigned int n);

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
  /*      Execution status          */
  /**********************************/

  /**
   * start the node execution *
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

protected:
  /*********************************************************************/
  /* protected fields                                                  */
  /*********************************************************************/

  /**
   * Node id *
   */
  string myId;

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
   * the previous nodes ids *
   */
  vector<string> prec_ids;

  /**
   * The previous nods map<id, reference> *
   */
  map <string, Node*>  myPrevNodes;

  /**
   * The following nodes reference vector *
   */
  vector<Node *> next;

  /**
   * the number of previous nodes not finished (negative) *
   */
  int prevNodes;

  /**
   * indicate if the task is done *
   */
  bool task_done;

  /**
   * indicate if execution of the task failed *
   */
  bool taskExecFailed;

private:
  /*********************************************************************/
  /* private fields                                                    */
  /*********************************************************************/

  /**
   * Workflow request ID
   */
  int wfReqId;

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
   * input ports map<id, reference> *
   */
  map<string, WfInPort*> inPorts;

  /**
   * output ports map<id, reference> *
   */
  map<string, WfOutPort*> outPorts;

  /**
   * inoutput ports map<id, reference> *
   */
  map<string, WfInOutPort*> inOutPorts;

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

  /*********************************************************************/
  /* private methods                                                   */
  /*********************************************************************/

  /**
   * called when the node execution is done *
   */
  void
  done();

  /**
   * called when a next node is done *
   */
  virtual void
  nextIsDone();
};


#endif   /* not defined _NODE_HH */
