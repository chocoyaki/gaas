/****************************************************************************/
/* The base abstract class for multi-workflow scheduler                     */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.2  2008/04/14 09:10:37  bisnard
 *  - Workflow rescheduling (CltReoMan) no longer used with MaDag v2
 *  - AbstractWfSched and derived classes no longer used with MaDag v2
 *
 * Revision 1.1  2008/04/10 09:13:29  bisnard
 * New version of the MaDag where workflow node execution is triggered by the MaDag agent and done by a new CORBA object CltWfMgr located in the client
 *
 ****************************************************************************/

#include "MultiWfScheduler.hh"

using namespace madag;

MultiWfScheduler::MultiWfScheduler() {
  this->myMetaDag = new MultiDag();
  this->mySched = NULL;
}

MultiWfScheduler::~MultiWfScheduler() {
}

/**
 * extract only the response for the specified dag
 */
wf_sched_response_t * 
MultiWfScheduler::extract(int dag_id, wf_sched_response_t * wf_resp) {
  string dagIdStr = itoa(dag_id);
  wf_sched_response_t * response = new wf_sched_response_t;

  response->dag_id = dag_id;

  unsigned index = 0;
  for (unsigned int ix=0; ix<wf_resp->wf_node_sched_seq.length(); ix++) {
    string id(wf_resp->wf_node_sched_seq[ix].node_id);
    cout << "testing the response for " << id << endl;
    if (id.substr(0, id.find("-")) == dagIdStr) {
      response->wf_node_sched_seq.length(index+1);
      response->wf_node_sched_seq[index].node_id = CORBA::string_dup(id.substr(id.find("-")+1).c_str());
      response->wf_node_sched_seq[index].server = wf_resp->wf_node_sched_seq[ix].server;
      cout << "Adding an item " << response->wf_node_sched_seq[index].node_id << 
	", mapped to server" << wf_resp->wf_node_sched_seq[ix].server.loc.ior << endl;
      index++;
    }
  }

  return response;
} // end extract

/**
 * Execution method
 */
void*
MultiWfScheduler::run() {
  return NULL;
}

class NodeRun: public Thread {
public:
  NodeRun(Node * node, SeD_var sed, MultiWfScheduler * scheduler) {
    this->myNode = node;
    this->mySeD = sed;
    this->myScheduler = scheduler;
  }

  virtual void*
  run() {
    cout << "NodeRun: running node " << this->myNode->getCompleteId() << endl;
    CltMan_ptr cltMan = 
      this->myScheduler->getCltMan(this->myNode->getDag()->getId());
    if (cltMan != CltMan::_nil()) {
      cout << "NodeRun: try to call client manager (ping)" << endl;
      cltMan->ping();
      cout << "NodeRun: try to call client manager (exec)" << endl;
      cltMan->execute(this->myNode->getId().c_str(),
                      this->myNode->getDag()->getId().c_str(),
                      this->mySeD);
      cout << "NodeRun: setting node as done" << endl;
      this->myNode->setAsDone();
      if ((this->myNode->getDag() != NULL) &&
          (this->myNode->getDag()->isDone())
          ) {
        cout << "NodeRun: The Dag " << this->myNode->getDag()->getId().c_str() << "is done " << endl;
        cltMan->release(this->myNode->getDag()->getId().c_str());
      } 
    }
    this->myScheduler->wakeUp();
  }
private:
  Node *  myNode;
  SeD_var mySeD;
  MultiWfScheduler * myScheduler;
};
/**
 * Execute a node
 */
Thread * 
MultiWfScheduler::runNode(Node * node, SeD_var sed) {
  Thread * thread = new NodeRun(node, sed, this);
  thread->start();
  return thread;
}

/**
 * Get the corresponding client manager
 */
CltMan_ptr
MultiWfScheduler::getCltMan(string dag_id) {
  if (this->cltMans.find(dag_id) == this->cltMans.end())
    return CltMan::_nil();
  return this->cltMans.find(dag_id)->second;
}
