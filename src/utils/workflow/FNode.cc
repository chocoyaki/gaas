/****************************************************************************/
/* The class representing the nodes of a functional workflow                */
/*                                                                          */
/* Author(s):                                                               */
/* - Benjamin ISNARD (benjamin.isnard@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.11  2009/04/17 09:04:07  bisnard
 * initial version for conditional nodes in functional workflows
 *
 * Revision 1.10  2009/04/09 09:56:20  bisnard
 * refactoring due to new class FActivityNode
 *
 * Revision 1.9  2009/04/08 09:34:56  bisnard
 * pending nodes mgmt moved to FWorkflow class
 * FWorkflow and FNode state graph revisited
 * FNodePort instanciation refactoring
 *
 * Revision 1.8  2009/02/24 14:01:05  bisnard
 * added dynamic parameter mgmt for wf processors
 *
 * Revision 1.7  2009/02/20 10:23:54  bisnard
 * use estimation class to reduce the nb of submit requests
 *
 * Revision 1.6  2009/02/06 14:55:08  bisnard
 * setup exceptions
 *
 * Revision 1.5  2009/01/16 16:31:54  bisnard
 * added option to specify data source file name
 *
 * Revision 1.4  2009/01/16 13:54:50  bisnard
 * new version of the dag instanciator with iteration strategies for nodes with multiple input ports
 *
 * Revision 1.3  2008/12/09 12:15:59  bisnard
 * pending instanciation handling (uses dag outputs for instanciation
 * of a functional wf)
 *
 * Revision 1.2  2008/12/02 10:09:36  bisnard
 * added instanciation methods
 *
 * Revision 1.1  2008/10/20 08:09:58  bisnard
 * new classes for functional workflows
 *
 */

#include <string>
#include "debug.hh"
#include "FNode.hh"
#include "FWorkflow.hh"
#include "DagNodePort.hh"
#include "DagWfParser.hh"

using namespace std;

FNode::FNode(FWorkflow* wf, const string& id, nodeInstStatus_t initStatus)
  : WfNode(id), wf(wf), myStatus(initStatus) {}

FNode::~FNode() {
  TRACE_TEXT (TRACE_ALL_STEPS,"~FNode() " << getId() << " destructor ..." <<  endl);
}

FWorkflow *
FNode::getWorkflow() {
  return wf;
}

const string&
FNode::getDefaultPortName() const {
  INTERNAL_ERROR("FNode::getDefaultPortName not defined for this class",1);
}

/* protected */
void
FNode::setStatusReady() {
  if (!instanciationOnHold())
    myStatus = N_INSTANC_READY;
}

bool
FNode::instanciationReady() {
  return (myStatus == N_INSTANC_READY);
}

bool
FNode::instanciationWaiting() {
  return (myStatus == N_INSTANC_WAITING);
}

bool
FNode::instanciationOnHold() {
  return (myStatus == N_INSTANC_ONHOLD);
}

bool
FNode::instanciationCompleted() {
  return (myStatus == N_INSTANC_END);
}

void
FNode::resumeInstanciation() {
  if (!instanciationCompleted())
    myStatus = N_INSTANC_READY;
}

WfPort *
FNode::newPort(string portId,
               unsigned int ind,
               WfPort::WfPortType portType,
               WfCst::WfDataType dataType,
               unsigned int depth) throw (WfStructException) {
  if (isPortDefined(portId))
    throw WfStructException(WfStructException::eDUPLICATE_PORT,"port id="+portId);
  WfPort * p = NULL;
  switch (portType) {
    case WfPort::PORT_PARAM:
      p = new FNodeParamPort(this, portId, dataType, ind);
      break;
    case WfPort::PORT_IN:
      p = new FNodeInPort(this, portId, dataType, depth, ind);
      break;
    case WfPort::PORT_INOUT:
      INTERNAL_ERROR("Functional Inout port not implemented yet",0);
      /* p = new FNodeInOutPort(this, portId, dataType, depth, ind); */
      break;
    case WfPort::PORT_OUT:
      p = new FNodeOutPort(this, portId, dataType, depth, ind);
      break;
  }
  this->ports[portId] = p;
  return p;
}

void
FNode::connectNodePorts() throw (WfStructException) {
  TRACE_TEXT (TRACE_ALL_STEPS,
 	      "connectNodePorts : processing node " << getId() << endl);
  for (map<string, WfPort*>::iterator p = ports.begin();
       p != ports.end();
       ++p) {
    try {
      ((WfPort*)(p->second))->connectPorts();
    } catch (WfStructException& e) {
      // depth mismatch is ok for functional nodes connections
      if (e.Type() != WfStructException::eDEPTH_MISMATCH)
        throw;
    }
  }
}

void
FNode::initialize() {}

/*****************************************************************************/
/*                           FConstantNode                                   */
/*****************************************************************************/
string FConstantNode::outPortName("out");

const string&
FConstantNode::getDefaultPortName() const {
  return outPortName;
}

FConstantNode::FConstantNode(FWorkflow* wf, const string& id,
                             WfCst::WfDataType type)
  : FNode(wf, id, N_INSTANC_READY) {
  this->newPort(outPortName,0,WfPort::PORT_OUT,type,0);
}
FConstantNode::~FConstantNode() {}

void
FConstantNode::setValue(const string& strVal) {
  myValue = strVal;
}

void
FConstantNode::instanciate(Dag* dag) {
  // create a WfDataHandle and set its value
  FDataTag singleTag(0,true);
  FDataHandle* dataHdl = new FDataHandle(singleTag, myValue);
  // set the out port as constant with this dataHdl
  FNodeOutPort* outPort = dynamic_cast<FNodeOutPort*>(ports[outPortName]);
  if (!outPort) {
    INTERNAL_ERROR("Missing out port in constant FNode",0);
  }
  outPort->setAsConstant(dataHdl);
  myStatus = N_INSTANC_END;
}

/*****************************************************************************/
/*                            FSourceNode                                    */
/*****************************************************************************/
string FSourceNode::outPortName("out");

FSourceNode::FSourceNode(FWorkflow* wf,
                         const string& id,
                         WfCst::WfDataType type)
  : FNode(wf, id, N_INSTANC_READY), myParser(NULL) {
  WfPort * outPort = this->newPort(outPortName,0,WfPort::PORT_OUT,type,0);
  myOutPort = dynamic_cast<FNodeOutPort*>(outPort);
  myParser = new DataSourceParser(id);
}

FSourceNode::~FSourceNode() {
  delete myParser;
}

const string&
FSourceNode::getDefaultPortName() const {
  return outPortName;
}

void
FSourceNode::initialize() {
  TRACE_TEXT (TRACE_ALL_STEPS,"Initializing data source :" << getId() << endl);
  TRACE_TEXT (TRACE_ALL_STEPS,"  1/ Parse XML data file :" << endl);
  myParser->parseXml(wf->getDataSrcXmlFile());
}

void
FSourceNode::instanciate(Dag* dag) {
  unsigned int valCount = 0;
  // LOOP while file contains data items
  while (!myParser->end()) {
    TRACE_TEXT (TRACE_ALL_STEPS, "Source " << getId()
                  << " : Found one value in XML file" << endl);
      // get the data item
    string * currVal = myParser->getValue();
      // move the parser to the next value
    try {
      myParser->goToNextValue();
    } catch (XMLParsingException& exception) {
      cerr <<  "Data source " << getId() << " XML Parsing error: "
            << exception.Info() << endl;
      delete currVal;
      break;
    }
      // if this is the last item, then set the tag as last
    bool isLast = myParser->end();
    FDataTag currTag(valCount++, isLast);
      // create data handle with the data item
    FDataHandle* currDataHdl = new FDataHandle(currTag, *currVal);
      // instanciate out port with this data handle
    myOutPort->instanciate(currDataHdl);
      // free data
    delete currVal;
  }
  myStatus = N_INSTANC_END;
}

/*****************************************************************************/
/*                             FSinkNode                                     */
/*****************************************************************************/

string FSinkNode::inPortName("in");

FSinkNode::FSinkNode(FWorkflow* wf,
                     const string& id,
                     WfCst::WfDataType type,
                     unsigned int depth)
  : FNode(wf, id, N_INSTANC_WAITING) {
  WfPort * inPort = this->newPort(inPortName,0,WfPort::PORT_IN,type,depth);
  myInPort = dynamic_cast<FNodeInPort*>(inPort);
}

FSinkNode::~FSinkNode() {
}

const string&
FSinkNode::getDefaultPortName() const {
  return inPortName;
}

void
FSinkNode::initialize() {
  TRACE_TEXT (TRACE_ALL_STEPS,"Initializing data sink :" << getId() << endl);
  TRACE_TEXT (TRACE_ALL_STEPS,"  1/ Connecting node ports" << endl);
  this->connectNodePorts();
}

void
FSinkNode::instanciate(Dag* dag) {
}

void
FSinkNode::displayResults(ostream& output) {
  output << "## WF OUTPUT (" << myId << ") :" << endl;
  myInPort->displayData(output);
}

/*****************************************************************************/
/*                             FProcNode                                     */
/*****************************************************************************/

FProcNode::FProcNode(FWorkflow* wf,
                     const string& id)
  : FNode(wf, id, N_INSTANC_WAITING), myRootIterator(NULL), cstDataLine(NULL)
{}

FProcNode::~FProcNode() {
  if (cstDataLine)
    delete cstDataLine;
  while (! myIterators.empty() ) {
    InputIterator * p = myIterators.begin()->second;
    myIterators.erase( myIterators.begin() );
    delete p;
  }
}

void
FProcNode::initDataLine() {
  if (cstDataLine == NULL) {
    unsigned int inPortNb = getPortNb(WfPort::PORT_IN) + getPortNb(WfPort::PORT_PARAM);
    cstDataLine = new vector<FDataHandle*>(inPortNb, (FDataHandle*) NULL);
    TRACE_TEXT (TRACE_ALL_STEPS,"Created base dataline ("
                << inPortNb << " ports)" << endl);
  }
}

PortInputIterator *
FProcNode::createPortInputIterator(const string& portId) {
  WfPort *wfPort;
  try {
    wfPort = getPort(portId);
  } catch (WfStructException& e) {
    throw WfStructException(WfStructException::eOTHER,
                              "unknown input for operator creation :" + e.Info());
  }
  FNodeInPort *inPort = dynamic_cast<FNodeInPort*>(wfPort);
  if (inPort == NULL)
    throw WfStructException(WfStructException::eOTHER,
                              "input operator applied to an output port");
  PortInputIterator *newPortIter = new PortInputIterator(inPort);
  myIterators[inPort->getId()] = (InputIterator*) newPortIter;
  return newPortIter;
}

bool
FProcNode::isIteratorDefined(const string& portId) {
  map<string,InputIterator*>::iterator iter = myIterators.find(portId);
  return (iter != myIterators.end());
}

const string&
FProcNode::createInputOperator(inputOperator_t opType, const vector<string>& inputIds) {
  // convert inputIds to a table of iterator ref
  vector<InputIterator*> *inputIterTab = new vector<InputIterator*>(inputIds.size());
  int inputIterIdx = 0;
  for (vector<string>::const_iterator idIter = inputIds.begin();
       idIter != inputIds.end();
       ++idIter) {
    const string& currId = *idIter;
    InputIterator* currInputIter = NULL;
    // first case: iterator already created => insert the ref in the table
    map<string,InputIterator*>::iterator iterFind = myIterators.find(currId);
    if (iterFind != myIterators.end()) {
      currInputIter = (InputIterator*) iterFind->second;
    } else {
    // second case: create iterator for single port
      currInputIter = (InputIterator*) createPortInputIterator(currId);
    }
    (*inputIterTab)[inputIterIdx++] = currInputIter;
  } // end for
  InputIterator *newOper;
  switch (opType) {
    case OPER_DOT:
      newOper = new DotIterator(*inputIterTab);
      break;
    case OPER_MATCH:
      INTERNAL_ERROR("Match operator not implemented yet",1);
      /* newOper = new MatchIterator(*inputIterTab); */
      break;
    case OPER_CROSS:
      newOper = new CrossIterator((*inputIterTab)[0], (*inputIterTab)[1]);
      break;
  }
  delete inputIterTab;
  myIterators[newOper->getId()] = newOper;
  return newOper->getId();
}

void
FProcNode::setRootInputOperator(const string& opId) {
  map<string,InputIterator*>::iterator iterFind = myIterators.find(opId);
  if (iterFind != myIterators.end()) {
    myRootIterator = (InputIterator*) iterFind->second;
  } else
    throw WfStructException(WfStructException::eOTHER,
                              "Root input operator '"+opId+"' not found");
}

void
FProcNode::setConstantInput(int idxPort, FDataHandle* dataHdl) {
  initDataLine();
  (*cstDataLine)[idxPort] = dataHdl;
}

bool
FProcNode::isConstantInput(int idxPort) {
  if (cstDataLine == NULL) return false;
  return ((*cstDataLine)[idxPort] != NULL);
}

void
FProcNode::checkDynamicParam(const string& paramName,
                             const string& paramValue) {
  // if value contains $ as first character then it's considered a variable
  string::size_type varTag = paramValue.find("$");
  if (varTag != 0) return;
  // get the name of the variable
  string paramVarName = paramValue.substr(1, paramValue.length()-1);
  // check if it's a valid dynamic parameter
  if (!(paramName == "path"))
    throw XMLParsingException(XMLParsingException::eINVALID_REF,
                              "attribute "+paramName+" cannot be dynamic");
  // store the association name => variable
  setDynamicParam(paramName, paramVarName);
}

void
FProcNode::setDynamicParam(const string& paramName,
                           const string& paramVarName) {
  dynParMap.insert(make_pair(paramName, paramVarName));
}

bool
FProcNode::isDynamicParam(const string& paramName) {
  map<string,string>::iterator iter = dynParMap.find(paramName);
  return (iter != dynParMap.end());
}

void
FProcNode::setDynamicParamValue(const string& paramVarName,
                                const string& paramValue) {
  varMap[paramVarName] = paramValue;
  TRACE_TEXT (TRACE_ALL_STEPS,"Set parameter variable: " << paramVarName
                              << "(value = " << paramValue << ")" << endl);
}

const string&
FProcNode::getDynamicParamValue(const string& paramName) {
  // get the name of the variable containing this parameter
  string varName;
  map<string,string>::iterator parIter = dynParMap.find(paramName);
  if (parIter != dynParMap.end())
    varName = parIter->second;
  else ERROR_EXIT("dynamic parameter '"+paramName+"' has no variable set");
  // get the value of the variable
  map<string,string>::iterator varIter = varMap.find(varName);
  if (varIter != varMap.end())
    return (varIter->second);
  else ERROR_EXIT("dynamic variable '"+varName+"' has no value set");
}

void
FProcNode::initialize() {
  TRACE_TEXT (TRACE_ALL_STEPS,"Initializing processor :" << getId() << endl);
  TRACE_TEXT (TRACE_ALL_STEPS,"  1/ Connecting node ports" << endl);
  this->connectNodePorts();
  TRACE_TEXT (TRACE_ALL_STEPS,"  2/ Checking input iterators" << endl);
  vector<string> *iterCreatedMap = new vector<string>(getPortNb());
  unsigned int iterCreatedNb = 0;
  // loop for all input or param ports
  for (int ix = 0; ix < getPortNb(); ++ix) {
    const FNodePort * port = dynamic_cast<const FNodePort*>(getPortByIndex(ix));
    string portId = port->getId();
    // check if current port is an input and is not constant
    if (((port->getPortType() == WfPort::PORT_IN)
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
      TRACE_TEXT (TRACE_ALL_STEPS,"Creating default DOT iterator for all inputs");
      string parentIter = createInputOperator(OPER_DOT, *iterCreatedMap);
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
FProcNode::instanciate(Dag* dag) {
  if (instanciationReady()) {

    // Initialize the root input iterator
    if (!myRootIterator) {
      INTERNAL_ERROR("FProcNode::instanciate : no root iterator defined",1);
    }
    myRootIterator->begin();

    // Copy the constant dataline into the dataline used for the iteration
    if (!cstDataLine) {
      INTERNAL_ERROR("FProcNode::instanciate : no constant dataline defined",1);
    }
    vector<FDataHandle*>* currDataLine = new vector<FDataHandle*>(*cstDataLine);

    // Other initializations
    initInstanciation();

    //
    // LOOP OVER ITEMS in the INPUT QUEUE
    //
    while ( !myRootIterator->isAtEnd() && !instLimitReached()) {

      // GET DATALINE
      TRACE_TEXT (TRACE_ALL_STEPS," ## Retrieve dataline from iterator..." << endl);
      FDataTag currTag = myRootIterator->getCurrentItem(*currDataLine);
      TRACE_TEXT (TRACE_ALL_STEPS," ## Remove selected item (" << currTag.toString() << ")" << endl);
      myRootIterator->removeItem(); // goes to next item at the same time

      // SET DYNAMIC PARAMETERS
      varMap.clear();
      for (map<string,WfPort*>::iterator portIter = ports.begin();
           portIter != ports.end();
           ++portIter) {
        WfPort* port = (WfPort*) portIter->second;
        if (port->getPortType() == WfPort::PORT_PARAM) {
          FNodeParamPort* paramPort = dynamic_cast<FNodeParamPort*>(port);
          FDataHandle* dataHdl = (*currDataLine)[paramPort->getIndex()];
          if (dataHdl == NULL) {
            INTERNAL_ERROR("Input data handle is invalid (dyn par)",1);
          }
          if (!dataHdl->isValueDefined()) { //TODO replace by exceptions
            INTERNAL_ERROR("Input data handle has no value (dyn par)",1);
          }
          setDynamicParamValue(paramPort->getId(), dataHdl->getValue());
        }
      }

      // create instance
      createInstance(dag, currTag, *currDataLine);

    }
    //
    // END LOOP
    //

    updateInstanciationStatus();

    delete currDataLine;
  }
}

