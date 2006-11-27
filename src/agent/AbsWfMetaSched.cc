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
 * Revision 1.2  2006/11/27 09:53:00  aamar
 * Correct headers of source files used in workflow support.
 *
 ****************************************************************************/
#include "AbsWfMetaSched.hh"

AbsWfMetaSched::AbsWfMetaSched() {
  this->myMetaDag = new MetaDag();
  this->mySched = NULL;
}

AbsWfMetaSched::~AbsWfMetaSched() {
}

/**
 * Add a new client reference
 */
void
AbsWfMetaSched::addClient(CltReoMan_var clt, const string dag) {
  
  this->myClients[dag] = clt;
}

/**
 * Remove a client from myClients map
 */
void
AbsWfMetaSched::removeClient(const string dag) {
  std::map<string, CltReoMan_var>::iterator p =
    this->myClients.find(dag);
  if (p !=  this->myClients.end()) {
    this->myClients.erase(p);
    cout << "+++++++++++++ " <<
      __FUNCTION__ << " remove the dag " << dag << " from the meta dag " << 
      endl;
    this->myMetaDag->removeDag(dag);
    cout << "+++++++++++++ " <<
      __FUNCTION__ << " the new size of the meta-dag is " <<
      this->myMetaDag->getLength() << endl;
  }
  else {
    cout << "+++++++++++++ " <<
      __FUNCTION__ << " can't find the client associated to " << dag << endl;
  }
}

/**
 * Set the node state as done
 */
void 
AbsWfMetaSched::setNodeAsDone(const char* dagId, const char* nodeId) {
  this->myMetaDag->setNodeAsDone(dagId, nodeId);
}

/**
 * extract only the response for the specified dag
 */
wf_sched_response_t * 
AbsWfMetaSched::extract(int dag_id, wf_sched_response_t * wf_resp) {
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
