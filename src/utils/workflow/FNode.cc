/**
* @file  FNode.cc
* 
* @brief  The classes representing the node functional workflow
* 
* @author  Benjamin ISNARD (Benjamin.Isnard@ens-lyon.fr)
* 
* @section Licence
*   |LICENSE|                                                                
*/


#include <string>
#include "debug.hh"
#include "FNode.hh"
#include "FWorkflow.hh"
#include "Dag.hh"
#include "DagNodePort.hh"
#include "DagWfParser.hh"
#include "EventTypes.hh"


FNode::FNode(FWorkflow* wf, const std::string& id, nodeInstStatus_t initStatus)
  : WfNode(id), wf(wf), myStatus(initStatus) {}

FNode::~FNode() {
  TRACE_TEXT(TRACE_ALL_STEPS, "~FNode() " << getId() << " destructor ...\n");
}

FWorkflow *
FNode::getWorkflow() const {
  return wf;
}

FWorkflow*
FNode::getRootWorkflow() const {
  if (wf == NULL) {
    return NULL;
  }
  return wf->getRootWorkflow();
}

const std::string&
FNode::getDefaultPortName() const {
  INTERNAL_ERROR("FNode::getDefaultPortName not defined for this class", 1);
}

/* protected */
void
FNode::setStatusReady() {
  if (!instanciationOnHold()) {
    myStatus = N_INSTANC_READY;
  }
}

bool
FNode::instanciationReady() {
  return (myStatus == N_INSTANC_READY);
}

bool
FNode::instanciationPending() {
  return (myStatus == N_INSTANC_PENDING);
}

bool
FNode::instanciationOnHold() {
  return (myStatus == N_INSTANC_ONHOLD);
}

bool
FNode::instanciationCompleted() {
  return (myStatus == N_INSTANC_END);
}

bool
FNode::instanciationStopped() {
  return (myStatus == N_INSTANC_STOPPED);
}

void
FNode::stopInstanciation() {
  myStatus = N_INSTANC_STOPPED;
}

void
FNode::setInstanciationCompleted() {
  myStatus = N_INSTANC_END;
}

void
FNode::resumeInstanciation() {
  if (!instanciationCompleted() && !instanciationStopped()) {
    myStatus = N_INSTANC_READY;
    if (wf) {
      wf->resumeInstanciation();
    }
  }
}

WfPort *
FNode::newPort(std::string portId,
               unsigned int ind,
               WfPort::WfPortType portType,
               WfCst::WfDataType dataType,
               unsigned int depth) throw(WfStructException) {
  WfPort * p = NULL;
  switch (portType) {
  case WfPort::PORT_PARAM:
    p = new FNodeParamPort(this, portId, dataType, ind);
    break;
  case WfPort::PORT_IN:
    p = new FNodeInPort(this, portId, portType, dataType, depth, ind);
    break;
  case WfPort::PORT_INOUT:
    p = new FNodeInOutPort(this, portId, dataType, depth, ind);
    break;
  case WfPort::PORT_OUT:
    p = new FNodeOutPort(this, portId, portType, dataType, depth, ind);
    break;
  default:
    INTERNAL_ERROR("Invalid port type for FNode port", 1);
  }
  return addPort(portId, p);
}

void
FNode::connectNodePorts() throw(WfStructException) {
  TRACE_TEXT(TRACE_ALL_STEPS, traceId() << "Connecting ports\n");
  for (std::map<std::string, WfPort*>::iterator p = ports.begin();
       p != ports.end();
       ++p) {
    try {
      ((WfPort*)(p->second))->connectPorts();
    } catch (WfStructException& e) {
      // depth mismatch is ok for functional nodes connections
      if (e.Type() != WfStructException::eDEPTH_MISMATCH) {
        throw;
      }
    }
  }
}

void
FNode::connectToWfPort(FNodePort* port) {}

void
FNode::initialize() {
  for (std::map<std::string, WfPort*>::iterator portIter = ports.begin();
       portIter != ports.end();
       ++portIter) {
    WfPort* port = (WfPort*) portIter->second;
    if (port->isInput() && !port->isConnected()) {
      WARNING("Input port '" << port->getCompleteId()
              << "' is not connected to a source");
    }
  }
}

void
FNode::finalize() {
  // Check if output ports have sent zero items
  // and if yes it updates the cardinal of connected ports
  // (this happens for ex to sources without data or to the output of a filter
  // node)
  for (std::map<std::string, WfPort*>::iterator portIter = ports.begin();
       portIter != ports.end();
       ++portIter) {
    WfPort* port = (WfPort*) portIter->second;
    if (port->isOutput()) {
      FNodeOutPort* outPort = dynamic_cast<FNodeOutPort*>(port);
      if (!outPort) {
        INTERNAL_ERROR("FNode Output port cannot be casted correctly", 1);
      }
      outPort->checkIfEmptyOutput();
    }
  }
}

void
FNode::freeNodePersistentData(MasterAgent_var& MA) {
  // LOOP for each out port
  for (std::map<std::string, WfPort*>::iterator portIter = ports.begin();
       portIter != ports.end();
       ++portIter) {
    WfPort* port = (WfPort*) portIter->second;
    if (port->isOutput()) {
      FNodeOutPort* outPort = dynamic_cast<FNodeOutPort*>(port);
      outPort->freePersistentData(MA);
    }
  }
}

std::string
FNode::traceId() {
  return "[" + myId + "]: ";
}

std::string
FNode::toString() const {
  return myId;
}


/*****************************************************************************/
/*                           FConstantNode                                   */
/*****************************************************************************/
std::string FConstantNode::outPortName("out");

const std::string&
FConstantNode::getDefaultPortName() const {
  return outPortName;
}

FConstantNode::FConstantNode(FWorkflow* wf, const std::string& id,
                             WfCst::WfDataType type)
  : FNode(wf, id, N_INSTANC_READY), myDH(NULL) {
  WfPort* outPort = this->newPort(outPortName, 0, WfPort::PORT_OUT, type, 0);
  myOutPort = dynamic_cast<FNodeOutPort*>(outPort);
}

FConstantNode::~FConstantNode() {
  // out port is deleted in ~WfNode()
}

void
FConstantNode::setValue(const std::string& strVal) {
  myValue = strVal;
}

void
FConstantNode::setDataID(const std::string& dataID) {
  myDataID = dataID;
}

void
FConstantNode::initialize() {
  FDataTag singleTag(0, true);
  // use data ID if available (value is not used)
  if (!myDataID.empty()) {
    TRACE_TEXT(TRACE_ALL_STEPS, traceId() << "Initialize constant (ID=/"
               << myDataID << "/)\n");
    myDH = new FDataHandle(singleTag, myOutPort->getBaseDataType(),
                           0, myDataID);
  } else {
    TRACE_TEXT(TRACE_ALL_STEPS, traceId() << "Initialize constant (value=/"
               << myValue << "/)\n");
    myDH = new FDataHandle(singleTag, myOutPort->getBaseDataType(), myValue);
    myDH->downloadDataID();  // add data to the platform dataMgr
  }
  myOutPort->setAsConstant(myDH);
}

void
FConstantNode::instanciate(Dag* dag) {
  myStatus = N_INSTANC_END;
}

/*****************************************************************************/
/*                            FSourceNode                                    */
/*****************************************************************************/
std::string FSourceNode::outPortName("out");

FSourceNode::FSourceNode(FWorkflow* wf, const std::string& id,
                         WfCst::WfDataType type)
  : FNode(wf, id, N_INSTANC_READY), myParser(NULL), isConnected(false) {
  WfPort * outPort = this->newPort(outPortName, 0, WfPort::PORT_OUT, type, 0);
  myOutPort = dynamic_cast<FNodeOutPort*>(outPort);
  myParser = new DataSourceParser(this);
}

FSourceNode::~FSourceNode() {
  // out port is deleted in ~WfNode()
  delete myParser;
}

const std::string&
FSourceNode::getDefaultPortName() const {
  return outPortName;
}

WfCst::WfDataType
FSourceNode::getDataType() const {
  return myOutPort->getBaseDataType();
}

unsigned int
FSourceNode::getDepth() const {
  return myOutPort->getDepth();
}

void
FSourceNode::connectToWfPort(FNodePort* port) {
  FNodeInPort*  inPort = dynamic_cast<FNodeInPort*>(port);
  if (inPort) {
    myConnectedPort = inPort;
    isConnected = true;
  } else {
    throw WfStructException(WfStructException::eOTHER,
                            "source connected to invalid port");
  }
}

bool
FSourceNode::isConnectedToWfPort() {
  return isConnected;
}

void
FSourceNode::instanciate(Dag* dag) {
  TRACE_TEXT(TRACE_ALL_STEPS, traceId() << "Instanciate source\n");
  if (!isConnected) {
    myParser->parseXml(wf->getDataSrcXmlFile());
    myOutPort->uploadAllData(dag->getExecutionAgent());
    myOutPort->updateAllDataCardinal();
    myOutPort->sendAllData();
    // check if empty
    if (myOutPort->getBufferRootDH()->isEmpty()) {
      WARNING("Workflow source '" << getId() << "' contains no data\n");
    }
    // instanciation is completed when file has been read
    myStatus = N_INSTANC_END;
    // send event containing the serialized data tree
    std::ostringstream dataTree;
    myOutPort->writeAllDataAsXML(dataTree);
    events::sendEventFrom<FSourceNode,
                          FSourceNode::DATATREE>(this, "data tree",
                                                 dataTree.str(),
                                                 EventBase::INFO);

  } else {
    // do nothing
  }
  TRACE_TEXT(TRACE_ALL_STEPS, traceId() << "Instanciate source completed\n");
}

void
FSourceNode::createInstance(const FDataTag& currTag,
                            std::vector<FDataHandle*>& currDataLine) {
  if (isConnected) {
    FDataHandle* inDH = currDataLine[myConnectedPort->getIndex()];
    FDataHandle* outDH = new FDataHandle(*inDH);
    myOutPort->storeData(outDH);
    myOutPort->sendData(outDH);
  }
}

void
FSourceNode::toXML(std::ostream& output) {
  output << "<source name=\"" << this->getId() << "\">\n";
  myOutPort->writeAllDataAsXML(output);
  output << "</source>\n";
}

FDataHandle*
FSourceNode::createData(const FDataTag& tag) {
  return new FDataHandle(tag, getDataType(), 0);
}

FDataHandle*
FSourceNode::createList(const FDataTag& tag) {
  // depth may be wrong but it will be updated at the end by
  // updateAllDataCardinal on the whole data tree
  return new FDataHandle(tag, getDataType(), 1);
}

void
FSourceNode::setDataValue(FDataHandle* DH, const std::string& value) {
  DH->setValue(getDataType(), value);
}

void
FSourceNode::setDataID(FDataHandle* DH, const std::string& dataID) {
  DH->setDataID(dataID);
}

void
FSourceNode::setDataProperty(FDataHandle* DH,
                             const std::string& propKey,
                             const std::string& propValue) {
  DH->addProperty(propKey, propValue);
}

void
FSourceNode::insertData(FDataHandle* newDH) {
  try {
    myOutPort->storeData(newDH);
  } catch (WfDataHandleException& e) {
    WARNING(e.ErrorMsg());
  }
}

/*****************************************************************************/
/*                             FSinkNode                                     */
/*****************************************************************************/

std::string FSinkNode::inPortName("in");

FSinkNode::FSinkNode(FWorkflow* wf,
                     const std::string& id,
                     WfCst::WfDataType type,
                     unsigned int depth)
  : FNode(wf, id, N_INSTANC_READY), isConnected(false), myIterator(NULL) {
  WfPort * inPort = this->newPort(inPortName, 0, WfPort::PORT_IN, type, depth);
  myInPort = dynamic_cast<FNodeInPort*>(inPort);
  WfPort * outPort = this->newPort("out", 0, WfPort::PORT_OUT, type, depth);
  myOutPort = dynamic_cast<FNodeOutPort*>(outPort);
  myIterator = new PortInputIterator(myInPort);
}

FSinkNode::~FSinkNode() {
  // ports are deleted in ~WfNode()
}

const std::string&
FSinkNode::getDefaultPortName() const {
  return inPortName;
}

void
FSinkNode::connectToWfPort(FNodePort* port) {
  FNodeOutPort*  outPort = dynamic_cast<FNodeOutPort*>(port);
  if (outPort) {
    myConnectedPort = outPort;
    isConnected = true;
  } else {
    throw WfStructException(WfStructException::eOTHER,
                            "sink connected to invalid port");
  }
}

bool
FSinkNode::isConnectedToWfPort() {
  return isConnected;
}

void
FSinkNode::instanciate(Dag* dag) {
  // data items will be sent either to my own buffer (if sink is within the main
  // workflow) or to my parent workflow's out port (if sink is in a sub-wf)
  FNodeOutPort* outPort = isConnected ? myConnectedPort : myOutPort;
  // use a basic port iterator to retrieve items from the input port
  std::vector<FDataHandle*>* DL =
    new std::vector<FDataHandle*>(1, (FDataHandle*) NULL);
  myIterator->begin();
  while (!myIterator->isAtEnd()) {
    FDataTag currTag = myIterator->getCurrentItem(*DL);
    myIterator->removeItem();
    FDataHandle* inDH = DL->front();
    if (inDH == NULL) {
      INTERNAL_ERROR("NULL data handle provided to sink", 1);
    }
    FDataHandle* outDH = new FDataHandle(*inDH);
    outPort->storeData(outDH);
    outPort->sendData(outDH);
  }
  if (myIterator->isDone()) {
    TRACE_TEXT(TRACE_ALL_STEPS, traceId() <<  "ALL INPUTS PROCESSED\n");
    myStatus = N_INSTANC_END;
  }
}

void
FSinkNode::finalize() { }

void
FSinkNode::downloadResults() {
  myOutPort->downloadAllData();
  std::string rootDataID = myOutPort->getBufferContainerID();
  events::sendEventFrom<FSinkNode, FSinkNode::DATAID>(this,
                                                      "data id",
                                                      rootDataID,
                                                      EventBase::INFO);
}

void
FSinkNode::displayResults(std::ostream& output) {
  output << "## WF OUTPUT (" << myId << ") :\n";
  myOutPort->writeAllDataAsList(output);
}

void
FSinkNode::toXML(std::ostream& output) {
  output << "<sink name=\"" << this->getId() << "\">\n";
  myOutPort->writeAllDataAsXML(output);
  output << "</sink>\n";
}

void
FSinkNode::getResultsInContainer(std::string& containerID) {
  containerID = myOutPort->getBufferContainerID();
}

/****************************************************************************/
/*                             FProcNode                                    */
/****************************************************************************/
FProcNode::FProcNode(FWorkflow* wf,
                     const std::string& id)
  : FNode(wf, id, N_INSTANC_READY), myRootIterator(NULL), cstDataLine(NULL) {
}

FProcNode::~FProcNode() {
  if (cstDataLine) {
    delete cstDataLine;
  }
  while (! myIterators.empty()) {
    InputIterator * p = myIterators.begin()->second;
    myIterators.erase(myIterators.begin());
    delete p;
  }
}

void
FProcNode::initDataLine() {
  if (cstDataLine == NULL) {
    // this data vector will be used as template for the data vector of each
    // instance. Its size equals the total nb of ports (including out ports)
    // but only in/inout/param port indexes will be used.
    cstDataLine =
      new std::vector<FDataHandle*>(getPortNb(), (FDataHandle*) NULL);
  }
}

PortInputIterator *
FProcNode::createPortInputIterator(const std::string& portId) {
  WfPort *wfPort;
  try {
    wfPort = getPort(portId);
  } catch (WfStructException& e) {
    throw WfStructException(WfStructException::eOTHER,
                            "unknown input for operator creation :" +
                            e.Info());
  }
  FNodeInPort *inPort = dynamic_cast<FNodeInPort*>(wfPort);
  if (inPort == NULL) {
    throw WfStructException(WfStructException::eOTHER,
                            "input operator applied to an output port");
  }
  PortInputIterator *newPortIter = new PortInputIterator(inPort);
  myIterators[inPort->getId()] = (InputIterator*) newPortIter;
  return newPortIter;
}

bool
FProcNode::isIteratorDefined(const std::string& portId) {
  std::map<std::string, InputIterator*>::iterator iter =
    myIterators.find(portId);
  return (iter != myIterators.end());
}

const std::string&
FProcNode::createInputOperator(inputOperator_t opType,
                               const std::vector<std::string>& inputIds) {
  std::vector<std::string> _inputIds(inputIds);
  // convert N-cross to 2-cross using recursive call
  if (((opType == OPER_CROSS) || (opType == OPER_FLATCROSS))
      && (inputIds.size() > 2)) {
    std::vector<std::string> rightInputIds(++inputIds.begin(), inputIds.end());
    _inputIds.resize(2);
    _inputIds[0] = inputIds[0];
    _inputIds[1] = createInputOperator(opType, rightInputIds);
  }
  // convert inputIds to a table of iterator ref
  std::vector<InputIterator*> *inputIterTab =
    new std::vector<InputIterator*>(_inputIds.size());
  int inputIterIdx = 0;
  std::vector<std::string>::const_iterator idIter = _inputIds.begin();
  for (; idIter != _inputIds.end(); ++idIter) {
    const std::string& currId = *idIter;
    InputIterator* currInputIter = NULL;
    // first case: iterator already created => insert the ref in the table
    std::map<std::string, InputIterator*>::iterator iterFind =
      myIterators.find(currId);
    if (iterFind != myIterators.end()) {
      currInputIter = (InputIterator*) iterFind->second;
    } else {
      // second case: create iterator for single port
      currInputIter = (InputIterator*) createPortInputIterator(currId);
    }
    (*inputIterTab)[inputIterIdx++] = currInputIter;
  }
  InputIterator *newOper;
  switch (opType) {
  case OPER_DOT:
    newOper = new DotIterator(*inputIterTab);
    break;
  case OPER_MATCH:
    WARNING("Match iterator currently experimental");
    newOper = new MatchIterator((*inputIterTab)[0], (*inputIterTab)[1]);
    break;
  case OPER_CROSS:
    newOper = new CrossIterator((*inputIterTab)[0], (*inputIterTab)[1]);
    break;
  case OPER_FLATCROSS:
    newOper = new FlatCrossIterator((*inputIterTab)[0], (*inputIterTab)[1]);
    break;
  }
  delete inputIterTab;
  myIterators[newOper->getId()] = newOper;
  return newOper->getId();
}

void
FProcNode::setRootInputOperator(const std::string& opId) {
  std::map<std::string, InputIterator*>::iterator iterFind =
    myIterators.find(opId);
  if (iterFind != myIterators.end()) {
    myRootIterator = (InputIterator*) iterFind->second;
  } else {
    throw WfStructException(WfStructException::eOTHER,
                            "Root input operator '" + opId + "' not found");
  }
}

void
FProcNode::setRootInputOperator(InputIterator * newOper) {
  std::map<std::string, InputIterator*>::iterator iterFind =
    myIterators.find(newOper->getId());
  if (iterFind != myIterators.end()) {
    throw WfStructException(WfStructException::eOTHER,
                            "Root input operator '" + newOper->getId() +
                            "' already defined");
  } else {
    myIterators[newOper->getId()] = newOper;
    myRootIterator = newOper;
  }
}

void
FProcNode::setConstantInput(int idxPort, FDataHandle* dataHdl) {
  initDataLine();
  (*cstDataLine)[idxPort] = dataHdl;
}

bool
FProcNode::isConstantInput(int idxPort) {
  if (cstDataLine == NULL) {
    return false;
  }
  return ((*cstDataLine)[idxPort] != NULL);
}

void
FProcNode::checkDynamicParam(const std::string& paramName,
                             const std::string& paramValue) {
  // if value contains $ as first character then it's considered a variable
  std::string::size_type varTag = paramValue.find("$");
  if (varTag != 0) {
    return;
  }
  // get the name of the variable
  std::string paramVarName = paramValue.substr(1, paramValue.length() - 1);
  // check if it's a valid dynamic parameter
  if (!(paramName == "path")) {
    throw XMLParsingException(XMLParsingException::eINVALID_REF,
                              "attribute " + paramName + " cannot be dynamic");
  }
  // store the association name => variable
  setDynamicParam(paramName, paramVarName);
}

void
FProcNode::setDynamicParam(const std::string& paramName,
                           const std::string& paramVarName) {
  dynParMap.insert(std::make_pair(paramName, paramVarName));
}

bool
FProcNode::isDynamicParam(const std::string& paramName) {
  std::map<std::string, std::string>::iterator iter = dynParMap.find(paramName);
  return (iter != dynParMap.end());
}

void
FProcNode::setDynamicParamValue(const std::string& paramVarName,
                                const std::string& paramValue) {
  varMap[paramVarName] = paramValue;
  TRACE_TEXT(TRACE_ALL_STEPS, traceId() << "Set parameter variable: "
             << paramVarName
             << "(value = " << paramValue << ")\n");
}

const std::string&
FProcNode::getDynamicParamValue(const std::string& paramName) {
  // get the name of the variable containing this parameter
  std::string varName;
  std::map<std::string, std::string>::iterator parIter =
    dynParMap.find(paramName);
  if (parIter != dynParMap.end()) {
    varName = parIter->second;
  } else {
    ERROR_EXIT("dynamic parameter '" + paramName + "' has no variable set");
  }
  // get the value of the variable
  std::map<std::string, std::string>::iterator varIter = varMap.find(varName);
  if (varIter != varMap.end()) {
    return (varIter->second);
  } else {
    ERROR_EXIT("dynamic variable '" + varName + "' has no value set");
  }
}

void
FProcNode::initialize() {
  TRACE_TEXT(TRACE_ALL_STEPS, traceId() << "Initializing processor\n");
  std::vector<std::string> *iterCreatedMap =
    new std::vector<std::string>(getPortNb());
  unsigned int iterCreatedNb = 0;
  // loop for all input or param ports
  for (unsigned int ix = 0; ix < getPortNb(); ++ix) {
    const FNodePort * port = dynamic_cast<const FNodePort*>(getPortByIndex(ix));
    std::string portId = port->getId();
    // check if current port is an input and is not constant
    if (((port->getPortType() == WfPort::PORT_IN)
         || (port->getPortType() == WfPort::PORT_INOUT)
         || (port->getPortType() == WfPort::PORT_PARAM))
        && !isConstantInput(ix)
        && !isIteratorDefined(portId)) {
      // create an iterator for the current port
      createPortInputIterator(portId);
      (*iterCreatedMap)[iterCreatedNb++] = portId;
    }
  }
  if (iterCreatedNb > 0) {
    if (iterCreatedNb == 1) {
      setRootInputOperator((*iterCreatedMap)[0]);
    } else {
      iterCreatedMap->resize(iterCreatedNb);
      TRACE_TEXT(TRACE_ALL_STEPS,
                 "Creating default DOT iterator for all inputs\n");
      std::string parentIter = createInputOperator(OPER_DOT, *iterCreatedMap);
      setRootInputOperator(parentIter);
    }
  }
  // Initialize the dataline (in case it was not created before)
  initDataLine();
  delete iterCreatedMap;
}

void
FProcNode::initInstanciation() {
}

bool
FProcNode::instLimitReached() {
  return false;
}

void
FProcNode::createVoidInstance(const FDataTag& currTag,
                              std::vector<FDataHandle*>& currDataLine) {
  TRACE_TEXT(TRACE_MAIN_STEPS, "  ## NEW VOID INSTANCE : " << getId()
             << currTag.toString() << "\n");
  // LOOP for each out port
  std::map<std::string, WfPort*>::iterator portIter = ports.begin();
  for (; portIter != ports.end(); ++portIter) {
    WfPort* port = (WfPort*) portIter->second;
    if (port->isOutput()) {
      FNodeOutPort* outPort = dynamic_cast<FNodeOutPort*>(port);
      // instanciate port with VOID data
      FDataHandle* dataHdl = outPort->createVoidInstance(currTag);
      // send data to connected nodes
      outPort->storeData(dataHdl);
      outPort->sendData(dataHdl);
    }
  }
  TRACE_TEXT(TRACE_ALL_STEPS, "  ## END OF VOID INSTANCE\n");
}

void
FProcNode::instanciate(Dag* dag) {
  if (instanciationReady()) {
    // Initialize the root input iterator
    if (!myRootIterator) {
      INTERNAL_ERROR("FProcNode::instanciate : no root iterator defined",
                     1);
    }
    myRootIterator->begin();

    // Copy the constant dataline into the dataline used for the iteration
    if (!cstDataLine) {
      INTERNAL_ERROR("FProcNode::instanciate : no constant dataline defined",
                     1);
    }
    std::vector<FDataHandle*>* currDataLine =
      new std::vector<FDataHandle*>(*cstDataLine);

    // Other initializations
    initInstanciation();

    //
    // LOOP OVER ITEMS in the INPUT QUEUE
    //
    while (!myRootIterator->isAtEnd() && !instLimitReached()) {
      // GET DATALINE
      FDataTag currTag = myRootIterator->getCurrentItem(*currDataLine);
      myRootIterator->removeItem();  // goes to next item at the same time

      // CHECK IF VOID
      bool dataIsVoid = false;
      std::vector<FDataHandle*>::const_iterator DLIter = currDataLine->begin();
      while ((!dataIsVoid) && (DLIter != currDataLine->end())) {
        FDataHandle* currDataHdl = *(DLIter++);
        dataIsVoid = currDataHdl ? currDataHdl->isVoid() : dataIsVoid;
      }

      if (!dataIsVoid) {
        // SET DYNAMIC PARAMETERS
        varMap.clear();
        std::map<std::string, WfPort*>::iterator portIter = ports.begin();
        for (; portIter != ports.end(); ++portIter) {
          WfPort* port = portIter->second;
          if (port->getPortType() == WfPort::PORT_PARAM) {
            FNodeParamPort* paramPort = dynamic_cast<FNodeParamPort*>(port);
            FDataHandle* dataHdl = (*currDataLine)[paramPort->getIndex()];
            setDynamicParamValue(paramPort->getId(), dataHdl->getValue());
          }
        }

        // create a real instance of the node
        createRealInstance(dag, currTag, *currDataLine);
      } else {
        createVoidInstance(currTag, *currDataLine);
      }
    }
    //
    // END LOOP
    //

    updateInstanciationStatus();

    delete currDataLine;
  }
}


void
FProcNode::updateInstanciationStatus() {
  if (myRootIterator->isAtEnd()) {
    if (myRootIterator->isDone()) {
      TRACE_TEXT(TRACE_ALL_STEPS,
                 traceId() <<  "########## ALL INPUTS PROCESSED\n");
      myStatus = N_INSTANC_END;

    } else {
      TRACE_TEXT(TRACE_ALL_STEPS,
                 traceId() <<  "########## WAITING FOR INPUTS \n");
      myStatus = N_INSTANC_READY;
    }
  }
}

bool
FProcNode::hasDataToProcess() {
  myRootIterator->begin();
  return (!myRootIterator->isAtEnd());
}
