/**
* @file  FLoopNode.cc
* 
* @brief  The classes representing the for/while loops of a functional workflow
* 
* @author  Benjamin ISNARD (Benjamin.Isnard@ens-lyon.fr)
* 
* @section Licence
*   |LICENSE|                                                                
*/


#include "FLoopNode.hh"

class FLoopInPort : public FNodeInPort {
  friend class FLoopNode;
public:
  FLoopInPort(WfNode * parent, const std::string& _id,
              WfPort::WfPortType _portType, WfCst::WfDataType _type,
              unsigned int _depth, unsigned int _ind)
    : FNodePort(parent, _id, _portType, _type, _depth, _ind),
      FNodeInPort(parent, _id, _portType, _type, _depth, _ind),
      setPrecedence(false) {
  }

  virtual void
  setNodePrecedence(NodeSet * contextNodeSet) throw(WfStructException);

private:
  bool setPrecedence;
};



class FLoopOutPort : public FNodeOutPort {
public:
  FLoopOutPort(WfNode * parent, const std::string& _id,
               WfPort::WfPortType _portType, WfCst::WfDataType _type,
               unsigned int _depth, unsigned int _ind)
    : FNodePort(parent, _id, _portType, _type, _depth, _ind),
      FNodeOutPort(parent, _id, _portType, _type, _depth, _ind) {
  }
};

void
FLoopInPort::setNodePrecedence(NodeSet * contextNodeSet)
  throw(WfStructException) {
  if (setPrecedence) {
    WfPort::setNodePrecedence(contextNodeSet);
  }
}

FLoopNode::FLoopNode(FWorkflow* wf, const std::string& id)
  : FProcNode(wf, id) , activeInstanceNb(0), loopTagLength(0) {
  myCondition = new WfBooleanExpression();
  myConditionVars = NULL;
}

FLoopNode::~FLoopNode() {
}

WfPort *
FLoopNode::newPort(std::string portId, unsigned int ind,
                   WfPort::WfPortType portType, WfCst::WfDataType dataType,
                   unsigned int depth) throw(WfStructException) {
  WfPort * p = NULL;
  switch (portType) {
  case WfPort::PORT_PARAM:
    p = new FNodeParamPort(this, portId, dataType, ind);
    addPort(portId, p);
    break;
  case WfPort::PORT_IN:
    p = new FNodeInPort(this, portId, portType, dataType, depth, ind);
    addPort(portId, p);
    break;
  case WfPort::PORT_INOUT:
    throw WfStructException("Inout port is not valid within conditional node");
    break;
  case WfPort::PORT_OUT:
    p = new FNodeOutPort(this, portId, portType, dataType, depth, ind);
    addPort(portId, p);
    break;
  case WfPort::PORT_IN_LOOP:
    p = new FLoopInPort(this, portId, portType, dataType, depth, ind);
    addPort(portId, p);
    break;
  case WfPort::PORT_OUT_LOOP:
    p = new FLoopOutPort(this, portId, portType, dataType, depth, ind);
    addPort(portId, p);
    break;
  default:
    throw WfStructException("Invalid port type in conditional node");
  }
  return p;
}

void
FLoopNode::connectNodePorts() throw(WfStructException) {
  // Reset the loop IN connections
  std::map<std::string, WfPort*>::iterator portIter = ports.begin();
  for (; portIter != ports.end(); ++portIter) {
    FLoopInPort* loopInPort =
      dynamic_cast<FLoopInPort*>((WfPort*) portIter->second);
    if (loopInPort) {
      // Reset precedence check for Loop In ports
      loopInPort->setPrecedence = true;
      loopInPort->setNodePrecedence((NodeSet*) wf);
    }
  }
  FNode::connectNodePorts();
}

void
FLoopNode::setDoMap(const std::string& leftPortName,
                    const std::string& rightPortName) throw(WfStructException) {
  FLoopOutPort* loopOutPort = checkAssignPort<FLoopOutPort>(leftPortName);
  FLoopInPort* loopInPort = checkAssignPort<FLoopInPort>(rightPortName);
  myDoMap.mapPorts(loopOutPort, loopInPort);

  // set final maps
  try {
    FNodeOutPort* outPort =
      checkAssignPort<FNodeOutPort>(loopOutPort->getInterfaceRef());
    myFinalOutMap.mapPorts(outPort, loopInPort);
    myFinalLoopMap.mapPortToVoid(loopOutPort);
    myFinalVoidOutMap.mapPortToVoid(outPort);
    // set direct in->out map (in case of no iteration at all)
    try {
      FNodeInPort* inPort =
        checkAssignPort<FNodeInPort>(loopInPort->getInterfaceRef());
      myDirectInOutMap.mapPorts(outPort, inPort);
    } catch (const WfStructException& e) {
      // missing init attribute for in port ==> DirectMap will assign VOID
      myDirectInOutMap.mapPortToVoid(outPort);
    }
  } catch (const WfStructException& e) {
    std::cerr << "SINK out port for LOOP out port '"
              << loopOutPort->getId() << "' not found\n";
    throw;
  }
}

void
FLoopNode::setWhileCondition(const std::string& conditionStr)
  throw(WfStructException) {
  myCondition->setExpression(conditionStr);
  checkCondition();
}

void
FLoopNode::checkCondition() throw(WfStructException) {
  // init variables table
  if (myConditionVars != NULL) {
    delete myConditionVars;
  }
  myConditionVars = new std::vector<WfExprVariable*>(getPortNb(), NULL);
  // loop over all inputs and set the variable in the table
  std::map<std::string, WfPort*>::iterator portIter = ports.begin();
  for (; portIter != ports.end(); ++portIter) {
    WfPort *port = (WfPort*) portIter->second;
    if (((port->getPortType() == WfPort::PORT_IN) ||
         (port->getPortType() == WfPort::PORT_IN_LOOP))
        && myCondition->isVariableUsed(port->getId())) {
      // each port involved in the condition must be modified to accept only
      // DH with a real value
      FNodeInPort* inPort = dynamic_cast<FNodeInPort*>(port);
      inPort->setValueRequired();
      if (inPort->getPortType() == WfPort::PORT_IN_LOOP) {
        // set the init port as value required too
        try {
          WfPort * port = getPort(inPort->getInterfaceRef());
          FNodeInPort* initPort = dynamic_cast<FNodeInPort*>(port);
          initPort->setValueRequired();
        } catch (const WfStructException& e) {
          INTERNAL_ERROR(__FUNCTION__
                         << "no corresponding in port for inLoop port", 1);
        }
      }
      // create a new variable
      WfExprVariable* newVar =
        new WfExprVariable(inPort->getId(), inPort->getDataType());
      newVar->setDefaultValue();
      // store the new variable in the table (index = port index)
      (*myConditionVars)[inPort->getIndex()] = newVar;
      // apply variable to condition
      myCondition->addVariable(newVar);
    }
  }
  // check if the expression evaluates
  try {
    myCondition->testIfTrue();
  } catch (...) {
    throw WfStructException(WfStructException::eINVALID_EXPR,
                            myCondition->getExpression());
  }
}

void
FLoopNode::initialize() {
  TRACE_TEXT(TRACE_ALL_STEPS, "Initializing loop node :" << getId() << "\n");
  std::vector<InputIterator*> *inputIterTab = new std::vector<InputIterator*>();

  std::map<std::string, WfPort*>::iterator portIter = ports.begin();
  for (; portIter != ports.end(); ++portIter) {
    FLoopInPort* loopInPort =
      dynamic_cast<FLoopInPort*>((WfPort*) portIter->second);
    if (loopInPort) {
      // Create port iterator on loop IN ports
      inputIterTab->push_back(new PortInputIterator(loopInPort));
    }
  }
  // Create DOT iterator on all loop IN ports
  myLoopIterator = new DotIterator(*inputIterTab);
  delete inputIterTab;

  // Run standard initialize
  // ==> creates root iterator on standard IN ports only
  FProcNode::initialize();
}

bool
FLoopNode::testCondition(const std::vector<FDataHandle*>& currDataLine) {
  // init variables for condition
  for (unsigned int ix = 0; ix < currDataLine.size(); ++ix) {
    // check if current input is used as variable in the condition
    if ((*myConditionVars)[ix] != NULL) {
      // check if current input has a value
      FDataHandle *dataHdl = (FDataHandle*) currDataLine[ix];
      if (dataHdl && dataHdl->isValueDefined()) {
        // set the value of the variable
        (*myConditionVars)[ix]->setValue(dataHdl->getValue());
      } else {
        throw WfDataHandleException(WfDataHandleException::eVALUE_UNDEF,
                                    "Condition variable '" +
                                    (*myConditionVars)[ix]->getName());
      }
    }
  }
  bool result = false;
  try {
    result = myCondition->testIfTrue();
  } catch (...) {
    WARNING("LOOP NODE name=" << getId()
            << " : Cannot evaluate WHILE expression : \n"
            << myCondition->getQueryString() << "\n"
            << " ===> considering expression is FALSE\n");
    //TODO it would be better to set all outputs as VOID
  }
  return result;
}

void
FLoopNode::instanciate(Dag* dag) {
  TRACE_TEXT(TRACE_ALL_STEPS, "   ## instanciate loop (internally) : \n");
  // Process ongoing loops
  std::vector<FDataHandle*>* loopDataLine =
    new std::vector<FDataHandle*>(getPortNb());
  myLoopIterator->begin();
  while (!myLoopIterator->isAtEnd()) {
    // GET DATALINE
    FDataTag currTag = myLoopIterator->getCurrentItem(*loopDataLine);
    myLoopIterator->removeItem();
    // CHECK if tag length matches loop tag length
    if (loopTagLength) {  // must have been set before
      if (currTag.getLevel() != loopTagLength) {
        INTERNAL_ERROR(__FUNCTION__
                       << "Loop contains workflow with different in/out"
                       "tag level", 1);
      }
    }

    // CHECK if data received is the last VOID item that terminates
    // the iteration
    if (currTag.isLastOfBranch()) {
      TRACE_TEXT(TRACE_ALL_STEPS,
                 "Received last void item " << currTag.toString() << "\n");
      continue;
    }

    // CHECK IF VOID // FIXME duplicated code with FProcNode::instanciate
    bool dataIsVoid = false;
    std::vector<FDataHandle*>::const_iterator DLIter = loopDataLine->begin();
    while ((!dataIsVoid) && (DLIter != loopDataLine->end())) {
      FDataHandle* currDataHdl = *(DLIter++);
      dataIsVoid = currDataHdl ? currDataHdl->isVoid() : dataIsVoid;
    }
    if (dataIsVoid) {
      TRACE_TEXT(TRACE_ALL_STEPS,
                 "Loop data is VOID ==> consider condition as FALSE\n");
    }

    // TEST IF NEXT ITERATION
    TRACE_TEXT(TRACE_ALL_STEPS,
               "Check while condition for iteration "
               << currTag.toString() << "\n");
    if (!dataIsVoid && testCondition(*loopDataLine)) {
      TRACE_TEXT(TRACE_ALL_STEPS,
                 "While condition is TRUE ==> Apply loop mapping\n");
      currTag.getSuccessor();
      myDoMap.applyMap(currTag, *loopDataLine);
    } else {
      TRACE_TEXT(TRACE_ALL_STEPS,
                 "While condition is FALSE ==> Apply final mappings\n");
      // apply VOID mapping for loop out ports
      FDataTag nextTag = currTag;
      nextTag.getSuccessor();
      nextTag.getTagAsLastOfBranch();
      myFinalLoopMap.applyMap(nextTag, *loopDataLine);

      // apply output mapping for out ports
      currTag.getParent();
      myFinalOutMap.applyMap(currTag, *loopDataLine);

      // Update nb of active instances (used for status update)
      activeInstanceNb--;
    }
  }
  // Process new data
  TRACE_TEXT(TRACE_ALL_STEPS, "   ## instanciate loop (externally) : \n");
  FProcNode::instanciate(dag);
}

/**
 * Copy the DH of IN ports onto LOOP IN ports
 */
void
FLoopNode::initLoopInPorts(std::vector<FDataHandle*>& currDataLine) {
  std::map<std::string, WfPort*>::iterator portIter = ports.begin();
  for (; portIter != ports.end(); ++portIter) {
    FLoopInPort* loopInPort =
      dynamic_cast<FLoopInPort*>((WfPort*) portIter->second);
    if (loopInPort) {
      try {
        WfPort * port = getPort(loopInPort->getInterfaceRef());
        currDataLine[loopInPort->getIndex()] = currDataLine[port->getIndex()];
      } catch (const WfStructException& e) {
        INTERNAL_ERROR(__FUNCTION__
                       << "no corresponding in port for inLoop port", 1);
      }
    }
  }
}

void
FLoopNode::createRealInstance(Dag* dag, const FDataTag& currTag,
                              std::vector<FDataHandle*>& currDataLine) {
  TRACE_TEXT(TRACE_ALL_STEPS, "  ## NEW LOOP INSTANCE : " << getId()
             << currTag.toString() << "\n");

  initLoopInPorts(currDataLine);

  // IF EXPRESSION IS +++++ TRUE ++++++
  if (testCondition(currDataLine)) {
    TRACE_TEXT(TRACE_ALL_STEPS,
               "==> While condition is TRUE ==> start LOOP\n");
    // create new tag for the first iteration of the loop
    FDataTag* firstIterTag = new FDataTag(currTag, 0, false);
    // copy input data to output
    myDoMap.applyMap(*firstIterTag, currDataLine);
    delete firstIterTag;
    // This starts a new active instance (used for status update)
    activeInstanceNb++;
    // Initializes the loop tag length
    if (!loopTagLength) {
      loopTagLength = firstIterTag->getLevel();
    }

  } else {
    TRACE_TEXT(TRACE_ALL_STEPS,
               "==> While condition is FALSE ==> "
               "apply direct in-out mappings\n");
    // create new tag for the first (and last) void iteration of the loop
    FDataTag* firstIterTag = new FDataTag(currTag, 0, true);
    // apply VOID mapping for loop out ports
    myFinalLoopMap.applyMap(*firstIterTag, currDataLine);
    delete firstIterTag;
    // apply DIRECT output mapping for out ports
    myDirectInOutMap.applyMap(currTag, currDataLine);
  }
}

// A LOOP VOID instance must create ONE VOID iteration
// because if that instance is the last one the last tag must be propagated
// to all nodes in the loop to make them completed
void
FLoopNode::createVoidInstance(const FDataTag& currTag,
                              std::vector<FDataHandle*>& currDataLine) {
  TRACE_TEXT(TRACE_ALL_STEPS, "  ## NEW LOOP VOID INSTANCE : " << getId()
             << currTag.toString() << "\n");

  initLoopInPorts(currDataLine);
  FDataTag* loopTag = new FDataTag(currTag, 0, true);

  // apply VOID mapping for loop out ports
  myFinalLoopMap.applyMap(*loopTag, currDataLine);

  // apply VOID mapping for out ports
  myFinalVoidOutMap.applyMap(currTag, currDataLine);

  delete loopTag;

  TRACE_TEXT(TRACE_ALL_STEPS, traceId() <<"  ## END OF LOOP VOID INSTANCE\n");
}


void
FLoopNode::updateInstanciationStatus() {
  if (myRootIterator->isAtEnd()) {
    if (myRootIterator->isDone()) {
      if (activeInstanceNb == 0) {
        TRACE_TEXT(TRACE_ALL_STEPS,
                   traceId() << "########## ALL ITERATIONS COMPLETED\n");
        myStatus = N_INSTANC_END;
      } else {  // still some loop instances running
        TRACE_TEXT(TRACE_ALL_STEPS,
                   traceId() << "########## ALL INPUTS PROCESSED\n");
      }
    } else {
      TRACE_TEXT(TRACE_ALL_STEPS,
                 traceId() << "########## WAITING FOR INPUTS \n");
      myStatus = N_INSTANC_READY;
    }
  }
}
