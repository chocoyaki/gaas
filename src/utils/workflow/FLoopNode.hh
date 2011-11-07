/**
* @file  FLoopNode.hh
* 
* @brief  The classes representing the for/while loops of a functional workflow
* 
* @author  Benjamin ISNARD (Benjamin.Isnard@ens-lyon.fr)
* 
* @section Licence
*   |LICENCE|                                                                
*/


#ifndef _FLOOPNODE_HH_
#define _FLOOPNODE_HH_

#include <string>
#include <vector>
#include "FNode.hh"
#include "WfExpression.hh"

class FLoopNode : public FProcNode {
public:
  FLoopNode(FWorkflow* wf, const std::string& id);
  virtual ~FLoopNode();

  // ******************** NODE SETUP *********************

  virtual WfPort *
  newPort(std::string portId,
          unsigned int ind,
          WfPort::WfPortType portType,
          WfCst::WfDataType dataType,
          unsigned int depth) throw(WfStructException);

  virtual void
  connectNodePorts() throw(WfStructException);

  void
  setDoMap(const std::string& leftPortName,
           const std::string& rightPortName)
    throw(WfStructException);

  void
  setWhileCondition(const std::string& conditionStr)
    throw(WfStructException);

  // ******************** INSTANCIATION *********************

  virtual void
  initialize();

  virtual void
  instanciate(Dag* dag);

  virtual void
  createRealInstance(Dag* dag, const FDataTag& currTag,
                     std::vector<FDataHandle*>& currDataLine);

  virtual void
  createVoidInstance(const FDataTag& currTag,
                     std::vector<FDataHandle*>& currDataLine);

  virtual void
  updateInstanciationStatus();

protected:
  virtual void
  checkCondition() throw(WfStructException);

  bool
  testCondition(const std::vector<FDataHandle*>& currDataLine);

  void
  initLoopInPorts(std::vector<FDataHandle*>& currDataLine);

  std::vector<WfExprVariable*>*  myConditionVars;
  WfBooleanExpression* myCondition;
  FNodePortMap myDoMap;        // used for IN LOOP => OUT LOOP
  FNodePortMap myFinalOutMap;  // used for IN LOOP => OUT
  FNodePortMap myFinalLoopMap;  // used for VOID => OUT LOOP
  FNodePortMap myFinalVoidOutMap;  // used for VOID => OUT
  FNodePortMap myDirectInOutMap;   // used for IN => OUT (no loop iteration)
  InputIterator* myLoopIterator;

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

#endif  // _FLOOPNODE_HH_
