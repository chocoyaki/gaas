/**
* @file  FIfNode.hh
* 
* @brief  The class representing the IF condition in a functional workflow
* 
* @author  Benjamin ISNARD (Benjamin.Isnard@ens-lyon.fr)
* 
* @section Licence
*   |LICENSE|                                                                
*/


#ifndef _FIFNODE_HH_
#define _FIFNODE_HH_

#include <map>
#include <string>
#include <vector>
#include "FNode.hh"
#include "WfUtils.hh"
#include "WfExpression.hh"


/*****************************************************************************/
/*                            FIfNode class                                  */
/*****************************************************************************/


class FIfNode : public FProcNode {
public:
  FIfNode(FWorkflow* wf, const std::string& id);

  virtual ~FIfNode();

  // ******************** NODE SETUP *********************

  virtual WfPort *
  newPort(std::string portId, unsigned int ind,
          WfPort::WfPortType portType, WfCst::WfDataType dataType,
          unsigned int depth) throw(WfStructException);

  void
  setCondition(const std::string& conditionStr)
    throw(WfStructException);

  void
  setThenMap(const std::string& leftPortName, const std::string& rightPortName)
    throw(WfStructException);

  void
  setElseMap(const std::string& leftPortName, const std::string& rightPortName)
    throw(WfStructException);

  // ******************** INSTANCIATION *********************

  /**
   * Instanciation of the activity as a DagNode (inside data processing loop)
   * @param dag ref to the dag that will contain the DagNode
   * @param currTag data tag of the current dataLine
   * @param currDataLine dataLine used to set input data for the DagNode
   */
  virtual void
  createRealInstance(Dag* dag, const FDataTag& currTag,
                     std::vector<FDataHandle*>& currDataLine);

protected:
  virtual void
  checkCondition() throw(WfStructException);

  FNodePortMap  myThenMap;
  FNodePortMap  myElseMap;
  std::vector<WfExprVariable*>* myConditionVars;
  WfBooleanExpression* myCondition;
};

/*****************************************************************************/
/*                           FMergeNode class                                */
/*****************************************************************************/

class FMergeNode : public FProcNode {
public:
  FMergeNode(FWorkflow* wf, const std::string& id);

  ~FMergeNode();

  /**
   * Create a new port
   * A merge node must create 2 input ports and 1 output port to be valid
   */
  virtual WfPort *
  newPort(std::string portId, unsigned int ind,
          WfPort::WfPortType portType, WfCst::WfDataType dataType,
          unsigned int depth) throw(WfStructException);

  virtual void
  createRealInstance(Dag* dag, const FDataTag& currTag,
                     std::vector<FDataHandle*>& currDataLine);

  virtual void
  createVoidInstance(const FDataTag& currTag,
                     std::vector<FDataHandle*>& currDataLine);

private:
  void
  createMergeInstance(const FDataTag& currTag,
                      std::vector<FDataHandle*>& currDataLine);

  FNodeOutPort* myOutPort;
};

/*****************************************************************************/
/*                           FFilterNode class                               */
/*****************************************************************************/

class FFilterNode : public FProcNode {
public:
  FFilterNode(FWorkflow* wf, const std::string& id);

  virtual ~FFilterNode();

  virtual WfPort *
  newPort(std::string portId, unsigned int ind,
          WfPort::WfPortType portType, WfCst::WfDataType dataType,
          unsigned int depth) throw(WfStructException);

  virtual void
  createRealInstance(Dag* dag, const FDataTag& currTag,
                     std::vector<FDataHandle*>& currDataLine);

  virtual void
  createVoidInstance(const FDataTag& currTag,
                     std::vector<FDataHandle*>& currDataLine);

private:
  struct filterNode_t {
    FDataHandle*  dataHdl;
    bool          voidDef;
    bool          indexOk;
    bool          lastFlagOk;
    bool          isDone;
    int           newIndex;   // may be negative (-1)
    bool          newLastFlag;
  };

  void
  updateUp(FDataHandle* DH, short depth);

  void
  updateRight(const FDataTag& tag, int precIdx, short depth);

  void
  updateLeftNonVoid(const FDataTag& tag, short depth);

  void
  updateLeftVoid(const FDataTag& tag, short depth);

  void
  sendDown(FDataHandle* DH, short depth);

  // test if there is at least one non-void element (only for container)
  bool
  isNonVoid(FDataHandle* DH);

  bool
  isReadyAssumingParentIs(const FDataTag& srcTag);

  bool
  isReady(const FDataTag& srcTag);

  FDataTag
  getNewTag(const FDataTag& srcTag);

  filterNode_t*
  getTreeNode(FDataHandle* DH);

  filterNode_t*
  getTreeNode(const FDataTag& tag);

  FNodeOutPort* myOutPort;

  std::map<FDataTag, filterNode_t*> myTree;
};


#endif  // _FIFNODE_HH_
