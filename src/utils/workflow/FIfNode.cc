/**
* @file  FIfNode.cc
* 
* @brief  The class representing the IF condition in a functional workflow
* 
* @author  Benjamin ISNARD (Benjamin.Isnard@ens-lyon.fr)
* 
* @section Licence
*   |LICENSE|                                                                
*/


#include "FIfNode.hh"
#include "DagNode.hh"


/*****************************************************************************/
/*                            FIfNode class                                  */
/*****************************************************************************/


FIfNode::FIfNode(FWorkflow* wf, const std::string& id)
  : FProcNode(wf, id) {
  myCondition = new WfBooleanExpression();
  myConditionVars = NULL;
}

FIfNode::~FIfNode() {}

WfPort *
FIfNode::newPort(std::string portId, unsigned int ind,
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
    throw WfStructException("Out port is not valid within conditional node "
                            "(Use outThen or outElse tag)");
    break;
  case WfPort::PORT_OUT_THEN:
    p = new FNodeOutPort(this, portId, portType, dataType, depth, ind);
    addPort(portId, p);
    // default mapping (to be overriden by explicit mapping)
    setThenMap(portId, "VOID");
    setElseMap(portId, "VOID");  // opposite case mapping
    break;
  case WfPort::PORT_OUT_ELSE:
    p = new FNodeOutPort(this, portId, portType, dataType, depth, ind);
    addPort(portId, p);
    // default mapping (to be overriden by explicit mapping)
    setElseMap(portId, "VOID");
    setThenMap(portId, "VOID");  // opposite case mapping
    break;
  default:
    throw WfStructException("Invalid port type in conditional node");
  }
  return p;
}

void
FIfNode::setCondition(const std::string& conditionStr)
  throw(WfStructException) {
  myCondition->setExpression(conditionStr);
  checkCondition();
}

void
FIfNode::checkCondition() throw(WfStructException) {
  // init variables table
  if (myConditionVars != NULL) {
    delete myConditionVars;
  }
  myConditionVars = new std::vector<WfExprVariable*>(getPortNb(), NULL);
  // loop over all inputs and set the variable in the table
  for (std::map<std::string, WfPort*>::iterator portIter = ports.begin();
       portIter != ports.end();
       ++portIter) {
    WfPort *port = (WfPort*) portIter->second;
    if ((port->getPortType() == WfPort::PORT_IN)
        && myCondition->isVariableUsed(port->getId())) {
      // each port involved in the condition must be modified to accept only
      // DH with a real value
      FNodeInPort* inPort = dynamic_cast<FNodeInPort*>(port);
      inPort->setValueRequired();
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
  //   myCondition->reset();
}

void
FIfNode::setThenMap(const std::string& leftPortName,
                    const std::string& rightPortName) throw(WfStructException) {
  FNodeOutPort* outPort = checkAssignPort<FNodeOutPort>(leftPortName);
  if (rightPortName != "VOID") {
    myThenMap.mapPorts(outPort, checkAssignPort<FNodeInPort>(rightPortName));
  } else {
    myThenMap.mapPortToVoid(outPort);
  }
}

void
FIfNode::setElseMap(const std::string& leftPortName,
                    const std::string& rightPortName) throw(WfStructException) {
  FNodeOutPort* outPort = checkAssignPort<FNodeOutPort>(leftPortName);
  if (rightPortName != "VOID") {
    myElseMap.mapPorts(outPort, checkAssignPort<FNodeInPort>(rightPortName));
  } else {
    myElseMap.mapPortToVoid(outPort);
  }
}

void
FIfNode::createRealInstance(Dag* dag, const FDataTag& currTag,
                            std::vector<FDataHandle*>& currDataLine) {
  TRACE_TEXT(TRACE_MAIN_STEPS, "  ## NEW IF INSTANCE : " << getId()
             << currTag.toString() << "\n");
  FNodePortMap* mapToApply;
  // Loop for all inputs
  for (unsigned int ix = 0; ix < currDataLine.size(); ++ix) {
    // check if current input is used as variable in the condition
    if ((*myConditionVars)[ix] != NULL) {
      // check if current input has a value
      FDataHandle *dataHdl = currDataLine[ix];
      if (dataHdl && dataHdl->isValueDefined()) {
        // set the value of the variable
        (*myConditionVars)[ix]->setValue(dataHdl->getValue());
      } else {
        // TODO throw exception
      }
    }
  }
  bool result = false;
  try {
    result = myCondition->testIfTrue();
  } catch (...) {
    WARNING("IF NODE name=" << getId() << " / tag=" << currTag.toString()
            << " : Cannot evaluate IF expression : \n"
            << myCondition->getQueryString() << "\n"
            << " ===> considering expression is FALSE\n");
    // TODO it would be better to set all outputs as VOID
  }
  if (result) {
    mapToApply = &myThenMap;
    TRACE_TEXT(TRACE_ALL_STEPS,
               "  # IF condition is TRUE => Apply THEN mapping\n");
  } else {
    mapToApply = &myElseMap;
    TRACE_TEXT(TRACE_ALL_STEPS,
               "  # IF condition is FALSE => Apply ELSE mapping\n");
  }
  // Apply the chosen map accordingly
  mapToApply->applyMap(currTag, currDataLine);
  //   myCondition->reset();
  TRACE_TEXT(TRACE_ALL_STEPS, "  ## END IF INSTANCE : "
             << getId() << currTag.toString() << "\n");
}

/*****************************************************************************/
/*                           FMergeNode class                                */
/*****************************************************************************/

FMergeNode::FMergeNode(FWorkflow* wf, const std::string& id)
  : FProcNode(wf, id) , myOutPort(NULL) {
}

FMergeNode::~FMergeNode() {
}

WfPort *
FMergeNode::newPort(std::string portId, unsigned int ind,
                    WfPort::WfPortType portType, WfCst::WfDataType dataType,
                    unsigned int depth) throw(WfStructException) {
  WfPort * p = NULL;
  switch (portType) {
  case WfPort::PORT_IN:
    if (getPortNb(WfPort::PORT_IN) > 1) {
      throw WfStructException("Merge node can have only 2 input ports");
    }
    p = new FNodeInPort(this, portId, portType, dataType, depth, ind);
    break;
  case WfPort::PORT_OUT:
    if (getPortNb(WfPort::PORT_OUT) > 0) {
      throw WfStructException("Merge node can have only 1 output port");
    }
    myOutPort = new FNodeOutPort(this, portId, portType, dataType, depth, ind);
    p = (WfPort *) myOutPort;
    break;
  default:
    throw WfStructException("Invalid port type in merge node");
  }
  return addPort(portId, p);
}

void
FMergeNode::createMergeInstance(const FDataTag& currTag,
                                std::vector<FDataHandle*>& currDataLine) {
  TRACE_TEXT(TRACE_MAIN_STEPS, "  ## NEW MERGE INSTANCE : " << getId()
             << currTag.toString() << "\n");
  FDataHandle* srcData = NULL;
  FDataHandle *outData = NULL;
  std::vector<FDataHandle*>::const_iterator DLIter = currDataLine.begin();
  while ((!srcData || srcData->isVoid()) && !(DLIter == currDataLine.end())) {
    srcData = (FDataHandle*) *(DLIter++);
  }

  if (srcData && !srcData->isVoid()) {
    // we have got a non-VOID datahandle on one of our inputs
    // COPY the input DH (with all its child tree)
    outData = new FDataHandle(currTag, *srcData);
  } else {
    outData = myOutPort->createVoidInstance(currTag);
  }
  // Submit to out port (ie will send it to connected ports)
  myOutPort->storeData(outData);
  myOutPort->sendData(outData);
}

void
FMergeNode::createRealInstance(Dag* dag, const FDataTag& currTag,
                               std::vector<FDataHandle*>& currDataLine) {
  WARNING("MERGE Node received two non-VOID inputs");
  createMergeInstance(currTag, currDataLine);
}

void
FMergeNode::createVoidInstance(const FDataTag& currTag,
                               std::vector<FDataHandle*>& currDataLine) {
  createMergeInstance(currTag, currDataLine);
}

/*****************************************************************************/
/*                           FFilterNode class                               */
/*****************************************************************************/
FFilterNode::FFilterNode(FWorkflow* wf, const std::string& id)
  : FProcNode(wf, id), myOutPort(NULL) {
}

FFilterNode::~FFilterNode() {
  // Free the tree
  while (!myTree.empty()) {
    filterNode_t* p = myTree.begin()->second;
    myTree.erase(myTree.begin());
    delete p;
  }
}

WfPort *
FFilterNode::newPort(std::string portId, unsigned int ind,
                     WfPort::WfPortType portType, WfCst::WfDataType dataType,
                     unsigned int depth) throw(WfStructException) {
  WfPort * p = NULL;
  switch (portType) {
  case WfPort::PORT_IN:
    if (getPortNb(WfPort::PORT_IN) > 0) {
      throw WfStructException("Filter node can have only 1 input port");
    }
    p = new FNodeInPort(this, portId, portType, dataType, depth, 0);
    break;
  case WfPort::PORT_OUT:
    if (getPortNb(WfPort::PORT_OUT) > 0) {
      throw WfStructException("Filter node can have only 1 output port");
    }
    myOutPort = new FNodeOutPort(this, portId, portType, dataType, depth, 1);
    p = (WfPort *) myOutPort;
    break;
  default:
    throw WfStructException("Invalid port type in filter node");
  }
  return addPort(portId, p);
}

void
FFilterNode::createRealInstance(Dag* dag, const FDataTag& currTag,
                                std::vector<FDataHandle*>& currDataLine) {
  TRACE_TEXT(TRACE_MAIN_STEPS, "  ## FILTER PROCESSES NON-VOID ITEM : "
             << currTag.toString() << "\n");
  FDataHandle* srcData = currDataLine[0];
  if (!srcData) {
    INTERNAL_ERROR("Invalid (NULL) data to process in filter node", 1);
  }
  updateUp(srcData, 0);
}

void
FFilterNode::createVoidInstance(const FDataTag& currTag,
                                std::vector<FDataHandle*>& currDataLine) {
  TRACE_TEXT(TRACE_MAIN_STEPS, "  ## FILTER PROCESSES VOID ITEM : "
             << currTag.toString() << "\n");
  FDataHandle* srcData = currDataLine[0];
  if (!srcData) {
    INTERNAL_ERROR("Invalid (NULL) data to process in filter node", 1);
  }
  updateUp(srcData, 0);
}

void
FFilterNode::updateUp(FDataHandle* DH, short depth) {
  filterNode_t* currTreeNode = getTreeNode(DH);
  bool voidDefChange = false;

  // Updates the voidDef flag
  // for leaves, it is simply the type of DH (void or not)
  // for nodes, it can be either void, non-void (if at least one child is
  // non-void), or undefined (if only void childs or missing childs)
  if (!currTreeNode->voidDef) {
    if ((depth == 0) || isNonVoid(DH) || DH->isVoid()) {
      currTreeNode->voidDef = true;
      voidDefChange = true;
    }
  }

  if (currTreeNode->voidDef) {
    const FDataTag& DHTag = DH->getTag();

    // Updates the indexOk flag (and the newIndex if available)
    // and trigger horizontal update (higher indexes)
    if (!currTreeNode->indexOk) {
      int precIdx = -2;

      // condition for setting index is either this is the first item
      // or the item's predecessor (index-1) has its new index defined
      if (DHTag.getLastIndex() == 0) {
        precIdx = -1;
      } else {
        FDataTag DHTagCpy = DHTag;
        filterNode_t* precTreeNode = getTreeNode(DHTagCpy.getPredecessor());
        if (precTreeNode->indexOk) {
          precIdx = precTreeNode->newIndex;
        }
      }

      if (precIdx > -2) {
        updateRight(DHTag, precIdx, depth);
      }
    }

    // Updates the lastFlagOk flag (and the newLastFlag if available)
    // and trigger horizontal update (lower indexes)
    if (!currTreeNode->lastFlagOk) {
      FDataTag DHTagCpy = DHTag;

      // condition for setting 'last' flag is either this is the last item of
      // the branch or the item's successor (index+1) has its 'last' flag
      // defined
      if (DHTag.isLastOfBranch() ||
         getTreeNode(DHTagCpy.getSuccessor())->lastFlagOk) {
        currTreeNode->lastFlagOk = true;
        // value of flag depends on type of current data (void or non-void only)
        if (DH->isVoid()) {
          currTreeNode->newLastFlag = false;
          if (DHTag.getLastIndex() > 0) {
            updateLeftVoid(DHTag, depth);
          }
        } else {
          currTreeNode->newLastFlag = true;
          if (DHTag.getLastIndex() > 0) {
            updateLeftNonVoid(DHTag, depth);
          }
        }
        // flag was modified for current node so check if ready for sending
        if (isReady(DHTag)) {
          sendDown(DH, depth);
        }
      } else if (!DH->isVoid()) {
        // 'last' flag of predecessors can be set if current item is non-void
        updateLeftNonVoid(DHTag, depth);
      }
    }

    // Recursive updates upward until level 1
    if (voidDefChange && (DHTag.getLevel() > 1)) {
      updateUp(DH->getParent(), depth+1);
    }
  }
}

void
FFilterNode::updateRight(const FDataTag& tag, int precIdx, short depth) {
  FDataTag currTag = tag;
  filterNode_t* currTreeNode = getTreeNode(tag);
  while (currTreeNode->voidDef) {
    currTreeNode->indexOk = true;

    // value of new index depends on type of current data
    // (void or non-void only)
    if (!currTreeNode->dataHdl) {
      INTERNAL_ERROR("Invalid data ref in FFilterNode::updateRight", 1);
    }
    if (currTreeNode->dataHdl->isVoid()) {
      currTreeNode->newIndex = precIdx;
    } else {
      currTreeNode->newIndex = precIdx + 1;
    }

    // flag was modified for current node so check if ready for sending
    if (isReady(currTag)) {
      sendDown(currTreeNode->dataHdl, depth);
    }

    // stop if current node is the last one of the branch
    if (currTreeNode->dataHdl->getTag().isLastOfBranch()) {
      break;
    }

    // next node (successor) - (last flags are invalid)
    currTag.getSuccessor();
    currTreeNode = getTreeNode(currTag);
  }
}

void
FFilterNode::updateLeftNonVoid(const FDataTag& tag, short depth) {
  FDataTag currTag = tag;
  while (currTag.getLastIndex() > 0) {
    currTag.getPredecessor();
    filterNode_t* currTreeNode = getTreeNode(currTag);

    currTreeNode->lastFlagOk = true;
    currTreeNode->newLastFlag = false;

    // flag was modified for current node so check if ready for sending
    if (isReady(currTag)) {
      sendDown(currTreeNode->dataHdl, depth);
    }
  }
}

void
FFilterNode::updateLeftVoid(const FDataTag& tag, short depth) {
  FDataTag currTag = tag;
  while (currTag.getLastIndex() > 0) {
    currTag.getPredecessor();
    filterNode_t* currTreeNode = getTreeNode(currTag);

    if (currTreeNode->voidDef) {
      // as long as we get VOID items, 'last' flag is set to false
      if (currTreeNode->dataHdl->isVoid()) {
        currTreeNode->lastFlagOk = true;
        currTreeNode->newLastFlag = false;
      } else {
        // when a non-VOID is found, this is the 'last' item
        currTreeNode->lastFlagOk = true;
        currTreeNode->newLastFlag = true;

        // flag was modified for current node so check if ready for sending
        if (isReady(currTag)) {
          sendDown(currTreeNode->dataHdl, depth);
        }
        // continue with update for non-void item
        updateLeftNonVoid(currTag, depth);
      }
    } else {
      break;  // an undefined item is found ==> stop update
    }
  }
}

// RECURSIVE
void
FFilterNode::sendDown(FDataHandle* DH, short depth) {
  if (!DH->isVoid()) {
    if (depth == 0) {
      // copy the data handle and send it
      FDataTag  newTag = getNewTag(DH->getTag());
      FDataHandle *dataHdl = new FDataHandle(newTag , *DH);
      myOutPort->storeData(dataHdl);
      myOutPort->sendData(dataHdl);
      myTree[DH->getTag()]->isDone = true;
    } else {
      std::map<FDataTag, FDataHandle*>::iterator childIter = DH->begin();
      for (; childIter != DH->end(); ++childIter) {
        FDataHandle* childDH = (FDataHandle*) childIter->second;
        if (isReadyAssumingParentIs(childDH->getTag())) {
          sendDown(childDH, depth-1);
        } else {
        }
      }
    }
  } else {
  }
}

bool
FFilterNode::isNonVoid(FDataHandle* DH) {
  bool nonVoidFound = false;
  std::map<FDataTag, FDataHandle*>::iterator childIter = DH->begin();
  while (!nonVoidFound && childIter != DH->end()) {
    FDataHandle * childData = (FDataHandle*) childIter->second;
    filterNode_t* childTreeNode = getTreeNode(childData);
    if ((childTreeNode->voidDef) && (!childTreeNode->dataHdl->isVoid())) {
      nonVoidFound = true;
    }
    ++childIter;
  }
  return nonVoidFound;
}

bool
FFilterNode::isReadyAssumingParentIs(const FDataTag& srcTag) {
  filterNode_t* tNode = getTreeNode(srcTag);
  return (!tNode->isDone && tNode->lastFlagOk && tNode->indexOk);
}

bool
FFilterNode::isReady(const FDataTag& srcTag) {
  filterNode_t* tNode = getTreeNode(srcTag);
  bool checkMe = !tNode->isDone && tNode->lastFlagOk && tNode->indexOk;
  bool checkParent = false;
  if (checkMe) {
    if (srcTag.getLevel() == 1) {
      checkParent = true;
    } else {
      FDataTag* srcTagCopy = new FDataTag(srcTag);
      FDataTag& parentTag = srcTagCopy->getParent();
      checkParent = isReady(parentTag);
      delete srcTagCopy;
    }
  }
  return (checkMe && checkParent);
}

FDataTag
FFilterNode::getNewTag(const FDataTag& srcTag) {
  unsigned int * idxTab = new unsigned int[srcTag.getLevel()];
  bool * lastTab = new bool[srcTag.getLevel()];

  FDataTag currTag = srcTag;
  while (!currTag.isEmpty()) {
    filterNode_t* tNode = getTreeNode(currTag);
    idxTab[currTag.getLevel()-1] = (unsigned int) tNode->newIndex;
    lastTab[currTag.getLevel()-1] = tNode->newLastFlag;
    currTag.getParent();
  }

  FDataTag newTag = FDataTag(idxTab, lastTab, srcTag.getLevel());
  delete [] idxTab;
  delete [] lastTab;

  return newTag;
}

FFilterNode::filterNode_t*
FFilterNode::getTreeNode(FDataHandle* DH) {
  const FDataTag& tag = DH->getTag();
  std::map<FDataTag, filterNode_t*>::iterator treeSrch = myTree.find(tag);
  if (treeSrch == myTree.end()) {
    // create new node in the tree
    filterNode_t* newNode = myTree[tag] = new filterNode_t;
    newNode->dataHdl = DH;
    newNode->voidDef = false;
    newNode->indexOk = false;
    newNode->lastFlagOk = false;
    newNode->isDone = false;
    return newNode;
  } else {
    filterNode_t *tNode = (filterNode_t*) treeSrch->second;
    if (tNode->dataHdl == NULL) {
      tNode->dataHdl = DH;
    }
    return tNode;
  }
}

FFilterNode::filterNode_t*
FFilterNode::getTreeNode(const FDataTag& tag) {
  std::map<FDataTag, filterNode_t*>::iterator treeSrch = myTree.find(tag);
  if (treeSrch == myTree.end()) {
    // create new node in the tree
    filterNode_t* newNode = myTree[tag] = new filterNode_t;
    newNode->dataHdl = NULL;
    newNode->voidDef = false;
    newNode->indexOk = false;
    newNode->lastFlagOk = false;
    newNode->isDone = false;
    return newNode;
  } else {
    return (filterNode_t*) treeSrch->second;
  }
}

