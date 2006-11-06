/****************************************************************************/
/* Basic Workflow Scheduler                                                 */
/* This scheduler use only the data dependencies to execute the workflow    */
/* The order between ready nodes is random                                  */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.5  2006/11/06 15:16:06  aamar
 * Workflow support: some correction about reqID.
 *
 * Revision 1.4  2006/11/06 12:00:26  aamar
 * *** empty log message ***
 *
 * Revision 1.3  2006/11/02 17:13:53  rbolze
 * add some commented code to be change
 *
 * Revision 1.2  2006/07/10 11:11:08  aamar
 * Adding time and rescheduling management
 *
 * Revision 1.1  2006/04/14 14:04:51  aamar
 * The basis scheduler (random scheduling).
 *
 ****************************************************************************/

#include <unistd.h>
#include <iostream>
#include "SimpleWfSched.hh"

using namespace std;

SimpleWfSched::SimpleWfSched() {
}

SimpleWfSched::~SimpleWfSched() {
}

/**
 * The scheduler execution method *
 * implicitely called by the runtime *
 */
void
SimpleWfSched::execute() {
  if (response.wfn_seq_resp.length() != 0) {
    cout << "---------------------" << endl;
    cout << "Display the response " << endl;
    for (unsigned int ix=0; 
	 ix < this->response.wfn_seq_resp.length(); 
	 ix++) {
      corba_response_t cr = this->response.wfn_seq_resp[ix].response;
      for (unsigned int jx=0; jx < cr.servers.length(); jx++) {
	corba_server_estimation_t cs_estim = cr.servers[jx];
	for (unsigned kx =0; kx < cs_estim.estim.estValues.length(); kx++) {
	  cout << "ix = " << ix <<
	    ", jx = " << jx <<
	    ", kx = " << kx << 
	    ", v_tag = " << cs_estim.estim.estValues[kx].v_tag << 
	    ", v_idx = " << cs_estim.estim.estValues[kx].v_idx << 
	    ", v_value = " << cs_estim.estim.estValues[kx].v_value << endl;
	}
      }
    }
  } // end if

  cout << "---------------------" << endl;
  cout << "The simple scheduler starts execution " << endl;
  cout << "Linking the dag nodes ... " << endl;
  myDag->linkAllPorts();
  cout << "linking is done " << endl;

  struct timeval tv;
  gettimeofday(&tv, NULL);
  myDag->setTheBeginning(tv);
  CORBA::Long reqID = response.firstReqID;

  // Map the results 
  // After the changes recommanded by Raph, we have a response by node
  // So the mapping done by the MA and we choose the first server
  // Normally the node in the dag and the response are ordered
  Node * n = NULL;
  for (unsigned int ix=0; ix<this->response.wfn_seq_resp.length(); ix++) {
    for (std::map <std::string, Node *>::iterator p = myDag->begin();
	 p != myDag->end();
	 p++) {
      Node * n = (Node*)(p->second);
      if (( n->getSeD() == SeD::_nil()) &&
	  (!strcmp(n->getPb().c_str(), 
		   this->response.wfn_seq_resp[ix].node_id))) {
	n->setSeD(this->response.wfn_seq_resp[ix].response.servers[0].loc.ior);
	break;
      }
    }    
  }

  while (true) {
    // get the ready nodes
    vector <Node*> readyNodes = myDag->getReadyNodes();
    unsigned len = readyNodes.size();
    for (uint ix=0; ix<len; ix++) {
      n = readyNodes[ix];
      // n->start();
      n->start(reqID++);
    }
    /*
    cout << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%" << endl;
    cout << myDag -> toXML();
    cout << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%" << endl;
    */
    if ((this->myCltReoMan != NULL) && (!myDag->checkScheduling())) {
      cout << "the scheduling is incorrect" << endl << 
	"try to reschedule" << endl;
    }
    if (readyNodes.size() == 0) {
      if (myDag->isDone())
	break;
      else {
	// no ready nodes but some nodes are running
	usleep(50);
      }
    }
  }

  cout << "... workflow execution done " << endl;
}
