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
 * Revision 1.1  2009/07/07 09:06:08  bisnard
 * new class FLoopNode to handle workflow loops
 *
 */

#include "FLoopNode.hh"

FLoopNode::FLoopNode(FWorkflow* wf,
                     const string& id)
  : FProcNode(wf,id) {
}

FLoopNode::~FLoopNode() {
}

void
FLoopNode::setWhileCondition(const string& conditionStr)
    throw (WfStructException) {

}

void
FLoopNode::checkCondition() throw (WfStructException) {
}

void
FLoopNode::createRealInstance(Dag* dag,
                              const FDataTag& currTag,
                              const vector<FDataHandle*>& currDataLine) {
}

