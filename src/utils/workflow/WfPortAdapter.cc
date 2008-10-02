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
        uint idx = atoi(strRef.substr(idxListLeft+1, idxListRight-idxListLeft-1).c_str());
        this->eltIdxList.push_back(idx);
        idxListLeft = strRef.find("[",idxListRight);
      }
    }
  } else {
    // TODO throw exception (# not found)
  }
}

bool
WfSimplePortAdapter::setNodePredecessors(Node* node, Dag* dag) {
  // if dagName different from dag's name, get the node from [dagName]
  nodePtr = dag->getNode(nodeName);
  if (nodePtr) {
    node->addPrevId(nodePtr->getId());
    return true;
  } else return false;
}

void
WfSimplePortAdapter::setPortDataLinks(WfInPort* inPort, Dag* dag) {
  if (nodePtr) {
    WfOutPort * out = nodePtr->getOutPort(portName);
    if (out != NULL) {
      this->portPtr = out;      // SET the SOURCE link
      out->setSink(inPort);     // SET the SINK link (used to determine if out port is a result)
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
  if (portPtr == NULL) {
    ERROR("WfSimplePortAdapter Error: invalid port reference" << endl, dataID);
  }
  // use the data ID of the source port itself
  dataID = portPtr->getDataID();
  // look for element's data ID in case of a container
  if (depth() > 0) {
#if HAVE_DAGDA
    const list<uint>& eltIdx = this->getElementIndexes();
    for (list<uint>::const_iterator idxIter = eltIdx.begin();
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

uint
WfSimplePortAdapter::depth() {
  return eltIdxList.size();
}

const list<uint>&
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
WfMultiplePortAdapter::setNodePredecessors(Node* node, Dag* dag) {
  for (list<WfPortAdapter*>::iterator iter = adapters.begin();
       iter != adapters.end();
       ++iter) {
    if (!((*iter)->setNodePredecessors(node, dag)))
      return false;
  }
  return true;
}

void
WfMultiplePortAdapter::setPortDataLinks(WfInPort* inPort, Dag* dag) {
  for (list<WfPortAdapter*>::iterator iter = adapters.begin();
       iter != adapters.end();
       ++iter) {
    (*iter)->setPortDataLinks(inPort, dag);
  }
}

static string errorID = string("ID_Error");

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
