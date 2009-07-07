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

#ifndef _FLOOPNODE_HH_
#define _FLOOPNODE_HH_

#include "FNode.hh"
#include "WfExpression.hh"

class FLoopNode : public FProcNode {

  public:

    FLoopNode(FWorkflow* wf,
              const string& id);
    virtual ~FLoopNode();

    void
        setWhileCondition(const string& conditionStr)
        throw (WfStructException);

    virtual void
        createRealInstance(Dag* dag,
                           const FDataTag& currTag,
                           const vector<FDataHandle*>& currDataLine);

  protected:

    virtual void
        checkCondition() throw (WfStructException);

    vector<WfExprVariable*>*  myConditionVars;
    WfBooleanExpression*      myCondition;

};

#endif // _FLOOPNODE_HH_
