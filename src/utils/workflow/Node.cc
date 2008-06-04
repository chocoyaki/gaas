/****************************************************************************/
/* Node description class                                                   */
/* This class contains the diet profile, the i/o ports and the execution    */
/* object                                                                   */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.13  2008/06/04 07:52:37  bisnard
 * SeD mapping done by MaDag just before node execution
 *
 * Revision 1.12  2008/06/03 12:14:29  bisnard
 * New lastQueue attribute to allow node go back to prev queue
 *
 * Revision 1.11  2008/06/02 08:35:39  bisnard
 * Avoid MaDag crash in case of client-SeD comm failure
 *
 * Revision 1.10  2008/06/01 14:06:56  rbolze
 * replace most ot the cout by adapted function from debug.cc
 * there are some left ...
 *
 * Revision 1.9  2008/06/01 09:16:56  rbolze
 * remove myreqID attribute from the RunnableNode
 * add getReqID() method which return the reqID stored in the diet_profile_t
 *
 * Revision 1.8  2008/05/28 20:53:33  rbolze
 * now DIET_PARAMSTRING type can be use in DAG.
 *
 * Revision 1.7  2008/05/16 12:33:32  bisnard
 * cleanup outputs of workflow node
 *
 * Revision 1.6  2008/04/30 07:28:56  bisnard
 * use relative timestamps for estimated and real completion time
 *
 * Revision 1.5  2008/04/28 12:06:03  bisnard
 * changed constructor for Node (new param wfReqId)
 * Node delay at execution: new attr & methods
 *
 * Revision 1.4  2008/04/21 14:36:59  bisnard
 * use nodeQueue to manage multiwf scheduling
 * use wf request identifer instead of dagid to reference client
 * renamed WfParser as DagWfParser
 *
 * Revision 1.3  2008/04/15 14:19:54  bisnard
 * - Postpone sed mapping until wf node is executed
 * - Removed diet_call_common2 function
 *
 * Revision 1.2  2008/04/14 09:10:40  bisnard
 *  - Workflow rescheduling (CltReoMan) no longer used with MaDag v2
 *  - AbstractWfSched and derived classes no longer used with MaDag v2
 *
 * Revision 1.1  2008/04/10 08:38:50  bisnard
 * New version of the MaDag where workflow node execution is triggered by the MaDag agent and done by a new CORBA object CltWfMgr located in the client
 *
 * Revision 1.13  2007/06/29 14:28:19  rbolze
 * update code following the change of the function diet_call_common
 * now the dietReqID is in the diet_profile
 *
 * Revision 1.12  2007/05/30 11:16:37  aamar
 * Updating workflow runtime to support concurrent call (Reordering is not
 * working now - TO FIX -).
 *
 * Revision 1.11  2006/11/27 10:15:12  aamar
 * Correct headers of files used in workflow support.
 *
 * Revision 1.10  2006/11/13 12:31:58  aamar
 * Remove reference to FD (not used).
 *
 * Revision 1.9  2006/11/06 15:16:06  aamar
 * Workflow support: some correction about reqID.
 *
 * Revision 1.8  2006/11/06 12:23:59  aamar
 * Workflow support: correct the value of reqID passed to RunnableNode.
 *
 * Revision 1.7  2006/11/06 11:57:07  aamar
 * *** empty log message ***
 *
 * Revision 1.6  2006/11/02 17:13:13  rbolze
 * dd some debug info
 *
 * Revision 1.5  2006/10/28 23:17:08  aamar
 * Free the persistent data when they are not needed anymore.
 * Replace cout/cerr debug message by debug.hh macros.
 *
 * Revision 1.4  2006/10/23 23:50:19  aamar
 * Coorect a problem with the use of INTERNAL_ERROR macro
 *
 * Revision 1.3  2006/10/20 08:42:14  aamar
 * Merging the base class BasicNode in Node class.
 * Adding some functions.
 * Correcting some bugs.
 *
 * Revision 1.2  2006/07/10 11:07:37  aamar
 * - Adding Matrix data type support
 * - The toXML (for Node and Ports) method that return XML
 * representation
 * - Some function and attributes used for scheduling and
 * rescheduling
 *
 * Revision 1.1  2006/04/14 13:50:40  aamar
 * Class representing a Dag node (source). This is a BasicNode
 * subclass that includes necessary code for execution).
 *
 ****************************************************************************/

#include <stdlib.h>
#include <sstream>
#include <algorithm>

#include "statistics.hh"
#include "debug.hh"
#include "marshalling.hh"
#include "MasterAgent.hh"

#include "Node.hh"
#include "Dag.hh"
#include "NodeQueue.hh"

MasterAgent_var getMA();

/**
 * Display a textual representation of a corba profile
 *
 * @param pb the corba profile to display
 */
void display(const corba_profile_t& pb) {
  TRACE_TEXT (TRACE_ALL_STEPS,
		  "  - last_in    : " << pb.last_in << endl);
  TRACE_TEXT (TRACE_ALL_STEPS,
	      "  - last inout : " << pb.last_inout << endl);
  TRACE_TEXT (TRACE_ALL_STEPS,
	      "  - last out   : " << pb.last_out << endl);
  TRACE_TEXT (TRACE_ALL_STEPS,
	      "  - param : " << pb.last_out << endl);
  for (unsigned int ix=0; ix < pb.parameters.length(); ix++) {
    TRACE_TEXT (TRACE_ALL_STEPS,
		"     - base_type = " << pb.parameters[ix].desc.base_type <<
		", specific = " << pb.parameters[ix].desc.specific._d() <<
		endl);
  }
  TRACE_TEXT (TRACE_ALL_STEPS,
	      "*************************" << endl);
}

/**
 * RunnableNode constructor
 */
/*RunnableNode::RunnableNode(Node * parent,
			   diet_reqID_t reqID)
  :Thread(false) {
  this->myParent = parent;
  this->myReqID = reqID;
} // end RunnableNode constructor*/
RunnableNode::RunnableNode(Node * parent)
	:Thread(false) {
  this->myParent = parent;
} // end RunnableNode constructor

/**
 * Node execution methods *
 */
void *
RunnableNode::run() {
  typedef size_t comm_failure_t;
  TRACE_TEXT (TRACE_ALL_STEPS,
	      "RunnableNode tries to a execute a service "<< endl);
  // create the node diet profile
  TRACE_TEXT (TRACE_ALL_STEPS, "create the node diet profile" << endl);
  myParent->createProfile();
  TRACE_TEXT (TRACE_ALL_STEPS, "profile creation ... done" << endl);
  TRACE_TEXT (TRACE_ALL_STEPS, "Init the ports " << endl);
  for (map<string, WfInPort*>::iterator p = myParent->inports.begin();
       p != myParent->inports.end();
       ++p) {
    WfInPort * in = (WfInPort*)(p->second);
    if (in->source_port) {
      WfOutPort * out = (WfOutPort*)(in->source_port);
        TRACE_TEXT (TRACE_ALL_STEPS,
		    "using the persistent data for " << in->index <<
		    " (out = " << out->index << ")" << endl);

      diet_use_data(diet_parameter(myParent->profile, in->index),
		    out->profile()->parameters[out->index].desc.id);

    }
  }
  TRACE_TEXT (TRACE_ALL_STEPS, "Init ports data ... done " << endl);

//  nodeIsRunning(myParent->getId().c_str());
  bool failed = false;
  try {
    try {
      if (!diet_call(myParent->profile)) {
        TRACE_TEXT (TRACE_MAIN_STEPS, "diet_call DONE" << endl);
        myParent->storePersistentData();
        //cout << " dietReqID : " << myParent->profile->dietReqID << endl;
        //this->myReqID=myParent->profile->dietReqID;
      }
      else {
        TRACE_TEXT (TRACE_MAIN_STEPS, "diet_call FAILED" << endl);
        failed = true;
      }
    } catch (CORBA::COMM_FAILURE& e) {
      throw (comm_failure_t)1;
    } catch (CORBA::TRANSIENT& e) {
      throw (comm_failure_t)1;
    }
  } catch (comm_failure_t& e) {
    if (e == 0 || e == 1) {
      WARNING("Connection problems with SeD occured - node exec cancelled");
      failed = true;
    }
  }

  TRACE_TEXT (TRACE_ALL_STEPS, "RunnableNode call ... done" << endl);
  if (!failed)  myParent->done();
  else          myParent->setAsFailed();

  return NULL;
} // end RunnableNode::run


/**
 * The Node constructor
 */
Node::Node(int wfReqId, string id, string pb_name,
	   int last_in, int last_inout, int last_out) {
  this->wfReqId = wfReqId;
  this->myId = id;
  this->myPb = pb_name;
  this->prevNodes = 0;
  this->task_done = false;
  this->profile = diet_profile_alloc((char*)pb_name.c_str(),
				     last_in, last_inout, last_out);
  this->node_running = false;
  this->taskExecFailed = false;
  this->myRunnableNode = NULL;
  this->chosenServer = SeD::_nil();
  this->nextDone = 0;
  this->priority = 0;
  this->myDag = NULL;
  this->myQueue = NULL;
  this->realCompTime = -1;
  this->estCompTime = -1;
} // end Node constructor

/**
 * Node destructor
 */
Node::~Node() {
  // if everything is alright, we need this line only for the exit node(s)
  TRACE_TEXT (TRACE_ALL_STEPS, "~Node() destructor..." << endl);
  if (profile != NULL)
    diet_profile_free(profile);
  if (myRunnableNode)
    delete (myRunnableNode);

  // free the ports map ()
  // free the parameters vectors
  {
    // char vector
    char * cx;
    while (charParams.size() != 0) {
      cx = charParams.back();
      charParams.pop_back();
      if (cx)
	delete (cx);
    }
  }
  {
    // short vector
    short * sx = NULL;
    while (shortParams.size() != 0) {
      sx = shortParams.back();
      shortParams.pop_back();
      if (sx) {
	delete(sx);
      }
    }
  }
  {
    // int vector
    int * ix = NULL;
    while (intParams.size() != 0) {
      ix = intParams.back();
      intParams.pop_back();
      if (ix)
	delete (ix);
    }
  }
  {
    // long vector
    long * lx = NULL;
    while (longParams.size() != 0) {
      lx = longParams.back();
      longParams.pop_back();
      if (lx)
	delete (lx);
    }
  }
  {
    // string
    char * cx = NULL;
    while (stringParams.size() != 0) {
      cx = stringParams.back();
      stringParams.pop_back();
      if (cx)
	delete(cx);
    }
  }

  {
    // file name vector
    char * cx = NULL;
    while (fileParams.size() != 0) {
      cx = fileParams.back();
      fileParams.pop_back();
      if (cx)
	delete(cx);
    }
  }

  {
    // float vector
    float * fx = NULL;
    while (floatParams.size() != 0) {
      fx = floatParams.back();
      floatParams.pop_back();
      if (fx)
	delete(fx);
    }
  }
  {
    // double vector
    double * dx = NULL;
    while (doubleParams.size() != 0) {
      dx = doubleParams.back();
      doubleParams.pop_back();
      if (dx)
	delete(dx);
    }
  }
}

/****************************************************************************/
/*                              Basic GET/SET                               */
/****************************************************************************/

/**
 * Set the node ID
 */
void
Node::setId(string id) {
  this->myId = id;
} // end setId

/**
 * Get the node id *
 */
string Node::getId() {
  return this->myId;
} // end getId

/**
 * Get the node complete id *
 */
string Node::getCompleteId() {
  if (this->myDag != NULL)
    return this->myDag->getId() + "-" + this->myId;
  return this->myId;
} // end getCompleteId

/**
 * Get the workflow request id associated with the node *
 */
int Node::getWfReqId() {
  return this->wfReqId;
}

/**
 * Set the NodeQueue ref when the node is inserted into it *
 * (This is used to notify the NodeQueue when the node state changes *
 */
void Node::setNodeQueue(NodeQueue * nodeQ) {
  this->myQueue = nodeQ;
}

/**
  * set the ref to the last nodeQueue occupied by the node
  */
void
Node::setLastQueue(NodeQueue * queue) {
  this->lastQueue = queue;
}

/**
 * get the ref to the last nodeQueue occupied by the node
 */
NodeQueue *
Node::getLastQueue() {
  return this->lastQueue;
}

/**
 * To get the node problem identifier
 */
string Node::getPb() {
  return this->myPb;
} // end getPb

/**
 * set the node profile
 */
void Node::set_pb_desc(diet_profile_t* profile) {
  this->profile = profile;
} // end set_pb_desc

/**
 * return the node profile
 */
diet_profile_t *
Node::getProfile() {
  return this->profile;
} // end getProfile


/**
 * set the parent Dag reference
 */
void
Node::setDag(Dag * dag) {
  this->myDag = dag;
}

/**
 * get the node Dag reference
 */
Dag *
Node::getDag() {
  if (this->myDag != NULL)
    return this->myDag;
  else {
    cout << "ERROR: calling getDag() on a node not linked to a dag" << endl;
    exit(0);
  }
}

/****************************************************************************/
/*                       Predecessor/Successors Mgmt                        */
/****************************************************************************/

/**
 * add a new previous node id *
 */
void Node::addPrecId(string str) {
  if (find(prec_ids.begin(), prec_ids.end(), str) == prec_ids.end())
    prec_ids.push_back(str);
} // end addPrecId

/**
 * Add a new previous node id and reference *
 */
void Node::addPrec(string str, Node * node) {
  // add the node as a previous one if not already done
  if (myPrevNodes.find(str) == myPrevNodes.end()) {
    TRACE_TEXT (TRACE_ALL_STEPS, "The node " << this->myId << " has a new previous node " <<
		str << endl);
    myPrevNodes[str] = node;
    this->addPrevNode();
    node->addNext(this);
    addPrecId(str);
  }
} // end addPrec

/**
 * Get the nb of previous nodes *
 */
unsigned int
Node::prevNb() {
  return prec_ids.size();
} // end prevNb

/**
 * Get the previous node id by index *
 */
string
Node::getPrecId(unsigned int n) {
  if (n<prec_ids.size())
    return prec_ids[n];
  else
    return string("");
} // end getPrecId

/**
 * Add a next node reference *
 */
void
Node::addNext(Node * n) {
  next.push_back(n);
} // end addNext

/**
 * add a new previous node *
 */
void
Node::addPrevNode() {
  prevNodes--;
} // end addPrevNode

/**
 * Add n new previous nodes *
 */
void
Node::addPrevNode(int n) {
  prevNodes -= n;
} // end addPrevNode

/**
 * return the number of next nodes
 */
unsigned int Node::nextNodesCount() {
  return next.size();
} // end nextNb

/**
 * return  next node
 */
Node * Node::getNext(unsigned int n) {
  if (n< next.size())
    return next[n];
  else
    return NULL;
} // end getNext

/**
 * return the number of previous nodes
 */
unsigned int Node::prevNodesCount() {
  return myPrevNodes.size();
} // end prevNb

/**
 * return  next node
 */
Node * Node::getPrev(unsigned int n) {
  if (n<myPrevNodes.size())
    return (Node*)(myPrevNodes.find(prec_ids[n])->second);
  else
    return NULL;
} // end getPrev

/**
 * return if the node is an input node *
 * only the nodes with no previous node are considered as dag input  *
 */
bool
Node::isAnInput() {
  return (myPrevNodes.size()==0);
} // end isAnInput

/**
 * return true if the node is an input node *
 * only the nodes with no next node are considered as dag exit  *
 */
bool
Node::isAnExit() {
  return (next.size() == 0);
} // end isAnExit

/****************************************************************************/
/*                          PRINT functions                                 */
/****************************************************************************/

/**
 * display the textual representation of a node *
 */
string
Node::toString() {
  string str = "Node representation\n";
  str += "Input Ports\n";
  for (map<string, WfInPort*>::iterator p = inports.begin();
       p != inports.end();
       p++) {
    WfInPort * in = (WfInPort*)(p->second);
    str += "\t\t"+ in->id + "\n";
  }

  str += "Output Ports\n";
  for (map<string, WfOutPort*>::iterator p = outports.begin();
       p != outports.end();
       p++) {
    WfOutPort * out = (WfOutPort*)(p->second);
    str += "\t\t"+ out->id + "\n";
  }
// return str + toStringBasis();
  return str;
} // end toString

/**
 * display an XML  representation of a node *
 * if b = false (value by default) the node representation doesn't include
 * the information about previous nodes (the source ports of input ports)
 */
string
Node::toXML(bool b) {
  string xml = "<node id=\""+ myId+"\" ";
  xml += "path=\""+ myPb +"\">\n";
  for (map<string, WfInPort*>::iterator p = inports.begin();
       p != inports.end();
       p++) {
    WfInPort * in = (WfInPort*)(p->second);
    xml += in->toXML(b);
  }
  for (map<string, WfOutPort*>::iterator p = outports.begin();
       p != outports.end();
       p++) {
    WfOutPort * out = (WfOutPort*)(p->second);
    xml += out->toXML();
  }
  xml += "</node>\n";
  return xml;
} // end toXML

/****************************************************************************/
/*                          Profile Allocation                              */
/****************************************************************************/

/**
 * Allocate a new char *
 */
char *
Node::newChar(const string value) {
  TRACE_TEXT (TRACE_ALL_STEPS, "new char ; value | " << value <<  " |" << endl);
  if (value != "") {
    char * cx = new char;
    *cx = value.c_str()[0];
    charParams.push_back(cx);
  }
  else {
    TRACE_TEXT (TRACE_ALL_STEPS, "$$$$$$$$$$$$$ Add a NULL" << endl);
    charParams.push_back(NULL);
  }
  return charParams[charParams.size() - 1];
} // end newChar

/**
 * Allocate a new short *
 */
short *
Node::newShort(const string value) {
  TRACE_TEXT (TRACE_ALL_STEPS, "new short ; value | " << value <<  " |" << endl);
  if (value != "") {
    short * sx = new short;
    *sx = atoi(value.c_str());
    shortParams.push_back(sx);
  }
  else {
    TRACE_TEXT (TRACE_ALL_STEPS, "$$$$$$$$$$$$$ Add a NULL" << endl);
    shortParams.push_back(NULL);
  }
  return shortParams[shortParams.size() - 1];
} // end newShort

/**
 * Allocate a new int  *
 */
int *
Node::newInt(const string value) {
  TRACE_TEXT (TRACE_ALL_STEPS,
	      "new int ; value | " << value <<  " |" << endl);
  if (value != "") {
    int * ix = new int;
    *ix = atoi(value.c_str());
    intParams.push_back(ix);
  }
  else {
    TRACE_TEXT (TRACE_ALL_STEPS,
		"$$$$$$$$$$$$$ Add a NULL" << endl);
    intParams.push_back(NULL);
  }
  return intParams[intParams.size() - 1];
} // end newInt

/**
 * Allocate a new long *
 */
long *
Node::newLong(const string value) {
  TRACE_TEXT (TRACE_ALL_STEPS,
	      "new long ; value | " << value <<  " |" << endl);
  if (value != "") {
    long * lx = new long;
    *lx = atoi(value.c_str());
    longParams.push_back(lx);
  }
  else {
    TRACE_TEXT (TRACE_ALL_STEPS,
		"$$$$$$$$$$$$$ Add a NULL" << endl);
    longParams.push_back(NULL);
  }
  return longParams[longParams.size() - 1];
} // end newLong

/**
 * Allocate a new string *
 */
char *
Node::newString (const string value) {
  char * str = new char[value.size()+1];
  strcpy(str, value.c_str());

  TRACE_TEXT (TRACE_ALL_STEPS,
	      "----> new string; value = " << value << ", " << str << endl);
  stringParams.push_back(str);
  return str;
} // end newString

/**
 * Allocate a new file *
 */
char *
Node::newFile (const string value) {
  char * str = new char[value.size()+1];
  strcpy(str, value.c_str());

  TRACE_TEXT (TRACE_ALL_STEPS,
	      "----> new file; value = " << value << ", " << str << endl);
  fileParams.push_back(str);
  return str;
} // end newFile

/**
 * Allocate a new float  *
 */
float *
Node::newFloat  (const string value) {
  float * fx = new float;
  *fx = (float)atof(value.c_str());
  floatParams.push_back(fx);
  return fx;
} // end newFloat

/**
 * Allocate a new double  *
 */
double *
Node::newDouble (const string value) {
  double * dx = new double;
  *dx = atof(value.c_str());
  doubleParams.push_back(dx);
  return dx;
} // end newDouble

/****************************************************************************/
/*                        Priority / Scheduling                             */
/****************************************************************************/

/**
 * set the node priority *
 */
void
Node::setPriority(double priority) {
  this->priority = priority;
} // end setPriority

/**
 * get the node priority *
 */
double
Node::getPriority() {
  return this->priority;
} // end getPriority

/****************************************************************************/
/*                           Node state                                     */
/****************************************************************************/

/**
 * test if the node is ready for execution (Madag side)
 * (check the counter of dependencies)
 */
bool
Node::isReady() {
  return (prevNodes == 0);
} // end isReady

/**
 * set the node as ready for execution (Madag side)
 * (Notifies the nodequeue if available)
 */
void
Node::setAsReady() {
  if (this->myQueue != NULL) {
    TRACE_TEXT (TRACE_ALL_STEPS,
      "Node " << this->getCompleteId() << " is ready: notify its queue" << endl);
    this->myQueue->notifyStateChange(this);
  }
}

/**
 * test if the node is running (Madag side)
 */
bool
Node::isRunning() {
  return node_running;
} // end isRunning

/**
 * Set node status as running (Madag side)
 */
void
Node::setAsRunning() {
  this->node_running = true;
} // end isRunning

/**
 * test if the execution is done (MaDag side)
 * (still used by HEFTscheduler to rank nodes)
 */
bool
Node::isDone() {
  return task_done;
} // end isDone

/**
 * Set the node status as done (MaDag side)
 */
void
Node::setAsDone(double compTime) {
  // update node scheduling info
  task_done = true;
  node_running = false;
  this->setRealCompTime(compTime);
  // notify the successors
  Node * n;
  TRACE_TEXT (TRACE_ALL_STEPS,
              "calling the " << next.size() << " next nodes" << endl);
  for (uint ix=0; ix< next.size(); ix++) {
    n = next[ix];
    n->prevNodeHasDone();
  }
} // end setAsDone

/**
 * Called when a previous node execution is done (MaDag side)
 */
void Node::prevNodeHasDone() {
  prevNodes++;
  if (this->isReady()) {
    this->setAsReady();
  }
} // end prevDone

/**
 * called when the node execution failed (MaDag & client side) *
 */
void
Node::setAsFailed() {
  this->getDag()->setNodeFailure(this->getId());
  taskExecFailed =  true;
  task_done = true;
  node_running = false;
}

/**
 * test if the execution failed (client side)
 */
bool
Node::hasFailed() {
  return taskExecFailed;
}

/****************************************************************************/
/*                        Node mapping to SeD                               */
/****************************************************************************/

/**
 * set the SeD reference to the node *
 */
void
Node::setSeD(const SeD_var& sed) {
  this->chosenServer = sed;
} // end setSeD

/**
 * return the SeD affected to the node
 */
SeD_var
Node::getSeD() {
  return this->chosenServer;
} // end getSeD

/****************************************************************************/
/*                           Timestamps (MaDag)                             */
/****************************************************************************/

/**
 * set the estimated completion time
 */
void
Node::setEstCompTime(double time) {
  this->estCompTime = time;
} // end setEstCompTime

/**
 * get the estimated completion time
 */
double
Node::getEstCompTime() {
  return this->estCompTime;
} // end getEstCompTime

/**
 * set the estimated delay
 * (can be increased or decreased)
 */
void
Node::setEstDelay(double delay) {
  // if this is an exit node then eventually updates the dag delay
  if (this->isAnExit()
      && (delay > this->estDelay)
      && (this->getDag() != NULL)) {
    this->getDag()->setEstDelay(delay);
  }
  this->estDelay = delay;
  TRACE_TEXT (TRACE_ALL_STEPS, "Updated est./real delay on node "
        << this->getCompleteId() << " : delay = " << delay << endl);
}

/**
 * get the estimated delay
 */
double
Node::getEstDelay() {
  return this->estDelay;
}

/**
 * set the real completion time
 */
void
Node::setRealCompTime(double time) {
  this->realCompTime = time;
} // end setRealCompTime

/**
 * get the real completion time
 */
double
Node::getRealCompTime() {
  return this->realCompTime;
} // end getRealCompTime

/**
 * get the real delay (positive) or 0 if no delay
 * or -1 if needed timestamps not set
 */
double
Node::getRealDelay() {
  if ((this->realCompTime != -1) && (this->estCompTime != -1)) {
    double delay = this->realCompTime - this->estCompTime;
    if (delay > 0) return delay;
    else return 0;
  } else return -1;
}

/****************************************************************************/
/*                       Ports & Profile Mgmt                               */
/****************************************************************************/

/**
 * Store the persistent data of the node profile *
 */
void
Node::storePersistentData() {
  for (map<string, WfOutPort*>::iterator p = outports.begin();
       p != outports.end();
       ++p) {
    WfOutPort * out = (WfOutPort*)(p->second);
    store_id(profile->parameters[out->index].desc.id,
	     (char *) "wf param");
  }
} // end storePersistentData

/**
 * Get the input port references by id *
 */
WfInPort *
Node::getInPort(string id) {
  map<string, WfInPort*>::iterator p = inports.find(id);
  if (p != inports.end())
    return ((WfInPort*)(p->second));
  else
    return NULL;
} // end getInPort

/**
 * Get the output port reference by id *
 */
WfOutPort *
Node::getOutPort(string id) {
  map<string, WfOutPort*>::iterator p = outports.find(id);
  if (p != outports.end())
    return ((WfOutPort*)(p->second));
  else
    return NULL;
} // end getOutPort

/**
 * Get the input/output port reference by id *
 */
WfInOutPort *
Node::getInOutPort(string id) {
  map<string, WfInOutPort*>::iterator p = inoutports.find(id);
  if (p != inoutports.end())
    return ((WfInOutPort*)(p->second));
  else
    return NULL;
} // end getInOutPort

/**
 * create the diet profile associated to the node *
 */
void
Node::createProfile() {
  int last_in = this->profile->last_in;
  int last_inout = this->profile->last_inout;
  int last_out = this->profile->last_out;
  diet_profile_free(this->profile);
  TRACE_TEXT (TRACE_ALL_STEPS,
	      "Reallocating a new profile " << myPb.c_str() << ", " <<
	      last_in << ", " <<
	      last_inout << ", " <<
	      last_out << endl);
  this->profile =  diet_profile_alloc((char*)(this->myPb.c_str()),
					   last_in, last_inout, last_out);

  int last = 0;
  // input ports
  TRACE_TEXT (TRACE_ALL_STEPS,
	      inports.size() << " input ports" << endl);
  for (map<string, WfInPort*>::iterator p = inports.begin();
       p != inports.end();
       ++p) {
    WfInPort * in = (WfInPort*)(p->second);
    if (in->isInput())
      this->set_profile_param(in,
			      in->type, in->index, in->value, DIET_VOLATILE);
    else
      this->set_profile_param(in,
			      in->type, in->index, in->value, DIET_PERSISTENT);

    last ++;
  }
  // output ports
  TRACE_TEXT (TRACE_ALL_STEPS,
	      outports.size() << " output ports" << endl);
  for (map<string, WfOutPort*>::iterator p = outports.begin();
       p != outports.end();
       ++p) {
    WfOutPort * out = (WfOutPort*)(p->second);
    TRACE_TEXT (TRACE_ALL_STEPS,
		"%%%%%%%% " << out->type << endl);
    this->set_profile_param(out,
			    out->type, out->index, out->value,
			    DIET_PERSISTENT);
    last ++;
  }
  // inoutput ports
  for (map<string, WfInOutPort*>::iterator p = inoutports.begin();
       p != inoutports.end();
       ++p) {
    WfInOutPort * inout = (WfInOutPort*)(p->second);
    this->set_profile_param(inout,
			    inout->type, inout->index,
			    inout->value, DIET_PERSISTENT);
    last ++;
  }
} // end createProfile

/**
 * set the node profile param *
 * @param type    parameter data type *
 * @param lastArg parameter index *
 * @param value   string representation of parameter value *
 */
void
Node::set_profile_param(WfPort * port,
			string type, const int lastArg, const string& value,
			const diet_persistence_mode_t mode) {
  TRACE_TEXT (TRACE_ALL_STEPS,
	      "\tset_profile_param : type = " << type <<
	      ", lastArg = " << lastArg << ", value = " << value <<
	      ", mode = " << mode << endl);
  if (type == WfCst::DIET_CHAR) {
    TRACE_TEXT (TRACE_ALL_STEPS,
		"char parameter "<< lastArg << endl);
    if (value != "")
      diet_scalar_set(diet_parameter(profile, lastArg),
		      this->newChar(value),
		      mode,
		      DIET_CHAR);
    else
      diet_scalar_set(diet_parameter(profile, lastArg),
		      this->newChar(),
		      mode,
		      DIET_CHAR);
  }
  if (type == WfCst::DIET_SHORT) {
    TRACE_TEXT (TRACE_ALL_STEPS,
		"short parameter "<< lastArg << endl);
    if (value != "")
      diet_scalar_set(diet_parameter(profile, lastArg),
		      this->newShort(value),
		      mode,
		      DIET_SHORT);
    else
      diet_scalar_set(diet_parameter(profile, lastArg),
		      this->newShort(),
		      mode,
		      DIET_SHORT);
  }
  if (type == WfCst::DIET_INT) {
    TRACE_TEXT (TRACE_ALL_STEPS,
		"int parameter "<< lastArg << endl);
    if (value != "")
      diet_scalar_set(diet_parameter(profile, lastArg),
		      this->newInt(value),
		      mode,
		      DIET_INT);
    else
      diet_scalar_set(diet_parameter(profile, lastArg),
		      this->newInt(),
		      mode,
		      DIET_INT);
  }
  if (type == WfCst::DIET_LONGINT) {
    TRACE_TEXT (TRACE_ALL_STEPS,
		"long parameter "<< lastArg << endl);
    if (value != "")
      diet_scalar_set(diet_parameter(profile, lastArg),
		      this->newLong(value),
		      mode,
		      DIET_LONGINT);
    else
      diet_scalar_set(diet_parameter(profile, lastArg),
		      this->newLong(),
		      mode,
		      DIET_LONGINT);
  }
  if (type == WfCst::DIET_PARAMSTRING) {
    TRACE_TEXT (TRACE_ALL_STEPS,
		"%%%%%%%%% PARAMSTRING parameter "<< lastArg << endl);
    if (value != "")
      diet_paramstring_set(diet_parameter(profile, lastArg),
		      this->newString(value),
		      mode);
    else
      diet_paramstring_set(diet_parameter(profile, lastArg),
		      this->newString(),
		      mode);
  }
  if (type == WfCst::DIET_STRING) {
    //
    //  diet_string_set(diet_arg_t* arg,
    //                  char* value, diet_persistence_mode_t mode);
    //
    TRACE_TEXT (TRACE_ALL_STEPS,
		"%%%%%%%%% STRING parameter "<< lastArg << endl);
    if (value != "")
      diet_string_set(diet_parameter(profile, lastArg),
		      this->newString(value),
		      mode);
    else
      diet_string_set(diet_parameter(profile, lastArg),
		      this->newString(),
		      mode);
  }
  if (type == WfCst::DIET_FLOAT) {
    if (value != "")
      diet_scalar_set(diet_parameter(profile, lastArg),
		      this->newFloat(value),
		      mode,
		      DIET_FLOAT);
    else
      diet_scalar_set(diet_parameter(profile, lastArg),
		      this->newFloat(),
		      mode,
		      DIET_FLOAT);
  }
  if (type == WfCst::DIET_DOUBLE) {
    if (value != "")
      diet_scalar_set(diet_parameter(profile, lastArg),
		      this->newDouble(value),
		      mode,
		      DIET_DOUBLE);
    else
      diet_scalar_set(diet_parameter(profile, lastArg),
		      this->newDouble(),
		      mode,
		      DIET_DOUBLE);
  }
  if (type == WfCst::DIET_FILE) {
    TRACE_TEXT (TRACE_ALL_STEPS,
		"%%%%%%%%% FILE parameter "<< lastArg << endl);
    if (value != "")
      diet_file_set(diet_parameter(profile, lastArg),
		    mode,
		    this->newFile(value));
    else
      diet_file_set(diet_parameter(profile, lastArg),
		    mode,
		    NULL);
  }

  if (type == WfCst::DIET_MATRIX) {
    TRACE_TEXT (TRACE_ALL_STEPS,
		"the profile contain a matrix" << endl);
    void * mat = NULL;
    switch (port->base_type) {
    case DIET_CHAR:
      mat = new char[port->nb_r*port->nb_c];
      break;
    case DIET_SHORT:
      mat = new int[port->nb_r*port->nb_c];
      break;
    case DIET_INT:
      mat = new int[port->nb_r*port->nb_c];
      break;
    case DIET_LONGINT:
      mat = new long[port->nb_r*port->nb_c];
      break;
    case DIET_FLOAT:
      mat = new float[port->nb_r*port->nb_c];
      break;
    case DIET_DOUBLE:
      mat = new double[port->nb_r*port->nb_c];
      break;
    default:
      // Nothing to do ?
      return;
      break;
    } // end (switch)
    if (value != "") {
      if (value.substr(0, (string("file->")).size()) == "file->") {
	string dataFileName = value.substr((string("file->")).size());
	unsigned len = port->nb_r*port->nb_c;
	TRACE_TEXT (TRACE_ALL_STEPS,
		    "reading the matrix data file" << endl);
	switch (port->base_type) {
	case DIET_CHAR:
	  //	  char * ptr1 = (char*)(mat);
	  WfCst::readChar(dataFileName.c_str(), (char*)(mat), len);
	  break;
	case DIET_SHORT:
	  //	  short * ptr2 = (short*)(mat);
	  WfCst::readShort(dataFileName.c_str(), (short*)(mat), len);
	  break;
	case DIET_INT:
	  //	  int * ptr3 = (int*)(mat);
	  WfCst::readInt(dataFileName.c_str(), (int*)(mat), len);
	  break;
	case DIET_LONGINT:
	  //	  long * ptr4 = (long*)(mat);
	  WfCst::readLong(dataFileName.c_str(), (long*)(mat), len);
	  break;
	case DIET_FLOAT:
	  //	  float * ptr5 = (float*)(mat);
	  WfCst::readFloat(dataFileName.c_str(), (float*)(mat), len);
	  break;
	case DIET_DOUBLE:
	  //	  double * ptr6 = (double*)(mat);
	  WfCst::readDouble(dataFileName.c_str(), (double*)(mat), len);
	  break;
	default:
	  // Nothing to do ?
	  return;
	  break;
	} // end switch

      }
      else {
	// get the data if included in the XML workflow description
	vector<string> v = getStringToken(port->value);
	unsigned int len = v.size();
	// fill the matrix with the given data
	TRACE_TEXT (TRACE_ALL_STEPS,
		    "filling the matrix with the data (" << len << ")" << endl);
	char  * ptr1(NULL);
	short * ptr2(NULL);
	int   * ptr3(NULL);
	long  * ptr4(NULL);
	float * ptr5(NULL);
	double * ptr6(NULL);
	switch (port->base_type) {
	case DIET_CHAR:
	  ptr1 = (char*)(mat);
	  for (unsigned int ix = 0; ix<len; ix++)
	    ptr1[ix] = v[ix][0];
	  break;
	case DIET_SHORT:
	  ptr2 = (short*)(mat);
	  for (unsigned int ix = 0; ix<len; ix++)
	    ptr2[ix] = atoi(v[ix].c_str());
	  break;
	case DIET_INT:
	  ptr3 = (int*)(mat);
	  for (unsigned int ix = 0; ix<len; ix++)
	    ptr3[ix] = atoi(v[ix].c_str());
	  break;
	case DIET_LONGINT:
	  ptr4 = (long*)(mat);
	  for (unsigned int ix = 0; ix<len; ix++)
	    ptr4[ix] = atoi(v[ix].c_str());
	  break;
	case DIET_FLOAT:
	  ptr5 = (float*)(mat);
	  for (unsigned int ix = 0; ix<len; ix++)
	    ptr5[ix] = atof(v[ix].c_str());
	  break;
	case DIET_DOUBLE:
	  ptr6 = (double*)(mat);
	  for (unsigned int ix = 0; ix<len; ix++)
	    ptr6[ix] = atof(v[ix].c_str());
	  break;
	default:
	  // Nothing to do ?
	  return;
	  break;
	} // end switch
      } // end else
    } // end if value
    diet_matrix_set(diet_parameter(profile,lastArg),
		    mat, mode,
		    port->base_type,
		    port->nb_r,
		    port->nb_c,
		    port->order);
    //    matrixParams.push_back(mat);
  }
} // end set_profile_param

/**
 * Free the profile and the persistent data of the node
 */
void
Node::freeProfileAndData() {
  TRACE_TEXT (TRACE_ALL_STEPS,
              myId << " Free profile and release persistent data" << endl);
    // Free persistent data
  for (map<string, WfOutPort*>::iterator p = outports.begin();
       p != outports.end();
       ++p) {
         WfOutPort * out = (WfOutPort*)(p->second);
//       if (!out->isResult()) {
         diet_free_persistent_data(profile->parameters[out->index].desc.id);
//       } // end if
  } // end for
  diet_profile_free(profile);
  profile = NULL;
}

/****************************************************************************/
/*                           Ports linking                                  */
/****************************************************************************/

/**
 * Link input port to output port by id and setting references link *
 */
void
Node::link_i2o(const string in, const string out) {
  // the port is supposed to be present
  if (out == "") {
    return;
  }
  WfInPort * inPort = getInPort(in);
  if (inPort != NULL)
    inPort->set_source(out);
  else {
    INTERNAL_ERROR ("The output port " << out << " was not found " << endl <<
		    "Check your XML file", 1);
  }
} // end link_i2o

/**
 * Link output port to input port by id and setting references link *
 */
void
Node::link_o2i(const string out, const string in) {
  if (in == "") {
    return;
  }
  // the port is supposed to be present
  WfOutPort * outPort = getOutPort(out);
  if (outPort != NULL)
    outPort->set_sink(in);
  else {
    INTERNAL_ERROR ("The output port " << out << " was not found " << endl <<
		    "Check your XML file", 1);
  }
} // end link_o2i

/**
 * Link inoutput port to input port by id and setting references link *
 */
void
Node::link_io2i(const string io, const string in) {
  // TO FIX
  // the port is supposed to be present
  getInOutPort(io)->set_sink(in);
} // end link_io2i

/**
 * Link inoutput port to output port by id and setting references link *
 */
void
Node::link_io2o(const string io, const string out) {
  // the port is supposed to be present
  getInOutPort(io)->set_source(out);
} // end link_io2o

/**
 * create and add a new port to the node *
 */
WfPort *
Node::newPort(string id, uint ind, wf_port_t type, string diet_type,
	      const string& v) {
  WfPort * p = NULL;
  switch (type) {
  case ARG_PORT:
    // nothing to do ??
     p = new WfInPort(this, id, diet_type, ind, v);
    inports[id] =  (WfInPort *) p;
    break;
  case IN_PORT :
    p = new WfInPort(this, id, diet_type, ind, v);
    inports[id] = (WfInPort *) p;
    break;
  case INOUT_PORT:
    p = new WfInOutPort(this, id, diet_type, ind, v);
    inoutports[id] = (WfInOutPort *) p;
    break;
  case OUT_PORT:
    p = new WfOutPort(this, id, diet_type, ind, v);
    outports[id] = (WfOutPort *)p;
    break;
  }
  return (WfPort*)p;
} // end newPort

/****************************************************************************/
/*                           CLIENT - SIDE                                  */
/****************************************************************************/

/**
 * start the node execution *
 * Note: the argument join are always set to default currently
 */
void Node::start(bool join) {
  TRACE_TEXT (TRACE_ALL_STEPS, " create the runnable node " << endl);
  node_running = true;
//  nodeIsStarting(myId.c_str());

  this->myRunnableNode = new RunnableNode(this);
  TRACE_TEXT (TRACE_ALL_STEPS, "The node " << myId << " tries to launch a RunnableNode "<< endl);
  this->myRunnableNode->start();
  TRACE_TEXT (TRACE_ALL_STEPS, "The node " << myId << " launched its RunnableNode" << endl);
//   AbstractWfSched::pop(this->myId);
  if (join)
    this->myRunnableNode->join();
  TRACE_TEXT (TRACE_ALL_STEPS, "The node " << myId << " joined its RunnableNode" << endl);
} // end start


/**
 * called when the node execution is done (client side) *
 */
void
Node::done() {
//   TRACE_TEXT (TRACE_ALL_STEPS,
// 	      "calling the " << myPrevNodes.size() << " previous nodes" << endl);
//   Node * n = NULL;
//   for (map<string, Node*>::iterator p = myPrevNodes.begin();
//        p != myPrevNodes.end();
//        ++p) {
//     n = (Node *)(p->second);
//     if (n) {
//       n->nextIsDone();
//     }
//   }
     task_done = true;
     node_running = false;
} // end done

diet_reqID_t
Node::getReqID(){
	//cout << "Node::" <<__FUNCTION__ << "()"<< endl;
	if(this->isDone())
		return this->getProfile()->dietReqID;
	return -1;
}
/**
 * called when a next node is done *
 */
void
Node::nextIsDone() {
  nextDone++;
//   if (nextDone == next.size()) {
//     TRACE_TEXT (TRACE_ALL_STEPS,
// 		myId << " Now I can free my profile !" << endl);
//     // Free persistent data
//     for (map<string, WfOutPort*>::iterator p = outports.begin();
// 	 p != outports.end();
// 	 ++p) {
//       WfOutPort * out = (WfOutPort*)(p->second);
//       if (!out->isResult()) {
// 	TRACE_TEXT (TRACE_ALL_STEPS,
// 		    "\tRelease the persistent data " <<
// 		    profile->parameters[out->index].desc.id << endl);
// 	diet_free_persistent_data(profile->parameters[out->index].desc.id);
//       } // end if
//     } // end for
//     diet_profile_free(profile);
//     profile = NULL;
//   }
} // end nextIsDone

