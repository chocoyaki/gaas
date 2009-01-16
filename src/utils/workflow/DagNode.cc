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
 * Revision 1.10  2009/01/16 13:55:36  bisnard
 * changes in dag event handling methods
 *
 * Revision 1.9  2008/12/09 12:14:05  bisnard
 * added reference to FNode to handle pending instanciation
 *
 * Revision 1.8  2008/12/02 14:17:49  bisnard
 * manage multi-dag cancellation when one dag fails
 *
 * Revision 1.7  2008/12/02 10:14:51  bisnard
 * modified nodes links mgmt to handle inter-dags links
 *
 * Revision 1.6  2008/11/08 19:12:40  bdepardo
 * A few warnings removal
 *
 * Revision 1.5  2008/11/07 13:42:05  bdepardo
 * Added two getters in WfPort:
 * - getDataType
 * - getEltDataType.
 * Use them in DagNode::displayResults
 *
 * Revision 1.4  2008/10/30 14:33:56  bisnard
 * updated destructor
 *
 * Revision 1.3  2008/10/22 09:29:00  bisnard
 * replaced uint by standard type
 *
 * Revision 1.2  2008/10/20 07:59:29  bisnard
 * file header creation
 *
 */

#include "debug.hh"
#include "marshalling.hh"
#include "MasterAgent.hh"
#include "SeDImpl.hh"

#include "Dag.hh"
#include "DagNode.hh"
#include "DagNodePort.hh"

#if HAVE_DAGDA
extern "C" {
#include "DIET_Dagda.h"
}
#endif


#define wf_dag_print_matrix_of_real(mat, m, n, rm)        \
  {                                        \
    size_t i, j;                           \
    printf("## WORKFLOW OUTPUT ## %s (%s-major) = \n", #mat,     \
           (rm) ? "row" : "column");       \
    for (i = 0; i < (m); i++) {            \
      printf("## WORKFLOW OUTPUT ## ");    \
      for (j = 0; j < (n); j++) {          \
        if (rm)                            \
	  printf("%3f ", (mat)[j + i*(n)]);\
        else                               \
	  printf("%3f ", (mat)[i + j*(m)]);\
      }                                    \
      printf("\n");                        \
    }                                      \
    printf("\n");                          \
  }

#define wf_dag_print_matrix_of_integer(mat, m, n, rm)        \
  {                                        \
    size_t i, j;                           \
    printf("## WORKFLOW OUTPUT ## %s (%s-major) = \n", #mat,     \
           (rm) ? "row" : "column");       \
    for (i = 0; i < (m); i++) {            \
      printf("## WORKFLOW OUTPUT ## ");    \
      for (j = 0; j < (n); j++) {          \
        if (rm)                            \
	  printf("%d ", (mat)[j + i*(n)]);\
        else                               \
	  printf("%d ", (mat)[i + j*(m)]);\
      }                                    \
      printf("\n");                        \
    }                                      \
    printf("\n");                          \
  }

#define wf_dag_print_matrix_of_longint(mat, m, n, rm)        \
  {                                        \
    size_t i, j;                           \
    printf("%s (%s-major) = \n", #mat,     \
           (rm) ? "row" : "column");       \
    for (i = 0; i < (m); i++) {            \
      for (j = 0; j < (n); j++) {          \
        if (rm)                            \
	  printf("%l ", (mat)[j + i*(n)]);\
        else                               \
	  printf("%l ", (mat)[i + j*(m)]);\
      }                                    \
      printf("\n");                        \
    }                                      \
    printf("\n");                          \
  }


/****************************************************************************/
/*                                                                          */
/*                         class RunnableNode                               */
/*                                                                          */
/****************************************************************************/

MasterAgent_var getMA();
extern diet_error_t
diet_call_common(diet_profile_t* profile,
                 SeD_var& chosenServer,
                 estVector_t estimVect);

RunnableNode::RunnableNode(DagNode * parent)
	:Thread(false) {
  this->myParent = parent;
}

void *
RunnableNode::run() {
  typedef size_t comm_failure_t;
  bool failed = false;
  string traceHeader = "[" + myParent->getId() + "] RunnableNode : ";
  TRACE_TEXT (TRACE_ALL_STEPS, traceHeader << " starting... " << endl);
  // create the node diet profile
//   failed = !(myParent->initProfileExec());
  if (!failed) {
    try {
      try {
        if (!diet_call_common(myParent->profile, myParent->chosenServer, myParent->estimVect)) {
          TRACE_TEXT (TRACE_MAIN_STEPS, traceHeader << " diet call DONE reqID=" <<
              myParent->profile->dietReqID << endl);
          myParent->storeProfileData();
        }
        else {
          TRACE_TEXT (TRACE_MAIN_STEPS, traceHeader << " diet call FAILED" << endl);
          failed = true;
        }
      } catch (CORBA::COMM_FAILURE& e) {
        throw (comm_failure_t)1;
      } catch (CORBA::TRANSIENT& e) {
        throw (comm_failure_t)1;
      }
    } catch (comm_failure_t& e) {
      if (e == 0 || e == 1) {
        WARNING(traceHeader << "Connection problems with DIET occured - node exec cancelled");
        failed = true;
      }
    } catch(CORBA::SystemException& e) {
       WARNING(traceHeader << "Got a CORBA " << e._name() << " exception ("
          << e.NP_minorString() << ")\n") ;
       failed = true;
    }
  }
  if (!failed)  myParent->setAsDone();
  else          myParent->setAsFailed();

  return NULL;
}

/****************************************************************************/
/*                                                                          */
/*                            class DagNode                                 */
/*                                                                          */
/****************************************************************************/

/****************************************************************************/
/*                       Constructors/Destructor                            */
/****************************************************************************/

DagNode::DagNode(Dag *dag, const string& id)
  : Node(id), myDag(dag), myFNode(NULL) {
  this->prevNodesTodoCount = 0;
  this->task_done = false;
  this->SeDDefined = false;
  this->profile = NULL;
  this->node_running = false;
  this->taskExecFailed = false;
  this->myRunnableNode = NULL;
  this->chosenServer = SeD::_nil();
  this->estimVect = NULL;
  this->nextDone = 0;
  this->priority = 0;
  this->myQueue = NULL;
  this->realCompTime = -1;
  this->realStartTime = -1;
  this->estCompTime = -1;
  this->estDelay = 0;
  this->submitIndex = 0;
}

/**
 * Node destructor
 */
DagNode::~DagNode() {
  TRACE_TEXT (TRACE_ALL_STEPS, "~Node() destructor (id: " << myId << ") ..." << endl);
  if (profile != NULL)
    diet_profile_free(profile);
  if (myRunnableNode)
    delete (myRunnableNode);
  if (myQueue)
    myQueue->removeNode(this);

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
	delete [] cx;
    }
  }

  {
    // file name vector
    char * cx = NULL;
    while (fileParams.size() != 0) {
      cx = fileParams.back();
      fileParams.pop_back();
      if (cx)
	delete [] cx;
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
 * Get the node complete id *
 */
string DagNode::getCompleteId() {
  if (this->myDag != NULL)
    return this->myDag->getId() + "-" + this->myId;
  return this->myId;
}

/**
 * get the node Dag reference
 */
Dag *
DagNode::getDag() {
  if (this->myDag != NULL)
    return this->myDag;
  else {
    INTERNAL_ERROR( "ERROR: calling getDag() on a node not linked to a dag" << endl, 1);
  }
}

/**
 * set the problem name
 */
void
DagNode::setPbName(const string& pbName) {
  this->myPb = pbName;
}

/**
 * get the problem name
 */
const string&
DagNode::getPbName() {
  return this->myPb;
}

/**
 * set the functional node for which this node is an instance
 */
void
DagNode::setFNode(FProcNode * fNode) {
  this->myFNode = fNode;
}

/**
 * get the functional node
 */
FProcNode *
DagNode::getFNode() {
  return this->myFNode;
}

/******************************/
/* Base methods override      */
/******************************/

/**
 * (public) Add a new predecessor
 * (may check some constraints before adding the predecessor effectively)
 */
void
DagNode::addNodePredecessor(Node * node, const string& fullNodeId) {
  // check if predecessor is not already done
  DagNode *predNode = dynamic_cast<DagNode*>(node);
  if (!predNode || !predNode->isDone())
      addPrevId(fullNodeId);
}

/**
 * (private)
 * Set a new previous node *
 */
void
DagNode::setPrev(int index, Node * node) {
  DagNode * prevDagNode = dynamic_cast<DagNode*>(node);
  Node::setPrev(index,node);
  prevNodesTodoCount++;
}

/**
 * create and add a new port to the node *
 */
WfPort *
DagNode::newPort(string portId, unsigned int ind,
                 WfPort::WfPortType portType, WfCst::WfDataType dataType,
                 unsigned int depth) {
  WfPort * p = NULL;
  switch (portType) {
    case WfPort::PORT_IN:
      p = new DagNodeInPort(this, portId, dataType, depth, ind);
      break;
    case WfPort::PORT_INOUT:
      p = new DagNodeInOutPort(this, portId, dataType, depth, ind);
      break;
    case WfPort::PORT_OUT:
      p = new DagNodeOutPort(this, portId, dataType, depth, ind);
      break;
  }
  this->ports[portId] = p;
  return p;
}

string
DagNode::toXML() {
  string xml = "<node id=\""+ myId +"\" ";
  xml += "path=\""+ myPb +"\">\n";
  for (map<string, WfPort*>::iterator p = ports.begin();
       p != ports.end();
       ++p) {
    DagNodePort * port = dynamic_cast<DagNodePort*>((WfPort*)(p->second));
    xml += port->toXML();
  }
  xml += "</node>\n";
  return xml;
}

/******************************/
/* DIET Profile Mgmt          */
/******************************/

/**
 * return the node profile
 */
diet_profile_t *
DagNode::getProfile() {
  return this->profile;
}

/**
 * return the reqID of the node
 */
diet_reqID_t
DagNode::getReqID(){
  if(this->isDone())
    return this->getProfile()->dietReqID;
  return -1;
}

/**
 * set the submit index
 */
void
DagNode::setSubmitIndex(int idx) {
  this->submitIndex = idx;
}

/**
 * get the submit index
 */
int
DagNode::getSubmitIndex() {
  return this->submitIndex;
}


/**
 * Creates the base profile without the parameters (COMMON)
 */
void
DagNode::createProfile() {
  if (this->profile) {
    WARNING("DagNode::createProfile : profile already defined!!" << endl);
  }
  int nbIn = 0, nbOut = 0, nbInOut = 0;
  for (map<string, WfPort*>::iterator p = ports.begin();
       p != ports.end();
       ++p) {
    WfPort * port = (WfPort *) p->second;
    switch (port->getPortType()) {
      case WfPort::PORT_IN:
        ++nbIn;
        break;
      case WfPort::PORT_INOUT:
        ++nbInOut;
        break;
      case WfPort::PORT_OUT:
        ++nbOut;
        break;
    }
  } // end for all ports
  int last_in    = nbIn - 1;
  int last_inout = last_in + nbInOut;
  int last_out   = last_inout + nbOut;
  TRACE_TEXT(TRACE_ALL_STEPS,"Creating DIET profile: pb=" << myPb
                              << " / last_in=" << last_in
                              << " / last_inout=" << last_inout
                              << " / last_out=" << last_out << endl);
  this->profile =  diet_profile_alloc((char*)(myPb.c_str()),
                                       last_in, last_inout, last_out);

}

/**
 * Creates the profile before submission of the node (MADAG SIDE)
 */
bool
DagNode::initProfileSubmit() {
  TRACE_TEXT(TRACE_ALL_STEPS,"Creating profile for Submit" << endl);
  createProfile();
  for (map<string, WfPort*>::iterator p = ports.begin();
       p != ports.end();
       ++p) {
    DagNodePort * dagPort = dynamic_cast<DagNodePort*>(p->second);
    if (!dagPort->initProfileSubmit()) {
      ERROR ("Profile init failed" << endl, 0);
    }
  }
  return true;
}

/**
 * Creates the profile before execution of the node (CLIENT SIDE)
 */
bool
DagNode::initProfileExec() {
  TRACE_TEXT(TRACE_ALL_STEPS,"Creating profile for Execution" << endl);
  createProfile();
  if (this->SeDDefined) {
    this->profile->dietReqID = this->dietReqID;
    TRACE_TEXT(TRACE_ALL_STEPS,"Setting reqID in profile to #"
      << this->dietReqID << endl);
  }
  for (map<string, WfPort*>::iterator p = ports.begin();
       p != ports.end();
       ++p) {
    DagNodePort * dagPort = dynamic_cast<DagNodePort*>(p->second);
    if (!dagPort->initProfileExec()) {
      ERROR ("Profile init failed" << endl, 0);
    }
  }
  TRACE_TEXT(TRACE_ALL_STEPS,"Profile for Execution done (" << myId << ")" << endl);
  return true;
}

/**
 * Store the persistent data of the node profile *
 */
void
DagNode::storeProfileData() {
  // store the data IDs for each port
  for (map<string, WfPort*>::iterator p = ports.begin();
       p != ports.end();
       ++p) {
    if (((WfPort *)p->second)->getPortType() == WfPort::PORT_OUT) {
      DagNodeOutPort * dagPort = dynamic_cast<DagNodeOutPort*>(p->second);
      dagPort->storeProfileData();
    }
  }
}

/**
 * Free the profile and the persistent data of the node
 */
void
DagNode::freeProfileAndData() {
  TRACE_TEXT (TRACE_ALL_STEPS,
              myId << " Free profile and release persistent data" << endl);
  // Free persistent data
  if (profile) {
    for (map<string, WfPort*>::iterator p = ports.begin();
         p != ports.end();
         ++p) {
      if (((WfPort *)p->second)->getPortType() == WfPort::PORT_OUT) {
        DagNodeOutPort * dagPort = dynamic_cast<DagNodeOutPort*>(p->second);
        dagPort->freeProfileData();
      }
    }
    diet_profile_free(profile);
    profile = NULL;
  }
}

/**
 * Allocate a new char *
 */
char *
DagNode::newChar(const string value) {
//   TRACE_TEXT (TRACE_ALL_STEPS, "new char ; value | " << value <<  " |" << endl);
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
}

/**
 * Allocate a new short *
 */
short *
DagNode::newShort(const string value) {
//   TRACE_TEXT (TRACE_ALL_STEPS, "new short ; value | " << value <<  " |" << endl);
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
}

/**
 * Allocate a new int  *
 */
int *
DagNode::newInt(const string value) {
//   TRACE_TEXT (TRACE_ALL_STEPS,
// 	      "new int ; value | " << value <<  " |" << endl);
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
}

/**
 * Allocate a new long *
 */
long *
DagNode::newLong(const string value) {
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
}

/**
 * Allocate a new string *
 */
char *
DagNode::newString (const string value) {
  char * str = new char[value.size()+1];
  strcpy(str, value.c_str());

//   TRACE_TEXT (TRACE_ALL_STEPS,
// 	      "----> new string; value = " << value << ", " << str << endl);
  stringParams.push_back(str);
  return str;
}

/**
 * Allocate a new file *
 */
char *
DagNode::newFile (const string value) {
  char * str = new char[value.size()+1];
  strcpy(str, value.c_str());

//   TRACE_TEXT (TRACE_ALL_STEPS,
// 	      "----> new file; value = " << value << ", " << str << endl);
  fileParams.push_back(str);
  return str;
}

/**
 * Allocate a new float  *
 */
float *
DagNode::newFloat  (const string value) {
  float * fx = new float;
  *fx = (float)atof(value.c_str());
  floatParams.push_back(fx);
  return fx;
}

/**
 * Allocate a new double  *
 */
double *
DagNode::newDouble (const string value) {
  double * dx = new double;
  *dx = atof(value.c_str());
  doubleParams.push_back(dx);
  return dx;
}

/**
 * Display the results of the node
 */
void
DagNode::displayResults() {
  if (hasFailed()) return;
  for (map<string, WfPort*>::iterator p = ports.begin();
       p != ports.end();
       ++p) {
    if (((WfPort *)p->second)->getPortType() == WfPort::PORT_OUT) {
      DagNodeOutPort * outp = dynamic_cast<DagNodeOutPort *>(p->second);
      TRACE_TEXT (TRACE_ALL_STEPS,"checking port " << outp->getId() << " of node "
           << outp->getParent()->getId() << endl);
      if (!outp->isConnected()) {
        short dataType = outp->getDataType();

	if (dataType == WfCst::TYPE_FILE) {
	  size_t size;
	  char * path;
	  diet_file_get(diet_parameter(outp->profile(),outp->getIndex()),
			NULL, &size, &path);
	  cout << "## DAG OUTPUT ## " <<
		      outp->getId() << " type = DIET_FILE, " <<
		      "File name = " << path << ", " <<
		      "File size = " << size  << endl;
	}
	else if (dataType == WfCst::TYPE_DOUBLE) {
          double * value = NULL;
          diet_scalar_get(diet_parameter(outp->profile(),outp->getIndex()),
                          &value, NULL);
          cout << "## DAG OUTPUT ## " <<
                          outp->getId() << " = " << *value << endl;
        }
        else if (dataType == WfCst::TYPE_FLOAT) {
          float * value = NULL;
          diet_scalar_get(diet_parameter(outp->profile(),outp->getIndex()),
                          &value, NULL);
          cout << "## DAG OUTPUT ## " <<
                          outp->getId() << " = " << *value << endl;
        }
        else if ( (dataType == WfCst::TYPE_CHAR) ||
		  (dataType == WfCst::TYPE_SHORT) ||
		  (dataType == WfCst::TYPE_INT) ||
		  (dataType == WfCst::TYPE_LONGINT)) {
          long * value = NULL;
          diet_scalar_get(diet_parameter(outp->profile(),outp->getIndex()),
                          &value, NULL);
          cout << "## DAG OUTPUT ## " <<
		      outp->getId() << " = " << *value << endl;
	}
	else if (dataType == WfCst::TYPE_STRING) {
	  char * value;
	  diet_string_get(diet_parameter(outp->profile(),outp->getIndex()),
			  &value, NULL);
	  cout << "## DAG OUTPUT ## " <<
		      outp->getId() << " = " << value << endl;
	}
        else if (dataType == WfCst::TYPE_CONTAINER) {
          short baseType = outp->getEltDataType();
          cout << "## DAG OUTPUT ## " <<
		      outp->getId() << " = LIST (" << endl;
          // Retrieve container elements
          const char *contID = (*diet_parameter(outp->profile(),outp->getIndex())).desc.id;
          dagda_get_container(contID);
          diet_container_t content;
          dagda_get_container_elements(contID, &content);
          for (int ix=0; ix<content.size; ++ix) {
            if (baseType == WfCst::TYPE_LONGINT) {
              long *value;
              dagda_get_scalar(content.elt_ids[ix],&value,NULL);
              cout <<  "element [" << ix << "] = " << *value << endl;
            }
          }
          TRACE_TEXT (TRACE_ALL_STEPS,")" << endl);
        } else if (dataType == WfCst::TYPE_MATRIX) {
	  size_t nb_rows, nb_cols;
	  diet_matrix_order_t order;
	  short baseType = outp->getEltDataType();

	  if (baseType == WfCst::TYPE_DOUBLE) {
	    double * value;
	    diet_matrix_get(diet_parameter(outp->profile(), outp->getIndex()),
			    &value, NULL,
			    &nb_rows, &nb_cols, &order);
	    wf_dag_print_matrix_of_real(value, nb_rows, nb_cols, order);
	  }
	  else if (baseType == WfCst::TYPE_FLOAT) {
	    float * value;
	    diet_matrix_get(diet_parameter(outp->profile(), outp->getIndex()),
			    &value, NULL,
			    &nb_rows, &nb_cols, &order);
	    wf_dag_print_matrix_of_real(value, nb_rows, nb_cols, order);
	  }
	  else if (baseType == WfCst::TYPE_CHAR) {
	    char * value;
	    diet_matrix_get(diet_parameter(outp->profile(), outp->getIndex()),
			    &value, NULL,
			    &nb_rows, &nb_cols, &order);
	    wf_dag_print_matrix_of_integer(value, nb_rows, nb_cols, order);
	  }
	  else if (baseType == WfCst::TYPE_SHORT) {
	    short * value;
	    diet_matrix_get(diet_parameter(outp->profile(), outp->getIndex()),
			    &value, NULL,
			    &nb_rows, &nb_cols, &order);
	    wf_dag_print_matrix_of_integer(value, nb_rows, nb_cols, order);
	  }
	  else if (baseType == WfCst::TYPE_INT) {
	    int * value;
	    diet_matrix_get(diet_parameter(outp->profile(), outp->getIndex()),
			    &value, NULL,
			    &nb_rows, &nb_cols, &order);
	    wf_dag_print_matrix_of_integer(value, nb_rows, nb_cols, order);
	  }
	  else if (baseType == WfCst::TYPE_LONGINT) {
	    long * value;
	    diet_matrix_get(diet_parameter(outp->profile(), outp->getIndex()),
			    &value, NULL,
			    &nb_rows, &nb_cols, &order);
	    wf_dag_print_matrix_of_longint(value, nb_rows, nb_cols, order);
	  }
	} // end if TYPE_MATRIX
      } // if isResult
    } // if out port
  } // end for ports
}

/**
 * set the SeD reference to the node *
 */
void
DagNode::setSeD(const SeD_var& sed, const unsigned long reqID, corba_estimation_t& ev) {
  this->SeDDefined = true;
  this->chosenServer = sed;
  this->dietReqID = reqID;
  this->estimVect = &ev;
}

/**
 * get the SeD
 */
SeD_var&
DagNode::getSeD() {
  return chosenServer;
}

/******************************/
/* Scheduling                 */
/******************************/

/**
 * set the node priority *
 */
void
DagNode::setPriority(double priority) {
  this->priority = priority;
}

/**
 * get the node priority *
 */
double
DagNode::getPriority() {
  return this->priority;
}

/**
 * Set the NodeQueue ref when the node is inserted into it *
 * (This is used to notify the NodeQueue when the node state changes *
 */
void DagNode::setNodeQueue(NodeQueue * nodeQ) {
  this->myQueue = nodeQ;
}

/**
 * Remove node from the NodeQueue it belongs to (if applicable)
 * This is used in case of dag cancellation
 */
void
DagNode::removeFromNodeQueue() {
  if (myQueue) {
    myQueue->removeNode(this);
  }
}

/**
  * set the ref to the last nodeQueue occupied by the node
  */
void
DagNode::setLastQueue(NodeQueue * queue) {
  this->lastQueue = queue;
}

/**
 * get the ref to the last nodeQueue occupied by the node
 */
NodeQueue *
DagNode::getLastQueue() {
  return this->lastQueue;
}

/******************************/
/* Timestamps (MaDag)         */
/******************************/

/**
 * set the estimated duration
 */
void
DagNode::setEstDuration(double time) {
  this->estDuration = time;
}

/**
 * get the estimated duration
 */
double
DagNode::getEstDuration() {
  return this->estDuration;
}

/**
 * set the estimated completion time
 */
void
DagNode::setEstCompTime(double time) {
  this->estCompTime = time;
}

/**
 * get the estimated completion time
 */
double
DagNode::getEstCompTime() {
  return this->estCompTime;
}

/**
 * set the estimated delay
 * (can be increased or decreased)
 */
void
DagNode::setEstDelay(double delay) {
  string traceHeader = "[" + getId() + "] setEstDelay() : ";
  // if this is an exit node then eventually updates the dag delay
  if (this->isAnExit()
      && (delay > this->estDelay)
      && (this->getDag() != NULL)) {
    this->getDag()->setEstDelay(delay);
  }
  this->estDelay = delay;
  TRACE_TEXT (TRACE_ALL_STEPS, traceHeader << "delay = " << delay << endl);
}

/**
 * set the real start time
 */
void
DagNode::setRealStartTime(double time) {
  this->realStartTime = time;
}

/**
 * get the real start time
 */
double
DagNode::getRealStartTime() {
  return this->realStartTime;
}

/**
 * get the estimated delay
 */
double
DagNode::getEstDelay() {
  return this->estDelay;
}

/**
 * set the real completion time
 */
void
DagNode::setRealCompTime(double time) {
  this->realCompTime = time;
}

/**
 * get the real completion time
 */
double
DagNode::getRealCompTime() {
  return this->realCompTime;
}

/**
 * get the real delay (positive) or 0 if no delay
 * or -1 if needed timestamps not set
 */
double
DagNode::getRealDelay() {
  if ((this->realCompTime != -1) && (this->estCompTime != -1)) {
    double delay = this->realCompTime - this->estCompTime;
    if (delay > 0) return delay;
    else return 0;
  } else return -1;
}

/**********************************/
/* Execution status               */
/**********************************/

/**
 * test if the node is ready for execution (Madag side)
 * (check the counter of dependencies)
 */
bool
DagNode::isReady() {
  return (prevNodesTodoCount == 0);
}

/**
 * set the node as ready for execution (Madag side)
 * (Notifies the nodequeue if available)
 */
void
DagNode::setAsReady() {
  string traceHeader = "[" + getId() + "] setAsReady() : ";
  if (this->myQueue != NULL) {
    TRACE_TEXT (TRACE_ALL_STEPS,traceHeader << "notify its queue" << endl);
    this->myQueue->notifyStateChange(this);
  }
}

/**
 * start the node execution (CLIENT SIDE ONLY) *
 * Note: the argument join are always set to default currently
 */
void DagNode::start(bool join) {
  string traceHeader = "[" + getId() + "] start() : ";
  node_running = true;
  this->myRunnableNode = new RunnableNode(this);
  TRACE_TEXT (TRACE_ALL_STEPS, traceHeader << "tries to launch a RunnableNode "<< endl);
  this->myRunnableNode->start();
  TRACE_TEXT (TRACE_ALL_STEPS, traceHeader << "launched its RunnableNode" << endl);
  if (join)
    this->myRunnableNode->join();
  TRACE_TEXT (TRACE_ALL_STEPS, traceHeader << "joined its RunnableNode" << endl);
}

/**
 * test if the node is running (Madag side)
 */
bool
DagNode::isRunning() {
  return node_running;
}

/**
 * Set node status as running (Madag side)
 */
void
DagNode::setAsRunning() {
  this->node_running = true;
}

/**
 * test if the execution is done (MaDag side)
 * (still used by HEFTscheduler to rank nodes)
 */
bool
DagNode::isDone() {
  return task_done;
}

/**
 * Set the node status as done (MaDag & client side)
 */
void
DagNode::setAsDone(DagScheduler* scheduler) {
  // update node scheduling info
  task_done = true;
  node_running = false;
  // the following applies only to MaDag
  if (scheduler) {
    setRealCompTime(scheduler->getRelCurrTime());
    // notify scheduler that node is done (depending on the scheduler, this may trigger
    //  a recursive update of the realCompTime in other nodes)
    scheduler->handlerNodeDone(this);
  }
  // notify the dag
  this->getDag()->setNodeDone(this, scheduler);
}

/**
 * Called when a previous node execution is done (MaDag side)
 */
void DagNode::prevNodeHasDone() {
  prevNodesTodoCount--;
  if (this->isReady()) {
    this->setAsReady();
  }
}

/**
 * called when the node execution failed (MaDag & client side) *
 */
void
DagNode::setAsFailed(DagScheduler* scheduler) {
  taskExecFailed =  true;
  node_running = false;
  this->getDag()->setNodeFailure(this->getId(), scheduler);
}

/**
 * test if the execution failed (client side)
 */
bool
DagNode::hasFailed() {
  return taskExecFailed;
}

