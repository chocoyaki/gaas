/****************************************************************************/
/* MetaDag is a structure containing multiple dags having                   */
/* inter-relationships                                                      */
/*                                                                          */
/* Author(s):                                                               */
/* - Benjamin Isnard (benjamin.isnard@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.3  2009/02/06 14:53:46  bisnard
 * make thread-safe
 *
 * Revision 1.2  2008/12/09 12:15:59  bisnard
 * pending instanciation handling (uses dag outputs for instanciation
 * of a functional wf)
 *
 * Revision 1.1  2008/12/02 10:07:07  bisnard
 * new classes for functional workflow instanciation and execution
 *
 */

#include <string>
#include <map>
#include "debug.hh"
#include "MetaDag.hh"

using namespace std;

MetaDag::MetaDag(const string& id) : myId(id), dagTodoCount(0), currDag(NULL)
                                     , releaseFlag(true) {}

MetaDag::~MetaDag() {
  // delete the dags map
  while (! myDags.empty() ) {
    Dag * p = myDags.begin()->second;
    myDags.erase( myDags.begin() );
    delete p;
  }
}

const string&
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
MetaDag::getDag(const string& dagId) throw (WfStructException) {
  lock();
  map<string,Dag*>::iterator dagIter = myDags.find(dagId);
  unlock();
  if (dagIter != myDags.end()) {
    return (Dag*) dagIter->second;
  } else {
    string errorMsg = "cannot find dag in metadag (dag id = " + dagId + ")";
    throw WfStructException(WfStructException::eUNKNOWN_DAG, errorMsg);
  }
}

void
MetaDag::removeDag(const string& dagId) throw (WfStructException) {
  lock();
  map<string,Dag*>::iterator dagIter = myDags.find(dagId);
  if (dagIter != myDags.end()) {
    myDags.erase(dagIter);
    unlock();
  } else {
    unlock();
    string errorMsg = "cannot find dag in metadag (dag id = " + dagId + ")";
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
    INTERNAL_ERROR("Conflict for access to MetaDag",0);
  }
  lock();
  this->currDag = dag;
  unlock();
}

void
MetaDag::setReleaseFlag(bool release) {
  releaseFlag = release;
}

Node*
MetaDag::getNode(const string& nodeId) throw (WfStructException) {
  TRACE_TEXT (TRACE_ALL_STEPS,"Searching node " << nodeId
                               << " in Metadag " << myId << endl);
  Dag * dag = NULL;
  string baseNodeId;
  string::size_type dagSep  = nodeId.find(":");
  if (dagSep != string::npos) {
    dag = getDag(nodeId.substr(0,dagSep));
    baseNodeId = nodeId.substr(dagSep+1, nodeId.length()-dagSep);
  } else {
    if (currDag == NULL) {
      INTERNAL_ERROR("Error: no default dag defined in metadag",1);
    }
    dag = currDag;
    baseNodeId = nodeId;
  }
  return dag->getNode(baseNodeId);
}

void
MetaDag::checkPrec(NodeSet* contextNodeSet) throw (WfStructException) {
  INTERNAL_ERROR("MetaDag::checkPrec NOT IMPLEMENTED",0);
}

void
MetaDag::handlerDagDone(Dag * dag) {
  lock();
  dagTodoCount--;
  unlock();
}

bool
MetaDag::isDone() {
  if (!releaseFlag)
    return false;
  else {
    lock();
    bool noDagTodo = (dagTodoCount == 0);
    unlock();
    return noDagTodo;
  }
}

void
MetaDag::lock() {
  myLock.lock();
}

void
MetaDag::unlock() {
  myLock.unlock();
}


