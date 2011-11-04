/**
* @file DagNodeLauncher.cc
* 
* @brief  TThe virtual class used for dagNode execution 
* 
* @author  Benjamin ISNARD (Benjamin.Isnard@ens-lyon.fr)
* 
* @section Licence
*   |LICENSE|                                                                
*/


#include "DagNodeLauncher.hh"
#include "DagNode.hh"

DagNodeLauncher::DagNodeLauncher(DagNode * parent,
                                 DagScheduler * scheduler)
  : myNode(parent), myDagScheduler(scheduler),
    isSeDDefinedFlag(false), myReqID(0),
    isSuccessfulFlag(false) {
  myChosenServer = NULL;
}

void
DagNodeLauncher::setSeD(const char* sed, const unsigned long reqID,
                        corba_estimation_t& ev) {
  myChosenServer = CORBA::string_dup(sed);
  myReqID = reqID;
  myEstimVect = ev;  // COPY of estimation vector (required)
  isSeDDefinedFlag = true;
}

// FIXME should return the hostname not the dataMgr name (temp for gwendia exp.)
std::string
DagNodeLauncher::getSeDName() const {
  if (isSeDDefined()) {
    return myChosenServer;
  } else {
    return "";
  }
}


std::string
DagNodeLauncher::toString() const {
  std::string s("DagNode Launcher (");
  s += myNode->getId();
  s += ")";
  return s;
}


void *
DagNodeLauncher::run() {
  execNode();
  if (isSuccessfulFlag) {
    myNode->setAsDone(myDagScheduler);
  } else {
    myNode->setAsFailed(myDagScheduler);
  }
  finishNode();
  return NULL;
};

void
DagNodeLauncher::finishNode() {
}
