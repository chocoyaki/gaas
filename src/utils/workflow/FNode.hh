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
 * Revision 1.4  2009/01/16 13:54:50  bisnard
 * new version of the dag instanciator with iteration strategies for nodes with multiple input ports
 *
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
#include "InputIterator.hh"
#include "WfUtils.hh"
class FWorkflow;

using namespace std;

/*****************************************************************************/
/*                                 FNode                                     */
/*****************************************************************************/

class FNode : public Node {

public:
  FNode(FWorkflow* wf, const string& id);
  virtual ~FNode();

  virtual WfPort *
      newPort(string portId,
              unsigned int ind,
              WfPort::WfPortType portType,
              WfCst::WfDataType dataType,
              unsigned int depth);

  /**
   * Get the name of the default port (only for interface nodes)
   */
  virtual const string&
      getDefaultPortName() const;

  /**
   * Initialization
   * (method called once before starting instanciation)
   */
  virtual void
      initialize();

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

    /**
     * Get the name of the default port
     */
    virtual const string&
      getDefaultPortName() const;

    /**
     * Set the value of the constant
     */
    void
        setValue(const string& strVal);

    /**
     * Instanciate a constant node
     * @param dag should be NULL (not used because no dag node is created)
     */
    virtual void
        instanciate(Dag* dag);

  private:

    /**
     * The default name of the output port
     * (a default port is automatically created to allow connections from
     *  other nodes input ports)
     */
    static string outPortName;

    string myValue;


}; // end class FConstantNode

/*****************************************************************************/
/*                            FSourceNode                                    */
/*****************************************************************************/

class DataSourceParser; // used to parse the data source XML file

/**
 * Class FSourceNode
 * This type of functional workflow node is used to provide a data stream to
 * the inputs of processor nodes of the workflow. The items sent through this
 * stream are described in a XML file
 */

class FSourceNode : public FNode {

  public:
    FSourceNode(FWorkflow* wf,
                const string& id,
                WfCst::WfDataType type);
    virtual ~FSourceNode();

    virtual const string&
      getDefaultPortName() const;

    virtual void
        initialize();

    virtual void
        instanciate(Dag* dag); // parameter is not used

  private:
    DataSourceParser * myParser;
    static string outPortName;
    FNodeOutPort* myOutPort;
}; // end class FSourceNode

/*****************************************************************************/
/*                             FSinkNode                                     */
/*****************************************************************************/

class FSinkNode : public FNode {

   public:
     FSinkNode(FWorkflow* wf,
               const string& id,
               WfCst::WfDataType type);
     virtual ~FSinkNode();

     virtual const string&
      getDefaultPortName() const;

     virtual void
        initialize();

     virtual void
        instanciate(Dag* dag);

  private:

    static string inPortName;
    FNodeInPort* myInPort;
}; // end class FSinkNode

/*****************************************************************************/
/*                             FProcNode                                     */
/*****************************************************************************/

class FProcNode : public FNode {

  public:
    FProcNode(FWorkflow* wf,
              const string& id);
    virtual ~FProcNode();

    /**
     * Iteration strategies
     *  OPER_MATCH: match operator => match tags of all inputs
     *  OPER_CROSS: cross operator => cartesian product of two inputs
     */
    enum inputOperator_t {
      OPER_DOT,
      OPER_MATCH,
      OPER_CROSS };

    /**
     * Create the iteration strategy tree nodes
     * @param opType  the type of operator
     * @param inputIds  the ids of the inputs of the operator
     * @return the id of the operator created
     */
    const string&
        createInputOperator(inputOperator_t opType, const vector<string>& inputIds);

    /**
     * Set the root element of the iteration strategy tree
     * @param opId  the id of the root operator
     */
    void
        setRootInputOperator(const string& opId);

    /**
     * Set one port of the node (identified by its index) as a constant
     * @param idxPort the index of the port
     * @param dataHdl the ref to the data provided as constant
     */
    void
        setConstantInput(int idxPort, FDataHandle* dataHdl);

    /**
     * Set the maximum nb of instances of this node inside one dag
     * @param maxInst the max nb of instances
     */
    void
        setMaxInstancePerDag(short maxInst);

    /**
     * Initialization
     *  - connection to other nodes
     *  - input iterators setup
     */
    virtual void
        initialize();

    /**
     * Instanciation of the processor as a DagNode
     * @param dag ref to the dag that will contain the DagNode
     */
    virtual void
        instanciate(Dag* dag);

    /**
     * Setup the information used when node instance (dagnode) is done
     * and some output ports instances have to be re-submitted to next
     * nodes (when cardinal was not known before node execution)
     */
    void
        setPendingInstanceInfo(DagNode * dagNode,
                               FDataHandle * dataHdl,
                               FNodeOutPort * outPort,
                               FNodeInPort * inPort);

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

    PortInputIterator *
        createPortInputIterator(const string& portId);

    void initDataLine();

    struct pendingDagNodeInfo_t {
      FDataHandle * dataHdl;
      FNodeOutPort * outPort;
      FNodeInPort * inPort;
    };

    /**
     * The map of all input operators
     */
    map<string,InputIterator*> myIterators;

    /**
     * The root operator
     */
    InputIterator*  myRootIterator;

    /**
     * The service path
     */
    string myPath;

    /**
     * Max number of instances per dag
     */
    short maxInstNb;

    /**
     * The list of instances that will trigger new instanciation (due to
     * dependency of other functional nodes on the cardinal of the outputs of
     * this node)
     */
    multimap<DagNode*, pendingDagNodeInfo_t> pendingNodes;

    /**
     * The template data line used for each instance
     * Contains the constant port values and is copied for each new instance
     */
    vector<FDataHandle*>* cstDataLine;

}; // end class FProcNode

/*****************************************************************************/
/*                           FConditionNode                                  */
/*****************************************************************************/

class FConditionNode : public FNode {

  public:
    FConditionNode(FWorkflow* wf, const string& id);
    virtual ~FConditionNode();

    virtual void
        instanciate(Dag* dag);
}; // end class FConditionNode

#endif // _FNODE_HH_
