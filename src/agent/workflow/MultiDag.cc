/****************************************************************************/
/* The Meta Dag class                                                       */
/* This class is used in multi-workflow support to manipulate more than     */
/* than one DAG                                                             */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.1  2008/04/10 09:13:29  bisnard
 * New version of the MaDag where workflow node execution is triggered by the MaDag agent and done by a new CORBA object CltWfMgr located in the client
 *
 ****************************************************************************/

#include <vector>

#include "MultiDag.hh"

#include "debug.hh"

using namespace std;
using namespace madag;

MultiDag::MultiDag() {
  // create the input and output nodes
  this->input = new Node("MultiDag_Input", "",
			 -1, -1, -1);
  this->output = new Node("MultiDag_Output", "",
			  -1, -1, -1);
  this->myNodes[this->input->getId()] =  this->input;
  this->myNodes[this->output->getId()] =  this->output;
  this->input->setAsDone();
} // end constructor

MultiDag::~MultiDag() {
  // free the input and the output nodes
  if (this->input != NULL)
    delete (this->input);
  if (this->output != NULL)
    delete (this->output);
  // free the dag map 
  // each dag nodes are realesaed in the dag constructor so there is no need
  // to clear the map myNodes
  Dag * dag = NULL;
  for (map<string, Dag *>::iterator p = this->myDags.begin( ); 
       p != this->myDags.end( ); 
       ++p ) { 
    dag = (Dag*) p->second;
    // delete the map entry
    myDags.erase(p);
    if (dag != NULL)
      delete(dag);
  } // end for
} // end destructor

/**
 * Add a dag to the meta-dag
 */
void
MultiDag::addDag(Dag * dag) {

  this->myMutex.lock();

  // Add the dag to dags map
  this->myDags[dag->getId()] = dag;

  // link the meta-dag input with the dag inputs
  std::vector<Node *> v = dag->getInputNodes();
  for (unsigned int ix=0; ix < v.size(); ix++) {
    v[ix]->addPrec(this->input->getId(),
		   this->input);
  }  

  // link the meta-dag output with the dag outputs
  v = dag->getOutputNodes();
  for (unsigned int ix=0; ix < v.size(); ix++) {
    this->output->addPrec(v[ix]->getCompleteId(),
			  v[ix]);
  }

  // add the dag nodes to myNodes map
  cout << "--- The previous size of meta dag is " <<
    this->myNodes.size() << endl;

  v = dag->getNodes();

  cout << "The dag to add contains " << v.size() << " nodes" << endl;
  for (unsigned int ix=0; ix < v.size(); ix++) {
    string complete_id (dag->getId());
    complete_id += string("-") + v[ix]->getId();
    cout << __FUNCTION__ << ": complete_id " << complete_id << endl;
    this->myNodes[complete_id] = v[ix];
    // set the node state as false (not finished)
    this->nodesState[v[ix]] = false;
  }
  cout << "--- The new meta dag size is " <<
    this->myNodes.size() << endl;

  this->myMutex.unlock();
}

/**
 * Update an already present dag in the meta-dag
 */
void 
MultiDag::updateDag(const string id) {
  // Find the dag
  map<string, Dag*>::iterator p = myDags.find(id);
  if (p == myDags.end()) {
    WARNING("The dag " << id << " is not present in the meta-dag");
    return;
  }
  rebuild();
}

/**
 * Update an already present dag in the meta-dag
 * @param id the dag id to update
 * @param doneNodes vector of already done nodes
 */
void 
MultiDag::updateDag(const string id, vector<Node*> doneNodes) {
  map<string, Dag*>::iterator p = this->myDags.find(id);
  if (p == this->myDags.end()) {
    WARNING("The dag " << id << " was not found in the meta-dag");
    return;
  }
  Dag * dag = (Dag *)(p->second);
  if (dag == NULL) 
    return;
  for (unsigned int ix=0; ix<doneNodes.size(); ix++) {
    dag->moveToTrash(doneNodes[ix]);
  }

  // FIXME
  // create a new dag without references to done nodes
  // Dag * newDag = dag->getRemainingDag();
  // this->removeDag(id);
  // delete dag;
  // this->addDag(newDag->getId());
  // this->rebuild();
}

/**
 * Remove a dag from the meta-dag
 */
void
MultiDag::removeDag(const string id) {
  this->myMutex.lock();

  // Find the dag
  map<string, Dag*>::iterator p = myDags.find(id);
  if (p == myDags.end()) {
    WARNING("The dag " << id << " is not present in the meta-dag");
    this->myMutex.unlock();
    return;
  }
  // Remove the dag
  Dag * dag = (Dag*)(p->second);
  if (dag != NULL) {
    myDags.erase(p);
    this->rebuild();
  }

  this->myMutex.unlock();
}

/**
 * Regenerate the meta-dag
 */
void
MultiDag::rebuild() {
  // clear the myNodes map
  myNodes.clear();
  // for each dag, add its nodes to the meta-dag (to myNodes map)
  cout << "rebuild (begin) : myDags.size() =  " << myDags.size() << endl;
  Dag * dag = NULL;
  for (map<string, Dag*>::iterator p = myDags.begin();
       p != myDags.end();
       ++p) {
    dag = (Dag*)(p->second);
    if (dag != NULL) {
      vector<Node *> nodes = dag->getNodes();
      cout << "Adding the nodes of " << dag->getId() << 
	"(" << nodes.size() << ")" << endl;
      for (unsigned int ix=0; ix<nodes.size(); ix++) {
	if ( (this->nodesState.find(nodes[ix]) != this->nodesState.end()) &&
	     (this->nodesState[nodes[ix]] == false) ) {
	  cout << "Adding the node " << nodes[ix]->getCompleteId() << endl;
	  this->myNodes[nodes[ix]->getCompleteId()] = nodes[ix];
	} // end if
      } // end for
    }    
  }
  cout << "rebuild (end) : myDags.size() =  " << myDags.size() << endl;
  linkDags();
}


/**
 * Links all the dags of the meta-dag
 *   - links the meta-dag input with dags inputs
 *   - links the dags outputs to the meta-dag output
 */
void
MultiDag::linkDags() {
  Dag * dag = NULL;
  for (map<string, Dag*>::iterator p = this->myDags.begin();
       p != this->myDags.end();
       p++) {
    dag = (Dag *)(p->second);
    if (dag != NULL) {
      linkDag(dag);
    }
  }
} // end linkDags

/**
 * Create the links between meta-dag input/output and the dag input/output
 */
void
MultiDag::linkDag(Dag * dag) {
  vector<Node *> ins = dag->getInputNodes();
  vector<Node *> outs = dag->getOutputNodes();
  
  for (unsigned int ix=0; ix<ins.size(); ix++) {
    ins[ix]->addPrec(this->input->getId(), 
		     this->input);
    
  }

  for (unsigned int ix=0; ix<outs.size(); ix++) {
    this->output->addPrec(outs[ix]->getId(), 
			  outs[ix]);
  }
} // end linkDag

/**
 * Get the XML representation of the meta-dag
 */
string
MultiDag::toXML() {
  this->myMutex.lock();

  Node * n = NULL;
  string xml("<dag>\n");
  for (  map <string, Node *>::iterator p = this->myNodes.begin();
	 p != this->myNodes.end();
	 ++p) {
    n = (Node*)(p->second);
    if ( (n != NULL) &&
	 (this->nodesState.find(n) != this->nodesState.end()) &&
	 (! this->nodesState[this->nodesState.find(n)->first]) )
      xml += n->toXML();
  }
  xml += "</dag>\n";

  this->myMutex.unlock();

  return xml;
}

/**
 * return a Dag object representing the meta-dag
 * Don't forget to free the memory after the use of the returned dag
 */
Dag *
MultiDag::getDag() {
  Dag * dag = new Dag();

  Node * n = NULL;

  for (map <string, Node *>::iterator p = this->myNodes.begin();
       p != this->myNodes.end();
       p++) {
    n = (Node*)(p->second);
    if ( (n != NULL) &&
	 (this->nodesState.find(n) != this->nodesState.end()) &&
	 (! this->nodesState[this->nodesState.find(n)->first]) ) {
      cout << __FUNCTION__ << " adding " << n->getId() << endl;
      dag->addNode(n->getId(), n);
    }
  } // end for

  return dag;
}

/**
 * set the node state as done
 */
void
MultiDag::setNodeAsDone(const char * dagId, const char * nodeId, 
		       bool state) {
  this->myMutex.lock();

  string completeId = string(dagId) + string("-") + string(nodeId);

  map<string, Node*>::iterator p = this->myNodes.find(completeId);

  if (p!= this->myNodes.end()) {
    Node * node = (Node*)(p->second);
    this->nodesState[node] = state;
    if (this->myDags.find(dagId) != this->myDags.end()) {
      Dag * dag = (Dag*)(this->myDags.find(dagId)->second);
      dag->moveToTrash(node);
    }
  }
  else {
    WARNING("Unable to find the node " << completeId << endl);
  }

  this->myMutex.unlock();
}

/**
 * get the dags that compose the meta-dag
 */
vector<Dag *>
MultiDag::getAllDags() {
  Dag * dag = NULL;
  vector<Dag*> v;
  for (map<string, Dag*>::iterator p = this->myDags.begin();
       p != this->myDags.end();
       ++p) {
    dag = (Dag*)(p->second);
    if (dag != NULL) 
      v.push_back(dag);
  }
  return v;
}

/**
 * return the node with identifier id
 */
Node *
MultiDag::getNode(const string id) {
  if (this->myNodes.find(id) != this->myNodes.end())
    return this->myNodes[id];
  return NULL;
}

/**
 * return the dag with identifier id
 */
Dag *
MultiDag::getDag(const string id) {
  if (this->myDags.find(id) != this->myDags.end())
    return this->myDags[id];
  return NULL;
}

/**
 * return the number of the DAGs in the meta-dag
 */
unsigned int 
MultiDag::getLength() {
  return this->myDags.size();
}

/**
 * Return a vector of ready nodes
 */
vector<Node *>
MultiDag::getReadyNodes() {
  vector<Node*> v;
  for (map<string, Node*>::iterator p = this->myNodes.begin();
       p != this->myNodes.end(); 
       p++) {
    if (
        (p->second->getSeD() != SeD::_nil()) &&
        (p->second->allPrevDone()) &&
        (!p->second->isDone()) &&
        (!p->second->isRunning())
        )
      v.push_back(p->second);
  } // end for
  return v;
} // end getReadyNodes

/**
 * Map scheduling 
 */
void
MultiDag::mapSeDs(wf_node_sched_seq_t& sched_seq) {
  for (int ix=0; ix<sched_seq.length(); ix++) {
    string node_id(sched_seq[ix].node_id);
    if (this->myNodes.find(node_id) != 
        this->myNodes.end()) {
      cout << "  ** mapping " << this->myNodes.find(node_id)->second->getId() 
           << " to " << sched_seq[ix].server.loc.hostName << endl;
      string hn(sched_seq[ix].server.loc.hostName);
      this->myNodes.find(node_id)->second->setSeD(sched_seq[ix].server.loc.ior,
                                                  hn);
    } // end if
  } // end for
  
} // end mapSeD
