/**
* @file  MetaDagr.cc
* 
* @brief  A structure containing multiple dags having inter-relationships
* 
* @author  Benjamin ISNARD (Benjamin.Isnard@ens-lyon.fr)
* 
* @section Licence
*   |LICENCE|                                                                
*/


#include <map>
#include <string>
#include "debug.hh"
#include "MetaDag.hh"
#include "Dag.hh"

MetaDag::MetaDag(const std::string& id)
  : myId(id),  currDag(NULL), dagTodoCount(0), releaseFlag(true),
    cancelFlag(false) {
}

MetaDag::~MetaDag() {
  // delete the dags map
  while (!myDags.empty()) {
    Dag * p = myDags.begin()->second;
    myDags.erase(myDags.begin());
    delete p;
  }
}

const std::string&
MetaDag::getId() {
  return myId;
}

void
MetaDag::addDag(Dag * dag) {
  lock();
  myDags[dag->getId()] = dag;
  dagTodoCount++;
  unlock();
}

Dag *
MetaDag::getDag(const std::string& dagId) throw(WfStructException) {
  lock();
  std::map<std::string, Dag*>::iterator dagIter = myDags.find(dagId);
  unlock();
  if (dagIter != myDags.end()) {
    return (Dag*) dagIter->second;
  } else {
    std::string errorMsg =
      "cannot find dag in metadag (dag id = " + dagId + ")";
    throw WfStructException(WfStructException::eUNKNOWN_DAG, errorMsg);
  }
}

void
MetaDag::removeDag(const std::string& dagId) throw(WfStructException) {
  lock();
  std::map<std::string, Dag*>::iterator dagIter = myDags.find(dagId);
  if (dagIter != myDags.end()) {
    myDags.erase(dagIter);
    unlock();
  } else {
    unlock();
    std::string errorMsg =
      "cannot find dag in metadag (dag id = " + dagId + ")";
    throw WfStructException(WfStructException::eUNKNOWN_DAG, errorMsg);
  }
}

int
MetaDag::getDagNb() {
  return myDags.size();
}

void
MetaDag::setCurrentDag(Dag * dag) {
  if ((dag != NULL) && (currDag != NULL)) {
    INTERNAL_ERROR("Conflict for access to MetaDag", 0);
  }
  lock();
  this->currDag = dag;
  unlock();
}

void
MetaDag::setReleaseFlag(bool release) {
  lock();
  releaseFlag = release;
  unlock();
}

void
MetaDag::setReleaseFlag(bool release, bool& isDone) {
  lock();
  releaseFlag = release;
  isDone = release && (dagTodoCount == 0);
  unlock();
}

WfNode*
MetaDag::getNode(const std::string& nodeId) throw(WfStructException) {
  Dag * dag = NULL;
  std::string baseNodeId;
  std::string::size_type dagSep  = nodeId.find(":");
  if (dagSep != std::string::npos) {
    dag = getDag(nodeId.substr(0, dagSep));
    baseNodeId = nodeId.substr(dagSep+1, nodeId.length()-dagSep);
  } else {
    if (currDag == NULL) {
      INTERNAL_ERROR("Error: no default dag defined in metadag", 1);
    }
    dag = currDag;
    baseNodeId = nodeId;
  }
  return dag->getNode(baseNodeId);
}

void
MetaDag::checkPrec(NodeSet* contextNodeSet) throw(WfStructException) {
  INTERNAL_ERROR("MetaDag::checkPrec NOT IMPLEMENTED", 0);
}

void
MetaDag::handlerDagDone(Dag * dag) {
  lock();
  dagTodoCount--;
  unlock();
}

bool
MetaDag::isDone() {
  bool isDone = false;
  lock();
  if (releaseFlag) {
    isDone = (dagTodoCount == 0);
  }
  unlock();
  return isDone;
}

void
MetaDag::cancelAllDags(DagScheduler * scheduler) {
  if (cancelFlag) {
    return;
  }
  lock();
  TRACE_TEXT(TRACE_ALL_STEPS, "Cancelling all dags of Metadag "
             << myId << "\n");
  std::map<std::string, Dag*>::iterator dagIter = myDags.begin();
  for (; dagIter != myDags.end(); ++dagIter) {
    Dag *currDag = (Dag*) dagIter->second;
    if (!currDag->isDone()) {
      TRACE_TEXT(TRACE_ALL_STEPS, "Cancelling dag "
                 << currDag->getId() << "\n");
      currDag->setAsCancelled(scheduler);
    }
  }
  cancelFlag = true;
  unlock();
}

void
MetaDag::lock() {
  myLock.lock();
}

void
MetaDag::unlock() {
  myLock.unlock();
}


