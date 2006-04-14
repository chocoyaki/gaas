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
  Node * n = NULL;
  cout << "The simple scheduler starts execution " << endl;
  cout << "Linking the dag nodes ... " << endl;
  myDag->linkAllPorts();
  cout << "linking is done " << endl;

  while (true) {
    // get the ready nodes
    vector <Node*> readyNodes = myDag->getReadyNodes();
    unsigned len = readyNodes.size();
    for (uint ix=0; ix<len; ix++) {
      n = readyNodes[ix];
      n->start();
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
