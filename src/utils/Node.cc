/****************************************************************************/
/* Extended Node description                                                */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
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

#include "Node.hh"

diet_error_t
diet_call_common(diet_profile_t* profile, SeD_var& chosenServer);


RunnableNode::RunnableNode(Node * parent):Thread(true) {
  this->myParent = parent;
}

/**
 * Node execution methods *
 */
void * 
RunnableNode::run() {
  cout << "RunnableNode tries to a execute a service "<< endl;
  // create the node diet profile
  cout << "create the node diet profile" << endl;
  myParent->createProfile();
  cout << "profile creation ... done" << endl;
  cout << "Init the ports " << endl;
  for (map<string, Node::WfInPort*>::iterator p = myParent->inports.begin();
       p != myParent->inports.end();
       ++p) {
    Node::WfInPort * in = (Node::WfInPort*)(p->second);
    if (in->source_port) {
      Node::WfOutPort * out = (Node::WfOutPort*)(in->source_port);
      diet_use_data(diet_parameter(myParent->profile, in->index),
		    out->profile()->parameters[out->index].desc.id);
    }
  }
  cout << "Init ports data ... done " << endl;

  nodeIsRunning(myParent->getId().c_str());

  if (!CORBA::is_nil(myParent->chosenServer)) {
    cout << "using the scheduling provided by the MA_DAG" << endl <<
      "call the chosenServer ..." << endl;
    if ( ! diet_call_common(myParent->profile, myParent->chosenServer)) {
      myParent->storePersistentData();
    }
    cout << "done" << endl;
  }
  else {
    if (!diet_call(myParent->profile)) {
      myParent->storePersistentData();
    }
  }
  cout << "RunnableNode call ... done" << endl;
  myParent->done();
  return NULL;
}

Node::Node(string id, string pb_name,
	   int last_in, int last_inout, int last_out) :
  BasicNode(id, pb_name) {
  //  profile = new diet_profile_t;
  profile =   profile = diet_profile_alloc((char*)pb_name.c_str(), 
					   last_in, last_inout, last_out);
  node_running = false;

  myRunnableNode = NULL;

  chosenServer = SeD::_nil();  

  nextDone = 0;

  myMark = false;
}

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
    // long vector
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
}

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
  return str + BasicNode::toString();
}

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
}

/**
 * set the node profile
 */
void
Node::set_pb_desc(diet_profile_t* profile) {
  this->profile = profile;
}

/**
 * start the node execution * 
 */
void
Node::start() {
  cout << " create the runnable node " << endl;
  node_running = true;
  nodeIsStarting(myId.c_str());
  myRunnableNode = new RunnableNode(this);
  cout << "The node " << myId << " tries to launch a RunnableNode "<< endl;
  myRunnableNode->start();
  cout << "The node " << myId << " launched it RunnableNode" << endl;
}

/**
 * Allocate a new char *
 */
char *  
Node::newChar(const string value) {
  char * cx = new char;
  *cx = value.c_str()[0];
  charParams.push_back(cx);
  return cx;
}

/**
 * Allocate a new short *
 */
short * 
Node::newShort(const string value) {
  short * sx = new short;
  *sx = atoi(value.c_str());
  shortParams.push_back(sx);
  return sx;
}

/**
 * Allocate a new int  *
 */
int *   
Node::newInt(const string value) {
  cout << "new int ; value " << value << endl;
  int * ix = new int;
  *ix = atoi(value.c_str());
  intParams.push_back(ix);
  return ix;
}

/**
 * Allocate a new long *
 */
long *  
Node::newLong(const string value) {
  long * lx = new long;
  *lx = atoi(value.c_str());
  longParams.push_back(lx);
  return lx;
}

/**
 * Allocate a new string *
 */
char * 
Node::newString (const string value) {
  char * str = new char[value.size()+1];
  strcpy(str, value.c_str());

  cout << "----> new string; value = " << value << ", " << str << endl;
  stringParams.push_back(str);
  return str;
}

/**
 * Allocate a new float  *
 */
float *   
Node::newFloat  (const string value) {
  float * fx = new float;
  *fx = (float)atof(value.c_str());
  floatParams.push_back(fx);
  return fx;
}

/**
 * Allocate a new double  *
 */
double *   
Node::newDouble (const string value) {
  double * dx = new double;
  *dx = atof(value.c_str());
  doubleParams.push_back(dx);
  return dx;
}

/**
 * set the node priority *
 */
void
Node::setPriority(double priority) {
  this->priority = priority;
}

/**
 * get the node priority *
 */
double  
Node::getPriority() {
  return this->priority;
}

/**
 * get if the node is ready for execution *
 */
bool 
Node::isReady() {
  return (prevNodes == 0);
}

/**
 * Link input port to output port by id and setting references link *
 */
void 
Node::link_i2o(const string in, const string out) {
  // the port is supposed to be present
  if (out == "") {
    return;
  } 
  getInPort(in)->set_source(out);
}

/**
 * Link output port to input port by id and setting references link *
 */
void 
Node::link_o2i(const string out, const string in) {
  if (in == "") {
    return;
  }
  // the port is supposed to be present
  getOutPort(out)->set_sink(in);
}

/**
 * Link inoutput port to input port by id and setting references link *
 */
void 
Node::link_io2i(const string io, const string in) {
  // TO FIX
  // the port is supposed to be present
  getInOutPort(io)->set_sink(in);
}

/**
 * Link inoutput port to output port by id and setting references link *
 */
void
Node::link_io2o(const string io, const string out) {
  // the port is supposed to be present
  getInOutPort(io)->set_source(out);
}

/**
 * create and add a new port to the node *
 */
Node::WfPort *
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
}

/**
 * set the SeD reference to the node *
 */
void
Node::setSeD(const SeD_var& sed) {
  this->chosenServer = sed;
}

/**
 * return the number of next nodes
 */
unsigned int 
Node::nextNb() {
  return next.size();
}

/**
 * return  next node 
 */
BasicNode *
Node::getNext(unsigned int n) {
  if (n< next.size())
    return next[n];
  else
    return NULL;
}

/**
 * return the number of previous nodes
 */
unsigned int 
Node::prevNb() {
  return prec_ids.size();
}

/**
 * return  next node 
 */
BasicNode *
Node::getPrev(unsigned int n) {
  if (n<prec_ids.size())
    return (BasicNode*)(prec.find(prec_ids[n])->second);
  else
    return NULL;
}


/**
 * Store the persistent data of the node profile *
 */
void 
Node::storePersistentData() {
  /*
  for (int ix=0; ix<profile->last_out; ix++) {
    store_id(profile->parameters[ix].desc.id, "wf param");
  }
  */
  
  for (map<string, WfOutPort*>::iterator p = outports.begin();
       p != outports.end();
       ++p) {
    WfOutPort * out = (WfOutPort*)(p->second); 
    //    cout << "try to store the parameter " << out->index << endl;
    store_id(profile->parameters[out->index].desc.id,
	     "wf param");
  }
}

/**
 * Get the input port references by id *
 */
Node::WfInPort *
Node::getInPort(string id) {
  map<string, WfInPort*>::iterator p = inports.find(id);
  if (p != inports.end())
    return ((WfInPort*)(p->second));
  else
    return NULL;
}

/**
 * Get the output port reference by id *
 */
Node::WfOutPort *
Node::getOutPort(string id) {
  map<string, WfOutPort*>::iterator p = outports.find(id);
  if (p != outports.end())
    return ((WfOutPort*)(p->second));
  else
    return NULL;
}

/**
 * Get the input/output port reference by id *
 */
Node::WfInOutPort *
Node::getInOutPort(string id) {
  map<string, WfInOutPort*>::iterator p = inoutports.find(id);
  if (p != inoutports.end())
    return ((WfInOutPort*)(p->second));
  else
    return NULL;
}

/**
 * create the diet profile associated to the node *
 */
void 
Node::createProfile() {
  int last = 0;
  // input ports
  for (map<string, WfInPort*>::iterator p = inports.begin();
       p != inports.end();
       ++p) {
    WfInPort * in = (WfInPort*)(p->second);
    if (in->isInput())
      this->set_profile_param(in, 
			      in->type, last, in->value, DIET_VOLATILE);
    else
      this->set_profile_param(in, 
			      in->type, last, in->value, DIET_PERSISTENT);
      
    last ++;
  }
  // output ports
  for (map<string, WfOutPort*>::iterator p = outports.begin();
       p != outports.end();
       ++p) {
    WfOutPort * out = (WfOutPort*)(p->second);
    this->set_profile_param(out, 
			    out->type, last, out->value, DIET_PERSISTENT);
    last ++;
  }
  // inoutput ports
  for (map<string, WfInOutPort*>::iterator p = inoutports.begin();
       p != inoutports.end();
       ++p) {
    WfInOutPort * inout = (WfInOutPort*)(p->second);
    this->set_profile_param(inout, 
			    inout->type, last, inout->value, DIET_PERSISTENT);
    last ++;
  }

}

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
  cout << "\tset_profile_param : type = " << type << 
    ", lastArg = " << lastArg << ", value = " << value << endl;
  if (type == WfCst::DIET_CHAR) {
    cout << "char parameter "<< endl;
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
    cout << "short parameter "<< endl;
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
    cout << "int parameter "<< endl;
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
    cout << "long parameter "<< endl;
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
		      DIET_DOUBLE);
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

  if (type == WfCst::DIET_MATRIX) {
    cout << "the profile contain a matrix" << endl;
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
	cout << "reading the matrix data file" << endl;
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
	cout << "filling the matrix with the data (" << len << ")" << endl;
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
}

/**
 * Set the node as running *
 */
void 
Node::set_as_running() {
  node_running = true;
}

/**
 * test if the node is running *
 */
bool 
Node::isRunning() {
  return node_running;
}

/**
 * test if the execution is done *
 */
bool 
Node::isDone() {
  return task_done;
}

/**
 * called when the node execution is done *
 */
void
Node::done() {
  nodeIsDone(myId.c_str());
  // get the current time and set the real time variable
  struct timeval tv;
  gettimeofday(&tv, NULL);
  this->setRealCompTime(tv);

  cout << "The node terminate, the estimate completion time is " <<
    EstCompTime << " and the real completion time is " <<
    RealCompTime.tv_sec << endl;

  BasicNode * n = NULL;
  cout << "calling the " << next.size() << " next nodes" << endl;
  for (uint ix=0; ix< next.size(); ix++) {
    n = next[ix];
    n->prevDone();
  }
  cout << "calling the " << prec.size() << " previous nodes" << endl;
  n = NULL;
  for (map<string, BasicNode*>::iterator p = prec.begin();
       p != prec.end();
       ++p) {
    n = (Node *)(p->second);
    if (n) {
      n->nextIsDone();
    }
  }
  task_done = true;
  node_running = false;
}

/**
 * called when a next node is done *
 */
void
Node::nextIsDone() {
  nextDone++;
  if (nextDone == next.size()) {
    cout << myId << " Now I can free my profile !" << endl;
    diet_profile_free(profile);
    profile = NULL;
  }
}

/**
 * set the estimated completion time
 */
void
Node::setEstCompTime(const long int est_comp_time) {
  this->EstCompTime = est_comp_time;
}

/**
 * get the estimated completion time
 */
long int
Node::getEstCompTime() {
  return this->EstCompTime;
}

/**
 * set the real completion time
 */
void
Node::setRealCompTime(const struct timeval& real_comp_time) {
  this->RealCompTime = real_comp_time;
}

/** 
 * get the real completion time
 */
struct timeval
Node::getRealCompTime() {
  return this->RealCompTime;
}

/**
 * get the node mark (used for reordering)
 */
bool
Node::getMark() {
  return this->myMark;
}

/**
 * set the node mark 
 */
void 
Node::setMark(bool b) {
  this->myMark = b;
}
