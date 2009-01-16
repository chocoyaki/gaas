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

#include "debug.hh"
#include "FNode.hh"
#include "FWorkflow.hh"
#include "DagNodePort.hh"
#include "DagWfParser.hh"

using namespace std;

FNode::FNode(FWorkflow* wf, const string& id)
  : Node(id),
    wf(wf),
    isFullInst(false),
    isOnHoldFlag(false) {}

FNode::~FNode() {
  TRACE_TEXT (TRACE_ALL_STEPS,"~FNode() " << getId() << " destructor ..." <<  endl);
}

const string&
FNode::getDefaultPortName() const {
  INTERNAL_ERROR("FNode::getDefaultPortName not defined for this class",1);
}

bool
FNode::instanciationOnHold() {
  return isOnHoldFlag;
}

void
FNode::resumeInstanciation() {
  isOnHoldFlag = false;
}

bool
FNode::instanciationCompleted() {
  return isFullInst;
}

WfPort *
FNode::newPort(string portId,
               unsigned int ind,
               WfPort::WfPortType portType,
               WfCst::WfDataType dataType,
               unsigned int depth) {
  WfPort * p = NULL;
  switch (portType) {
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
  : FNode(wf, id) {
  this->newPort(outPortName,0,WfPort::PORT_OUT,type,0);
}
FConstantNode::~FConstantNode() {}

void
FConstantNode::setValue(const string& strVal) {
  myValue = strVal;
}

void
FConstantNode::instanciate(Dag* dag) {
  if (!instanciationOnHold() && !instanciationCompleted()) {
    // create a WfDataHandle and set its value
    FDataTag singleTag(0,true);
    FDataHandle* dataHdl = new FDataHandle(singleTag, myValue);
    // set the out port as constant with this dataHdl
    FNodeOutPort* outPort = dynamic_cast<FNodeOutPort*>(ports[outPortName]);
    if (!outPort) {
      INTERNAL_ERROR("Missing out port in constant FNode",0);
    }
    outPort->setAsConstant(dataHdl);
    this->isFullInst = true;
  }
}

/*****************************************************************************/
/*                            FSourceNode                                    */
/*****************************************************************************/
string FSourceNode::outPortName("out");

FSourceNode::FSourceNode(FWorkflow* wf,
                         const string& id,
                         WfCst::WfDataType type)
  : FNode(wf, id), myParser(NULL) {
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
  if (!instanciationOnHold() && !instanciationCompleted()) {
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
  } // end if
}

/*****************************************************************************/
/*                             FSinkNode                                     */
/*****************************************************************************/

string FSinkNode::inPortName("in");

FSinkNode::FSinkNode(FWorkflow* wf,
                     const string& id,
                     WfCst::WfDataType type)
  : FNode(wf, id) {
  WfPort * inPort = this->newPort(inPortName,0,WfPort::PORT_IN,type,0);
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

/*****************************************************************************/
/*                             FProcNode                                     */
/*****************************************************************************/

FProcNode::FProcNode(FWorkflow* wf,
                     const string& id)
  : FNode(wf, id), maxInstNb(0), myPath(), myRootIterator(NULL),
    cstDataLine(NULL) {}

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
FProcNode::setDIETServicePath(const string& path) {
  myPath = path;
}

void
FProcNode::setMaxInstancePerDag(short maxInst) {
  maxInstNb = maxInst;
}

void
FProcNode::initDataLine() {
  if (cstDataLine == NULL) {
    cstDataLine = new vector<FDataHandle*>(getInPortNb());
    TRACE_TEXT (TRACE_ALL_STEPS,"Created base dataline ("
                << getInPortNb() << " ports)" << endl);
  }
}

PortInputIterator *
FProcNode::createPortInputIterator(const string& portId) {
  WfPort *wfPort = getPort(portId);
  if (wfPort == NULL)
    throw XMLParsingException(XMLParsingException::eBAD_STRUCT,
                              "unknown input for operator creation (" + portId + ")");
  FNodeInPort *inPort = dynamic_cast<FNodeInPort*>(wfPort);
  if (inPort == NULL)
    throw XMLParsingException(XMLParsingException::eBAD_STRUCT,
                              "input operator applied to an output port");
  PortInputIterator *newPortIter = new PortInputIterator(inPort);
  myIterators[inPort->getId()] = (InputIterator*) newPortIter;
  return newPortIter;
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
    throw XMLParsingException(XMLParsingException::eBAD_STRUCT,
                              "root input operator not found");
}

void
FProcNode::setConstantInput(int idxPort, FDataHandle* dataHdl) {
  initDataLine();
  (*cstDataLine)[idxPort] = dataHdl;
}

void
FProcNode::initialize() {
  TRACE_TEXT (TRACE_ALL_STEPS,"Initializing processor :" << getId() << endl);
  TRACE_TEXT (TRACE_ALL_STEPS,"  1/ Connecting node ports" << endl);
  this->connectNodePorts();
  TRACE_TEXT (TRACE_ALL_STEPS,"  2/ Checking input iterators" << endl);
  if (!myRootIterator) {
    if (this->getInPortNb() > 1) {
      INTERNAL_ERROR("no root iterator defined for processor node",1);
    } else {
      // define a default iterator for single port
      WfPort *port = this->getPortByIndex(0);
      PortInputIterator *inputIter = createPortInputIterator(port->getId());
      this->setRootInputOperator(inputIter->getId());
    }
  }
  // Initialize the dataline (in case it was not created before)
  initDataLine();
}

void
FProcNode::instanciate(Dag* dag) {
  if (!instanciationOnHold() && !isFullInst) {
    TRACE_TEXT (TRACE_ALL_STEPS,"#### Processor " << this->getId()
                    << " instanciation START" << endl);

    // Initialize the root input iterator
    if (!myRootIterator) {
      INTERNAL_ERROR("FProcNode::instanciate : no root iterator defined",1);
    }
    myRootIterator->begin();

    // Initialize the instance limit (if defined)
    int  nbInstances = 0;
    bool limitInstances = this->maxInstNb != 0;
    if (limitInstances) {
      TRACE_TEXT (TRACE_ALL_STEPS, "########## INSTANCE COUNT LIMIT : "
          << maxInstNb << endl);
    }

    // Copy the constant dataline into the dataline used for the iteration
    if (!cstDataLine) {
      INTERNAL_ERROR("FProcNode::instanciate : no constant dataline defined",1);
    }

    vector<FDataHandle*>* currDataLine = new vector<FDataHandle*>(*cstDataLine);

    TRACE_TEXT (TRACE_ALL_STEPS,"Starting iterator loop" << endl);
    // LOOP OVER ITEMS in the INPUT QUEUE
    // (until queue is empty OR until limit of max instances is reached)
    while ( !myRootIterator->isAtEnd()
            && (!limitInstances || (++nbInstances <= maxInstNb))) {
      TRACE_TEXT (TRACE_ALL_STEPS," ## Retrieve dataline from iterator..." << endl);
      // get the data line
      FDataTag currTag = myRootIterator->getCurrentItem(*currDataLine);
      TRACE_TEXT (TRACE_ALL_STEPS," ## Remove selected item (" << currTag.toString() << ")" << endl);
      myRootIterator->removeItem(); // goes to next item at the same time
      // create a new DagNode
      string   dagNodeId = this->getId() + currTag.toString();
      TRACE_TEXT (TRACE_ALL_STEPS,"  ## NEW NODE INSTANCE : " << dagNodeId << endl);
      DagNode* dagNode = dag->createDagNode(dagNodeId);
      if (dagNode == NULL) {
        INTERNAL_ERROR("Could not create dag node " << dagNodeId << endl,0);
      }
      dagNode->setPbName(myPath);
      dagNode->setFNode(this);
      // LOOP for each port
      for (map<string,WfPort*>::iterator portIter = ports.begin();
           portIter != ports.end();
           ++portIter) {
        WfPort* port = (WfPort*) portIter->second;
        FNodeInPort* inPort;
        FNodeOutPort* outPort;
        FDataHandle* dataHdl;
        switch(port->getPortType()) {
          case WfPort::PORT_IN:
            inPort = dynamic_cast<FNodeInPort*>(port);
            // get the input data handle from the map (if not found set as NULL)
            dataHdl = (*currDataLine)[inPort->getIndex()];
            if (dataHdl == NULL) {
              INTERNAL_ERROR("Input data handle is invalid",1);
            }
            // instanciate port with data handle
            inPort->instanciate(dag, dagNode, dataHdl);
            break;
          case WfPort::PORT_OUT:
            outPort = dynamic_cast<FNodeOutPort*>(port);
            // instanciate port with dagNode and tag
            outPort->instanciate(dag, dagNode, currTag);
            break;
          case WfPort::PORT_INOUT:
            ;
        }
      } // end for ports
      TRACE_TEXT (TRACE_ALL_STEPS,"  ## END OF CREATION OF NEW NODE INSTANCE : "
                                << dagNodeId << endl);
    }
    //
    // END LOOP
    //

    if (myRootIterator->isAtEnd()) {
      TRACE_TEXT (TRACE_ALL_STEPS, "########## NO MORE MATCHES FOR NODE INPUTS " << endl);
      if (myRootIterator->isDone()) {
        TRACE_TEXT (TRACE_ALL_STEPS, "########## ALL INPUTS PROCESSED" << endl);
        this->isFullInst = true;
      }
    } else {
      TRACE_TEXT (TRACE_ALL_STEPS, "########## SET NODE ON HOLD" << endl);
      this->isOnHoldFlag = true;
    }

    delete currDataLine;
  } // end if (!isFullyInstantiated())
}

void
FProcNode::setPendingInstanceInfo(DagNode * dagNode,
                                      FDataHandle * dataHdl,
                                      FNodeOutPort * outPort,
                                      FNodeInPort * inPort) {
  pendingDagNodeInfo_t info;
  info.dataHdl = dataHdl;
  info.outPort = outPort;
  info.inPort  = inPort;
  pendingNodes.insert(make_pair(dagNode,info));
  TRACE_TEXT (TRACE_ALL_STEPS, "[" << getId() << "] : added one entry in pending list (size = "
      << pendingNodes.size() << ")" << endl);
}

bool
FProcNode::instanciationPending() {
  return (pendingNodes.size() > 0);
}

bool
FProcNode::instanciationCompleted() {
  return isFullInst && !instanciationPending();
}

void
FProcNode::instanceIsDone(DagNode * dagNode, bool& statusChange) {
  // search the instance in the pending list
  multimap<DagNode*, pendingDagNodeInfo_t>::iterator pendingIter, pendingStart;
  pendingStart = pendingIter = pendingNodes.lower_bound(dagNode);
  // if found, resubmit the datahandle to the in port(s)
  while (pendingIter != pendingNodes.upper_bound(dagNode)) {
    pendingDagNodeInfo_t info = (pendingDagNodeInfo_t) (pendingIter++)->second;
    // get the dag port providing the data
    WfPort * portPtr = dagNode->getPort(info.outPort->getId());
    DagNodeOutPort * dagOutPort = dynamic_cast<DagNodeOutPort*>(portPtr);
    // submit data to the in port
    TRACE_TEXT (TRACE_ALL_STEPS,"[" << getId() << "] : Process pending entry for node "
                                << dagNode->getId() << endl);
    if (!info.outPort->addDataToInPort(info.inPort, info.dataHdl, dagOutPort)) {
      INTERNAL_ERROR("Error during data transfer to IN port",1);
    }
    // re-start instanciation
    statusChange = true;
  }
  // remove entries from the pending list
  pendingNodes.erase(pendingStart,pendingIter);
}

/*****************************************************************************/
/*                           FConditionNode                                  */
/*****************************************************************************/

FConditionNode::FConditionNode(FWorkflow* wf, const string& id)
  : FNode(wf, id) {}
FConditionNode::~FConditionNode() {}


void
FConditionNode::instanciate(Dag* dag) {
}

