/****************************************************************************/
/* The classes representing the for/while loops of a functional workflow    */
/*                                                                          */
/* Author(s):                                                               */
/* - Benjamin ISNARD (benjamin.isnard@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.2  2009/07/10 12:55:59  bisnard
 * implemented while loop workflow node
 *
 * Revision 1.1  2009/07/07 09:06:08  bisnard
 * new class FLoopNode to handle workflow loops
 *
 */

#include "FLoopNode.hh"

class FLoopInPort : public FNodeInPort {

  friend class FLoopNode;

  public:

    FLoopInPort(WfNode * parent, const string& _id, WfPort::WfPortType _portType,
              WfCst::WfDataType _type, unsigned int _depth, unsigned int _ind)
      : FNodeInPort(parent,_id,_portType,_type,_depth,_ind),
        setPrecedence(false) {
    }

    virtual void
    setNodePrecedence(NodeSet * contextNodeSet) throw (WfStructException);

  private:
    bool setPrecedence;

};



class FLoopOutPort : public FNodeOutPort {

  public:
    FLoopOutPort(WfNode * parent, const string& _id, WfPort::WfPortType _portType,
               WfCst::WfDataType _type, unsigned int _depth, unsigned int _ind)
      : FNodeOutPort(parent,_id,_portType,_type,_depth,_ind) {
    }

};

void
FLoopInPort::setNodePrecedence(NodeSet * contextNodeSet) throw (WfStructException) {
  if (setPrecedence)
    WfPort::setNodePrecedence(contextNodeSet);
}


FLoopNode::FLoopNode(FWorkflow* wf,
                     const string& id)
  : FProcNode(wf,id) , activeInstanceNb(0) {
  myCondition = new WfBooleanExpression();
  myConditionVars = NULL;
}

FLoopNode::~FLoopNode() {
}

WfPort *
FLoopNode::newPort(string portId,
                   unsigned int ind,
                   WfPort::WfPortType portType,
                   WfCst::WfDataType dataType,
                   unsigned int depth) throw (WfStructException) {

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
FLoopNode::connectNodePorts() throw (WfStructException) {

  // Reset the loop IN connections
  for (map<string, WfPort*>::iterator portIter = ports.begin();
       portIter != ports.end();
       ++portIter) {
    FLoopInPort* loopInPort = dynamic_cast<FLoopInPort*>((WfPort*) portIter->second);
    if (loopInPort) {
      // Reset precedence check for Loop In ports
      loopInPort->setPrecedence = true;
      loopInPort->setNodePrecedence((NodeSet*) wf);
    }
  }

  WfNode::connectNodePorts();
}

void
FLoopNode::setDoMap(const string& leftPortName,
                    const string& rightPortName) throw (WfStructException) {
  FLoopOutPort* loopOutPort = checkAssignPort<FLoopOutPort>(leftPortName);
  FLoopInPort* loopInPort = checkAssignPort<FLoopInPort>(rightPortName);
  myDoMap.mapPorts(loopOutPort, loopInPort);

  // set final maps
  try {
    FNodeOutPort* outPort = checkAssignPort<FNodeOutPort>(loopOutPort->getInterfaceRef());
    myFinalOutMap.mapPorts(outPort, loopInPort);
    myFinalLoopMap.mapPortToVoid(loopOutPort);
  } catch (const WfStructException& e) {
    cerr << "SINK out port for LOOP out port '" << loopOutPort->getId() << "' not found" << endl;
    throw;
  }
}

void
FLoopNode::setWhileCondition(const string& conditionStr)
    throw (WfStructException) {
  myCondition->setExpression(conditionStr);
  checkCondition();
}

void
FLoopNode::checkCondition() throw (WfStructException) {
  // init variables table
  if (myConditionVars != NULL)  delete myConditionVars;
  myConditionVars = new vector<WfExprVariable*>(getPortNb(), NULL);
  // loop over all inputs and set the variable in the table
  for (map<string, WfPort*>::iterator portIter = ports.begin();
       portIter != ports.end();
       ++portIter) {
    WfPort *port = (WfPort*) portIter->second;
    if (((port->getPortType() == WfPort::PORT_IN) || (port->getPortType() == WfPort::PORT_IN_LOOP))
         && myCondition->isVariableUsed(port->getId())) {
      // each port involved in the condition must be modified to accept only
      // DH with a real value
      FNodeInPort* inPort = dynamic_cast<FNodeInPort*>(port);
      inPort->setValueRequired();
      // create a new variable
      WfExprVariable* newVar = new WfExprVariable(inPort->getId(), inPort->getDataType());
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

  TRACE_TEXT (TRACE_ALL_STEPS,"Initializing loop node :" << getId() << endl);
  vector<InputIterator*> *inputIterTab = new vector<InputIterator*>();

  for (map<string, WfPort*>::iterator portIter = ports.begin();
       portIter != ports.end();
       ++portIter) {
    FLoopInPort* loopInPort = dynamic_cast<FLoopInPort*>((WfPort*) portIter->second);
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
FLoopNode::testCondition(const vector<FDataHandle*>& currDataLine) {
  // init variables for condition
  for (int ix = 0; ix < currDataLine.size(); ++ix) {

    // check if current input is used as variable in the condition
    if ((*myConditionVars)[ix] != NULL) {

      // check if current input has a value
      FDataHandle *dataHdl = (FDataHandle*) currDataLine[ix];
      if (dataHdl && dataHdl->isValueDefined()) {

        // set the value of the variable
        (*myConditionVars)[ix]->setValue(dataHdl->getValue());

      } else {
        cerr << "ERROR: condition variable not defined" << endl;
        //TODO throw exception
      }
    }
  }
  bool result = false;
  try {
    result = myCondition->testIfTrue();
  } catch (...) {
    WARNING("LOOP NODE name=" << getId()
            << " : Cannot evaluate WHILE expression : " << endl
            << myCondition->getQueryString() << endl
            << " ===> considering expression is FALSE" << endl);
    //TODO it would be better to set all outputs as VOID
  }
  return result;
}

void
FLoopNode::instanciate(Dag* dag) {
  TRACE_TEXT (TRACE_ALL_STEPS,"   ## instanciate loop items : " << endl);
  // Process ongoing loops
  vector<FDataHandle*>* loopDataLine = new vector<FDataHandle*>(getPortNb());
  myLoopIterator->begin();
  while ( !myLoopIterator->isAtEnd()) {

    // GET DATALINE
    FDataTag currTag = myLoopIterator->getCurrentItem(*loopDataLine);
    myLoopIterator->removeItem();

    // CHECK if data received is the last VOID item that terminates the iteration
    if (currTag.isLastOfBranch()) {
      cout << "Received last void item " << currTag.toString() << endl;
      continue;
    }

    // CHECK IF VOID // FIXME duplicated code with FProcNode::instanciate
    bool dataIsVoid = false;
    vector<FDataHandle*>::const_iterator DLIter = loopDataLine->begin();
    while ((!dataIsVoid) && (DLIter!=loopDataLine->end())) {
      FDataHandle* currDataHdl = *(DLIter++);
      dataIsVoid = currDataHdl ? currDataHdl->isVoid() : dataIsVoid;
    }
    if (dataIsVoid) {
      TRACE_TEXT (TRACE_ALL_STEPS,"Loop data is VOID ==> consider condition as FALSE" << endl);
    }

    // TEST IF NEXT ITERATION
    TRACE_TEXT (TRACE_ALL_STEPS,"Check while condition for iteration " << currTag.toString() << endl);
    if (!dataIsVoid && testCondition(*loopDataLine)) {
      TRACE_TEXT (TRACE_ALL_STEPS,"While condition is TRUE ==> Apply loop mapping" << endl);
      currTag.getSuccessor();
      myDoMap.applyMap(currTag, *loopDataLine);

    } else {
      TRACE_TEXT (TRACE_ALL_STEPS,"While condition is FALSE ==> Apply final mappings" << endl);
      // apply VOID mapping for loop out ports
      cout << "Apply Final Loop mapping" << endl;
      FDataTag nextTag = currTag;
      nextTag.getSuccessor();
      nextTag.getTagAsLastOfBranch();
      myFinalLoopMap.applyMap(nextTag, *loopDataLine);

      // apply output mapping for out ports
      cout << "Apply Final Out mapping" << endl;
      currTag.getParent();
      myFinalOutMap.applyMap(currTag, *loopDataLine);

      // Update loop node status // FIXME move this to updateInstanciationStatus??
      activeInstanceNb--;
      if ((activeInstanceNb == 0) && myRootIterator->isDone()) {
        cout << " ## END OF ALL ITERATIONS" << endl;
        myStatus = N_INSTANC_END;
      }
    }
  }
  // Process new data
  TRACE_TEXT (TRACE_ALL_STEPS,"   ## instanciate processor items : " << endl);
  FProcNode::instanciate(dag);
}

void
FLoopNode::createRealInstance(Dag* dag,
                              const FDataTag& currTag,
                              const vector<FDataHandle*>& currDataLine) {
  TRACE_TEXT (TRACE_ALL_STEPS,"  ## NEW LOOP INSTANCE : " << getId()
                              << currTag.toString() << endl);

  // copy dataline (because modified later)
  vector<FDataHandle*>  currDataLineCpy = currDataLine;
  // map loop in ports on in ports
  for (map<string, WfPort*>::iterator portIter = ports.begin();
       portIter != ports.end();
       ++portIter) {
    FLoopInPort* loopInPort = dynamic_cast<FLoopInPort*>((WfPort*) portIter->second);
    if (loopInPort) {
      try {
        WfPort * port = getPort(loopInPort->getInterfaceRef());
        currDataLineCpy[loopInPort->getIndex()] = currDataLineCpy[port->getIndex()];
      } catch (const WfStructException& e) {
          cerr << "ERROR: init for inLoop port not found" << endl;
          throw; //TODO throw exception
      }
    }
  }

  // IF EXPRESSION IS +++++ TRUE ++++++
  if (testCondition(currDataLineCpy)) {
    // create new tag for the first iteration of the loop
    FDataTag* firstIterTag = new FDataTag(currTag,0,false);
    // copy input data to output
    myDoMap.applyMap(*firstIterTag, currDataLineCpy);
    delete firstIterTag;
    // This starts a new active instance (used for status update)
    activeInstanceNb++;

  } else {
    // TODO
  }
}

// A LOOP VOID instance must create ONE VOID iteration
// because if that instance is the last one the last tag must be propagated to all
// nodes in the loop to make them completed
void
FLoopNode::createVoidInstance(const FDataTag& currTag,
                              const vector<FDataHandle*>& currDataLine) {
  TRACE_TEXT (TRACE_ALL_STEPS,"  ## NEW LOOP VOID INSTANCE : " << getId()
                              << currTag.toString() << endl);

/** This block is a copy of same in createRealInstance **/
  // copy dataline (because modified later)
  vector<FDataHandle*>  currDataLineCpy = currDataLine;
  // map loop in ports on in ports
  for (map<string, WfPort*>::iterator portIter = ports.begin();
       portIter != ports.end();
       ++portIter) {
    FLoopInPort* loopInPort = dynamic_cast<FLoopInPort*>((WfPort*) portIter->second);
    if (loopInPort) {
      try {
        WfPort * port = getPort(loopInPort->getInterfaceRef());
        currDataLineCpy[loopInPort->getIndex()] = currDataLineCpy[port->getIndex()];
      } catch (const WfStructException& e) {
          cerr << "ERROR: init for inLoop port not found" << endl;
          throw; //TODO throw exception
      }
    }
  }
/** end of block **/

  activeInstanceNb++;
  FDataTag* loopTag = new FDataTag(currTag,0,false);

/** This block is a copy of same in instanciate (almost)
 *  (replaced loopDataLine by currDataLineCpy) + diffs due to currTag not defined as previous
 *  loop iteration tag **/
      // apply VOID mapping for loop out ports
      cout << "Apply Final Loop mapping" << endl;
  /*    FDataTag nextTag = currTag; */
  /*    nextTag.getSuccessor(); */
      FDataTag nextTag = *loopTag;
      nextTag.getTagAsLastOfBranch();
      myFinalLoopMap.applyMap(nextTag, currDataLineCpy);

      // apply output mapping for out ports
      cout << "Apply Final Out mapping" << endl;
  /*    currTag.getParent(); */
      loopTag->getParent();
      myFinalOutMap.applyMap(*loopTag, currDataLineCpy);

      // Update loop node status // FIXME move this to updateInstanciationStatus??
      activeInstanceNb--;
      if ((activeInstanceNb == 0) && myRootIterator->isDone()) {
        cout << " ## END OF ALL ITERATIONS" << endl;
        myStatus = N_INSTANC_END;
      }
/** end of block **/

  delete loopTag;

  TRACE_TEXT (TRACE_ALL_STEPS,"  ## END OF LOOP VOID INSTANCE" << endl);
}


void
FLoopNode::updateInstanciationStatus() {
  if (myRootIterator->isAtEnd()) {
    if (myRootIterator->isDone()) {
      TRACE_TEXT (TRACE_ALL_STEPS, "########## ALL INPUTS PROCESSED" << endl);
//       myStatus = N_INSTANC_END;

      // CHECK for each out port if the output is empty
      // NOT APPLICABLE TO WHILE NODES (output not known yet)
//       for (map<string,WfPort*>::iterator portIter = ports.begin();
//            portIter != ports.end();
//            ++portIter) {
//         WfPort* port = (WfPort*) portIter->second;
//         if (port->isOutput()) {
//           FNodeOutPort* outPort = dynamic_cast<FNodeOutPort*>(port);
//           outPort->checkIfEmptyOutput();
//         }
//       }

    } else {
      TRACE_TEXT (TRACE_ALL_STEPS, "########## WAITING FOR INPUTS " << endl);
      myStatus = N_INSTANC_READY;
    }
  }
}
