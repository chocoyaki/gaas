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
 * Revision 1.10  2008/12/02 10:14:51  bisnard
 * modified nodes links mgmt to handle inter-dags links
 *
 * Revision 1.9  2008/11/14 09:35:28  bdepardo
 * Bug correction in the multiple port adapter initialization
 *
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

#include <sstream>
#include "DagWfParser.hh"
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
 * Base descructor
 */
WfPortAdapter::~WfPortAdapter() {}

/**
 * Static factory method for adapters
 * Note: the caller is responsible for freeing the memory
 */
WfPortAdapter*
WfPortAdapter::createAdapter(const string& strRef) {
  string::size_type refSep = strRef.find("(");
  if (refSep == string::npos) {
    return new WfSimplePortAdapter(strRef);
  } else {
    string::size_type refSepLast = strRef.rfind(")");
    if (refSepLast == string::npos) {
      INTERNAL_ERROR("No closing bracket in " << strRef << std::endl, 1);
    }
    return new WfMultiplePortAdapter(strRef.substr(refSep+1, refSepLast-1));
  }
}

/**
 * Constructor for simple port adapter
 * PARSING of a simple reference (with or without subports)
 */
WfSimplePortAdapter::WfSimplePortAdapter(const string& strRef) : dagName() {
  TRACE_TEXT (TRACE_ALL_STEPS,"Creating simple port adapter ref=" << strRef << endl);
  string::size_type nodeSep = strRef.find(":");
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
    throw XMLParsingException(XMLParsingException::eINVALID_REF,
              "Invalid port reference (missing #) : " + strRef);
  }
}

WfSimplePortAdapter::WfSimplePortAdapter(WfPort * port,
                                         const string& portDagName) {
  TRACE_TEXT (TRACE_ALL_STEPS,"Creating simple port adapter TO port "
                              << port->getId() << endl);
  nodePtr  = port->getParent();
  portPtr  = port;
  nodeName = nodePtr->getId();
  portName = portPtr->getId();
  dagName  = portDagName;
}

WfSimplePortAdapter::WfSimplePortAdapter(WfSimplePortAdapter* parentAdapter,
                                         unsigned int index) {
  TRACE_TEXT (TRACE_ALL_STEPS,"Creating simple port adapter using parent adapter "
                               << endl);
  portPtr  = parentAdapter->getSourcePort();
  nodePtr  = portPtr->getParent();
  nodeName = nodePtr->getId();
  portName = portPtr->getId();
  dagName  = parentAdapter->getDagName();
  eltIdxList = parentAdapter->getElementIndexes();
  eltIdxList.push_back(index);
}

WfSimplePortAdapter::~WfSimplePortAdapter() {}

string
WfSimplePortAdapter::getSourceRef() const {
  stringstream ss;
  if (!dagName.empty()) {
    ss << dagName << ":";
  }
  ss << nodeName << "#" << portName;
  for (list<unsigned int>::const_iterator idxIter = eltIdxList.begin();
       idxIter != eltIdxList.end();
       ++idxIter) {
    ss << "[" << (unsigned int) *idxIter << "]";
  }
  return ss.str();
}

bool
WfSimplePortAdapter::setNodePrecedence(Node* node, NodeSet* nodeSet) {
  // create the full node name (including dag prefix if needed)
  string dagPrefix;
  if (!dagName.empty()) {
    dagPrefix = dagName + ":";
  }
  // get the node pointer from the nodeSet
  nodePtr = nodeSet->getNode(dagPrefix + nodeName);
  if (nodePtr != NULL) {
    node->addNodePredecessor(nodePtr, dagPrefix + nodePtr->getId());
    return true;
  } else {
    TRACE_TEXT (TRACE_ALL_STEPS,"Linked node not found! (" << nodeName << ")" << endl);
    return false;
  }
}

/**
 * Initializes the connection between two WfPorts
 * This method searches the remote port then updates its own pointer
 * and calls the method WfPort::connectToPort on both sides
 */
void
WfSimplePortAdapter::connectPorts(WfPort* port) {
  if (nodePtr) {
    WfPort * linkedPort = nodePtr->getPort(portName);
    if (linkedPort != NULL) {
      this->portPtr = linkedPort;       // SET the port ref FOR THE ADAPTER
      port->connectToPort(linkedPort);  // SET the connection on my port
      linkedPort->connectToPort(port);  // SET the connection on remote port
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

WfPort *
WfSimplePortAdapter::getSourcePort() {
  return this->portPtr;
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

const string&
WfSimplePortAdapter::getDagName() const {
  return dagName;
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

WfMultiplePortAdapter::WfMultiplePortAdapter() {
  TRACE_TEXT (TRACE_ALL_STEPS,"Creating multiple ports adapter (empty)" << endl);
}

WfMultiplePortAdapter::WfMultiplePortAdapter(const WfMultiplePortAdapter& mpa) {
  cout << "WfMultiplePortAdapter: COPY CONSTRUCTOR NOT DEFINED" << endl;
}

WfMultiplePortAdapter::~WfMultiplePortAdapter() {
  cout << "~WfMultiplePortAdapter" << endl;
  // Free the adapters list
  while (! adapters.empty() ) {
    WfPortAdapter * p = adapters.front();
    adapters.pop_front();
    delete p;
  }
}

void
WfMultiplePortAdapter::addSubAdapter(WfPortAdapter* subAdapter) {
  TRACE_TEXT (TRACE_ALL_STEPS,"Adding subadapter to multiple port adapter" << endl);
  adapters.push_back(subAdapter);
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
WfMultiplePortAdapter::connectPorts(WfPort* port) {
  for (list<WfPortAdapter*>::iterator iter = adapters.begin();
       iter != adapters.end();
       ++iter) {
    (*iter)->connectPorts(port);
  }
}

string WfMultiplePortAdapter::errorID = string("ID_Error");

string
WfMultiplePortAdapter::getSourceRef() const {
  string s = "(";
  list<WfPortAdapter*>::const_iterator adaptIter = adapters.begin();
  while (adaptIter != adapters.end()) {
    s += ((WfPortAdapter*) *adaptIter)->getSourceRef();
    if (++adaptIter != adapters.end())
      s += ";";
  }
  s += ")";
  return s;
}

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
     TRACE_TEXT(TRACE_ALL_STEPS, "## merging " << idElt << " into " << idCont << std::endl);
     dagda_add_container_element(idCont,idElt.c_str(),ix++);
  }
  containerID = idCont;
  TRACE_TEXT (TRACE_ALL_STEPS,"## End of merge ports" << endl);
#else
  ERROR("WfMultiplePortAdapter Error: trying to use containers without Dagda enabled" << endl, errorID);
#endif
  return containerID;
}
