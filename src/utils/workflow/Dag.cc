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
  this->current_node = this->nodes.end();
  this->myId    = "";
  this->nbSec   = 5;
  this->nbNodes = 1;
  this->tmpDag  = false;
}

Dag::~Dag() {
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
 * add a node to the dag
 */
void
Dag::addNode (string nodeName, Node * node) {
  nodes[nodeName] = node;
  TRACE_TEXT (TRACE_ALL_STEPS,
	      "\t" << "The new size of the dag is " << nodes.size() <<  endl);
}

/**
 * check the precedence between node *
 * this function check only the precedence between node, it doesn't *
 * link the ports *
 */
bool
Dag::checkPrec() {
  bool result = true;
  Node * node = NULL;
  unsigned int n = 0;
  for (map<string, Node * >::iterator p = nodes.begin( ); p != nodes.end( );
       ++p ) {
    node = (Node*) p->second;
    n = node->prevNb();
    for (uint ix = 0; ix < n; ix++) {
      map<string, Node * >::iterator q = nodes.find(node->getPrecId(ix));
      if (q != nodes.end()) {
	node->addPrec(node->getPrecId(ix),
		      (Node*)(q->second));
      }
      else
	return false;
    }
  }

  for (map<string, Node * >::iterator p = nodes.begin( ); p != nodes.end( );
       ++p ) {
    node = (Node*) p->second;

    for (map<string, WfInPort*>::iterator p = node->inports.begin();
	 p != node->inports.end();
	 ++p) {
      // get the port ref
      WfInPort * in = (WfInPort*)(p->second);
      // get the linked port id (source id)
      string lp_id = in->getSourceId();
      if (lp_id == "")
	continue;

      // get the linked node name
      string linkedNode_name = lp_id.substr(0, lp_id.find("#"));
      // linked port id
      lp_id = lp_id.substr(lp_id.find("#")+1);

      TRACE_FUNCTION (TRACE_ALL_STEPS,
		      "\tprevious node name : "<< linkedNode_name);
      // get the linked node ref
      map<string, Node *>::iterator lnp = nodes.find(linkedNode_name);
      Node * ln = NULL;
      if (lnp != nodes.end()) {
	ln = (Node *)(lnp->second);
	node->addPrec(ln->getId(), ln);
      }
    } // end for in

    for (map<string, WfInOutPort*>::iterator p = node->inoutports.begin();
	 p != node->inoutports.end();
	 ++p) {
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
 * return an XML representation of the DAG
 * if b = true, return the complete DAG representation
 * otherwise (b = false value by default) only the remaining DAG
 */
string
Dag::toXML(bool b) {
  string xml = "<dag>\n";
  Node * n = NULL;
  for (map<string, Node *>::iterator p = nodes.begin();
       p != nodes.end();
       ++p) {
    n = (Node *)(p->second);
    if (b) {
      // we need all nodes representation
      xml += n->toXML(b);
    }
    else {
      // we need only remaining nodes
      if (!(n->isRunning()) && !(n->isDone()))
	xml += n->toXML(b);
    }
  }
  xml += "</dag>\n";
  return xml;
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
 */
void
Dag::linkNodePorts(Node * n) {
  TRACE_TEXT (TRACE_ALL_STEPS,
	      "linkNodePorts : processing node " << n->getId() << endl);
  // link Input ports with output ports
  for (map<string, WfInPort*>::iterator p = n->inports.begin();
       p != n->inports.end();
       ++p) {
    // get the port ref
    WfInPort * in = (WfInPort*)(p->second);
    if (in != NULL)
      TRACE_TEXT (TRACE_ALL_STEPS,
		  "in found " << in->getId() << endl);
    string node_name = in->getId().substr(0, in->getId().find("#"));
    // get the linked port id
    string lp_id = in->getSourceId();
    if (lp_id == "")
      continue;

    // get the linked node name
    string linkedNode_name = lp_id.substr(0, lp_id.find("#"));
    // linked port id
    lp_id = lp_id.substr(lp_id.find("#")+1);

    TRACE_TEXT (TRACE_ALL_STEPS,
		"linked node name : "<< linkedNode_name <<
		", linked port : " << lp_id << endl);
    // get the linked node ref
    map<string, Node *>::iterator lnp = nodes.find(linkedNode_name);
    Node * ln = NULL;
    if (lnp != nodes.end()) {
      ln = (Node *)(lnp->second);
    }
    else {
      lnp = nodes.find(this->myId + "-" + linkedNode_name);
      if (lnp != nodes.end()) {
        ln = (Node *)(lnp->second);
      }
      else {
        INTERNAL_ERROR("FATAL ERROR" << endl <<
                       "Unable to find the linked node : " <<
                       node_name, 1);
      }
    }
    // get outpur port
    WfOutPort * out = ln->getOutPort(linkedNode_name + "#" + lp_id);

    // link the input port to the outport
    TRACE_TEXT (TRACE_ALL_STEPS,
		"linking the input port " << in->getId() <<
		" to the output port " <<
		out->getId() << endl);
    in->set_source(out);
    out->set_sink(in);
  }

  // link output ports with input ports
  for (map<string, WfOutPort*>::iterator p = n->outports.begin();
       p != n->outports.end();
       ++p) {
    // get the port ref
    WfOutPort * out = (WfOutPort*)(p->second);
    if (out != NULL)
      TRACE_TEXT (TRACE_ALL_STEPS,
		  "Out found " << out->getId() << endl);
    string node_name = out->getId().substr(0, out->getId().find("#"));
    // get the linked port id
    string lp_id = out->getSinkId();
    if (lp_id == "")
      continue;

    // get the linked node name
    string linkedNode_name = lp_id.substr(0, lp_id.find("#"));
    // linked port id
    lp_id = lp_id.substr(lp_id.find("#")+1);

    TRACE_TEXT (TRACE_ALL_STEPS,
		"linked node name : "<< linkedNode_name <<
		", linked port : " << lp_id << endl);
    // get the linked node ref
    map<string, Node *>::iterator lnp = nodes.find(linkedNode_name);
    Node * ln = NULL;
    if (lnp != nodes.end()) {
      ln = (Node *)(lnp->second);
    }
    else {
      INTERNAL_ERROR ("FATAL ERROR" << endl <<
		      "Unable to find the linked node : " <<
		      node_name << endl,
		      1);
    }
    // get outpur port
    WfInPort * in = ln->getInPort(linkedNode_name + "#" + lp_id);

    // link the input port to the outport
    TRACE_TEXT (TRACE_ALL_STEPS,
		"linking the output port " << out->getId() <<
		" to the input port " <<
		in->getId() << endl);
    out->set_sink(in);
    in->set_source(out);
  }
  // link inout ports with ... <TO DO>
}

/**
 * execute the workflow *
 * @deprecated
 */
// void
// Dag::exec() {
// }

/**
 * Get the ready nodes *
 * @deprecated
 */
// void
// Dag::getReadyNodes(map<string, Node *>& readyNodes) {
//   for (map <string, Node *>::iterator p = nodes.begin();
//        p != nodes.end();
//        ++p) {
//     Node * n = (Node *)p->second;
//     if ((n) && !(n->isRunning()) && !(n->isDone()) && (n->isReady())) {
//       readyNodes[n->getId()] = n;
//     }
//   }
// } // end getReadyNodes

/**
 * Check if the dag execution is completed
 *
 * this methods loops through all the nodes of the dag to check if they are done
 * @return bool
 */
bool
Dag::isDone() {
  Node * dagNode = NULL;
  for (map<string, Node *>::iterator p = nodes.begin();
       p != nodes.end();
       ++p) {
    dagNode = (Node*)p->second;
    if ((dagNode) && !(dagNode->isDone()))
      return false;
  }
  return true;
}

/**
 * set the workflow scheduling response *
 * @deprecated
 */
void
Dag::setSchedResponse(wf_node_sched_seq_t * response) {
  this->response = response;

  if (response->length() != nodes.size()) {
    TRACE_TEXT (TRACE_ALL_STEPS,
		"the scheduling response length is different from dag length "
		<< endl);
  }

  for (unsigned int ix=0;
       ix < response->length();
       ix++) {
    string nid((*response)[ix].node_id);
    map <string, Node *>::iterator p =
      nodes.find(nid);
    if (p != nodes.end()) {
      TRACE_TEXT (TRACE_ALL_STEPS,
		  "The nodes " << ((Node*)(p->second))->getId() <<
		  " is mapped to a SeD"  << endl);
      ((Node*)(p->second))->setSeD((*response)[ix].server.loc.ior, "");
    }
    else {
      // Try with complete id
      nid = nid.substr(nid.find("-")+1);
      p = nodes.find(nid);
      if (p != nodes.end()) {
        TRACE_TEXT (TRACE_ALL_STEPS,
                    "The nodes " << ((Node*)(p->second))->getId() <<
                    " is mapped to a SeD"  << endl);
        ((Node*)(p->second))->setSeD((*response)[ix].server.loc.ior,
                                     "");
      }
      else {
        cout << " *** " << nid << " not found!!!" <<endl;
      }
    }
  }
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
 * tag the dag *
 * the input node will receive a tag equal to zero *
 * so of the level 1 will have a tag equal to 1 and so on *
 * the output nodes will have a tag equal to the length of the dag *
 */
// void
// Dag::setTags() {
//   Node * n = NULL;
//   for (map<string, Node*>::iterator p = nodes.begin();
//        p != nodes.end();
//        ++p) {
//     n = (Node *)(p->second);
//     if (n != NULL) {
//       if (n->isAnInput()) {
// 	n->setTag(0);
//       }
//     }
//   }
// }

/**
 * check the scheduling
 */
bool
Dag::checkScheduling() {
  Node * n = NULL;
  vector<Node*> nodes_with_pb;
  //  cout << "The DAG checks the schedling" << endl;
  for (map<string, Node *>::iterator p = nodes.begin();
       p != nodes.end();
       ++p) {
    n = (Node *)(p->second);
    if ((n != NULL) && (n->isDone()) && (!n->getMark())) {
      if (
	  (WfCst::diff(n->getRealCompTime(), this->beginning,
		       n->getEstCompTime())>this->nbSec)
	  ) {
	nodes_with_pb.push_back(n);
      }
    }
  }
  if (nodes_with_pb.size()>0)
    TRACE_TEXT (TRACE_ALL_STEPS,
		nodes_with_pb.size() << " node(s) out of predicted time" <<endl);

  if (nodes_with_pb.size() >= this->nbNodes) {
    for (unsigned int ix=0; ix<nodes_with_pb.size(); ix++)
      nodes_with_pb[ix]->setMark(true);
    return false;
  }
  return true;
}

/**
 * set the beginning time of execution
 */
void
Dag::setTheBeginning(struct timeval tv) {
  this->beginning = tv;
  TRACE_TEXT (TRACE_ALL_STEPS,
	      "---- The beginning time is " << this->beginning.tv_sec <<
	      endl);
}

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
 * get the dag nodes as a vector of node reference
 */
vector<Node*>
Dag::getNodes() {
  vector<Node*> v;
  Node * node = NULL;
  for (map<string, Node * >::iterator p = nodes.begin( ); p != nodes.end( );
       ++p ) {
    node = (Node*) p->second;
    if (node != NULL)
      v.push_back(node);
  }
  return v;
} // end getNodes

/**
 * Get the node with given identifier
 *
 */
Node *
Dag::getNode(std::string node_id) {
  map<string, Node*>::iterator p = this->nodes.find(node_id);
  if ( p != this->nodes.end())
    return p->second;
  else
    return NULL;
} // end getNode

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
	      TRACE_TEXT (TRACE_MAIN_STEPS,
			  "## WORKFLOW OUTPUT ## " <<
			  outp->getId() << " = " << *value << endl);
	    }
	    if (outp->profile()->parameters[outp->getIndex()].desc.generic.base_type == DIET_FLOAT) {
	      float * value = NULL;
	      diet_scalar_get(diet_parameter(outp->profile(),outp->getIndex()),
			      &value, NULL);
	      TRACE_TEXT (TRACE_MAIN_STEPS,
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
	      TRACE_TEXT (TRACE_MAIN_STEPS,
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
	      TRACE_TEXT (TRACE_MAIN_STEPS,
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
	    TRACE_TEXT (TRACE_MAIN_STEPS,
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
 * get the dag id
 */
string
Dag::getId() {
  return this->myId;
}

/**
 * set the dag id
 */
void
Dag::setId(const string id) {
  this->myId = id;
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
 * get all profiles in the dag
 */

vector<diet_profile_t *>
Dag::getAllProfiles() {
  vector<diet_profile_t*> v;
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
	for (unsigned int ix=0; ix<v.size(); ix++) {
	  diet_profile_t * another_profile = v[ix];
	  if ( (*profile) == *(another_profile) ) {
	    found = true;
	  }
	} // end for ix
	if (!found)
	  v.push_back(profile);
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

