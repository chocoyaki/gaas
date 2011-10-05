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
 * Revision 1.6  2009/07/23 12:30:10  bisnard
 * new method finalize() for functional wf nodes
 * removed const on currDataLine parameter for instance creation
 *
 * Revision 1.5  2009/07/07 09:03:22  bisnard
 * changes for sub-workflows (FWorkflow class now inherits from FProcNode)
 *
 * Revision 1.4  2009/06/15 12:11:12  bisnard
 * use new XML Parser (SAX) for data source file
 * use new class WfValueAdapter to avoid data duplication
 * use new method FNodeOutPort::storeData
 * changed method to compute total nb of data items
 *
 * Revision 1.3  2009/05/27 08:49:43  bisnard
 * - modified condition output: new IF_THEN and IF_ELSE port types
 * - implemented MERGE and FILTER workflow nodes
 *
 * Revision 1.2  2009/05/15 11:10:20  bisnard
 * release for workflow conditional structure (if)
 *
 * Revision 1.1  2009/04/17 09:04:07  bisnard
 * initial version for conditional nodes in functional workflows
 *
 */

#ifndef _FIFNODE_HH_
#define _FIFNODE_HH_

#include "FNode.hh"
#include "WfUtils.hh"
#include "WfExpression.hh"


/*****************************************************************************/
/*                            FIfNode class                                  */
/*****************************************************************************/


class FIfNode : public FProcNode {

public:

  FIfNode(FWorkflow* wf, const string& id);
  virtual ~FIfNode();

  // ******************** NODE SETUP *********************

  virtual WfPort *
  newPort(string portId,
          unsigned int ind,
          WfPort::WfPortType portType,
          WfCst::WfDataType dataType,
          unsigned int depth) throw (WfStructException);

  void
  setCondition(const string& conditionStr)
    throw (WfStructException);

  void
  setThenMap(const string& leftPortName,
             const string& rightPortName)
    throw (WfStructException);

  void
  setElseMap(const string& leftPortName,
             const string& rightPortName)
    throw (WfStructException);

  // ******************** INSTANCIATION *********************

  /**
   * Instanciation of the activity as a DagNode (inside data processing loop)
   * @param dag ref to the dag that will contain the DagNode
   * @param currTag data tag of the current dataLine
   * @param currDataLine dataLine used to set input data for the DagNode
   */
  virtual void
  createRealInstance(Dag* dag,
                     const FDataTag& currTag,
                     vector<FDataHandle*>& currDataLine);

protected:

  virtual void
  checkCondition() throw (WfStructException);

  FNodePortMap  myThenMap;
  FNodePortMap  myElseMap;
  vector<WfExprVariable*>*  myConditionVars;
  WfBooleanExpression*      myCondition;

};

/*****************************************************************************/
/*                           FMergeNode class                                */
/*****************************************************************************/

class FMergeNode : public FProcNode {

public:

  FMergeNode(FWorkflow* wf, const string& id);
  ~FMergeNode();

  /**
   * Create a new port
   * A merge node must create 2 input ports and 1 output port to be valid
   */
  virtual WfPort *
  newPort(string portId,
          unsigned int ind,
          WfPort::WfPortType portType,
          WfCst::WfDataType dataType,
          unsigned int depth) throw (WfStructException);

  virtual void
  createRealInstance(Dag* dag,
                     const FDataTag& currTag,
                     vector<FDataHandle*>& currDataLine);

  virtual void
  createVoidInstance(const FDataTag& currTag,
                     vector<FDataHandle*>& currDataLine);


private:

  void
  createMergeInstance(const FDataTag& currTag,
                      vector<FDataHandle*>& currDataLine);

  FNodeOutPort* myOutPort;

};

/*****************************************************************************/
/*                           FFilterNode class                               */
/*****************************************************************************/

class FFilterNode : public FProcNode {

public:

  FFilterNode(FWorkflow* wf, const string& id);
  virtual ~FFilterNode();

  virtual WfPort *
  newPort(string portId,
          unsigned int ind,
          WfPort::WfPortType portType,
          WfCst::WfDataType dataType,
          unsigned int depth) throw (WfStructException);

  virtual void
  createRealInstance(Dag* dag,
                     const FDataTag& currTag,
                     vector<FDataHandle*>& currDataLine);

  virtual void
  createVoidInstance(const FDataTag& currTag,
                     vector<FDataHandle*>& currDataLine);

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

  void updateUp(FDataHandle* DH, short depth);
  void updateRight(const FDataTag& tag, int precIdx, short depth);
  void updateLeftNonVoid(const FDataTag& tag, short depth);
  void updateLeftVoid(const FDataTag& tag, short depth);
  void sendDown(FDataHandle* DH, short depth);

  bool isNonVoid(FDataHandle* DH);  // test if there is at least one non-void element (only for container)
  bool isReadyAssumingParentIs(const FDataTag& srcTag);
  bool isReady(const FDataTag& srcTag);
  FDataTag getNewTag(const FDataTag& srcTag);
  filterNode_t* getTreeNode(FDataHandle* DH);
  filterNode_t* getTreeNode(const FDataTag& tag);

  FNodeOutPort* myOutPort;

  map<FDataTag, filterNode_t*> myTree;

};


#endif // _FIFNODE_HH_
