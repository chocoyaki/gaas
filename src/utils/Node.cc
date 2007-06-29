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

#include "statistics.hh"
#include "debug.hh"
#include "marshalling.hh"
#include "MasterAgent.hh"

#include "Node.hh"

#include "Dag.hh"

MasterAgent_var getMA();
bool useMaDagSched();

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
 * This function is based on the function diet_call_common of DIET_client.cc
 * It was added to manage the reqID which are not set if we use the original
 * function
 * To keep the code short, the JUXMEM part was removed
 */
diet_error_t
diet_call_common(diet_profile_t* profile, SeD_var& chosenServer) {
  int solve_res(0);
  corba_profile_t corba_profile;

  char statMsg[128];
  stat_in("Client","diet_call");

  /* Send Datas */

  if (mrsh_profile_to_in_args(&corba_profile, profile)) {
    ERROR("profile is wrongly built", 1);
  }
  
  int j = 0;
  bool found = false;
  while ((j <= corba_profile.last_out) && (found == false)) {
    if (diet_is_persistent(corba_profile.parameters[j])) {      
      found = true;
    }
    j++;
  }
  if(found == true){
    create_file();
  }


  /* data property base_type and type retrieval : used for scheduler */
  for(int i = 0 ; i <= corba_profile.last_out ; i++) {
    char* new_id = strdup(corba_profile.parameters[i].desc.id.idNumber);
    if(strlen(new_id) != 0) {
      corba_data_desc_t* arg_desc = new corba_data_desc_t;
      arg_desc = getMA()->get_data_arg(new_id);
      const_cast<corba_data_desc_t&>(corba_profile.parameters[i].desc) = *arg_desc;
    }
  }  
  
  /* generate new ID for data if not already existant */
  for(int i = 0 ; i <= corba_profile.last_out ; i++) {
    if((corba_profile.parameters[i].desc.mode > DIET_VOLATILE ) && 
       (corba_profile.parameters[i].desc.mode < DIET_PERSISTENCE_MODE_COUNT) &&
       (getMA()->dataLookUp(strdup(corba_profile.parameters[i].desc.id.idNumber))))
      {
	char* new_id = getMA()->get_data_id(); 
	corba_profile.parameters[i].desc.id.idNumber = new_id;
      }
  }


  /* Computation */
  sprintf(statMsg, "computation %ld", (unsigned long) profile->dietReqID);
  try {
    stat_in("Client",statMsg);

    TRACE_TEXT(TRACE_MAIN_STEPS, 
	       "Calling the ref Corba of the SeD (reqID = " << profile->dietReqID << ")\n");
    TRACE_TEXT(TRACE_MAIN_STEPS, statMsg << endl);

    TRACE_TEXT (TRACE_ALL_STEPS,
		  "Calling the service " << profile->pb_name << endl);
    display(corba_profile);
    solve_res = chosenServer->solve(profile->pb_name, corba_profile);
    stat_out("Client",statMsg);
   } catch(CORBA::MARSHAL& e) {
    ERROR("got a marchal exception\n"
          "Maybe your giopMaxMsgSize is too small",1) ;
  }

  /* reaffect identifier */
  for(int i = 0;i <= profile->last_out;i++) {
    if ((corba_profile.parameters[i].desc.mode > DIET_VOLATILE ) && 
        (corba_profile.parameters[i].desc.mode < DIET_PERSISTENCE_MODE_COUNT)) {
      profile->parameters[i].desc.id = strdup(corba_profile.parameters[i].desc.id.idNumber);
    }
  }

  if (unmrsh_out_args_to_profile(profile, &corba_profile)) {
    INTERNAL_ERROR("returned profile is wrongly built", 1);
  }

  sprintf(statMsg, "diet_call %ld", (unsigned long) profile->dietReqID);
  TRACE_TEXT(TRACE_MAIN_STEPS, statMsg << endl);
  stat_out("Client",statMsg);
  return solve_res;

} // end diet_call_common

/**
 * RunnableNode constructor
 */
RunnableNode::RunnableNode(Node * parent,
			   diet_reqID_t reqID)
  :Thread(true) {
  this->myParent = parent;
  this->myReqID = reqID;
} // end RunnableNode constructor

/**
 * Node execution methods *
 */
void * 
RunnableNode::run() {
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

  nodeIsRunning(myParent->getId().c_str());

  if (!CORBA::is_nil(myParent->chosenServer) 
      &&
      useMaDagSched()) {
    TRACE_TEXT (TRACE_MAIN_STEPS, 
		"Using the scheduling of the mapped SeD" << endl <<
		"call the chosenServer ..." << endl);
    if ( ! diet_call_common(myParent->profile, myParent->chosenServer)) {
      myParent->storePersistentData();
    }
    TRACE_TEXT (TRACE_ALL_STEPS, "done" << endl);
  }
  else {
    TRACE_TEXT (TRACE_MAIN_STEPS, "Using the MA to call the SeD"<< endl);
    if (!diet_call(myParent->profile)) {
      myParent->storePersistentData();
    }
  }
  TRACE_TEXT (TRACE_ALL_STEPS, "RunnableNode call ... done" << endl);
  myParent->done();
  return NULL;
} // end RunnableNode::run

/**
 * The Node constructor
 */
Node::Node(string id, string pb_name,
	   int last_in, int last_inout, int last_out) {
  this->myId = id;
  this->myPb = pb_name;
  this->prevNodes = 0;
  this->task_done = false;
  this->myTag = 0;
  this->profile = diet_profile_alloc((char*)pb_name.c_str(), 
				     last_in, last_inout, last_out);
  this->node_running = false;
  this->myRunnableNode = NULL;
  this->chosenServer = SeD::_nil();  
  this->nextDone = 0;
  this->myMark = false;
} // end Node constructir

/**
 * Node destructor
 */
Node::~Node() {
  // if everything is alright, we need this line only for the exit node(s)
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
    // string vector
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

/**
 * add a new previous node id *
 */
void Node::addPrecId(string str) {
  prec_ids.push_back(str);
} // end addPrecId

/**
 * Add a new previous node id and reference *
 */
void Node::addPrec(string str, Node * node) {
  // add the node as a previous one if not already done
  if (prec.find(str) == prec.end()) {
    TRACE_TEXT (TRACE_ALL_STEPS, "The node " << this->myId << " has a new previous node " << 
		str << endl);
    prec[str] = node; 
    prevNodes--;
    node->addNext(this);
  }
} // end addPrec

/**
 * To get the node id *
 */
string Node::getId() {
  return this->myId;
} // end getId

/**
 * To get the node id *
 */
string Node::getPb() {
  return this->myPb;
} // end getPb

/**
 * Return a string representation of the node *
 */
string
Node::toStringBasis() {
  string str = "THIS FUNCTION CALL MUST BE REMOVED";
  return str;
}

/**
 * Return the number of the previous nodes *
 */  
unsigned int
Node::prec_ids_nb() {
  return prec_ids.size();
} // end prec_ids_nb

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
 * Called when a previous node execution is done *
 */
void 
Node::prevDone() {
  prevNodes++;
} // end prevDone

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
 * set the node tag value *
 */
void
Node::setTag(unsigned int t) {
  this->myTag = MAX(this->myTag, t);
  for (unsigned int ix=0; ix < next.size(); ix++) {
    next[ix] -> setTag(1+ (this->myTag));
  }
} // end setTag

/**
 * return if the node is an input node *
 * only the nodes with no previous node are considered as dag input  * 
 */
bool
Node::isAnInput() {
  return (prec.size()==0);
} // end isAnInput

/**
 * return true if the node is an input node *
 * only the nodes with no next node are considered as dag exit  * 
 */
bool
Node::isAnExit() {
  return (next.size() == 0);
} // end isAnExit

/**
 * Set the node ID
 */
void 
Node::setId(string id) {
  this->myId = id;
} // end setId

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
  return str + toStringBasis();
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

/**
 * set the node profile
 */
void
Node::set_pb_desc(diet_profile_t* profile) {
  this->profile = profile;
} // end set_pb_desc

/**
 * start the node execution * 
 */
void
Node::start(diet_reqID_t reqID) {
  TRACE_TEXT (TRACE_ALL_STEPS, " create the runnable node " << endl);
  node_running = true;
  nodeIsStarting(myId.c_str());
  myRunnableNode = new RunnableNode(this, reqID);
  TRACE_TEXT (TRACE_ALL_STEPS, "The node " << myId << " tries to launch a RunnableNode "<< endl);
  myRunnableNode->start();
  TRACE_TEXT (TRACE_ALL_STEPS, "The node " << myId << " launched it RunnableNode" << endl);
} // end start

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

/**
 * get if the node is ready for execution *
 */
bool 
Node::isReady() {
  return (prevNodes == 0);
} // end isReady

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

/**
 * return the number of next nodes
 */
unsigned int 
Node::nextNb() {
  return next.size();
} // end nextNb

/**
 * return  next node 
 */
Node *
Node::getNext(unsigned int n) {
  if (n< next.size())
    return next[n];
  else
    return NULL;
} // end getNext

/**
 * return the number of previous nodes
 */
unsigned int 
Node::prevNb() {
  return prec_ids.size();
} // end prevNb

/**
 * return  next node 
 */
Node *
Node::getPrev(unsigned int n) {
  if (n<prec_ids.size())
    return (Node*)(prec.find(prec_ids[n])->second);
  else
    return NULL;
} // end getPrev


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
	     "wf param");
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
 * Set the node as running *
 */
void 
Node::set_as_running() {
  node_running = true;
} // end set_as_running

/**
 * test if the node is running *
 */
bool 
Node::isRunning() {
  return node_running;
} // end isRunning

/**
 * test if the execution is done *
 */
bool 
Node::isDone() {
  return task_done;
} // end isDone

/**
 * called when the node execution is done *
 */
void
Node::done() {
  nodeIsDone(myId.c_str(), this->myDag->getId().c_str());
  // get the current time and set the real time variable
  struct timeval tv;
  gettimeofday(&tv, NULL);
  this->setRealCompTime(tv);

  TRACE_TEXT (TRACE_ALL_STEPS,
	      "The node terminate, the estimate completion time is " <<
	      EstCompTime << " and the real completion time is " <<
	      RealCompTime.tv_sec << endl);

  Node * n = NULL;
  TRACE_TEXT (TRACE_ALL_STEPS,
	      "calling the " << next.size() << " next nodes" << endl);
  for (uint ix=0; ix< next.size(); ix++) {
    n = next[ix];
    n->prevDone();
  }
  TRACE_TEXT (TRACE_ALL_STEPS,
	      "calling the " << prec.size() << " previous nodes" << endl);
  n = NULL;
  for (map<string, Node*>::iterator p = prec.begin();
       p != prec.end();
       ++p) {
    n = (Node *)(p->second);
    if (n) {
      n->nextIsDone();
    }
  }
  task_done = true;
  node_running = false;
} // end done

/**
 * called when a next node is done *
 */
void
Node::nextIsDone() {
  nextDone++;
  if (nextDone == next.size()) {
    TRACE_TEXT (TRACE_ALL_STEPS,
		myId << " Now I can free my profile !" << endl);
    // Free persistent data
    for (map<string, WfOutPort*>::iterator p = outports.begin();
	 p != outports.end();
	 ++p) {
      WfOutPort * out = (WfOutPort*)(p->second); 
      if (!out->isResult()) {
	TRACE_TEXT (TRACE_ALL_STEPS,
		    "\tRelease the persistent data " <<
		    profile->parameters[out->index].desc.id << endl);
	diet_free_persistent_data(profile->parameters[out->index].desc.id);
      } // end if
    } // end for
    diet_profile_free(profile);
    profile = NULL;
  }
} // end nextIsDone

/**
 * set the estimated completion time
 */
void
Node::setEstCompTime(const long int est_comp_time) {
  this->EstCompTime = est_comp_time;
} // end setEstCompTime

/**
 * get the estimated completion time
 */
long int
Node::getEstCompTime() {
  return this->EstCompTime;
} // end getEstCompTime

/**
 * set the real completion time
 */
void
Node::setRealCompTime(const struct timeval& real_comp_time) {
  this->RealCompTime = real_comp_time;
} // end setRealCompTime

/** 
 * get the real completion time
 */
struct timeval
Node::getRealCompTime() {
  return this->RealCompTime;
} // end getRealCompTime

/**
 * get the node mark (used for reordering)
 */
bool
Node::getMark() {
  return this->myMark;
} // end getMark

/**
 * set the node mark 
 */
void 
Node::setMark(bool b) {
  this->myMark = b;
} // end setMark

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
  return this->myDag;
}

