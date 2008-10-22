/****************************************************************************/
/* Port adapter classes used to split or merge container content            */
/*                                                                          */
/*                                                                          */
/* Author(s):                                                               */
/* - Benjamin Isnard (benjamin.isnard@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.8  2008/10/22 09:29:00  bisnard
 * replaced uint by standard type
 *
 * Revision 1.7  2008/10/14 13:31:01  bisnard
 * new class structure for dags (DagNode,DagNodePort)
 *
 * Revision 1.6  2008/10/02 09:10:51  bisnard
 * incorrect constant definition
 *
 * Revision 1.5  2008/10/02 07:35:10  bisnard
 * new constants definitions (matrix order and port type)
 *
 * Revision 1.4  2008/09/30 15:32:53  bisnard
 * - using simple port id instead of composite ones
 * - dag nodes linking refactoring
 * - prevNodes and nextNodes data structures modified
 * - prevNodes initialization by Node::setNodePredecessors
 *
 * Revision 1.3  2008/09/30 09:23:29  bisnard
 * removed diet profile initialization from DagWfParser and replaced by node methods initProfileSubmit and initProfileExec
 *
 * Revision 1.2  2008/09/19 14:01:30  bisnard
 * allow compile wf support with or without DAGDA
 *
 * Revision 1.1  2008/09/19 13:16:07  bisnard
 * New class to manage containers split/merge in workflows
 *
 */

#include "Dag.hh"
#include "Node.hh"
#include "WfPort.hh"
#include "debug.hh"
#if HAVE_DAGDA
extern "C" {
#include "DIET_Dagda.h"
}
#endif

using namespace std;

/**
 * Static factory method for adapters
 */
WfPortAdapter*
WfPortAdapter::createAdapter(const string& strRef) {
  string::size_type refSep = strRef.find(";");
  if (refSep == string::npos) {
    return new WfSimplePortAdapter(strRef);
  } else {
    return new WfMultiplePortAdapter(strRef);
  }
}

/**
 * Constructor for simple port adapter
 * PARSING of a simple reference (with or without subports)
 */
WfSimplePortAdapter::WfSimplePortAdapter(const string& strRef) {
  TRACE_TEXT (TRACE_ALL_STEPS,"Creating simple port adapter ref=" << strRef << endl);
  string::size_type nodeSep = strRef.find("%");
  string::size_type nodeStart = 0;
  if (nodeSep != string::npos) {
    this->dagName = strRef.substr(0, nodeSep);
    nodeStart = nodeSep + 1;
  }
  string::size_type portSep = strRef.find("#",nodeStart);
  if (portSep != string::npos) {
    this->nodeName = strRef.substr(nodeStart, portSep-nodeStart);
    string::size_type idxListLeft = strRef.find("[");
    if (idxListLeft == string::npos) {
      this->portName = strRef.substr(portSep+1);
    } else {
      this->portName = strRef.substr(portSep+1, idxListLeft-portSep-1);
      while (idxListLeft != string::npos) {
        string::size_type idxListRight = strRef.find("]",idxListLeft);
        if (idxListRight == string::npos) {
          // throw exception (brackets not closed)
        }
        unsigned int idx = atoi(strRef.substr(idxListLeft+1, idxListRight-idxListLeft-1).c_str());
        this->eltIdxList.push_back(idx);
        idxListLeft = strRef.find("[",idxListRight);
      }
    }
  } else {
    // TODO throw exception (# not found)
  }
}

bool
WfSimplePortAdapter::setNodePrecedence(Node* node, NodeSet* nodeSet) {
  // if dagName different from dag's name, get the node from [dagName]
  nodePtr = nodeSet->getNode(nodeName);
  if (nodePtr) {
    node->addPrevId(nodePtr->getId());
    return true;
  } else return false;
}

void
WfSimplePortAdapter::connectPorts(WfPort* port, NodeSet* nodeSet) {
  if (nodePtr) {
    WfPort * linkedPort = nodePtr->getPort(portName);
    if (linkedPort != NULL) {
      this->portPtr = linkedPort;      // SET the SOURCE link
      linkedPort->setAsConnected();    // SET the linked port as connected (used to determine
                                       // if the port is a result or an input
      // if external link (different dags) then use setExternalSink
    } else {
      INTERNAL_ERROR("FATAL ERROR:" << endl << "Cannot find linked port '"
          << nodeName + "#" + portName << "'" << endl, 1);
    }
  } else {
    INTERNAL_ERROR("FATAL ERROR" << endl <<
        "Cannot find linked node : " << nodeName, 1);
  }
}

const string&
WfSimplePortAdapter::getSourceDataID() {
  DagNodeOutPort* dagNodeOutPortPtr = dynamic_cast<DagNodeOutPort*>(portPtr);
  if (dagNodeOutPortPtr == NULL) {
    ERROR("WfSimplePortAdapter Error: invalid port reference" << endl, dataID);
  }
  // use the data ID of the source port itself
  dataID = dagNodeOutPortPtr->getDataID();
  // look for element's data ID in case of a container
  if (depth() > 0) {
#if HAVE_DAGDA
    const list<unsigned int>& eltIdx = this->getElementIndexes();
    for (list<unsigned int>::const_iterator idxIter = eltIdx.begin();
         idxIter != eltIdx.end();
         ++idxIter) {
      if (!dataID.empty()) {
        dagda_get_container(dataID.c_str());
        diet_container_t content;
        dagda_get_container_elements(dataID.c_str(), &content);
        if (content.size >= (*idxIter + 1)) {
          dataID = content.elt_ids[*idxIter];
        } else {
          ERROR("WfSimplePortAdapter Error: cannot find container item" << endl, dataID);
        }
      } else {
        ERROR("WfSimplePortAdapter Error: empty data ID" << endl, dataID);
      }
    }
#else
    ERROR("WfSimplePortAdapter Error: trying to use containers without Dagda enabled" << endl, dataID);
#endif
  }
  return dataID;
}

const string&
WfSimplePortAdapter::getPortName() const {
  return portName;
}

const string&
WfSimplePortAdapter::getNodeName() const {
  return nodeName;
}

unsigned int
WfSimplePortAdapter::depth() {
  return eltIdxList.size();
}

const list<unsigned int>&
WfSimplePortAdapter::getElementIndexes() {
  return eltIdxList;
}

/**
 * Constructor for multiple port adapter
 * Builds a hierarchy of adapters
 */
WfMultiplePortAdapter::WfMultiplePortAdapter(const string& strRef) {
  TRACE_TEXT (TRACE_ALL_STEPS,"Creating multiple ports adapter ref=[" << strRef << "]" << endl);
  string::size_type refStart = 0;
  while (refStart < strRef.length()) {
    string::size_type parLeft  = strRef.find("(",refStart);
    string::size_type refSep   = strRef.find(";",refStart);
    if (parLeft < refSep) { // multiple refs inside ( )
      string::size_type parRight = strRef.find(")",refStart);
      if (parRight != string::npos) {
        WfMultiplePortAdapter* mulAd =
            new WfMultiplePortAdapter(strRef.substr(parLeft+1, parRight-parLeft-1));
        adapters.push_back(mulAd);
        refStart = parRight+2; // right ) is either followed by ; or is at the end
      } else {
      // exception non-closed parenthesis
      }
    } else {  // simple ref
      string::size_type refEnd = (refSep == string::npos) ? strRef.length()-1 : refSep-1;
      WfSimplePortAdapter* splAd =
           new WfSimplePortAdapter(strRef.substr(refStart, refEnd-refStart+1));
      adapters.push_back(splAd);
      refStart = refEnd+2;
    }
  } // end while
}

bool
WfMultiplePortAdapter::setNodePrecedence(Node* node, NodeSet* nodeSet) {
  for (list<WfPortAdapter*>::iterator iter = adapters.begin();
       iter != adapters.end();
       ++iter) {
    if (!((*iter)->setNodePrecedence(node, nodeSet)))
      return false;
  }
  return true;
}

void
WfMultiplePortAdapter::connectPorts(WfPort* port, NodeSet* nodeSet) {
  for (list<WfPortAdapter*>::iterator iter = adapters.begin();
       iter != adapters.end();
       ++iter) {
    (*iter)->connectPorts(port, nodeSet);
  }
}

string WfMultiplePortAdapter::errorID = string("ID_Error");

const string&
WfMultiplePortAdapter::getSourceDataID() {
  char* idCont;
#if HAVE_DAGDA
  TRACE_TEXT (TRACE_ALL_STEPS,"## Creating container to merge ports" << endl);
  dagda_create_container(&idCont);
  int ix=0;
  for (list<WfPortAdapter*>::iterator iter = adapters.begin();
       iter != adapters.end();
       ++iter) {
     const string& idElt = (*iter)->getSourceDataID();
     dagda_add_container_element(idCont,idElt.c_str(),ix++);
  }
  containerID = idCont;
  TRACE_TEXT (TRACE_ALL_STEPS,"## End of merge ports" << endl);
#else
  ERROR("WfMultiplePortAdapter Error: trying to use containers without Dagda enabled" << endl, errorID);
#endif
  return containerID;
}
