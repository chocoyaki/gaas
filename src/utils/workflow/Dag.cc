/****************************************************************************/
/* Dag description                                                          */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/* - Benjamin ISNARD (Benjamin.Isnard@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.15  2008/09/19 13:11:07  bisnard
 * - added support for containers split/merge in workflows
 * - added support for multiple port references
 * - profile for node execution initialized by port (instead of node)
 * - ports linking managed by ports (instead of dag)
 *
 * Revision 1.14  2008/07/17 12:19:18  bisnard
 * Added dag cancellation method
 *
 * Revision 1.13  2008/07/11 07:56:01  bisnard
 * provide list of failed nodes in case of cancelled dag
 *
 * Revision 1.12  2008/07/08 11:15:58  bisnard
 * Correct dag/node destruction with nodequeues
 *
 * Revision 1.11  2008/06/25 10:07:12  bisnard
 * - removed debug messages
 * - Node index in wf_response stored in Node class (new attribute submitIndex)
 *
 * Revision 1.10  2008/06/19 10:17:13  bisnard
 * new attribute to store DAG start time
 *
 * Revision 1.9  2008/06/18 14:58:57  bisnard
 * bug in updateDelayRec
 *
 * Revision 1.8  2008/06/02 08:35:39  bisnard
 * Avoid MaDag crash in case of client-SeD comm failure
 *
 * Revision 1.7  2008/06/01 15:50:59  rbolze
 * less verbose
 *
 * Revision 1.6  2008/06/01 09:18:43  rbolze
 * remove myreqID attribute from the RunnableNode
 * add getReqID() method which return the reqID stored in the diet_profile_t
 * add 2 methods: showDietReqID and getAllDietReqID
 * this is use to send feedback through logservice
 *
 * Revision 1.5  2008/05/16 12:32:10  bisnard
 * API function to retrieve all workflow results
 *
 * Revision 1.4  2008/04/28 12:14:59  bisnard
 * new NodeQueue implementation for FOFT
 * nodes sorting done by Dag instead of scheduler
 * method to propagate delay at execution (FOFT)
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
 * Revision 1.6  2006/11/08 15:18:58  aamar
 * Complete get_all_results method to handle matrix and file types.
 *
 * Revision 1.5  2006/11/06 11:56:42  aamar
 * Workflow support:
 *    - Adding get_file_output and get_matrix_output (to get workflow file
 *      and matrix results)
 *
 * Revision 1.4  2006/10/26 13:57:53  aamar
 * Replace cout/cerr by debug macro TRACE_XXX
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
 * Revision 1.1  2006/04/14 13:46:51  aamar
 * Direct acyclic graph class (source).
 *
 ****************************************************************************/

#include "debug.hh"

#include "Dag.hh"
#include "DagWfParser.hh"

using namespace std;

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


Dag::Dag() {
  this->myId    = "";
  this->tmpDag  = false;
  this->estDelay = 0;
  this->cancelled = false;
}

/**
 * DAG destructor: will delete all the nodes of the dag (if not a temp dag)
 */
Dag::~Dag() {
  TRACE_TEXT (TRACE_ALL_STEPS,"~Dag() destructor ..." <<  endl);
  if (! this->tmpDag) {
    Node * node = NULL;
    for (map<string, Node * >::iterator p = nodes.begin( );
	 p != nodes.end( );
	 ++p ) {
      node = (Node*) p->second;
      nodes.erase(p);
      delete(node);
    }
    nodes.clear();
  }
}

/**
 * set the dag id
 */
void
Dag::setId(const string id) {
  this->myId = id;
}

/**
 * get the dag id
 */
string
Dag::getId() {
  return this->myId;
}

/**
 * add a node to the dag
 */
void
Dag::addNode (string nodeName, Node * node) {
  nodes[nodeName] = node;
  TRACE_TEXT (TRACE_ALL_STEPS,
	      "\t" << "The new size of the dag is " << nodes.size() <<  endl);
}

/**
 * Get the node with given identifier (only node id, not the complete id)
 */
Node *
Dag::getNode(std::string node_id) {
  map<string, Node*>::iterator p = this->nodes.find(node_id);
  if ( p != this->nodes.end())
    return p->second;
  else
    return NULL;
}

/**
 * check the precedence between node *
 * this function checks the precedence between node; it doesn't *
 * link the WfPorts but it creates the list of predecessors of each node *
 */
bool
Dag::checkPrec() {
  bool result = true;
  Node * node = NULL;
  unsigned int n = 0;
  // Add to node predecessors those already registered with their ID only
  // ie nodes that were declared predecessors in the XML (<prec> tag)
  for (map<string, Node * >::iterator p = nodes.begin( ); p != nodes.end( );
       ++p ) {
    node = (Node*) p->second;
    n = node->prevNb();
    // loop over all IDs stored in prec_ids vector
    for (uint ix = 0; ix < n; ix++) {
      map<string, Node * >::iterator q = nodes.find(node->getPrecId(ix));
      if (q != nodes.end()) {
        Node * precNode = (Node *) q->second;
	node->addPrec(node->getPrecId(ix), precNode);
        TRACE_FUNCTION (TRACE_ALL_STEPS," Loop1: Add prec " << precNode->getId()
            << " to " << node->getId() << endl);
      }
      else
	return false;
    }
  }

  // Add to node predecessors the nodes linked via an input port
  for (map<string, Node * >::iterator p = nodes.begin( ); p != nodes.end( );
       ++p ) {
    node = (Node*) p->second;

    for (map<string, WfInPort*>::iterator p = node->inports.begin();
	 p != node->inports.end();
	 ++p) {
      WfInPort * in = (WfInPort*)(p->second);
      in->setNodePredecessors(this);
    } // end for in

    for (map<string, WfInOutPort*>::iterator p = node->inoutports.begin();
	 p != node->inoutports.end();
	 ++p) {
      WfInOutPort * inout = (WfInOutPort*)(p->second);
      inout->setNodePredecessors(this);
    } // end for inout

  }
  return result;
} // end checkPrec

/**
 * return a string representation of the Dag *
 */
string
Dag::toString() {
  Node * node = NULL;
  string str;
  str  = "##############################################################\n";
  str += "# Dag representation :\n";
  str += "####################\n";

  for (map<string, Node * >::iterator p = nodes.begin( ); p != nodes.end( );
       ++p ) {
    node = (Node*) p->second;
    str += node->toString();
  }
  str += "##############################################################";
  return str;
}

/**
 * return the size of the Dag (the nodes number)
 */
// The four next methods are only for testing
unsigned int
Dag::size() {
  return nodes.size();
}

/**
 * return an iterator on the first node *
 * (according to the map and not to the dag structure) *
 */
map <string, Node *>::iterator
Dag::begin() {
  return nodes.begin();
}

/**
 * return an iterator on the last node *
 * (according to the map and not to the dag structure) *
 */
map <string, Node *>::iterator
Dag::end() {
 return nodes.end();
}


/**
 * link all ports of the dag *
 */
void
Dag::linkAllPorts() {
  // check every node and link it
  for (map<string, Node*>::iterator p = nodes.begin();
       p != nodes.end();
       ++p) {
    Node * n = (Node*)(p->second);
    linkNodePorts(n);
  }
}

/**
 * link the port of the node @n
 * TODO currently does not manage <sink> links (only <source>)
 */
void
Dag::linkNodePorts(Node * n) {
   TRACE_TEXT (TRACE_ALL_STEPS,
 	      "linkNodePorts : processing node " << n->getId() << endl);
  // INPUT ==== ref to ===> OUTPUT
  for (map<string, WfInPort*>::iterator p = n->inports.begin();
       p != n->inports.end();
       ++p) {
    WfInPort * in = (WfInPort*)(p->second);
    in->setPortDataLinks(this);
  } // end for (inports)

  // OUTPUT ==== ref to ===> INPUT
  for (map<string, WfOutPort*>::iterator p = n->outports.begin();
       p != n->outports.end();
       ++p) {
    // manage linking using <sink> links
  }

  // INOUT ==== ref to ===> OUTPUT
  for (map<string, WfInOutPort*>::iterator p = n->inoutports.begin();
       p != n->inoutports.end();
       ++p) {
    WfInOutPort * inout = (WfInOutPort*)(p->second);
    inout->setPortDataLinks(this);
  }
}


/**
 * printDietReqID the dag execution is completed
 *
 * this methods loops through all the nodes of the dag and getReqID of each node
 * @return reqID[]
 */
void
Dag::showDietReqID() {
  Node * dagNode = NULL;
  cout << "@@@@ BEGIN Dag::" <<__FUNCTION__  << "()" << endl;
  cout << "dag_id =" << this->myId << endl;
  for (map<string, Node *>::iterator p = nodes.begin();
       p != nodes.end();
       ++p) {
    dagNode = (Node*)p->second;
    //if ((dagNode) && !(dagNode->isDone()))
    cout << " dagNode->getPb() = "  << dagNode->getPb() <<endl;
    cout << " dagNode->getProfile()->dietReqID =" << dagNode->getProfile()->dietReqID << endl;
  }
  cout << "@@@@ END Dag::" <<__FUNCTION__  << "()" << endl;
}
/**
 * printDietReqID the dag execution is completed
 *
 * this methods loops through all the nodes of the dag and getReqID of each node
 * @return diet_reqID_t[]
 */
vector<diet_reqID_t>
Dag::getAllDietReqID() {
	Node * dagNode = NULL;
	vector<diet_reqID_t> request_ids;
	//cout << "dag_id =" << this->myId << endl;
	for (map<string, Node *>::iterator p = nodes.begin();
		    p != nodes.end();
		    ++p) {
			    dagNode = (Node*)p->second;
			    request_ids.push_back(dagNode->getReqID());
			    //cout << "reqID ="<< dagNode->getReqID() << endl;
		    }
	return request_ids;
}

bool
Dag::isDone() {
  Node * dagNode = NULL;
  bool res = true;
  this->myLock.lock();  /** LOCK */
  for (map<string, Node *>::iterator p = nodes.begin();
       p != nodes.end();
       ++p) {
    dagNode = (Node*)p->second;
    if ((dagNode) && !(dagNode->isDone())) {
      res = false;
      break;
    }
  }
  this->myLock.unlock(); /** UNLOCK */
  return res;
}

/**
 * check if the dag execution is ongoing
 * (checks all nodes status and returns true if at least one node is running)
 */
bool
Dag::isRunning() {
  bool res = false;
  Node * dagNode = NULL;
  this->myLock.lock();  /** LOCK */
  for (map<string, Node *>::iterator p = nodes.begin();
       p != nodes.end();
       ++p) {
    dagNode = (Node*)p->second;
    if ((dagNode) && (dagNode->isRunning())) {
      res = true;
    }
  }
  this->myLock.unlock(); /** UNLOCK */
  return res;
}

/**
 * check if the dag execution is cancelled *
 */
bool
Dag::isCancelled() {
  return cancelled;
}

/**
 * Get a scalar result of the workflow *
 *
 * this methods loops through all dag nodes to find the output port identified
 * by id
 * @param id identifier of the node
 * @param value pointer to pointer to the value of the output port
 *
 * @todo unify the different methods used to retrieve an output for the different types
 */
int
Dag::get_scalar_output(const char * id,
		    void** value) {
  TRACE_TEXT (TRACE_ALL_STEPS,
	      "\t" << "get_scalar_output : searching for " << id << endl);
  Node * n = NULL;
  for (map<string, Node *>::iterator p = nodes.begin();
       p != nodes.end();
       ++p) {
    n = (Node *)(p->second);
    if (n != NULL) {
      map<string, WfOutPort*>::iterator outp_iter =
	n->outports.find(id);
      if (outp_iter != n->outports.end()) {
	TRACE_TEXT (TRACE_ALL_STEPS,
		    "\t" << "found an output port with the id " << id << endl);
	WfOutPort * outp = (WfOutPort *)(outp_iter->second);
	if (outp->isResult()) {
	    TRACE_TEXT (TRACE_ALL_STEPS,
			"\t" << "return the result" << endl);
	  //       diet_scalar_get(diet_parameter(profile,2), &pl3, NULL);
	  return diet_scalar_get(diet_parameter(outp->profile(),outp->getIndex()),
				 value, NULL);
	}
      }
    }
  }

  return 1;
}

/**
 * get a string result of the workflow *
 */
int
Dag::get_string_output(const char * id,
		       char** value) {
  string port_id(id);
  string node_id = port_id.substr(0, port_id.find("#"));
  map<string, Node *>::iterator p = nodes.find(node_id);
  if (p!= nodes.end()) {
    Node * n = (Node *)(p->second);
    map<string, WfOutPort*>::iterator outp_iter =
      n->outports.find(port_id);
    if (outp_iter != n->outports.end()) {
      TRACE_TEXT (TRACE_ALL_STEPS,
		  "######## found an output port with the id " << id << endl);
      WfOutPort * outp = (WfOutPort *)(outp_iter->second);
      if (outp->isResult()) {
	TRACE_TEXT (TRACE_ALL_STEPS,
		    "######## return the result" << endl);
	return diet_string_get(diet_parameter(outp->profile(),outp->getIndex()),
			       value, NULL);
      }
    }
  }
  return 1;
}

/**
 * Get a file result of the workflow
 *
 */
int
Dag::get_file_output (const char * id,
		      size_t* size, char** path) {
  string port_id(id);
  string node_id = port_id.substr(0, port_id.find("#"));
  map<string, Node *>::iterator p = nodes.find(node_id);
  if (p!= nodes.end()) {
    Node * n = (Node *)(p->second);
    map<string, WfOutPort*>::iterator outp_iter =
      n->outports.find(port_id);
    if (outp_iter != n->outports.end()) {
      TRACE_TEXT (TRACE_ALL_STEPS,
		  "######## found an output port with the id " << id << endl);
      WfOutPort * outp = (WfOutPort *)(outp_iter->second);
      if (outp->isResult()) {
	TRACE_TEXT (TRACE_ALL_STEPS,
		    "######## return the result" << endl);
	return diet_file_get(diet_parameter(outp->profile(), outp->getIndex()),
					    NULL, size, path);
      }
    }
  }
  return 1;
} // end get_file_output

/**
 * Get a matrix result of the workflow
 */
int
Dag::get_matrix_output (const char * id, void** value,
			size_t* nb_rows, size_t *nb_cols,
			diet_matrix_order_t* order) {
  string port_id(id);
  string node_id = port_id.substr(0, port_id.find("#"));
  map<string, Node *>::iterator p = nodes.find(node_id);
  if (p!= nodes.end()) {
    Node * n = (Node *)(p->second);
    map<string, WfOutPort*>::iterator outp_iter =
      n->outports.find(port_id);
    if (outp_iter != n->outports.end()) {
      TRACE_TEXT (TRACE_ALL_STEPS,
		  "######## found an output port with the id " << id << endl);
      WfOutPort * outp = (WfOutPort *)(outp_iter->second);
      if (outp->isResult()) {
	TRACE_TEXT (TRACE_ALL_STEPS,
		    "######## return the result" << endl);
	/**
int
_matrix_get(diet_arg_t* arg, void** value, diet_persistence_mode_t* mode,
	    size_t* nb_rows, size_t *nb_cols, diet_matrix_order_t* order);
	*/
	return diet_matrix_get(diet_parameter(outp->profile(), outp->getIndex()),
			       value, NULL,
			       nb_rows, nb_cols, order);
      }
    }
  }
  return 1;
} // end get_matrix_output

/**
 * Move a node to the trash vector (called when rescheduling)
 */
void
Dag::moveToTrash(Node * n) {
  trash.push_back(n);
  if (nodes.find(n->getId())  == nodes.end()) {
    // The node complete id begin with dag id while the nodes map uses
    // the simple id of the nodes
    string id = n->getId();
    if ( this->myId == (id.substr(0, id.find("-"))) ) {
      TRACE_TEXT (TRACE_ALL_STEPS,
		  " ERASING THE NODE " << id << endl);
      nodes.erase(id.substr(id.find("-")+1));
    }
    else {
      // a priori not used
      TRACE_TEXT (TRACE_ALL_STEPS,
		  " ERASING THE NODE " << n->getId() << endl);
      nodes.erase(n->getId());
    }
  }
  else {
    TRACE_TEXT (TRACE_ALL_STEPS,
		" The node " << n->getId() << " was not found!!!" << endl);
  }
}

/**
 * Get the nodes sorted according to their priority using insertion sort
 * (output vector must be deleted)
 */
std::vector<Node*>&
Dag::getNodesByPriority() {
  std::vector<Node*> * sorted_list = new std::vector<Node*>;
  Node * n1 = NULL;
  TRACE_TEXT (TRACE_ALL_STEPS, "Sorting dag nodes by priority" << endl);
  for (std::map <std::string, Node *>::iterator p = this->begin();
       p != this->end();
       p++) {
    n1 = (Node*)(p->second);
    // found where insert the node
    std::vector<Node*>::iterator p = sorted_list->begin();
    bool b = false;
    Node * n2 = NULL;
    while ((p != sorted_list->end()) && (!b)) {
      n2 = *p;
      if (n2->getPriority() < n1->getPriority())
        b = true;
      else
        p++;
    }
    sorted_list->insert(p, n1);
  }
  return *sorted_list;
}

/**
 * get all the results
 */
int
Dag::get_all_results() {
  Node * n = NULL;
  for (map<string, Node *>::iterator p = nodes.begin();
       p != nodes.end();
       ++p) {
    n = (Node *)(p->second);
    if ((n != NULL) && (n->isAnExit())) {
      for (map<string, WfOutPort*>::iterator outp_iter = n->outports.begin();
	   outp_iter != n->outports.end();
	   outp_iter++) {
	WfOutPort * outp = (WfOutPort *)(outp_iter->second);
	if (outp->isResult()) {
	  // ******************* SCALAR
	  if (outp->profile()->parameters[outp->getIndex()].desc.generic.type == DIET_SCALAR) {
	    // DOUBLE OR FLOAT
	    if (outp->profile()->parameters[outp->getIndex()].desc.generic.base_type == DIET_DOUBLE) {
	      double * value = NULL;
	      diet_scalar_get(diet_parameter(outp->profile(),outp->getIndex()),
			      &value, NULL);
	      TRACE_TEXT (TRACE_ALL_STEPS,
			  "## WORKFLOW OUTPUT ## " <<
			  outp->getId() << " = " << *value << endl);
	    }
	    if (outp->profile()->parameters[outp->getIndex()].desc.generic.base_type == DIET_FLOAT) {
	      float * value = NULL;
	      diet_scalar_get(diet_parameter(outp->profile(),outp->getIndex()),
			      &value, NULL);
	      TRACE_TEXT (TRACE_ALL_STEPS,
			  "## WORKFLOW OUTPUT ## " <<
			  outp->getId() << " = " << *value << endl);
	    }
	    // INTEGER OR CHAR
	    if ( (outp->profile()->parameters[outp->getIndex()].desc.generic.base_type == DIET_CHAR) ||
		 (outp->profile()->parameters[outp->getIndex()].desc.generic.base_type == DIET_SHORT) ||
		 (outp->profile()->parameters[outp->getIndex()].desc.generic.base_type == DIET_INT) ||
		 (outp->profile()->parameters[outp->getIndex()].desc.generic.base_type == DIET_LONGINT)) {
	      long * value = NULL;
	      diet_scalar_get(diet_parameter(outp->profile(),outp->getIndex()),
			      &value, NULL);
	      TRACE_TEXT (TRACE_ALL_STEPS,
			  "## WORKFLOW OUTPUT ## " <<
			  outp->getId() << " = " << *value << endl);
	    }
	  // ******************* END SCALAR

	  // ******************* STRING
	  } // end if SCALAR

	  if (outp->profile()->parameters[outp->getIndex()].desc.generic.type == DIET_STRING) {
	    char * value;
	    diet_string_get(diet_parameter(outp->profile(),outp->getIndex()),
			    &value, NULL);
	    TRACE_TEXT (TRACE_ALL_STEPS,
			  "## WORKFLOW OUTPUT ## " <<
			  outp->getId() << " = " << value << endl);
	  } // end if STRING
	  // ******************* END STRING

	  // ******************* FILE
	  if (outp->profile()->parameters[outp->getIndex()].desc.generic.type == DIET_FILE) {
	    size_t size;
	    char * path;
	    diet_file_get(diet_parameter(outp->profile(),outp->getIndex()),
			  NULL, &size, &path);
	    TRACE_TEXT (TRACE_ALL_STEPS,
			"## WORKFLOW OUTPUT ## " <<
			outp->getId() << " type = DIET_FILE, " <<
			"File name = " << path << ", " <<
			"File size = " << size  << endl);
	  } // end if STRING
	  // ******************* END FILE

	  // ******************* MATRIX
	  if (outp->profile()->parameters[outp->getIndex()].desc.generic.type == DIET_MATRIX) {
	      size_t nb_rows, nb_cols;
	      diet_matrix_order_t order;
	    if (outp->profile()->parameters[outp->getIndex()].desc.generic.base_type == DIET_DOUBLE) {
	      double * value;
	      diet_matrix_get(diet_parameter(outp->profile(), outp->getIndex()),
			      &value, NULL,
			      &nb_rows, &nb_cols, &order);
	      wf_dag_print_matrix_of_real(value, nb_rows, nb_cols, order);
	    } // end if base_type == DOUBLE

	    if (outp->profile()->parameters[outp->getIndex()].desc.generic.base_type == DIET_FLOAT) {
	      float * value;
	      diet_matrix_get(diet_parameter(outp->profile(), outp->getIndex()),
			      &value, NULL,
			      &nb_rows, &nb_cols, &order);
	      wf_dag_print_matrix_of_real(value, nb_rows, nb_cols, order);
	    } // end if base_type == FLOAT

	    if (outp->profile()->parameters[outp->getIndex()].desc.generic.base_type == DIET_CHAR) {
	      char * value;
	      diet_matrix_get(diet_parameter(outp->profile(), outp->getIndex()),
			      &value, NULL,
			      &nb_rows, &nb_cols, &order);
	      wf_dag_print_matrix_of_integer(value, nb_rows, nb_cols, order);
	    } // end if base_type == CHAR

	    if (outp->profile()->parameters[outp->getIndex()].desc.generic.base_type == DIET_SHORT) {
	      short * value;
	      diet_matrix_get(diet_parameter(outp->profile(), outp->getIndex()),
			      &value, NULL,
			      &nb_rows, &nb_cols, &order);
	      wf_dag_print_matrix_of_integer(value, nb_rows, nb_cols, order);
	    } // end if base_type == SHORT

	    if (outp->profile()->parameters[outp->getIndex()].desc.generic.base_type == DIET_INT) {
	      int * value;
	      diet_matrix_get(diet_parameter(outp->profile(), outp->getIndex()),
			      &value, NULL,
			      &nb_rows, &nb_cols, &order);
	      wf_dag_print_matrix_of_integer(value, nb_rows, nb_cols, order);
	    } // end if base_type == INT

	    if (outp->profile()->parameters[outp->getIndex()].desc.generic.base_type == DIET_LONGINT) {
	      long * value;
	      diet_matrix_get(diet_parameter(outp->profile(), outp->getIndex()),
			      &value, NULL,
			      &nb_rows, &nb_cols, &order);
	      wf_dag_print_matrix_of_longint(value, nb_rows, nb_cols, order);
	    } // end if base_type == LONG


	  }
	  // ******************* MATRIX
	} // if isResult
      } // end for outports
    } // end if n != NULL
  } // end for p

  return 0;
}

/**
 * free all persistent data used by the dag (includes intermediate and
 * final results)
 */
void
Dag::deleteAllResults() {
  Node * n = NULL;
  for (map<string, Node *>::iterator p = nodes.begin();
       p != nodes.end();
       ++p) {
    n = (Node *)(p->second);
    if (n != NULL) {
      n->freeProfileAndData();
    }
  }
}

/**
 * get the input nodes
 */
vector<Node *>
Dag::getInputNodes() {
  vector<Node *> v;
  Node * node = NULL;
  for (map<string, Node *>::iterator p = this->nodes.begin();
       p != this->nodes.end();
       p++) {
    node = (Node *)(p->second);
    if ((node != NULL) && (node->isAnInput()))
      v.push_back(node);
  }
  return v;
} // end getInputNodes

/**
 * set all input nodes as ready
 */
void
Dag::setInputNodesReady() {
  vector<Node *> inputs = this->getInputNodes();
  for (vector<Node *>::iterator p = inputs.begin();
       p != inputs.end();
       p++) {
    Node * node = (Node *) *p;
    node->setAsReady();
  }
}

/**
 * get the output nodes
 */
vector<Node *>
Dag::getOutputNodes() {
  vector<Node *> v;
  Node * node = NULL;
  for (map<string, Node *>::iterator p = this->nodes.begin();
       p != this->nodes.end();
       p++) {
    node = (Node *)(p->second);
    if ((node != NULL) && (node->isAnExit()))
      v.push_back(node);
  }
  return v;
}

/**
 * get all profiles in the dag (thread-safe)
 * (vector must be deleted after usage)
 */

vector<diet_profile_t *>*
Dag::getAllProfiles() {
  vector<diet_profile_t*> * v = new vector<diet_profile_t*>();
  Node * n = NULL;
  diet_profile_t * profile = NULL;
  bool found = false;

  for (std::map <std::string, Node *>::iterator p = this->nodes.begin();
       p != this->nodes.end();
       p++) {
    n = (Node*)(p->second);
    if (n != NULL) {
      profile = n->profile;
      if (profile != NULL) {
	found = false;
	for (unsigned int ix=0; ix<v->size(); ix++) {
	  diet_profile_t * another_profile = (*v)[ix];
	  if ( (*profile) == *(another_profile) ) {
	    found = true;
	  }
	} // end for ix
	if (!found)
	  v->push_back(profile);
      } // end if profile != NULL
    } // end if n != NULL
  } // end for iterator

  return v;
}

/**
 * set the dag as a temporary object
 * Used to not delete the nodes of the dag
 */
void
Dag::setAsTemp(bool b) {
  this->tmpDag = b;
}

/**
 * get the estimated makespan of the DAG
 * @deprecated
 */
double
Dag::getEstMakespan() {
  double makespan = -1;
  Node * n = NULL;
  for (map<string, Node*>::iterator p = this->nodes.begin();
       p != this->nodes.end();
       ++p) {
    n = (Node*)(p->second);
    if ( (n != NULL) &&
	 (n->getEstCompTime() > makespan) )
      makespan = n->getEstCompTime();
  }
  return makespan;
}

/**
 * get the estimated earliest finish time of the DAG
 */
double
Dag::getEFT() {
  double EFT = -1;
  Node * n = NULL;
  for (map<string, Node*>::iterator p = this->nodes.begin();
       p != this->nodes.end();
       ++p) {
    n = (Node*)(p->second);
    if ( (n != NULL) &&
	 (n->getEstCompTime() > EFT) )
      EFT = n->getEstCompTime();
  }
  return EFT;
}

/**
 * set the start time of the DAG
 */
void
Dag::setStartTime(double time) {
  this->startTime = time;
}

/**
 * get the start time of the DAG
 */
double
Dag::getStartTime() {
  return this->startTime;
}

/**
 * get the estimated delay of the DAG
 */
double
Dag::getEstDelay() {
  return this->estDelay;
}

/**
 * set the estimated delay of the DAG
 * (updated by an exit node)
 * (can be increased or decreased)
 */
void
Dag::setEstDelay(double delay) {
  this->estDelay = delay;
  TRACE_TEXT (TRACE_ALL_STEPS, "Updated est. delay on DAG "
        << this->getId() << " : delay = " << delay << endl);
}

bool
Dag::updateDelayRec(Node * node, double newDelay) {
  bool res = true;
  this->myLock.lock();  /** LOCK */
  res = _updateDelayRec(node, newDelay);
  this->myLock.unlock();  /** UNLOCK */
  return res;
}

/**
 * RECURSIVE
 * updates the estDelay of node and propagates it to the successors
 * returns true if propagation worked well (no pb in getting delay info)
 */
bool
Dag::_updateDelayRec(Node * node, double newDelay) {
  bool res = true;
  if (newDelay > node->getEstDelay()) {
    // the node is/will be late compared to the last estimated delay
    // so the new delay must be propagated to the successors
    node->setEstDelay(newDelay);
    for (unsigned int ix=0; ix < node->nextNodesCount(); ix++) {
      Node * succ = (Node*)(node->getNext(ix));
      res = res && this->_updateDelayRec(succ, newDelay);
    }
  }
  else {
    TRACE_TEXT (TRACE_ALL_STEPS, "Delay estimate for node "
        << node->getCompleteId() << " unchanged (newDelay = "
        << newDelay << " / previous delay = " << node->getEstDelay()
        << ")" << endl);
  }
  return res;
}

/**
 * notify the dag of node execution failure (MADAG & CLIENT-SIDE)
 */
void
Dag::setNodeFailure(string nodeId) {
  this->cancelled = true;
  this->failedNodes.push_front(nodeId);
}

/**
 * get the list of failed nodes
 */
const std::list<string>&
Dag::getNodeFailureList() {
  return this->failedNodes;
}
/**
 * set the dag as cancelled
 * (used when failure happens after all nodes execution)
 */
void
Dag::setAsCancelled() {
  this->cancelled = true;
}

/**
 * Compare two profiles
 * FIXME : need to be completed by param comparison
 */
bool operator == (diet_profile_t& a,   diet_profile_t& b) {
  if (strcmp(a.pb_name, b.pb_name) ||
      (a.last_in    != b.last_in) ||
      (a.last_inout != b.last_inout) ||
      (a.last_out   != b.last_out))
    return false;
  return true;
}

