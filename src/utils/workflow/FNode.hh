/****************************************************************************/
/* The classes representing the nodes of a functional workflow              */
/*                                                                          */
/* Author(s):                                                               */
/* - Benjamin ISNARD (benjamin.isnard@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.3  2008/12/09 12:15:59  bisnard
 * pending instanciation handling (uses dag outputs for instanciation
 * of a functional wf)
 *
 * Revision 1.2  2008/12/02 10:09:36  bisnard
 * added instanciation methods
 *
 * Revision 1.1  2008/10/20 08:09:58  bisnard
 * new classes for functional workflows
 *
 */

#ifndef _FNODE_HH_
#define _FNODE_HH_

#include <map>

#include "Node.hh"
#include "FNodePort.hh"
#include "WfUtils.hh"
class FWorkflow;

using namespace std;

/*****************************************************************************/
/*                                 FNode                                     */
/*****************************************************************************/

class FNode : public Node {

public:
  FNode(FWorkflow* wf, const string& id, short maxInstances);
  virtual ~FNode();

  virtual WfPort *
  newPort(string portId,
          unsigned int ind,
          WfPort::WfPortType portType,
          WfCst::WfDataType dataType,
          unsigned int depth);
  /**
   * instanciation
   */

  virtual void
  instanciate(Dag* dag) = 0;

  bool
  instanciationOnHold();

  void
  resumeInstanciation();

  virtual bool
  instanciationCompleted();

protected:
  FWorkflow * wf;

  /**
   * Status 'on hold' (instance limitation)
   */
  bool isOnHoldFlag;

  /**
   * Status 'fully instantiated' used when last input has been processed
   */
  bool isFullInst;

  /**
   * Max number of instances per dag
   */
  short maxInstNb;

}; // end class FNode

/*****************************************************************************/
/*                           FConstantNode                                   */
/*****************************************************************************/

class FConstantNode : public FNode {

  public:
    FConstantNode(FWorkflow* wf,
                  const string& id,
                  WfCst::WfDataType type);
    virtual ~FConstantNode();
    void setValue(const string& strVal);

    virtual void instanciate(Dag* dag); // parameter is not used

  private:
    string myValue;
    static string outPortName;
}; // end class FConstantNode

/*****************************************************************************/
/*                            FSourceNode                                    */
/*****************************************************************************/

class FSourceNode : public FNode {

  public:
    FSourceNode(FWorkflow* wf,
                const string& id,
                WfCst::WfDataType type,
                short maxInstances);
    virtual ~FSourceNode();

    virtual void instanciate(Dag* dag); // parameter is not used

  private:

    static string outPortName;
}; // end class FSourceNode

/*****************************************************************************/
/*                             FProcNode                                     */
/*****************************************************************************/

class FProcNode : public FNode {

  public:
    FProcNode(FWorkflow* wf,
              const string& id,
              short maxInstances);
    virtual ~FProcNode();

    virtual void instanciate(Dag* dag);

    /**
     * Setup the information used when node instance (dagnode) is done
     * and some output ports instances have to be re-submitted to next
     * nodes (when cardinal was not known before node execution)
     */
    void
    setPendingInstanceInfo(DagNode * dagNode,
                           FDataHandle * dataHdl,
                           FNodeOutPort * outPort,
                           FNodeInPort * inPort); // adds info to the multimap

    /**
     * Returns true if the node instanciation is pending until some dag
     * node execution
     */
    bool
    instanciationPending();

    /**
     * Returns true when instanciation is completed
     */
    virtual bool
    instanciationCompleted();

    /**
     * Handle dag node execution event
     * Checks the pending dag node info map and if the dag node is present
     * then use info to re-submit the datahandles to the correct in ports
     */
    void
    instanceIsDone(DagNode * dagNode, bool& statusChange);

    void
    setDIETServicePath(const string& path);

  protected:

    struct pendingDagNodeInfo_t {
      FDataHandle * dataHdl;
      FNodeOutPort * outPort;
      FNodeInPort * inPort;
    };

    string myPath;

    multimap<DagNode*, pendingDagNodeInfo_t> pendingNodes;

}; // end class FProcNode

/*****************************************************************************/
/*                           FConditionNode                                  */
/*****************************************************************************/

class FConditionNode : public FNode {

  public:
    FConditionNode(FWorkflow* wf, const string& id);
    virtual ~FConditionNode();

    virtual void instanciate(Dag* dag);
}; // end class FConditionNode

#endif // _FNODE_HH_
