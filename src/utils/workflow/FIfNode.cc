/****************************************************************************/
/* The class representing the IF condition in a functional workflow         */
/*                                                                          */
/* Author(s):                                                               */
/* - Benjamin ISNARD (benjamin.isnard@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.2  2009/05/15 11:10:20  bisnard
 * release for workflow conditional structure (if)
 *
 * Revision 1.1  2009/04/17 09:04:07  bisnard
 * initial version for conditional nodes in functional workflows
 *
 */

#include "FIfNode.hh"
#include "DagNode.hh"

FIfNode::FIfNode(FWorkflow* wf, const string& id) : FProcNode(wf,id) {
  myCondition = new WfBooleanExpression();
}

FIfNode::~FIfNode() {}

void
FIfNode::setCondition(const string& conditionStr)
  throw (WfStructException) {
  myCondition->setExpression(conditionStr);
  checkCondition();
}

void
FIfNode::checkCondition() throw (WfStructException) {
  // loop over all inputs and set a default value (0) for the variable
  for (map<string, WfPort*>::iterator portIter = ports.begin();
       portIter != ports.end();
       ++portIter) {
    WfPort *port = (WfPort*) portIter->second;
    if ((port->getPortType() == WfPort::PORT_IN)
         && myCondition->isVariableUsed(port->getId())) {
      // each port involved in the condition must be modified to accept only
      // DH with a real value
      FNodeInPort* inPort = dynamic_cast<FNodeInPort*>(port);
      inPort->setValueRequired();
      // check if port depth is maximum 1
      if (port->getDepth() > 1)
        throw WfStructException(WfStructException::eINVALID_EXPR,
                                "variable in condition is a container of depth > 1");
      bool seq = port->getDepth() > 0;
      // set the default value for the condition check
      myCondition->setVariableDefaultValue(port->getId(), port->getBaseDataType(), seq);
    }
  }
  // check if the expression evaluates
  try {
    myCondition->testIfTrue();
  } catch (...) {
    throw WfStructException(WfStructException::eINVALID_EXPR,
                            myCondition->getExpression());
  }
  myCondition->reset();
}

void
FIfNode::setThenMap(const string& leftPortName,
                    const string& rightPortName) throw (WfStructException) {
  FNodeOutPort* outPort = checkAssignPort<FNodeOutPort>(leftPortName);
  if (rightPortName != "VOID") {
    myThenMap.mapPorts(outPort, checkAssignPort<FNodeInPort>(rightPortName));
  } else {
    myThenMap.mapPortToVoid(outPort);
  }
}

void
FIfNode::setElseMap(const string& leftPortName,
                    const string& rightPortName) throw (WfStructException) {
  FNodeOutPort* outPort = checkAssignPort<FNodeOutPort>(leftPortName);
  if (rightPortName != "VOID") {
    myElseMap.mapPorts(outPort, checkAssignPort<FNodeInPort>(rightPortName));
  } else {
    myElseMap.mapPortToVoid(outPort);
  }
}

void
FIfNode::createRealInstance(Dag* dag,
                            const FDataTag& currTag,
                            const vector<FDataHandle*>& currDataLine) {
  TRACE_TEXT (TRACE_ALL_STEPS,"  ## NEW IF INSTANCE : " << getId()
                              << currTag.toString() << endl);
  FNodePortMap* mapToApply;
  // Loop for all inputs (NOT OPTIMIZED: some inputs may be unused in the expression)
  for (int ix = 0; ix < currDataLine.size(); ++ix) {
    FDataHandle *dataHdl = (FDataHandle*) currDataLine[ix];
    const WfPort *port = getPortByIndex(ix);
    if (dataHdl && dataHdl->isValueDefined()) {
      myCondition->setVariable(port->getId(),
                               port->getBaseDataType(),
                               port->getDepth() > 0,
                               dataHdl->getValue());
    }
  }
  bool result = false;
  try {
    result = myCondition->testIfTrue();
  } catch (...) {
    WARNING("IF NODE name=" << getId() << " / tag=" << currTag.toString()
            << " : Cannot evaluate IF expression : " << endl
            << myCondition->getQueryString() << endl
            << " ===> considering expression is FALSE" << endl);
    //TODO it would be better to set all outputs as VOID
  }
  if (result) {
    mapToApply = &myThenMap;
    TRACE_TEXT (TRACE_ALL_STEPS,"  # IF condition is TRUE => Apply THEN mapping" << endl);
  } else {
    mapToApply = &myElseMap;
    TRACE_TEXT (TRACE_ALL_STEPS,"  # IF condition is FALSE => Apply ELSE mapping" << endl);
  }
  // Apply the map accordingly
  mapToApply->applyMap(currTag, currDataLine);
  myCondition->reset();
  TRACE_TEXT (TRACE_ALL_STEPS,"  ## END OF CREATION OF NEW IF INSTANCE : "
                              << getId() << currTag.toString() << endl);
}

void
FIfNode::updateInstanciationStatus() {
  if (myRootIterator->isAtEnd()) {
    if (myRootIterator->isDone()) {
      TRACE_TEXT (TRACE_ALL_STEPS, "########## ALL INPUTS PROCESSED" << endl);
      myStatus = N_INSTANC_END;
    } else {
      TRACE_TEXT (TRACE_ALL_STEPS, "########## WAITING FOR INPUTS " << endl);
      myStatus = N_INSTANC_WAITING;
    }
  } else {
    INTERNAL_ERROR("FIfNode => iterator stopped before end without reason",1);
  }
}

