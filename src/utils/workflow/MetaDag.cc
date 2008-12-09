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
  TRACE_TEXT (TRACE_ALL_STEPS,"Adding dag " << dag->getId()
                              << " to Metadag " << myId << endl);
  myDags[dag->getId()] = dag;
  dagTodoCount++;
}

bool
MetaDag::removeDag(Dag * dag) {
  TRACE_TEXT (TRACE_ALL_STEPS,"Removing dag " << dag->getId()
                              << " from Metadag " << myId << endl);
  map<string,Dag*>::iterator dagIter = myDags.find(dag->getId());
  if (dagIter != myDags.end()) {
    myDags.erase(dagIter);
    return true;
  } else return false;
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
  this->currDag = dag;
}

void
MetaDag::setReleaseFlag(bool release) {
  releaseFlag = release;
}

Node*
MetaDag::getNode(const string& nodeId) {
  TRACE_TEXT (TRACE_ALL_STEPS,"Searching node " << nodeId
                               << " in Metadag" << myId << endl);
  Dag * dag = NULL;
  string baseNodeId;
  string::size_type dagSep  = nodeId.find(":");
  if (dagSep != string::npos) {
    string dagId = nodeId.substr(0,dagSep);
    baseNodeId = nodeId.substr(dagSep+1, nodeId.length()-dagSep);
    map<string,Dag*>::iterator dagIter = myDags.find(dagId);
    if (dagIter != myDags.end()) {
      dag = (Dag*) dagIter->second;
    } else {
      TRACE_TEXT (TRACE_ALL_STEPS,"Error: cannot find dag for external ref (dag id = "
                                  << dagId << ")" << endl);
      return NULL;
    }
  } else {
    dag = currDag;
    if (dag == NULL) {
      TRACE_TEXT (TRACE_ALL_STEPS,"Error: no default dag defined in metadag" << endl);
      return NULL;
    }
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
  dagTodoCount--;
}

bool
MetaDag::isDone() {
  return (dagTodoCount == 0) && releaseFlag;
}

