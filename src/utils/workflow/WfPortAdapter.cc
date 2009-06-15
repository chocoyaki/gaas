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
 * Revision 1.14  2009/06/15 12:13:12  bisnard
 * added data type handling in WfPortAdapter
 * added new class WfValueAdapter to avoid data duplication
 *
 * Revision 1.13  2009/05/15 11:10:20  bisnard
 * release for workflow conditional structure (if)
 *
 * Revision 1.12  2009/04/17 09:02:15  bisnard
 * container empty elements (added WfVoidAdapter class)
 *
 * Revision 1.11  2009/02/06 14:54:43  bisnard
 * - setup exceptions
 * - added data type & depth check
 *
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
#include "WfNode.hh"
#include "WfPort.hh"
#include "DagNode.hh"
#include "debug.hh"
#if HAVE_DAGDA
extern "C" {
#include "DIET_Dagda.h"
}
#endif

using namespace std;

string WfMultiplePortAdapter::parLeftChar = "(";
string WfMultiplePortAdapter::parRightChar = ")";
string WfMultiplePortAdapter::separatorChar = ";";


/*****************************************************************************/
/*                        WfPortAdapter (ABSTRACT)                           */
/*****************************************************************************/

/**
 * Base descructor
 */
WfPortAdapter::~WfPortAdapter() {}

/**
 * Static factory method for adapters
 * Note: the caller is responsible for freeing the memory
 * Example of input: (nodeA#port1;nodeB#port2[0][1];#UNDEF;#VALDEB#toto#VALFIN#)
 */
WfPortAdapter*
WfPortAdapter::createAdapter(const string& strRef) {
  string::size_type refSep = strRef.find("(");
  if (refSep == string::npos) {
    string::size_type testVoid = strRef.find(WfVoidAdapter::voidRef);
    if (testVoid == string::npos) {
      string::size_type startValue = strRef.find(WfValueAdapter::valStartTag);
      if (startValue == string::npos) {
        string::size_type startID = strRef.find(WfValueAdapter::IDStartTag);
        if (startID == string::npos) {
          // case of reference to a dag port
          return new WfSimplePortAdapter(strRef);
        } else {
          // case of data ID
          startID += WfValueAdapter::IDStartTag.length();
          string::size_type endID = strRef.find(WfValueAdapter::IDFinishTag);
          return new WfValueAdapter(strRef.substr(startID, endID - startID),"");
        }
      } else {
        // case of data value
        startValue += WfValueAdapter::valStartTag.length();
        string::size_type endValue = strRef.find(WfValueAdapter::valFinishTag);
        return new WfValueAdapter(strRef.substr(startValue, endValue - startValue));
      }
    } else {
      // case of VOID data
      return new WfVoidAdapter();
    }
  } else {
    // case of container data
    string::size_type refSepLast = strRef.rfind(")");
    if (refSepLast == string::npos) {
      INTERNAL_ERROR("No closing bracket in " << strRef << endl, 1);
    }
    return new WfMultiplePortAdapter(strRef.substr(refSep+1, refSepLast-1));
  }
}

/*****************************************************************************/
/*                        WfSimplePortAdapter                                */
/*****************************************************************************/

/**
 * Constructor for simple port adapter
 * PARSING of a simple reference (with or without subports)
 */
WfSimplePortAdapter::WfSimplePortAdapter(const string& strRef)
  : dagName() {
//   TRACE_TEXT (TRACE_ALL_STEPS,"Creating simple port adapter ref=" << strRef << endl);
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
//   TRACE_TEXT (TRACE_ALL_STEPS,"Creating simple port adapter TO port "
//                               << port->getId() << endl);
  nodePtr  = port->getParent();
  portPtr  = port;
  nodeName = nodePtr->getId();
  portName = portPtr->getId();
  dagName  = portDagName;
}

WfSimplePortAdapter::WfSimplePortAdapter(WfPort* port,
                                         const list<unsigned int>& indexes,
                                         const string& portDagName) {
  nodePtr  = port->getParent();
  portPtr  = port;
  nodeName = nodePtr->getId();
  portName = portPtr->getId();
  dagName  = portDagName;
  eltIdxList = indexes;
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

void
WfSimplePortAdapter::setNodePrecedence(WfNode* node, NodeSet* nodeSet) throw (WfStructException) {
  // create the full node name (including dag prefix if needed)
  string dagPrefix;
  if (!dagName.empty()) {
    dagPrefix = dagName + ":";
  }
  // get the node pointer from the nodeSet
  nodePtr = nodeSet->getNode(dagPrefix + nodeName);
  node->addNodePredecessor(nodePtr, dagPrefix + nodePtr->getId());
}

/**
 * Initializes the connection between two WfPorts
 * This method searches the remote port then updates its own pointer
 * and calls the method WfPort::connectToPort on both sides
 * The adapterLevel parameter is used only to check depth compatibility between
 * the adapter and the linked port
 */
void
WfSimplePortAdapter::connectPorts(WfPort* port, unsigned int adapterLevel)
    throw (WfStructException)
{
  if (nodePtr == NULL) {
    INTERNAL_ERROR(__FUNCTION__ << "NULL node pointer" << endl, 1);
  }
  WfPort * linkedPort = nodePtr->getPort(portName);
  string errorMsg = "connect " + port->getCompleteId()
                       + " to " + getSourceRef();

  // check data type compatibility
  if (port->getBaseDataType() != linkedPort->getBaseDataType())
    throw WfStructException(WfStructException::eTYPE_MISMATCH,errorMsg);

  this->portPtr = linkedPort;       // SET the port ref FOR THE ADAPTER
  port->connectToPort(linkedPort);  // SET the connection on my port
  linkedPort->connectToPort(port);  // SET the connection on remote port

  // check data depth compatibility
  if (port->getDepth() != adapterLevel + linkedPort->getDepth() - getDepth())
    throw WfStructException(WfStructException::eDEPTH_MISMATCH,errorMsg);
}

DagNodeOutPort*
WfSimplePortAdapter::getSourcePort() const {
  DagNodeOutPort* p = dynamic_cast<DagNodeOutPort*>(portPtr);
  if (!p) {
    INTERNAL_ERROR(__FUNCTION__ << " used with adapter to incorrect port type" << endl, 1);
  }
  return p;
}

const string&
WfSimplePortAdapter::getSourceDataID() {
  if (!dataID.empty()) return dataID;
  if (getDepth() == 0)
    dataID = getSourcePort()->getDataID();
  else
    dataID = getSourcePort()->getElementDataID(eltIdxList);
  return dataID;
}

WfCst::WfDataType
WfSimplePortAdapter::getSourceDataType() {
  return getSourcePort()->getDataType(getDepth());
}

unsigned int
WfSimplePortAdapter::getSourceDataCardinal() {
  return getSourcePort()->getDataIDCardinal(getSourceDataID());
}

void
WfSimplePortAdapter::writeData(WfDataWriter* dataWriter) {
  if (getDepth() == 0)
    getSourcePort()->writeData(dataWriter);
  else
    getSourcePort()->writeDataElement(dataWriter, eltIdxList);
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
WfSimplePortAdapter::getDepth() const {
  return eltIdxList.size();
}

const list<unsigned int>&
WfSimplePortAdapter::getElementIndexes() {
  return eltIdxList;
}

/*****************************************************************************/
/*                       WfMultiplePortAdapter                               */
/*****************************************************************************/

/**
 * Constructor for multiple port adapter
 * Builds a hierarchy of adapters
 * Example of input: (nodeA#port1;nodeB#port1[0]);(nodeC#port1;#UNDEF)
 * (note that createAdapter strips the toplevel parenthesis)
 */
WfMultiplePortAdapter::WfMultiplePortAdapter(const string& strRef) {
//   TRACE_TEXT (TRACE_ALL_STEPS,"Creating multiple ports adapter ref=/" << strRef << "/" << endl);
  string::size_type refStart = 0;
  parse(strRef, refStart);
}

void WfMultiplePortAdapter::parse(const string& strRef,
                                  string::size_type& startPos) {
//   cout << "parse adapter string at position " << itoa(startPos) << endl;
  while (startPos < strRef.length()) {
    string::size_type parLeft  = strRef.find(parLeftChar,startPos);
    string::size_type parRight = strRef.find(parRightChar,startPos);
    string::size_type sepRight = strRef.find(separatorChar,startPos);
    if (parLeft < sepRight) { // multiple refs inside ( )
      startPos = parLeft+1;
      WfMultiplePortAdapter* mulAd = new WfMultiplePortAdapter();
      mulAd->parse(strRef, startPos);
      addSubAdapter(mulAd);
      // re-initialize separator position (because startPos changed)
      sepRight = strRef.find(separatorChar,startPos);
      parRight = strRef.find(parRightChar,startPos);

    } else {  // simple ref
      string::size_type refEnd = (parRight < sepRight) ? parRight-1 :
                                 (sepRight == string::npos) ? strRef.length()-1 : sepRight-1;
      WfPortAdapter* adapt = createAdapter(strRef.substr(startPos, refEnd-startPos+1));
      addSubAdapter(adapt);
    }
    // if followed by a ; => skip it and continue parsing
    if (sepRight < parRight)
      startPos = sepRight + 1;
    // in other cases i.e. ) or nothing, this is the end of the current adapter
    else {
      startPos = (parRight == string::npos) ? strRef.length()-1 : parRight + 1;
      break;
    }
  } // end while
//   cout << "end of parse - pos = " << startPos << endl;
}

WfMultiplePortAdapter::WfMultiplePortAdapter() {
//   cout << "creating empty multipleAdapter" << endl;
}

WfMultiplePortAdapter::WfMultiplePortAdapter(const WfMultiplePortAdapter& mpa) {
  // WfMultiplePortAdapter: COPY CONSTRUCTOR NOT DEFINED
}

WfMultiplePortAdapter::~WfMultiplePortAdapter() {
  // Free the adapters list
  while (! adapters.empty() ) {
    WfPortAdapter * p = adapters.front();
    adapters.pop_front();
    delete p;
  }
}

void
WfMultiplePortAdapter::addSubAdapter(WfPortAdapter* subAdapter) {
//   TRACE_TEXT (TRACE_ALL_STEPS,"Adding child adapter to multiple adapter" << endl);
  adapters.push_back(subAdapter);
}

void
WfMultiplePortAdapter::setNodePrecedence(WfNode* node, NodeSet* nodeSet)
    throw (WfStructException)
{
  for (list<WfPortAdapter*>::iterator iter = adapters.begin();
       iter != adapters.end();
       ++iter) {
    (*iter)->setNodePrecedence(node, nodeSet);
  }
}

void
WfMultiplePortAdapter::connectPorts(WfPort* port, unsigned int adapterLevel)
    throw (WfStructException)
{
  for (list<WfPortAdapter*>::iterator iter = adapters.begin();
       iter != adapters.end();
       ++iter) {
    (*iter)->connectPorts(port, adapterLevel+1);
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
  if (!containerID.empty()) return containerID;
  // First check if all adapters have either their ID defined or are VOID
  // (will throw exception if one is not defined)
  for (list<WfPortAdapter*>::iterator iter = adapters.begin();
         iter != adapters.end();
         ++iter) {
     try {
      (*iter)->getSourceDataID();
     } catch (WfDataException& e) {
       if (e.Type() != WfDataException::eVOID_DATA)
         throw;
     }
  }
  // If ok then create container to merge all adapters
  char* idCont;
#if HAVE_DAGDA
  TRACE_TEXT (TRACE_ALL_STEPS,"## Creating container to merge ports" << endl);
  dagda_create_container(&idCont);
  int ix=0;
  for (list<WfPortAdapter*>::iterator iter = adapters.begin();
       iter != adapters.end();
       ++iter) {
     try {

      const string& idElt = (*iter)->getSourceDataID();
      TRACE_TEXT(TRACE_ALL_STEPS, "## merging " << idElt << " into " << idCont << endl);
      dagda_add_container_element(idCont,idElt.c_str(),ix++);

     } catch (WfDataException& e) {
        if (e.Type() == WfDataException::eVOID_DATA) {
          TRACE_TEXT(TRACE_ALL_STEPS, "## merging NULL elt into " << idCont << endl);
          dagda_add_container_null_element(idCont, ix++);
        }
     }
  }
  containerID = idCont;
  TRACE_TEXT (TRACE_ALL_STEPS,"## End of merge ports" << endl);
#else
  ERROR("WfMultiplePortAdapter Error: trying to use containers without Dagda enabled" << endl, errorID);
#endif
  return containerID;
}

WfCst::WfDataType
WfMultiplePortAdapter::getSourceDataType() {
  return WfCst::TYPE_CONTAINER;
}

unsigned int
WfMultiplePortAdapter::getSourceDataCardinal() {
  getSourceDataID(); // to initialize data if not already done
  return adapters.size();
}

void
WfMultiplePortAdapter::writeData(WfDataWriter* dataWriter) {
  dataWriter->startContainer();
  for (list<WfPortAdapter*>::const_iterator adaptIter = adapters.begin();
       adaptIter != adapters.end();
       ++adaptIter) {
     ((WfPortAdapter*) *adaptIter)->writeData(dataWriter);
  }
  dataWriter->endContainer();
}

/*****************************************************************************/
/*                            WfVoidAdapter                                  */
/*****************************************************************************/

WfVoidAdapter::WfVoidAdapter() {
//   TRACE_TEXT (TRACE_ALL_STEPS,"Creating VOID adapter" << endl);
}

WfVoidAdapter::~WfVoidAdapter() {}

string WfVoidAdapter::voidRef = string("#UNDEF");

void
WfVoidAdapter::setNodePrecedence(WfNode* node, NodeSet* nodeSet)
throw (WfStructException) {
}

void
WfVoidAdapter::connectPorts(WfPort* port, unsigned int adapterLevel)
throw (WfStructException) {
}

string
WfVoidAdapter::getSourceRef() const {
  return voidRef;
}

const string&
WfVoidAdapter::getSourceDataID() {
  throw WfDataException(WfDataException::eVOID_DATA,"");
}

WfCst::WfDataType
WfVoidAdapter::getSourceDataType() {
  throw WfDataException(WfDataException::eVOID_DATA,"");
}

unsigned int
WfVoidAdapter::getSourceDataCardinal() {
  return 1;
}

void
WfVoidAdapter::writeData(WfDataWriter* dataWriter) {
  dataWriter->voidElement();
}

/*****************************************************************************/
/*                          WfValueAdapter                                   */
/*****************************************************************************/

string WfValueAdapter::valStartTag = string("#VALDEB#");
string WfValueAdapter::valFinishTag = string("#VALFIN#");
string WfValueAdapter::IDStartTag = string("#IDDEB#");
string WfValueAdapter::IDFinishTag = string("#IDFIN#");

WfValueAdapter::WfValueAdapter(const string& value)
  : myValue(value), myDataType(WfCst::TYPE_UNKNOWN),
  cx(NULL), sx(NULL), ix(NULL), lx(NULL), fx(NULL), dx(NULL), str(NULL)
{
  TRACE_TEXT (TRACE_ALL_STEPS,"Creating VALUE adapter value=/"
                               << value << "/" << endl);
}

WfValueAdapter::WfValueAdapter(const string& dataID, string value)
  : myValue(value), myDataID(dataID), myDataType(WfCst::TYPE_UNKNOWN),
  cx(NULL), sx(NULL), ix(NULL), lx(NULL), fx(NULL), dx(NULL), str(NULL)
{
  TRACE_TEXT (TRACE_ALL_STEPS,"Creating VALUE adapter dataID=/"
                               << dataID << "/" << endl);
}

WfValueAdapter::WfValueAdapter(WfCst::WfDataType valueType, const string& value)
  : myValue(value), myDataType(valueType),
  cx(NULL), sx(NULL), ix(NULL), lx(NULL), fx(NULL), dx(NULL), str(NULL)
{
  TRACE_TEXT (TRACE_ALL_STEPS,"Creating VALUE adapter (TYPE=" << valueType
                               << ") value=/" << value << "/" << endl);
}

WfValueAdapter::WfValueAdapter(const string& dataID,
                               WfCst::WfDataType valueType,
                               string value)
  : myDataID(dataID), myValue(value), myDataType(valueType),
    cx(NULL), sx(NULL), ix(NULL), lx(NULL), fx(NULL), dx(NULL), str(NULL)  {
  TRACE_TEXT (TRACE_ALL_STEPS,"Creating VALUE adapter (TYPE=" << valueType
                              << ") dataID=/" << dataID << "/" << endl);
}

WfValueAdapter::~WfValueAdapter() {
  if (cx) delete cx;
  if (sx) delete sx;
  if (ix) delete ix;
  if (lx) delete lx;
  if (fx) delete fx;
  if (dx) delete dx;
  if (str) delete str;
}

void
WfValueAdapter::setNodePrecedence(WfNode* node, NodeSet* nodeSet)
throw (WfStructException) {
}

void
WfValueAdapter::connectPorts(WfPort* port, unsigned int adapterLevel)
throw (WfStructException) {
}

string
WfValueAdapter::getSourceRef() const {
  if (myDataID.empty())
    return valStartTag + myValue + valFinishTag;
  else
    return IDStartTag + myDataID + IDFinishTag;
}

const string&
WfValueAdapter::getSourceDataID() {
  if (!myDataID.empty()) return myDataID;
  char *valID;
  char **valIDPtr = &valID;
#if HAVE_DAGDA
  switch (myDataType) {
    case WfCst::TYPE_CHAR:
      dagda_put_scalar(newChar(), DIET_CHAR, DIET_PERSISTENT, valIDPtr);
      break;
    case WfCst::TYPE_SHORT:
      dagda_put_scalar(newShort(), DIET_SHORT, DIET_PERSISTENT, valIDPtr);
      break;
    case WfCst::TYPE_INT:
      dagda_put_scalar(newInt(), DIET_INT, DIET_PERSISTENT, valIDPtr);
      break;
    case WfCst::TYPE_LONGINT:
      dagda_put_scalar(newLong(), DIET_LONGINT, DIET_PERSISTENT, valIDPtr);
      break;
    case WfCst::TYPE_FLOAT:
      dagda_put_scalar(newFloat(), DIET_FLOAT, DIET_PERSISTENT, valIDPtr);
      break;
    case WfCst::TYPE_DOUBLE:
      dagda_put_scalar(newDouble(), DIET_DOUBLE, DIET_PERSISTENT, valIDPtr);
      break;
    case WfCst::TYPE_PARAMSTRING:
      dagda_put_paramstring(newString(), DIET_PERSISTENT, valIDPtr);
      break;
    case WfCst::TYPE_STRING:
      dagda_put_string(newString(), DIET_PERSISTENT, valIDPtr);
      break;
    case WfCst::TYPE_FILE:
      dagda_put_file(newFile(), DIET_PERSISTENT, valIDPtr);
      break;
    default:
      throw WfDataException(WfDataException::eWRONGTYPE,
                          "Cannot initialize data due to unknown type in value adapter");
  } // end (switch)
  myDataID = valID;
#else
  ERROR("WfValueAdapter Error: Dagda not enabled" << endl, errorID);
#endif
  return myDataID;
}

WfCst::WfDataType
WfValueAdapter::getSourceDataType() {
  return myDataType;
}

unsigned int
WfValueAdapter::getSourceDataCardinal() {
  WARNING("Cannot get cardinal of a WfValueAdapter");
  return 0;
}

void
WfValueAdapter::writeData(WfDataWriter* dataWriter) {
  dataWriter->itemValue(myValue);
}

char *
WfValueAdapter::newChar() {
    cx = new char;
    *cx = myValue.c_str()[0];
  return cx;
}

short *
WfValueAdapter::newShort() {
  sx = new short;
  *sx = atoi(myValue.c_str());
  return sx;
}

int *
WfValueAdapter::newInt() {
  ix = new int;
  *ix = atoi(myValue.c_str());
  return ix;
}

long *
WfValueAdapter::newLong() {
  lx = new long;
  *lx = atoi(myValue.c_str());
  return lx;
}

char *
WfValueAdapter::newString() {
  str = new char[myValue.size()+1];
  strcpy(str, myValue.c_str());
  return str;
}

char *
WfValueAdapter::newFile() {
  str = new char[myValue.size()+1];
  strcpy(str, myValue.c_str());
  return str;
}

float *
WfValueAdapter::newFloat() {
  fx = new float;
  *fx = (float) atof(myValue.c_str());
  return fx;
}

double *
WfValueAdapter::newDouble() {
  dx = new double;
  *dx = atof(myValue.c_str());
  return dx;
}


