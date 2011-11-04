/**
* @file  FNodePort.cc
* 
* @brief  The classes representing the ports the nodes of functional workflow
* 
* @author  Benjamin ISNARD (Benjamin.Isnard@ens-lyon.fr)
* 
* @section Licence
*   |LICENSE|                                                                
*/


#include "debug.hh"
#include "DIET_data.h"
#include "FNodePort.hh"
#include "FNode.hh"
#include "FWorkflow.hh"
#include "DagNodePort.hh"
#include "Dag.hh"
extern "C" {
#include "DIET_Dagda.h"
}
#include "EventTypes.hh"

/**
 * Constructors & destructors
 */
FNodePort::FNodePort(WfNode * parent,
                     const std::string& _id,
                     WfPort::WfPortType _portType,
                     WfCst::WfDataType _type,
                     unsigned int _depth,
                     unsigned int _ind)
  : WfPort(parent, _id, _portType, _type, _depth, _ind) {
}

FNodePort::~FNodePort() {
}

FNode*
FNodePort::getParentFNode() {
  FNode* parent = dynamic_cast<FNode*>(getParent());
  return parent;
}

FProcNode*
FNodePort::getParentProcNode() {
  FProcNode* parent = dynamic_cast<FProcNode*>(getParent());
  if (parent == NULL) {
    INTERNAL_ERROR("Invalid type of parent node for FNodePort "
                   << getId() << "\n", 1);
  }
  return parent;
}

std::string
FNodePort::toString() const {
  return "FPORT " + getId() + "[FNODE=" + getParent()->getId() + "]";
}

/*****************************************************************************/
/*                           FNodeOutPort                                    */
/*****************************************************************************/

FNodeOutPort::FNodeOutPort(WfNode * parent,
                           const std::string& _id,
                           WfPort::WfPortType _portType,
                           WfCst::WfDataType _type,
                           unsigned int _depth,
                           unsigned int _ind)
  : FNodePort(parent, _id, _portType, _type, _depth, _ind)
  , myBuffer(FDataTag(), _type, _depth+1) {
}

FNodeOutPort::~FNodeOutPort() {
  // buffer is deleted automatically
}

/**
 * Ports connection setup method
 * Called by WfSimpleAdapter on both sides to connect node ports
 */
void
FNodeOutPort::connectToPort(WfPort* remPort) {
  // set connected flag
  WfPort::connectToPort(remPort);
  // cast & add to list of connected in ports
  FNodeInPort* srcPort = dynamic_cast<FNodeInPort*>(remPort);
  if (srcPort == NULL) {
    std::cerr << "OUT=" << getId() << " / IN=" << remPort->getId() << "\n";
    INTERNAL_ERROR("Wrong connected node type in functional node OUT port", 0);
  }
  myConnectedPorts.push_back(srcPort);
}

/**
 * Instanciation
 */
FDataHandle*
FNodeOutPort::createRealInstance(Dag* dag, DagNode* nodeInst,
                                 const FDataTag& tag) {
  if (!nodeInst) {
    INTERNAL_ERROR("FNodeOutPort::instanciate called with NULL instance", 1);
  }
  FDataHandle* dataHdl;
  // create the portInst (DagNodeOutPort) for the nodeInst
  std::string portId = this->getId();
  TRACE_TEXT(TRACE_ALL_STEPS,
             "   # Creating new instance of OUT port: " << portId << "\n");
  WfPort* portInst = nodeInst->newPort(portId,
                                       getIndex(),
                                       portType,
                                       getBaseDataType(),
                                       depth);
  DagNodeOutPort* portInstPtr = dynamic_cast<DagNodeOutPort*>(portInst);
  // create a FDataHandle corresponding to the new port instance
  //  - its depth equals the depth of the out port
  //  - its tag equals the tag of the node instance
  //  - its port equals the portInst
  dataHdl = new FDataHandle(tag, depth, false, portInstPtr);
  // set the cardinal information if available
  if (card) {
    dataHdl->setCardinalList(*card);
  }
  return dataHdl;
}

FDataHandle*
FNodeOutPort::createVoidInstance(const FDataTag& tag) {
  // create a VOID FDataHandle
  return new FDataHandle(tag, depth, true);
}

void
FNodeOutPort::storeData(FDataHandle* dataHdl) {
  TRACE_TEXT(TRACE_ALL_STEPS,
             "   # Insert data into buffer (" << getId() << ")\n");
  myBuffer.insertInTree(dataHdl);
}

void
FNodeOutPort::sendData(FDataHandle* dataHdl) {
  // add the new dataHandle to all the connected in ports ==> inPort.addData
  // all the inPort(s) that return false must be included in the FNode pending
  // list
  if (myConnectedPorts.empty()) {
    return;
  }

  TRACE_TEXT(TRACE_ALL_STEPS, "   # Insert data into connected ports\n");
  for (std::list<FNodeInPort*>::iterator inPortIter = myConnectedPorts.begin();
       inPortIter != myConnectedPorts.end();
       ++inPortIter) {
    FNodeInPort* inPort = (FNodeInPort*) *inPortIter;
    try {
      inPort->addData(dataHdl);
      checkTotalDataNb(inPort);
    } catch (WfDataHandleException& e) {
      if  (e.Type() == WfDataHandleException::eADAPT_UNDEF) {
        TRACE_TEXT(TRACE_ALL_STEPS, " ==> insert failed (missing adapter)\n");
      } else if ((e.Type() == WfDataHandleException::eCARD_UNDEF)
                 || (e.Type() == WfDataHandleException::eVALUE_UNDEF)) {
        TRACE_TEXT(TRACE_ALL_STEPS,
                   " ==> insert failed (missing value or cardinal)\n");
        if (dataHdl->isDataIDDefined()) {
          reSendData(dataHdl, inPort);
        } else {
          setPendingDataTransfer(dataHdl, inPort);
        }
      } else {
        throw;
      }
    }
  }
}

void
FNodeOutPort::setPendingDataTransfer(FDataHandle* dataHdl, FNodeInPort* inPort)
  throw(WfDataHandleException) {
  DagNodeOutPort* dagOutPort =
    dynamic_cast<DagNodeOutPort*>(dataHdl->getSourcePort());
  DagNode* dagNode = dynamic_cast<DagNode*>(dagOutPort->getParent());
  FWorkflow* wf = dagNode->getWorkflow();
  wf->setPendingInstanceInfo(dagNode, dataHdl, this, inPort);
}

/**
 * Dynamic sendData (if data available)
 * Used by workflow when processing pending instances
 */
void
FNodeOutPort::reSendData(FDataHandle* dataHdl, FNodeInPort* inPort) {
  TRACE_TEXT(TRACE_ALL_STEPS, "   # reSendData: uses data ID = "
              << dataHdl->getDataID() << "\n");
  // send the data to IN port
  try {
    inPort->addData(dataHdl);
    checkTotalDataNb(inPort);
  } catch (WfDataHandleException& e) {
    // a VALUE_UNDEF may be thrown in case the current data must be merged with
    // other data that is not yet available. In this case there is nothing to do
    // because the container DH will be received when the last element is added.
    if ((e.Type() != WfDataHandleException::eVALUE_UNDEF) &&
        (e.Type() != WfDataHandleException::eADAPT_UNDEF)) {
      throw;
    }
  }
}

void
FNodeOutPort::sendAllData() {
  sendData(&myBuffer);
}

void
FNodeOutPort::checkTotalDataNb(FNodeInPort *inPort) {
  // determine the level corresponding to data items of the input port
  unsigned int inLevel = inPort->getDataLevel();
  // check if the tree is complete at this level
  if (myBuffer.checkIfComplete(inLevel)) {
    inPort->setTotalDataNb(myBuffer.getChildCount(inLevel));
  }
}

void
FNodeOutPort::uploadAllData(MasterAgent_var& MA) {
  myBuffer.uploadTreeData(MA);
}

void
FNodeOutPort::downloadAllData() {
  myBuffer.downloadTreeData();
}

void
FNodeOutPort::updateAllDataCardinal() {
  myBuffer.updateTreeCardinal();
}

void
FNodeOutPort::writeAllDataAsXML(std::ostream& output) {
  WfXMLDataWriter xmlWriter(output);
  myBuffer.toXML(xmlWriter);
}

void
FNodeOutPort::writeAllDataAsList(std::ostream& output) {
  if (myBuffer.isCardinalDefined()) {
    output << "TOTAL NB OF RESULTS: " << myBuffer.getCardinal() << "\n";
  } else {
    output << "Warning: undefined total nb of results "
      "(error during instanciation)\n";
  }
  int ix = 0;
  std::map<FDataTag, FDataHandle*>::iterator dataIter = myBuffer.begin();
  for (; dataIter != myBuffer.end(); ++dataIter) {
    FDataHandle*  currData = (FDataHandle*) dataIter->second;
    WfDataWriter* dataWriter = new WfListDataWriter(output);
    output << "[" << ix++ << "]=";
    try {
      currData->writeValue(dataWriter);
    } catch (WfDataException& e) {
      output << "<Error: " << e.ErrorMsg() << ">";
    }
    output << "\n";
  }
}

FDataHandle*
FNodeOutPort::getBufferRootDH() {
  return &myBuffer;
}

const std::string&
FNodeOutPort::getBufferContainerID() {
  return myBuffer.getDataID();
}

void
FNodeOutPort::checkIfEmptyOutput() {
  if (myBuffer.isEmpty()) {
    for (std::list<FNodeInPort*>::iterator inPortIter = myConnectedPorts.begin();
         inPortIter != myConnectedPorts.end();
         ++inPortIter) {
      FNodeInPort* inPort = (FNodeInPort*) *inPortIter;
      inPort->setTotalDataNb(0);
    }
  }
}

void
FNodeOutPort::setAsConstant(FDataHandle* dataHdl) {
  if (dataHdl->getTag().getLevel() != 1) {
    INTERNAL_ERROR(__FUNCTION__ << "Invalid DH tag level", 1);
  }
  // store this handle in my buffer (for deallocation)
  myBuffer.insertInTree(dataHdl);
  // set the connected in ports as constant with this handle
  for (std::list<FNodeInPort*>::iterator inPortIter = myConnectedPorts.begin();
       inPortIter != myConnectedPorts.end();
       ++inPortIter) {
    ((FNodeInPort*) *inPortIter)->setAsConstant(dataHdl);
  }
}

void
FNodeOutPort::freePersistentData(MasterAgent_var& MA) {
  TRACE_TEXT(TRACE_ALL_STEPS,
             "  # free persistent data (port " << getCompleteId() << ")\n");
  myBuffer.freePersistentDataRec(MA);
}

/*****************************************************************************/
/*                            FNodeInPort                                    */
/*****************************************************************************/

FNodeInPort::FNodeInPort(WfNode * parent,
                         const std::string& _id,
                         WfPort::WfPortType _portType,
                         WfCst::WfDataType _type,
                         unsigned int _depth,
                         unsigned int _ind)
  : FNodePort(parent, _id, _portType, _type, _depth, _ind),
    dataTotalNb(0), totalDef(false), valueRequired(false) { }

FNodeInPort::~FNodeInPort() {
}

void
FNodeInPort::setValueRequired() {
  valueRequired = true;
}

bool
FNodeInPort::isValueRequired() {
  return valueRequired;
}

/**
 * addData (RECURSIVE)
 */
void
FNodeInPort::addData(FDataHandle* dataHdl) {
  TRACE_TEXT(TRACE_ALL_STEPS, "     # add data " << dataHdl->getTag().toString()
              << " to port " << getCompleteId()
              << (dataHdl->isValueDefined() ?
                  " /value=" + dataHdl->getValue() : "")
              << (dataHdl->isDataIDDefined() ?
                  " /ID=" + dataHdl->getDataID() : "")
              << "\n");

  if (!dataHdl->isAdapterDefined()) {
    TRACE_TEXT(TRACE_ALL_STEPS, "Cannot add data to inPort (no adapter)\n");
    throw WfDataHandleException(WfDataHandleException::eADAPT_UNDEF,
                                dataHdl->getTag().toString());
  }

  if (dataHdl->getDepth() == depth) {
    const FDataTag& dataTag = dataHdl->getTag();
    // check if value is required
    TRACE_TEXT(TRACE_ALL_STEPS,
               "Checking value: " << (valueRequired ? "yes" : "no") << "\n");
    if ((valueRequired) && (!dataHdl->isValueDefined())) {
      dataHdl->downloadValue();  // may throw VALUE_UNDEF exception
    }
    TRACE_TEXT(TRACE_ALL_STEPS, "Adding data in input port queue (tag="
                << dataTag.toString() << ")\n");
    myQueue.insert(std::pair<FDataTag, FDataHandle*>(dataTag, dataHdl));
    // updates node status
    getParentFNode()->setStatusReady();

    // SPLIT CURRENT DATA => calls addData on the childrens
  } else if (dataHdl->getDepth() > depth) {
    try {
      dataHdl->downloadCardinal();
    } catch (WfDataException& e) {
      if (e.Type() != WfDataException::eID_UNDEF) {
        throw;
      }
    }
    if (dataHdl->isCardinalDefined()) {
      TRACE_TEXT(TRACE_ALL_STEPS, "Adding data elements (SPLIT)\n");
      // loop over all elements of the data handle (begin may throw exception)
      std::map<FDataTag, FDataHandle*>::iterator eltIter = dataHdl->begin();
      for (; eltIter != dataHdl->end(); ++eltIter) {
        this->addData((FDataHandle*) eltIter->second);
      }
    } else {
      TRACE_TEXT(TRACE_ALL_STEPS,
                 "Cardinal is required but not available (ADD cancelled)\n");
      throw WfDataHandleException(WfDataHandleException::eCARD_UNDEF,
                                  dataHdl->getTag().toString());
    }
    // MERGE => calls addData on the parent
  } else {
    if ((valueRequired) && (!dataHdl->isValueDefined())) {
      dataHdl->downloadValue();
    }
    if (dataHdl->isParentDefined()) {
      if (dataHdl->getTag().getLevel() <= 1) {
        // throw exception when the parent DH is the buffer's ROOT which should
        // never be used as a data itself.
        // This means the workflow is badly built.
        throw WfDataHandleException(WfDataHandleException::eINVALID_ADAPT,
                                    "Cannot merge data higher than level 1 data");
      }
      TRACE_TEXT(TRACE_ALL_STEPS, "Trying to add parent (MERGE)\n");
      this->addData(dataHdl->getParent());
    } else {
      INTERNAL_ERROR("Missing data handle parent", 0);
    }
  }
}

unsigned int
FNodeInPort::getDataLevel() {
  std::map<FDataTag, FDataHandle*>::const_iterator dataIter = myQueue.begin();
  if (dataIter != myQueue.end()) {
    return ((FDataHandle*)dataIter->second)->getTag().getLevel();
  } else {
    INTERNAL_ERROR("Called getDataLevel() on empty IN port", 1);
  }
}

void
FNodeInPort::setTotalDataNb(unsigned int total) {
  dataTotalNb = total;
  totalDef = true;
  TRACE_TEXT(TRACE_ALL_STEPS, "Setting total to " << total
              << " for IN port : " << getCompleteId() << "\n");
}

void
FNodeInPort::setAsConstant(FDataHandle* dataHdl) {
  if ((valueRequired) && (!dataHdl->isValueDefined())) {
    dataHdl->downloadValue();
  }
  FProcNode* node = getParentProcNode();
  node->setConstantInput(getIndex(), dataHdl);
  //  node->setStatusReady(); ???
}

void
FNodeInPort::createRealInstance(Dag* dag, DagNode* nodeInst,
                                FDataHandle* dataHdl) {
  if (!nodeInst) {
    INTERNAL_ERROR("FNodeInPort::instanciate called with NULL instance", 1);
  }
  if (!dataHdl) {
    INTERNAL_ERROR("FNodeInPort::instanciate called with NULL data handle", 1);
  }
  // create a DagNodeInPort for the nodeInst
  std::string portId = this->getId();
  TRACE_TEXT(TRACE_ALL_STEPS,
             "   # Creating new instance of IN port: " << portId << "\n");
  WfPort* portInst = nodeInst->newPort(portId, getIndex(),
                                       portType, getBaseDataType(),
                                       depth);
  WfPortAdapter * portAdapter = dataHdl->createPortAdapter(dag->getId());
  portInst->setPortAdapter(portAdapter);
  TRACE_TEXT(TRACE_ALL_STEPS,
             "Setting ADAPTER = " << portAdapter->getSourceRef() << "\n");
}

/*****************************************************************************/
/*                          FNodeInOutPort                                   */
/*****************************************************************************/


FNodeInOutPort::FNodeInOutPort(WfNode * parent,
                               const std::string& _id,
                               WfCst::WfDataType _type,
                               unsigned int _depth,
                               unsigned int _ind)
  : FNodePort(parent, _id, WfPort::PORT_INOUT, _type, _depth, _ind),
    FNodeInPort(parent, _id, WfPort::PORT_INOUT, _type, _depth, _ind),
    FNodeOutPort(parent, _id, WfPort::PORT_INOUT, _type, _depth, _ind) {
}

void
FNodeInOutPort::connectToPort(WfPort* remPort, bool endOfLink) {
  switch (remPort->getPortType()) {
  case WfPort::PORT_IN:
  case WfPort::PORT_IN_LOOP:
    FNodeOutPort::connectToPort(remPort);
    break;
  case WfPort::PORT_ARG:
  case WfPort::PORT_OUT:
  case WfPort::PORT_OUT_LOOP:
  case WfPort::PORT_OUT_THEN:
  case WfPort::PORT_OUT_ELSE:
    WfPort::connectToPort(remPort);
    break;
  default: {
    if (endOfLink) {
      FNodeOutPort::connectToPort(remPort);
    } else {
      WfPort::connectToPort(remPort);
    }
  }
  }
}

FDataHandle*
FNodeInOutPort::createRealInstance(Dag* dag, DagNode* nodeInst,
                                   const FDataTag& tag, FDataHandle* dataHdl) {
  if (!nodeInst) {
    INTERNAL_ERROR("FNodeInPort::instanciate called with NULL instance", 1);
  }
  if (!dataHdl) {
    INTERNAL_ERROR("FNodeInPort::instanciate called with NULL data handle", 1);
  }
  // create a DagNodeInOutPort for the nodeInst
  std::string portId = this->getId();
  TRACE_TEXT(TRACE_ALL_STEPS,
             "   # Creating new instance of IN-OUT port: " << portId << "\n");
  WfPort* portInst = nodeInst->newPort(portId, getIndex(),
                                       portType, getBaseDataType(),
                                       depth);
  WfPortAdapter * portAdapter = dataHdl->createPortAdapter(dag->getId());
  portInst->setPortAdapter(portAdapter);
  TRACE_TEXT(TRACE_ALL_STEPS,
             "Setting ADAPTER = " << portAdapter->getSourceRef() << "\n");
  DagNodeOutPort* portInstPtr = dynamic_cast<DagNodeOutPort*>(portInst);
  FDataHandle* outDataHdl;
  // create a FDataHandle corresponding to the new port instance
  //  - its depth equals the depth of the out port
  //  - its tag equals the tag of the node instance
  //  - its port equals the portInst
  outDataHdl = new FDataHandle(tag, depth, false, portInstPtr);
  // set the cardinal information if available
  if (card) {
    outDataHdl->setCardinalList(*card);
  }
  return outDataHdl;
}

/*****************************************************************************/
/*                          FNodeParamPort                                   */
/*****************************************************************************/
FNodeParamPort::FNodeParamPort(WfNode * parent,
                               const std::string& _id,
                               WfCst::WfDataType _type,
                               unsigned int _ind)
  : FNodePort(parent, _id, WfPort::PORT_PARAM, _type, 0, _ind),
    FNodeInPort(parent, _id, WfPort::PORT_PARAM, _type, 0, _ind) {
  setValueRequired();
}

FNodeParamPort::~FNodeParamPort() {
}

/*****************************************************************************/
/*                           FNodePortMap                                    */
/*****************************************************************************/
FNodePortMap::FNodePortMap() {
}

void
FNodePortMap::mapPorts(FNodeOutPort* outPort, FNodeInPort* inPort) {
  myPortMap[outPort] = inPort;
}

void
FNodePortMap::mapPortToVoid(FNodeOutPort* outPort) {
  myPortMap[outPort] = NULL;
}

void
FNodePortMap::applyMap(const FDataTag& tag,
                       const std::vector<FDataHandle*>& dataLine) {
  std::map<FNodeOutPort*, FNodeInPort*>::const_iterator iter =
    myPortMap.begin();
  for (; iter != myPortMap.end(); ++iter) {
    FNodeOutPort* outPort = (FNodeOutPort*) iter->first;
    FDataHandle* dataHdl;
    if (iter->second) {
      FNodeInPort* inPort = (FNodeInPort*) iter->second;
      TRACE_TEXT(TRACE_ALL_STEPS, " # Mapping " << inPort->getId()
                  << " to " << outPort->getId() << "\n");
      // COPY the input DH (with all its child tree)
      if (!dataLine[inPort->getIndex()]) {
        INTERNAL_ERROR("data handle for in port "
                       << inPort->getId() << " not defined\n", 1);
      }
      dataHdl = new FDataHandle(tag, *dataLine[inPort->getIndex()]);
    } else {
      TRACE_TEXT(TRACE_ALL_STEPS,
                 " # Mapping VOID to " << outPort->getId() << "\n");
      // CREATE a new VOID datahandle
      dataHdl = new FDataHandle(tag, outPort->getDepth(), true);
    }
    // Submit to out port (ie will send it to connected ports)
    outPort->storeData(dataHdl);
    outPort->sendData(dataHdl);
  }
}
