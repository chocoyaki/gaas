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
 * Revision 1.1  2009/04/17 09:04:07  bisnard
 * initial version for conditional nodes in functional workflows
 *
 */

#include "FIfNode.hh"

FIfNode::FIfNode(FWorkflow* wf, const string& id)
  : FProcNode(wf,id) {}

FIfNode::~FIfNode() {}

void
FIfNode::setCondition(const string& conditionStr)
  throw (WfStructException) {
  // each port involved in the condition must be modified to accept only
  // DH with a real value (setValueRequired())
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
FIfNode::createInstance(Dag* dag,
                        const FDataTag& currTag,
                        const vector<FDataHandle*>& currDataLine) {
  // Check the condition
  FNodePortMap* mapToApply;
//   if (myCondition.testIfTrue(currDataLine)) {
  if (true) {
    mapToApply = &myThenMap;
    TRACE_TEXT (TRACE_ALL_STEPS," ## IF condition is TRUE => Apply THEN mapping" << endl);
  } else {
    mapToApply = &myElseMap;
    TRACE_TEXT (TRACE_ALL_STEPS," ## IF condition is FALSE => Apply ELSE mapping" << endl);
  }
  // Apply the map accordingly
  mapToApply->applyMap(currTag, currDataLine);
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

