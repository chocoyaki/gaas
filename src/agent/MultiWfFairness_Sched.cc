/****************************************************************************/
/* Another multi-workflow scheduler                                         */
/* This scheduler is based on Fairness On Finish Time algorithm             */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$ 
 * $Log$
 * Revision 1.4  2006/11/27 09:53:00  aamar
 * Correct headers of source files used in workflow support.
 *
 ****************************************************************************/

#include "debug.hh"
#include "marshalling.hh"

#include "MultiWfFairness_Sched.hh"
#include "RoundRobbin_MaDag_Sched.hh"
#include "HEFT_MaDag_Sched.hh"

MultiWfFairness_Sched::MultiWfFairness_Sched() {
  this->mySched = new HEFT_MaDag_Sched();
}
  
MultiWfFairness_Sched::~MultiWfFairness_Sched() {
  if (this->myMetaDag != NULL)
    delete this->myMetaDag;
  
  if (this->mySched != NULL)
    delete this->mySched;
}

/**
 * set the scheduler used by the MA DAG
 */
void 
MultiWfFairness_Sched::setSched(MaDag_sched * sched) {
  this->mySched = sched;
}

/**
 * Workflow submission function. 
 */
wf_sched_response_t * 
MultiWfFairness_Sched::submit_wf (const corba_wf_desc_t& wf_desc, int dag_id,
				  MasterAgent_var parent,
				  const bool used) {
  wf_sched_response_t * wf_resp = new wf_sched_response_t;
  wf_resp->dag_id = dag_id;
  // The submited new dag
  Dag * newDag = NULL;
  // The meta dag as a Dag object
  Dag * metaDag = NULL;

  wf_resp->dag_id = dag_id;
  // Parse the XML of the new DAG
  WfExtReader reader(wf_desc.abstract_wf);
  reader.setup();
  newDag = reader.getDag();
  newDag->setId(itoa(dag_id));

  // Send all the profiles (contained in the Meta-dag) to the MA
  metaDag = this->myMetaDag->getDag();
  metaDag->setAsTemp(true);
  vector<diet_profile_t*> v1 = metaDag->getAllProfiles();
  vector<diet_profile_t*> v2 = newDag->getAllProfiles();
  unsigned int len = v1.size() + v2.size();
  corba_pb_desc_seq_t pbs_seq;
  pbs_seq.length(len);
  for (unsigned int ix=0; ix< v1.size(); ix++) {
    mrsh_pb_desc(&pbs_seq[ix], v1[ix]);
  }
  for (unsigned int ix=0; ix< v2.size(); ix++) {
    mrsh_pb_desc(&pbs_seq[ix+v1.size()], v2[ix]);
  }

  TRACE_FUNCTION(TRACE_ALL_STEPS,
		 "MultiWfFairness contacts the MA  ... " << endl);
  wf_response_t * wf_response = parent->submit_pb_set(pbs_seq, 
						      len,
						      used);
  TRACE_FUNCTION(TRACE_ALL_STEPS,
		 "... submit_pb_set done" << endl);

  wf_resp->dag_id = wf_response->dag_id;
  wf_resp->firstReqID = wf_response->firstReqID;
  wf_resp->lastReqID = wf_response->lastReqID;
  wf_resp->ma_response = *wf_response;

  // construct the response/scheduling
  if ( ! wf_response->complete) {
    WARNING("The response of the MA for dag " << dag_id << 
	    " is incomplete" << endl);
    return wf_resp;
  }

  // By default use the Round Robbin scheduler
  if (this->mySched == NULL) {
    this->mySched = new RoundRobbin_MaDag_Sched();
  }

  // Schedule each dag alone 
  Dag * dag = NULL;
  vector<Dag*> allDags = this->myMetaDag->getAllDags();
  for (unsigned int ix=0; ix<allDags.size(); ix++) {
    dag = allDags[ix];
    TRACE_FUNCTION(TRACE_ALL_STEPS,
		   "scheduling the dag " << dag->getId() << endl);
    this->sOwn[dag] = this->mySched->schedule(wf_response, dag);
  }
  // schedule the new Dag
  this->sOwn[newDag] = this->mySched->schedule(wf_response, newDag);
  wf_resp->wf_node_sched_seq = this->sOwn[newDag];

  // Now we can add the new dag to the meta-dag
  this->myMetaDag->addDag(newDag);

  // Init the scheduling
  init();

  // Schedule the meta-dag
  fairnessOnFinishTime(wf_response);

  // Based on the sMulti construct the workflow scheduling
  string dagIdStr(itoa(dag_id));
  Node * node = NULL;
  unsigned index = 0;
  for (map<Node*, wf_node_sched_t>::iterator p = this->sMulti.begin();
       p != this->sMulti.end();
       p++) {
    node = (p->first);
    string id(node->getId());
    if (id.substr(0, id.find("-")) == dagIdStr) {
      wf_resp->wf_node_sched_seq.length(index+1);
      wf_resp->wf_node_sched_seq[index].node_id = 
	CORBA::string_dup(id.substr(id.find("-")+1).c_str());
      wf_resp->wf_node_sched_seq[index].server = 
	this->sMulti[node].server;
      cout << "++++++++++++++++==" << index << id << endl;
      index++;
    }
  }
  // Construct the new scheduling to other clients
  map<string, wf_node_sched_seq_t> remainingSched;
  string id("");
  string dagId("");
  for (map<Node*, wf_node_sched_t>::iterator p = this->sMulti.begin();
       p != this->sMulti.end();
       p++) {
    node = (p->first);
    id = node->getId();
    dagId = id.substr(0, id.find("-"));
    len = remainingSched[dagId].length();
    remainingSched[dagId].length(len + 1 );
    remainingSched[dagId][len].node_id = 
      CORBA::string_dup(id.substr(id.find("-")+1).c_str());
    remainingSched[dagId][len].server = 
      this->sMulti[node].server;
  }
  // Send the scheduling to the other clients
  CltReoMan_var clt = NULL;
  for (map<string, CltReoMan_var>::iterator p = this->myClients.begin();
       p != this->myClients.end();
       ++p) {
    dagId = p->first;
    clt = (CltReoMan_var)(p->second);
    map<string, wf_node_sched_seq_t>::iterator q = 
      remainingSched.find(dagId);
    if ( (q != remainingSched.end()) &&
	 (clt != NULL)) {
      cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$ Sending a new Scheduling " << endl;
      clt->remainingSched(q->second);
    }
  }
  

  // release the Meta Dag
  if (metaDag != NULL)
    delete (metaDag);

  return wf_resp;
}

/**
 * Init the scheduling
 */
void
MultiWfFairness_Sched::init() {
  TRACE_FUNCTION(TRACE_ALL_STEPS,
		 "MultiWfFairness ::init ...  " << endl);
  Dag * dag = NULL;

  DagState initState;
  // The next three instruction are not necessary (see default constructor)
  initState.executed = false;
  initState.makespan = -1;
  initState.slowdown = 0;

  NodeState nodeInitState;
  // The next three instruction are not necessary (see default constructor)
  nodeInitState.executed = false;
  nodeInitState.multiFT = 0;
  nodeInitState.ownFT = 0;

  // Mark each dag as unexecuted
  // set the slowdown of each dag as 0
  vector<Dag*> v = this->myMetaDag->getAllDags();
  unsigned int len = v.size();
  cout << "+++++++++++++ " <<
    __FUNCTION__ << " init state of each DAG " <<
    len << "(" << this->myMetaDag->getLength() << ")" << endl;

  for (unsigned int ix=0; ix<len; ix++) {
    initState.makespan = v[ix]->getEstMakespan();
    this->dagsState[v[ix]] = initState;
    // Insert the dag in U sorting in descending order of makespan
    vector<Dag*>::iterator p = this->U.begin();
    while ( (p != this->U.end()) &&
	    (dag != NULL) && // this condition depends on the previous one
	    (dag->getEstMakespan() > v[ix]->getEstMakespan()) ) {
      dag = (Dag*)(*p);
      p++;
    } 
    this->U.insert(p, v[ix]);

  // mark each task in each dag as unexecuted
    vector<Node *> dagNodes = v[ix]->getNodes();
    for (unsigned int jx=0; jx<dagNodes.size(); jx++) {
      cout << "+++++++++++++ " <<
	__FUNCTION__ << " init state of " <<
	dagNodes[jx]->getId() << endl;
      this->nodesState[dagNodes[jx]] = nodeInitState;
    }
  } // end for ix
  
  TRACE_FUNCTION(TRACE_ALL_STEPS,
		 "MultiWfFairness ::init ... done " << endl);
} // end init

void
MultiWfFairness_Sched::fairnessOnFinishTime(const wf_response_t * wf_response) {
  TRACE_FUNCTION(TRACE_ALL_STEPS,
		 "MultiWfFairness ::fairnessOnFinishTime ... \n");
  Dag * dag = NULL;
  Node * node = NULL;
  while (this->U.size() != 0) {
    cout << "U.size() = " << U.size() << endl;
    dag = *(this->U.begin());
    cout << "processing dag " << dag->getId() << endl;
    vector<Node *> dagNodes = dag->getNodes();
    node = get1stReadyNode(dagNodes);
    
    if (node == NULL) {
      cout << "+++++++++++++++++ " << __FUNCTION__ <<
	" node is NULL for dag " << dag->getId() << " which contains " <<
	dagNodes.size() << " nodes" << endl;
      this->U.erase( this->U.begin());
      continue;
    }
    // store finish time in Sown
    cout << "store finish time in Sown of " << node->getId() << endl;
    this->nodesState[node].ownFT =
      this->mySched->getAFT(node->getId());
    // Schedule the node using mySched and construct the response
    
    wf_node_sched_t node_sched = this->mySched->schedule(wf_response,
							 node);
    // Add the node_sched to S_multi
    this->sMulti[node] = node_sched;
    this->dagsState[dag].executedNodes++;
    cout << "dagId = " << dag->getId() <<
      ", this->dagsState[dag].executedNodes = " <<
      this->dagsState[dag].executedNodes <<
      ", dag->size() = " << 
      dag->size() << endl;
    if (this->dagsState[dag].executedNodes >= dag->size()) {
      this->U.erase( this->U.begin());
    }
    else {
      this->nodesState[node].multiFT = 
	this->mySched->getAFT(node->getId());
      // The FTown is already calculated
      this->dagsState[dag].slowdown = 
	this->nodesState[node].ownFT/this->nodesState[node].multiFT;
      cout << "sortU ... " << endl;
      sortU();
      cout << "... done" << endl;
    }
  }
  TRACE_FUNCTION(TRACE_ALL_STEPS,
		 "MultiWfFairness ::fairnessOnFinishTime ... done\n");
} // end FairnessOnFinishTime

Node *
MultiWfFairness_Sched::get1stReadyNode(vector<Node*>& v) {
  bool ready = false;
  unsigned int len = v.size();
  unsigned int nbPrev = 0;
  Node * prevNode = NULL;
  for (unsigned int ix=0; ix<len; ix++) {
    if (this->nodesState[v[ix]].executed == false) {
      cout << "+++++++++++++ " <<
	__FUNCTION__ << " checking " << v[ix]->getId() << " " <<
	ix << "/" << len << endl;
      ready = true;
      nbPrev = v[ix]->prevNb();
      cout << "+++++++++++++ " <<
	__FUNCTION__ << " checking " << nbPrev << " previous nodes of " <<
	v[ix]->getId() << endl;
      for (unsigned int jx=0; jx<nbPrev; jx++) {
	prevNode = (Node*)(v[ix]->getPrev(jx));
	cout << "+++++++++++++ " <<
	  __FUNCTION__ << " checking " << prevNode->getId() << 
	  " (a previous nodes of " <<  v[ix]->getId() << endl;
	if ( (prevNode != NULL) &&
	     (this->nodesState.find(prevNode) != this->nodesState.end()) &&
	     (this->nodesState[prevNode].executed == false) ) {
	  ready = false;
	  break;
	}
	else {
	  cout << "++++++++++++++++++" <<
	    prevNode->getId() << " is not ready" << endl;
	}
      } // end for jx
      if (ready) {      
	this->nodesState[v[ix]].executed = true;
	return v[ix];
      }
    }
  } // end for ix
  
  /** must not be reached since the dag is removed when the last node is
   * schedule */
  return NULL;
}

void
MultiWfFairness_Sched::sortU() {
  Dag * dag = NULL;
  DagState dagState;
  this->U.clear();
  for (map<Dag*, DagState>::iterator p = this->dagsState.begin();
       p != this->dagsState.end();
       ++p) {
    dag = (Dag*)(p->first);
    dagState = p->second;
    vector<Dag*>::iterator q = this->U.begin();
    Dag * dag2 = NULL;
    while ( (q != this->U.end()) &&
	    (dag2 != NULL) && // this condition depends on the previous one
	    (dag2->getEstMakespan() < dag->getEstMakespan()) ) {
      dag2 = (Dag*)(*q);
      q++;
    } 
    this->U.insert(q, dag);
  }
} // end sortU
/** 
 * DagState default constructor
 */
DagState::DagState() {
  this->executed = false;
  this->makespan = -1;
  this->slowdown = 0;
  this->executedNodes = 0;
}

/**
 * NodeState default constructo
 */
NodeState::NodeState() {
  this->executed = false;
  this->multiFT = 0;
  this->ownFT = 0;

}

/**
 * Remove a client from myClients map
 */
void
MultiWfFairness_Sched::removeClient(const string dagId) {
  // get the dag reference before the dag is remove from the meta-dag (next instruction)
  Dag * dag = this->myMetaDag->getDag(dagId);
  AbsWfMetaSched::removeClient(dagId);
  cout << "MultiWfFairness_Sched::removeClient" << dag->getId() <<
    endl;
  vector<Dag*>::iterator p = find(this->U.begin(),
				  this->U.end(),
				  dag);
  if (p != this->U.end())
    this->U.erase(p);
  else 
    cerr << "Can't find the DAG " << dagId << endl;
  
  if (this->sOwn.find(dag) != this->sOwn.end())
    this->sOwn.erase(this->sOwn.find(dag));
  else 
    cerr << "Can't find the DAG  " << dagId << 
      " in sOwn" << endl;
  
  if (this->dagsState.find(dag) != this->dagsState.end())
    this->dagsState.erase(this->dagsState.find(dag));
  else 
    cerr << "Can't find the DAG " << dagId << 
      " in dagsState " << endl;
}

/**
 * Set the node state as done
 */
void 
MultiWfFairness_Sched::setNodeAsDone(const char* dagId, const char* nodeId) {
  AbsWfMetaSched::setNodeAsDone(dagId, nodeId);
}

/**
   TO DO

   - In Workflow Scheduler : add the following function
   schedule(const wf_response_t * response, Node * node);   
        - Added in HEFT scheduler. Need to be implemented in RoundRobbin

   - Schedule the node using mySched and construct the response

   - this->nodesState[node].multiFT = ;
   - this->nodesState[node].ownFT = ;
   - Sort U

*/
