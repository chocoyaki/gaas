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
 * Revision 1.2  2006/07/10 11:08:12  aamar
 * - Adding the toXML function that return the DAG XML
 * representation
 * - Adding reordering (rescheduling) management
 *
 * Revision 1.1  2006/04/14 13:46:51  aamar
 * Direct acyclic graph class (source).
 *
 ****************************************************************************/

#include "Dag.hh"


Dag::Dag() {
  this->current_node = NULL;
  this->nbSec = 5;
  this->nbNodes = 1;
}

Dag::~Dag() {
  Node * node = NULL;
  for (map<string, Node * >::iterator p = nodes.begin( ); p != nodes.end( ); 
       ++p ) { 
    node = (Node*) p->second;
    // TO FIX
    nodes.erase(p);
    delete(node);
  }
  nodes.clear();
}

/**
 * add a node to the dag
 */
void 
Dag::addNode (string nodeName, Node * node) {
  nodes[nodeName] = node;
  cout << "\t" << "The new size of the dag is " << nodes.size() <<  endl;
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
    n = node->prec_ids_nb();
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

    for (map<string, Node::WfInPort*>::iterator p = node->inports.begin();
	 p != node->inports.end();
	 ++p) {
      // get the port ref
      Node::WfInPort * in = (Node::WfInPort*)(p->second);
      // get the linked port id (source id)
      string lp_id = in->source_port_id;
      if (lp_id == "")
	continue;
      
      // get the linked node name
      string linkedNode_name = lp_id.substr(0, lp_id.find("#"));
      // linked port id
      lp_id = lp_id.substr(lp_id.find("#")+1);
      
      cout << "\tprevious node name : "<< linkedNode_name << endl;
      // get the linked node ref
      map<string, Node *>::iterator lnp = nodes.find(linkedNode_name);
      Node * ln = NULL;
      if (lnp != nodes.end()) {
	ln = (Node *)(lnp->second);
	node->addPrec(ln->getId(), ln);
      }
    } // end for in

    for (map<string, Node::WfInOutPort*>::iterator p = node->inoutports.begin();
	 p != node->inoutports.end();
	 ++p) {
    } // end for inout

  }
  return result;
}

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

/*********************************************************************/
/* the size, begin end end methods are only for testing & debugging  */
/*********************************************************************/
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
 * (according to the std::map and not to the dag structure) *
 */
std::map <std::string, Node *>::iterator
Dag::begin() {
  return nodes.begin();
}

/**
 * return an iterator on the last node *
 * (according to the std::map and not to the dag structure) *
 */
std::map <std::string, Node *>::iterator
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
  cout << "linkNodePorts : processing node " << n->getId() << endl;
  // link Input ports with output ports
  for (map<string, Node::WfInPort*>::iterator p = n->inports.begin();
       p != n->inports.end();
       ++p) {
    // get the port ref
    Node::WfInPort * in = (Node::WfInPort*)(p->second);
    if (in != NULL)
      cout << "in found" << in->id << endl;
    string node_name = in->id.substr(0, in->id.find("#"));
    // get the linked port id
    string lp_id = in->source_port_id;
    if (lp_id == "")
      continue;

    // get the linked node name
    string linkedNode_name = lp_id.substr(0, lp_id.find("#"));
    // linked port id
    lp_id = lp_id.substr(lp_id.find("#")+1);

    cout << "linked node name : "<< linkedNode_name << 
      ", linked port : " << lp_id << endl;
    // get the linked node ref
    map<string, Node *>::iterator lnp = nodes.find(linkedNode_name);
    Node * ln = NULL;
    if (lnp != nodes.end()) {
      ln = (Node *)(lnp->second);
    }
    else {
      cout << "FATAL ERROR" << endl << "Unable to find the linked node : " <<
	node_name << endl;
    }
    // get outpur port
    Node::WfOutPort * out = ln->getOutPort(linkedNode_name + "#" + lp_id); 

    // link the input port to the outport
    cout << "linking the input port " << in->id << " to the output port " <<
      out->id << endl;
    in->set_source(out);
  }

  // link output ports with input ports
  for (map<string, Node::WfOutPort*>::iterator p = n->outports.begin();
       p != n->outports.end();
       ++p) {
    // get the port ref
    Node::WfOutPort * out = (Node::WfOutPort*)(p->second);
    if (out != NULL)
      cout << "Out found " << out->id << endl;
    string node_name = out->id.substr(0, out->id.find("#"));
    // get the linked port id
    string lp_id = out->sink_port_id;
    if (lp_id == "")
      continue;

    // get the linked node name
    string linkedNode_name = lp_id.substr(0, lp_id.find("#"));
    // linked port id
    lp_id = lp_id.substr(lp_id.find("#")+1);

    cout << "linked node name : "<< linkedNode_name << 
      ", linked port : " << lp_id << endl;
    // get the linked node ref
    map<string, Node *>::iterator lnp = nodes.find(linkedNode_name);
    Node * ln = NULL;
    if (lnp != nodes.end()) {
      ln = (Node *)(lnp->second);
    }
    else {
      cout << "FATAL ERROR" << endl << "Unable to find the linked node : " <<
	node_name << endl;
    }
    // get outpur port
    Node::WfInPort * in = ln->getInPort(linkedNode_name + "#" + lp_id); 

    // link the input port to the outport
    cout << "linking the output port " << out->id << " to the input port " <<
      in->id << endl;
    out->set_sink(in);
  }    
  // link inout ports with ... <TO DO>
}

/**
 * execute the workflow *
 */
void
Dag::exec() {
}

/**
 * Get the ready nodes *
 */
vector<Node *> 
Dag::getReadyNodes() {
  vector<Node *> rn;
  //  cout << "total nodes : " << nodes.size() << " nodes" << endl;
  for (map <std::string, Node *>::iterator p = nodes.begin();
       p != nodes.end();
       ++p) {
    Node * n = (Node *)p->second;
    if ((n) && !(n->isRunning()) && !(n->isDone()) && (n->isReady())) {
      rn.push_back(n);
    }
  }
  //  cout << "ready nodes : " << rn.size() << " nodes" << endl;
  return rn;
}

/**
 * check if the dag execution is completed *
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
 */
void
Dag::setSchedResponse(wf_node_sched_seq_t * response) {
  this->response = response;
  
  if (response->length() != nodes.size()) {
    cout << "the scheduling response length is different from dag length " 
	 << endl;
  }

  for (unsigned int ix=0; 
       ix < response->length(); 
       ix++) {
    string nid((*response)[ix].node_id);
    map <std::string, Node *>::iterator p = 
      nodes.find(nid);
    if (p != nodes.end()) {
      cout << "the nodes " << ((Node*)(p->second))->getId() << 
	" is mapped to a SeD"  << endl;
      ((Node*)(p->second))->setSeD((*response)[ix].server.loc.ior);
    }
  }
}


/**
 * get a scalar result of the workflow *
 */
int
Dag::get_scalar_output(const char * id,
		    void** value) {
  cout << "\t" << "get_scalar_output : searching for " << id << endl;
  Node * n = NULL;
  for (map<string, Node *>::iterator p = nodes.begin();
       p != nodes.end();
       ++p) {
    n = (Node *)(p->second);
    if (n != NULL) {
      map<string, Node::WfOutPort*>::iterator outp_iter =
	n->outports.find(id);
      if (outp_iter != n->outports.end()) {
	cout << "\t" << "found an output port with the id " << id << endl;
	Node::WfOutPort * outp = (Node::WfOutPort *)(outp_iter->second);
	if (outp->isResult()) {
	    cout << "\t" << "return the result" << endl;
	  //       diet_scalar_get(diet_parameter(profile,2), &pl3, NULL);
	  return diet_scalar_get(diet_parameter(outp->profile(),outp->index), 
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
      map<string, Node::WfOutPort*>::iterator outp_iter =
	n->outports.find(port_id);
      if (outp_iter != n->outports.end()) {
	cout << "######## found an output port with the id " << id << endl;
	Node::WfOutPort * outp = (Node::WfOutPort *)(outp_iter->second);
	if (outp->isResult()) {
	  cout << "######## return the result" << endl;
	  //       diet_scalar_get(diet_parameter(profile,2), &pl3, NULL);
	  return diet_string_get(diet_parameter(outp->profile(),outp->index), 
				 value, NULL);
	}
      }
  }
  return 1;
}

/**
 * tag the dag *
 * the input node will receive a tag equal to zero *
 * so of the level 1 will have a tag equal to 1 and so on *
 * the output nodes will have a tag equal to the length of the dag *
 */
void
Dag::setTags() {
  Node * n = NULL;
  for (map<string, Node*>::iterator p = nodes.begin();
       p != nodes.end();
       ++p) {
    n = (Node *)(p->second);
    if (n != NULL) {
      if (n->isAnInput()) {
	n->setTag(0);
      }
    }
  }
}


/**
 * set the reordering parameters
 * nb_sec is the number of seconds
 * nb_node is the number of nodes
 */
void 
Dag::set_reordering_delta(const long int nb_sec, 
			  const unsigned long int nb_nodes) {
  this->nbSec   = nb_sec;
  this->nbNodes = nb_nodes;
}

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
    cout << nodes_with_pb.size() << " node(s) out of predicted time" <<endl;
  
  if (nodes_with_pb.size() >= this->nbNodes) {
    for (unsigned int ix=0; ix<nodes_with_pb.size(); ix++)
      nodes_with_pb[ix]->setMark(true);
    return false;
  }
  return true;
}


/**
 * set the client reordering manager
 */
void
Dag::setCltReoMan(CltReoMan_impl * crm) {
  cout << "---- DAG::setCltReoMan(";
  if (crm != NULL)
    cout << "<>NULL)" << endl;
  else
    cout << "NULL)" << endl;

  this->myCltReoMan = crm;
}

/**
 * set the beginning time of execution
 */
void
Dag::setTheBeginning(struct timeval tv) {
  this->beginning = tv;
  cout << "---- The beginning time is " << this->beginning.tv_sec <<
    endl;
}

/**
 * Move a node to the trash vector (called when rescheduling) 
 */
void
Dag::moveToTrash(Node * n) {
  trash.push_back(n);
  nodes.erase(n->getId());
}
