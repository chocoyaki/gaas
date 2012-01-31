/**
* @file  WfPortAdapter.cc
*
* @brief  Port adapter classes used to split or merge container content
*
* @author  Benjamin ISNARD (benjamin.isnard@ens-lyon.fr)
*
* @section Licence
*   |LICENCE|
*/


#include <sstream>
#include <boost/format.hpp>
#include <boost/scoped_ptr.hpp>
#include "debug.hh"

// DIET core headers
extern "C" {
#include "DIET_Dagda.h"
}

// DIET workflows headers

#include "WfPortAdapter.hh"
#include "DagWfParser.hh"
#include "Dag.hh"
#include "DagNode.hh"
#include "DagNodePort.hh"
#include "EventTypes.hh"
#include "FNodePort.hh"


std::string WfMultiplePortAdapter::parLeftChar("(");
std::string WfMultiplePortAdapter::parRightChar(")");
std::string WfMultiplePortAdapter::separatorChar(";");


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
WfPortAdapter::createAdapter(const std::string& strRef) {
  std::string::size_type refSep = strRef.find("(");
  if (refSep == std::string::npos) {
    std::string::size_type testVoid = strRef.find(WfVoidAdapter::voidRef);
    if (testVoid == std::string::npos) {
      std::string::size_type startValue =
        strRef.find(WfValueAdapter::valStartTag);
      if (startValue == std::string::npos) {
        std::string::size_type startID =
          strRef.find(WfDataIDAdapter::IDStartTag);
        if (startID == std::string::npos) {
          // case of reference to a dag port
          return new WfSimplePortAdapter(strRef);
        } else {
          // case of data ID
          startID += WfDataIDAdapter::IDStartTag.length();
          std::string::size_type endID =
            strRef.find(WfDataIDAdapter::IDFinishTag);
          return new WfDataIDAdapter(strRef.substr(startID, endID - startID));
        }
      } else {
        // case of data value
        startValue += WfValueAdapter::valStartTag.length();
        std::string::size_type endValue =
          strRef.find(WfValueAdapter::valFinishTag);
        return new WfValueAdapter(strRef.substr(startValue,
                                                endValue - startValue));
      }
    } else {
      // case of VOID data
      return new WfVoidAdapter();
    }
  } else {
    // case of container data
    std::string::size_type refSepLast = strRef.rfind(")");
    if (refSepLast == std::string::npos) {
      INTERNAL_ERROR("No closing bracket in " << strRef << "\n", 1);
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
WfSimplePortAdapter::WfSimplePortAdapter(const std::string& strRef)
  : nodePtr(NULL), portPtr(NULL) {
  std::string::size_type nodeSep = strRef.find(":");
  std::string::size_type nodeStart = 0;
  if (nodeSep != std::string::npos) {
    this->dagName = strRef.substr(0, nodeSep);
    nodeStart = nodeSep + 1;
  }
  std::string::size_type portSep = strRef.find("#", nodeStart);
  if (portSep != std::string::npos) {
    this->nodeName = strRef.substr(nodeStart, portSep-nodeStart);
    std::string::size_type idxListLeft = strRef.find("[");
    if (idxListLeft == std::string::npos) {
      this->portName = strRef.substr(portSep+1);
    } else {
      this->portName = strRef.substr(portSep+1, idxListLeft-portSep-1);
      while (idxListLeft != std::string::npos) {
        std::string::size_type idxListRight = strRef.find("]", idxListLeft);
        if (idxListRight == std::string::npos) {
          // throw exception (brackets not closed)
        }
        unsigned int idx =
          atoi(strRef.substr(idxListLeft+1,
                             idxListRight-idxListLeft-1).c_str());
        this->eltIdxList.push_back(idx);
        idxListLeft = strRef.find("[", idxListRight);
      }
    }
  } else {
    throw XMLParsingException(XMLParsingException::eINVALID_REF,
                              "Invalid port reference (missing #) : " + strRef);
  }
}

WfSimplePortAdapter::WfSimplePortAdapter(WfPort * port,
                                         const std::string& portDagName) {
  nodePtr  = port->getParent();
  portPtr  = port;
  nodeName = nodePtr->getId();
  portName = portPtr->getId();
  dagName  = portDagName;
}

WfSimplePortAdapter::WfSimplePortAdapter(WfPort* port,
                                         const std::list<unsigned int>& indexes,
                                         const std::string& portDagName) {
  nodePtr  = port->getParent();
  portPtr  = port;
  nodeName = nodePtr->getId();
  portName = portPtr->getId();
  dagName  = portDagName;
  eltIdxList = indexes;
}

WfSimplePortAdapter::~WfSimplePortAdapter() {}

std::string
WfSimplePortAdapter::getSourceRef() const {
  std::stringstream ss;
  if (!dagName.empty()) {
    ss << dagName << ":";
  }
  ss << nodeName << "#" << portName;
  for (std::list<unsigned int>::const_iterator idxIter = eltIdxList.begin();
       idxIter != eltIdxList.end();
       ++idxIter) {
    ss << "[" << (unsigned int) *idxIter << "]";
  }
  return ss.str();
}

void
WfSimplePortAdapter::setNodePrecedence(WfNode* node, NodeSet* nodeSet)
  throw(WfStructException) {
  // create the full node name (including dag prefix if needed)
  std::string dagPrefix;
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
  throw(WfStructException) {
  if (!nodePtr) {
    INTERNAL_ERROR(__FUNCTION__ << "NULL node pointer\n", 1);
  }
  WfPort *linkedPort = nodePtr->getPort(portName);
  std::string errorMsg = "connect " + port->getCompleteId()
    + " to " + linkedPort->getCompleteId();

  // check data type compatibility
  if (port->getBaseDataType() != linkedPort->getBaseDataType()) {
    throw WfStructException(WfStructException::eTYPE_MISMATCH, errorMsg);
  }

  this->portPtr = linkedPort;       // SET the port ref FOR THE ADAPTER
  if (port->getPortType() == WfPort::PORT_INOUT
      && NULL != dynamic_cast<FNodeInOutPort*>(port)
      && linkedPort->getPortType() == WfPort::PORT_INOUT
      && NULL != dynamic_cast<FNodeInOutPort*>(linkedPort)) {
    /* As two INOUT ports cannot internally disambiguate inOut to inOut link
     * we need to specifically set each side separately */
    /* SET the connection on my port, use WfPort::connectToPort as it is
     * an "IN" port */
    dynamic_cast<FNodeInOutPort*>(port)->WfPort::connectToPort(linkedPort);
    /* SET the connection on remote port, use FNodeOutPort::connectToPort as it
     * is an "OUT" port */
    dynamic_cast<FNodeInOutPort*>(linkedPort)->FNodeOutPort::connectToPort(port);
  } else {
    // SET the connection on my port
    port->connectToPort(linkedPort);
    // SET the connection on remote port
    linkedPort->connectToPort(port);
  }

  // check data depth compatibility
  if (port->getDepth() != adapterLevel + linkedPort->getDepth() - getDepth()) {
    throw WfStructException(WfStructException::eDEPTH_MISMATCH, errorMsg);
  }
}

DagNodeOutPort*
WfSimplePortAdapter::getSourcePort() const {
  DagNodeOutPort* p = dynamic_cast<DagNodeOutPort*>(portPtr);
  if (!p) {
    INTERNAL_ERROR(__FUNCTION__ << " used with adapter to incorrect port type"
                   << "\n", 1);
  }
  return p;
}

const std::string&
WfSimplePortAdapter::getSourceDataID() {
  if (!dataID.empty()) {
    return dataID;
  }

  if (getDepth() == 0) {
    dataID = getSourcePort()->getDataID();
  } else {
    dataID = getSourcePort()->getElementDataID(eltIdxList);
  }

  return dataID;
}

WfCst::WfDataType
WfSimplePortAdapter::getSourceDataType() {
  return getSourcePort()->getDataType(getDepth());
}

bool
WfSimplePortAdapter::isDataIDCreator() {
  return false;
}

void
WfSimplePortAdapter::writeDataValue(WfDataWriter* dataWriter) {
  if (getDepth() == 0) {
    getSourcePort()->writeData(dataWriter);
  } else {
    getSourcePort()->writeDataElement(dataWriter, eltIdxList);
  }
}

const std::string&
WfSimplePortAdapter::getPortName() const {
  return portName;
}

const std::string&
WfSimplePortAdapter::getNodeName() const {
  return nodeName;
}

const std::string&
WfSimplePortAdapter::getDagName() const {
  return dagName;
}

unsigned int
WfSimplePortAdapter::getDepth() const {
  return eltIdxList.size();
}

const std::list<unsigned int>&
WfSimplePortAdapter::getElementIndexes() {
  return eltIdxList;
}

void
WfSimplePortAdapter::freeAdapterPersistentData(MasterAgent_var& MA) {
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
WfMultiplePortAdapter::WfMultiplePortAdapter(const std::string& strRef) {
  std::string::size_type refStart = 0;
  parse(strRef, refStart);
}

void
WfMultiplePortAdapter::parse(const std::string& strRef,
                             std::string::size_type& startPos) {
  while (startPos < strRef.length()) {
    std::string::size_type parLeft  = strRef.find(parLeftChar, startPos);
    std::string::size_type parRight = strRef.find(parRightChar, startPos);
    std::string::size_type sepRight = strRef.find(separatorChar, startPos);
    if (parLeft < sepRight) {  // multiple refs inside ()
      startPos = parLeft+1;
      WfMultiplePortAdapter* mulAd = new WfMultiplePortAdapter();
      mulAd->parse(strRef, startPos);
      addSubAdapter(mulAd);
      // re-initialize separator position (because startPos changed)
      sepRight = strRef.find(separatorChar, startPos);
      parRight = strRef.find(parRightChar, startPos);

    } else {  // simple ref
      std::string::size_type refEnd = (parRight < sepRight) ? parRight-1 :
        (sepRight == std::string::npos) ? strRef.length()-1 : sepRight-1;
      WfPortAdapter* adapt =
        createAdapter(strRef.substr(startPos, refEnd-startPos+1));
      addSubAdapter(adapt);
    }
    // if followed by a; => skip it and continue parsing
    if (sepRight < parRight) {
      startPos = sepRight + 1;
      // in other cases i.e.) or nothing, this is the end of the current adapter
    } else {
      startPos = (parRight == std::string::npos) ?
        strRef.length()-1 : parRight + 1;
      break;
    }
  }
}

WfMultiplePortAdapter::WfMultiplePortAdapter() {
}

WfMultiplePortAdapter::WfMultiplePortAdapter(const WfMultiplePortAdapter& mpa) {
  // WfMultiplePortAdapter: COPY CONSTRUCTOR NOT DEFINED
}

WfMultiplePortAdapter::~WfMultiplePortAdapter() {
  // Free the adapters list
  while (!adapters.empty()) {
    WfPortAdapter *p = adapters.front();
    adapters.pop_front();
    delete p;
  }
}

void
WfMultiplePortAdapter::addSubAdapter(WfPortAdapter* subAdapter) {
  adapters.push_back(subAdapter);
}

void
WfMultiplePortAdapter::setNodePrecedence(WfNode* node, NodeSet* nodeSet)
  throw(WfStructException) {
  for (std::list<WfPortAdapter*>::iterator iter = adapters.begin();
       iter != adapters.end();
       ++iter) {
    (*iter)->setNodePrecedence(node, nodeSet);
  }
}

void
WfMultiplePortAdapter::connectPorts(WfPort* port, unsigned int adapterLevel)
  throw(WfStructException) {
  for (std::list<WfPortAdapter*>::iterator iter = adapters.begin();
       iter != adapters.end();
       ++iter) {
    (*iter)->connectPorts(port, adapterLevel+1);
  }
}

std::string WfMultiplePortAdapter::errorID("ID_Error");

std::string
WfMultiplePortAdapter::getSourceRef() const {
  std::string s("(");
  std::list<WfPortAdapter*>::const_iterator adaptIter = adapters.begin();
  while (adaptIter != adapters.end()) {
    s += ((WfPortAdapter*) *adaptIter)->getSourceRef();
    if (++adaptIter != adapters.end()) {
      s += ";";
    }
  }
  s += ")";
  return s;
}

const std::string&
WfMultiplePortAdapter::getSourceDataID() {
  if (!containerID.empty()) {
    return containerID;
  }
  // First check if all adapters have either their ID defined or are VOID
  // (will throw exception if one is not defined)
  for (std::list<WfPortAdapter*>::iterator iter = adapters.begin();
       iter != adapters.end();
       ++iter) {
    try {
      (*iter)->getSourceDataID();
    } catch (WfDataException& e) {
      if (e.Type() != WfDataException::eVOID_DATA) {
        throw;
      }
    }
  }
  // If ok then create container to merge all adapters
  char* idCont;
  TRACE_TEXT(TRACE_ALL_STEPS, "## Creating container to merge ports\n");
  dagda_create_container(&idCont);
  int ix = 0;
  for (std::list<WfPortAdapter*>::iterator iter = adapters.begin();
       iter != adapters.end();
       ++iter) {
    try {
      const std::string& idElt = (*iter)->getSourceDataID();
      TRACE_TEXT(TRACE_ALL_STEPS, "## merging "
                 << idElt << " into " << idCont << "\n");
      dagda_add_container_element(idCont, idElt.c_str(), ix++);
    } catch (WfDataException& e) {
      if (e.Type() == WfDataException::eVOID_DATA) {
        TRACE_TEXT(TRACE_ALL_STEPS, "## merging NULL elt into "
                   << idCont << "\n");
        dagda_add_container_null_element(idCont, ix++);
      } else {
        throw;
      }
    }
  }
  containerID = idCont;
  TRACE_TEXT(TRACE_ALL_STEPS, "## End of merge ports\n");
  return containerID;
}

WfCst::WfDataType
WfMultiplePortAdapter::getSourceDataType() {
  return WfCst::TYPE_CONTAINER;
}

bool
WfMultiplePortAdapter::isDataIDCreator() {
  return (!containerID.empty());
}

void
WfMultiplePortAdapter::writeDataValue(WfDataWriter* dataWriter) {
  dataWriter->startContainer();
  for (std::list<WfPortAdapter*>::const_iterator adaptIter = adapters.begin();
       adaptIter != adapters.end();
       ++adaptIter) {
    ((WfPortAdapter*) *adaptIter)->writeDataValue(dataWriter);
  }
  dataWriter->endContainer();
}

void
WfMultiplePortAdapter::freeAdapterPersistentData(MasterAgent_var& MA) {
  for (std::list<WfPortAdapter*>::const_iterator adaptIter = adapters.begin();
       adaptIter != adapters.end();
       ++adaptIter) {
    ((WfPortAdapter*) *adaptIter)->freeAdapterPersistentData(MA);
  }
  if (!containerID.empty()) {
    // This class of adapter is always the owner of its dataID
    TRACE_TEXT(TRACE_ALL_STEPS, "Deleting persistent container: "
               << containerID << "\n");
    char *dataId = const_cast<char*>(containerID.c_str());
    if (MA->diet_free_pdata(dataId) == 0) {
      WARNING("Could not delete persistent data: " << dataId);
    }
  }
}

/*****************************************************************************/
/*                            WfVoidAdapter                                  */
/*****************************************************************************/

WfVoidAdapter::WfVoidAdapter() {
}

WfVoidAdapter::~WfVoidAdapter() {}

std::string WfVoidAdapter::voidRef("#UNDEF");

void
WfVoidAdapter::setNodePrecedence(WfNode* node, NodeSet* nodeSet)
  throw(WfStructException) {
}

void
WfVoidAdapter::connectPorts(WfPort* port, unsigned int adapterLevel)
  throw(WfStructException) {
}

std::string
WfVoidAdapter::getSourceRef() const {
  return voidRef;
}

const std::string&
WfVoidAdapter::getSourceDataID() {
  throw WfDataException(WfDataException::eVOID_DATA, "");
}

WfCst::WfDataType
WfVoidAdapter::getSourceDataType() {
  throw WfDataException(WfDataException::eVOID_DATA, "");
}

bool
WfVoidAdapter::isDataIDCreator() {
  return false;
}

void
WfVoidAdapter::writeDataValue(WfDataWriter* dataWriter) {
  dataWriter->voidElement();
}

void
WfVoidAdapter::freeAdapterPersistentData(MasterAgent_var& MA) {
}

/*****************************************************************************/
/*                          WfValueAdapter                                   */
/*****************************************************************************/

std::string WfValueAdapter::valStartTag("#VALDEB#");
std::string WfValueAdapter::valFinishTag("#VALFIN#");

WfValueAdapter::WfValueAdapter(const std::string& value)
  : myValue(value), myDataType(WfCst::TYPE_UNKNOWN),
    cx(NULL), sx(NULL), ix(NULL), lx(NULL), fx(NULL), dx(NULL), str(NULL) {
}

WfValueAdapter::WfValueAdapter(WfCst::WfDataType valueType,
                               const std::string& value)
  : myValue(value), myDataType(valueType),
    cx(NULL), sx(NULL), ix(NULL), lx(NULL), fx(NULL), dx(NULL), str(NULL) {
}

WfValueAdapter::~WfValueAdapter() {
  /* as long they were properly initialized at construction time,
   * no need to check pointers as delete 0; is safe in C++
   */
  delete cx;
  delete sx;
  delete ix;
  delete lx;
  delete fx;
  delete dx;
  delete str;
}

void
WfValueAdapter::setNodePrecedence(WfNode* node, NodeSet* nodeSet)
  throw(WfStructException) {
}

void
WfValueAdapter::connectPorts(WfPort* port, unsigned int adapterLevel)
  throw(WfStructException) {
}

std::string
WfValueAdapter::getSourceRef() const {
  return valStartTag + myValue + valFinishTag;
}

const std::string&
WfValueAdapter::getSourceDataID() {
  if (!myDataID.empty()) {
    return myDataID;
  }

  char *valID = NULL;
  char **valIDPtr = &valID;
  boost::format errorTpl("DAGDA failed to upload data to the platform"
                         "(value '%1%')%2%");
  std::cout << "GetSourceDataID: " << myValue << "\n";
  try {
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
                            "Cannot initialize data due to unknown"
                            "type in value adapter");
    }
  } catch (Dagda::ReadError& ex) {
    errorTpl % myValue % "(Read Error)";
    throw WfDataException(WfDataException::eREADFILERROR, errorTpl.str());
  } catch (...) {
    errorTpl % myValue % "(Dagda Exception)";
    throw WfDataException(WfDataException::eINVALID_VALUE, errorTpl.str());
  }
  myDataID = valID;

  return myDataID;
}

WfCst::WfDataType
WfValueAdapter::getSourceDataType() {
  return myDataType;
}

bool
WfValueAdapter::isDataIDCreator() {
  return (!myDataID.empty());
}

void
WfValueAdapter::writeDataValue(WfDataWriter* dataWriter) {
  dataWriter->itemValue(myValue);
}

void
WfValueAdapter::freeAdapterPersistentData(MasterAgent_var& MA) {
  // This class of adapter is always the owner of its dataID because
  // the dataID can be provided in the constructor
  if (isDataIDCreator()) {
    TRACE_TEXT(TRACE_ALL_STEPS, "Deleting persistent data (value adapter): "
               << myDataID << "\n");
    char *dataId = const_cast<char*>(myDataID.c_str());
    if (MA->diet_free_pdata(dataId) == 0) {
      WARNING("Could not delete persistent data: " << dataId);
    }
  }
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
  size_t sz = myValue.size() + 1;
  str = new char[sz];
#ifdef __WIN32__
  _strncpy(str, myValue.c_str(), sz);
#else
  strncpy(str, myValue.c_str(), sz);
#endif

  return str;
}

char *
WfValueAdapter::newFile() {
  size_t sz = myValue.size() + 1;
  str = new char[sz];
#ifdef __WIN32__
  _snprintf(str, sz, myValue.c_str());
#else
  snprintf(str, sz, myValue.c_str());
#endif
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

/*****************************************************************************/
/*                         WfDataIDAdapter                                   */
/*****************************************************************************/

std::string WfDataIDAdapter::IDStartTag("#IDDEB#");

std::string WfDataIDAdapter::IDFinishTag("#IDFIN#");

std::map<std::string, std::vector<std::string> > WfDataIDAdapter::myCache;

WfDataIDAdapter::WfDataIDAdapter(WfCst::WfDataType dataType,
                                 unsigned int dataDepth,
                                 const std::string& dataID)
  : myDataID(dataID), myDataType(dataType), myDepth(dataDepth) {
  if (dataID.empty()) {
    WARNING("Creating WfDataIDAdapter with empty data ID");
  }
}

WfDataIDAdapter::WfDataIDAdapter(const std::string& dataID)
  : myDataID(dataID), myDataType(WfCst::TYPE_UNKNOWN), myDepth(0) {
  if (dataID.empty()) {
    WARNING("Creating WfDataIDAdapter with empty data ID");
  }
}

WfDataIDAdapter::~WfDataIDAdapter() {}

void
WfDataIDAdapter::setNodePrecedence(WfNode* node, NodeSet* nodeSet)
  throw(WfStructException) {
}

void
WfDataIDAdapter::connectPorts(WfPort* port, unsigned int adapterLevel)
  throw(WfStructException) {
}

std::string
WfDataIDAdapter::getSourceRef() const {
  return IDStartTag + myDataID + IDFinishTag;
}

const std::string&
WfDataIDAdapter::getSourceDataID() {
  return myDataID;
}

WfCst::WfDataType
WfDataIDAdapter::getSourceDataType() {
  return myDataType;
}

void
WfDataIDAdapter::getElements(std::vector<std::string >& vectID) {
  if (myDataID.empty()) {
    return;
  }

  std::map<std::string, std::vector<std::string> >::const_iterator cacheIter =
    myCache.find(myDataID);
  if (cacheIter != myCache.end()) {
    vectID = cacheIter->second;
  } else {
    boost::scoped_ptr<diet_container_t> content(new diet_container_t);
    boost::format errorTpl("containerID '%1%' %2%");
    content->size = 0;
    try {
      dagda_get_container(myDataID.c_str());
    } catch (...) {
      errorTpl % myDataID % "' : not found or invalid type";
      throw WfDataException(WfDataException::eNOTFOUND, errorTpl.str());
    }
    if (dagda_get_container_elements(myDataID.c_str(), content.get())) {
      errorTpl % myDataID % "' : cannot get container elements";
      throw WfDataException(WfDataException::eINVALID_CONTAINER,
                            errorTpl.str());
    }
    if (content->size != vectID.size()) {
      vectID.resize(content->size);
    }
    std::string eltIdsMsg = "";
    for (unsigned int i = 0; i < content->size; ++i) {
      if (content->elt_ids[i] != NULL) {
        vectID[i] = content->elt_ids[i];
        eltIdsMsg += vectID[i];
      } else {
        eltIdsMsg += WfVoidAdapter::voidRef;
      }
      if (i != content->size-1) {
        eltIdsMsg += ";";
      }
    }
    events::sendEventFrom<WfDataIDAdapter,
                          WfDataIDAdapter::ELTIDLIST>(this,
                                                      "Container elements",
                                                      eltIdsMsg,
                                                      EventBase::INFO);
    free(content->elt_ids);

    // update cache
    myCache[myDataID] = vectID;
  }
}

std::string
WfDataIDAdapter::getDataID() const {
  return myDataID;
}

std::string
WfDataIDAdapter::toString() const {
  return "Adapter ID=" + myDataID;
}

bool
WfDataIDAdapter::isDataIDCreator() {
  return false;
}

void
WfDataIDAdapter::writeDataValue(WfDataWriter* dataWriter) {
  if (myDataID.empty()) {
    getSourceDataID();
  }
  getAndWriteData(dataWriter, myDataID, myDataType, myDepth);
}

// static & recursive
void
WfDataIDAdapter::getAndWriteData(WfDataWriter* dataWriter,
                                 const std::string& dataID,
                                 WfCst::WfDataType dataType,
                                 unsigned int dataDepth) {
  if (dataID.empty()) {
    dataWriter->voidElement();
    return;
  }
  try {
    if (dataDepth > 0) {
      dataWriter->startContainer();
      WfDataIDAdapter adapter(dataID);
      std::vector<std::string> vectID;
      adapter.getElements(vectID);

      for (std::vector<std::string>::iterator eltIter = vectID.begin();
           eltIter != vectID.end();
           ++eltIter) {
        if ((*eltIter).empty()) {
          dataWriter->voidElement();
        } else {
          getAndWriteData(dataWriter, *eltIter, dataType, dataDepth-1);
        }
      }

      dataWriter->endContainer();

    } else if (dataType == WfCst::TYPE_DOUBLE) {
      double * value;
      dagda_get_scalar(dataID.c_str(), &value, NULL);
      dataWriter->itemValue(value, (WfCst::WfDataType) dataType);
    } else if (dataType == WfCst::TYPE_INT) {
      int *value = NULL;
      dagda_get_scalar(dataID.c_str(), &value, NULL);
      dataWriter->itemValue(value, (WfCst::WfDataType) dataType);
    } else if (dataType == WfCst::TYPE_LONGINT) {
      long *value;
      dagda_get_scalar(dataID.c_str(), &value, NULL);
      dataWriter->itemValue(value, (WfCst::WfDataType) dataType);
    } else if (dataType == WfCst::TYPE_FLOAT) {
      float *value;
      dagda_get_scalar(dataID.c_str(), &value, NULL);
      dataWriter->itemValue(value, (WfCst::WfDataType) dataType);
    } else if (dataType == WfCst::TYPE_CHAR) {
      char *value;
      dagda_get_scalar(dataID.c_str(), &value, NULL);
      dataWriter->itemValue(value, (WfCst::WfDataType) dataType);
    } else if (dataType == WfCst::TYPE_SHORT) {
      short *value;
      dagda_get_scalar(dataID.c_str(), &value, NULL);
      dataWriter->itemValue(value, (WfCst::WfDataType) dataType);
    } else if (dataType == WfCst::TYPE_PARAMSTRING) {
      char *value;
      dagda_get_paramstring(dataID.c_str(), &value);
      dataWriter->itemValue(value, (WfCst::WfDataType) dataType);
    } else if (dataType == WfCst::TYPE_STRING) {
      char *value;
      dagda_get_string(dataID.c_str(), &value);
      dataWriter->itemValue(value, (WfCst::WfDataType) dataType);
    } else if (dataType == WfCst::TYPE_FILE) {
      char *path;
      dagda_get_file(dataID.c_str(), &path);
      dataWriter->itemValue(path, (WfCst::WfDataType) dataType);
    }
  } catch (Dagda::DataNotFound& e) {
    std::string errorMsg = "Data ID = " + std::string(dataID.c_str());
    throw WfDataException(WfDataException::eNOTFOUND, errorMsg);
  } catch (Dagda::ReadError& e) {
    std::string errorMsg = "Data ID = " + std::string(dataID.c_str());
    throw WfDataException(WfDataException::eREADFILERROR, errorMsg);
  }
}

void
WfDataIDAdapter::freeAdapterPersistentData(MasterAgent_var& MA) {
}


