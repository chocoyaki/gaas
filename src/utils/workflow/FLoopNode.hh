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
 * Revision 1.4  2010/04/28 14:11:49  bdepardo
 * Bug correction for while condition.
 *
 * Revision 1.3  2009/07/23 12:30:10  bisnard
 * new method finalize() for functional wf nodes
 * removed const on currDataLine parameter for instance creation
 *
 * Revision 1.2  2009/07/10 12:55:59  bisnard
 * implemented while loop workflow node
 *
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

    // ******************** NODE SETUP *********************

    virtual WfPort *
        newPort(string portId,
                unsigned int ind,
                WfPort::WfPortType portType,
                WfCst::WfDataType dataType,
                unsigned int depth) throw (WfStructException);

    virtual void
        connectNodePorts() throw (WfStructException);

    void
        setDoMap(const string& leftPortName,
                 const string& rightPortName)
        throw (WfStructException);

    void
        setWhileCondition(const string& conditionStr)
        throw (WfStructException);

    // ******************** INSTANCIATION *********************

    virtual void
        initialize();

    virtual void
        instanciate(Dag* dag);

    virtual void
        createRealInstance(Dag* dag,
                           const FDataTag& currTag,
                           vector<FDataHandle*>& currDataLine);

    virtual void
        createVoidInstance(const FDataTag& currTag,
                           vector<FDataHandle*>& currDataLine);

    virtual void
        updateInstanciationStatus();

  protected:

    virtual void
        checkCondition() throw (WfStructException);

    bool
        testCondition(const vector<FDataHandle*>& currDataLine);
    void
        initLoopInPorts(vector<FDataHandle*>& currDataLine);

    vector<WfExprVariable*>*  myConditionVars;
    WfBooleanExpression*      myCondition;
    FNodePortMap  myDoMap;        // used for IN LOOP => OUT LOOP
    FNodePortMap  myFinalOutMap;  // used for IN LOOP => OUT
    FNodePortMap  myFinalLoopMap; // used for VOID => OUT LOOP
    FNodePortMap  myFinalVoidOutMap;  // used for VOID => OUT
    FNodePortMap  myDirectInOutMap;   // used for IN => OUT (no loop iteration)
    InputIterator*  myLoopIterator;

  private:
    /**
     * Total nb of running loop instances
     */
    int activeInstanceNb;

    /**
     * Length of loop tags
     */
    unsigned int loopTagLength;

};

#endif // _FLOOPNODE_HH_
