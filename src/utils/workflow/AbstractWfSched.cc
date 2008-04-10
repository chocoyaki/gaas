/****************************************************************************/
/* Workflow scheduler (Abstract class)                                      */
/* This abstract class must be implemented to write other workflow          */
/* schedulers                                                               */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.1  2008/04/10 08:38:50  bisnard
 * New version of the MaDag where workflow node execution is triggered by the MaDag agent and done by a new CORBA object CltWfMgr located in the client
 *
 * Revision 1.4  2006/11/27 10:15:12  aamar
 * Correct headers of files used in workflow support.
 *
 * Revision 1.3  2006/11/06 11:53:48  aamar
 * Workflow support:
 *   - Add a new setResponse function.
 *   - Changing the response attribute type.
 *
 * Revision 1.2  2006/07/10 11:15:29  aamar
 * Adding the rescheduling management
 *
 * Revision 1.1  2006/04/14 14:03:17  aamar
 * Abstract class that defines the workflow scheduler interface.
 *
 ****************************************************************************/

#include "AbstractWfSched.hh"

map<string, SeDSynchro> AbstractWfSched::mySeDSynchro;

AbstractWfSched::AbstractWfSched() {
  this->myDag = NULL;
  //  this->response = NULL;
  this->myCltReoMan = NULL;
}

AbstractWfSched::~AbstractWfSched() {
}

/**
 * Assign a dag to the scheduler *
 * implicitely called by the runtime *
 */
void
AbstractWfSched::setDag(Dag * dag) {
  this->myDag = dag;
  //  this->response = NULL;
  //
}


/**
 * Set the workflow submission response *
 * implicitely called by the runtime *
 */
void 
AbstractWfSched::setResponse(wf_response_t *response) {
  this->response = *response;
}


/**
 * Another version of the above function *
 * Used when we use the MA DAG
 */
void 
AbstractWfSched::setResponse(wf_sched_response_t * response) {
  this->response = response->ma_response;
}
/**
 * set the client reordering manager
 */
void
AbstractWfSched::setCltReoMan(CltReoMan_impl * crm) {
  this->myCltReoMan = crm;
}

/**
 * Rescheduling methods
 */
void
AbstractWfSched::reSchedule() {
  // nothing to do
}

/**
 * Get access to a SeD
 */
void
AbstractWfSched::getAccess(std::string hostName) {
  if (mySeDSynchro.find(hostName) == mySeDSynchro.end()) {
    SeDSynchro synchro;
    mySeDSynchro[hostName] = synchro;
  }
  else {
    mySeDSynchro.find(hostName)->second.getAccess();
  }
} // end getAccess

/**
 * Release a SeD
 */
void
AbstractWfSched::pop(std::string node_id) {
	for (map<string, SeDSynchro>::iterator p = mySeDSynchro.begin(); 
	p != mySeDSynchro.end();
	++p) {
		if (p->second.top() == node_id) {
			p->second.pop();
			return;
		}
	} // end for
} // end release

/**
 * Get the nodes in the top of each SeD allocation queue
 */
void
AbstractWfSched::getTopSeDQueues(std::vector<string>& topQueues) {
	for (map<string, SeDSynchro>::iterator p = mySeDSynchro.begin(); 
	p != mySeDSynchro.end();
	p++) {
		if (p->second.getLength() > 0) {
			topQueues.push_back(p->second.top());
		} // end if
	} // end for
} // end getTopSeDQueues

/**
 * Put the node in the SeD queue
 * 
 * @param node_id  Node identifier
 * @param hostName SeD hostname
 */
void
AbstractWfSched::allocateToSeD(std::string node_id, std::string hostName) {
	if (mySeDSynchro.find(hostName) == mySeDSynchro.end()) {
		SeDSynchro synchro;
		mySeDSynchro[hostName] = synchro;
	}	
	mySeDSynchro[hostName].add(node_id);
} // end allocateToSeD
