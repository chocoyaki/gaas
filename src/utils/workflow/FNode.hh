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
 * Revision 1.15  2009/07/07 09:03:22  bisnard
 * changes for sub-workflows (FWorkflow class now inherits from FProcNode)
 *
 * Revision 1.14  2009/06/15 12:11:13  bisnard
 * use new XML Parser (SAX) for data source file
 * use new class WfValueAdapter to avoid data duplication
 * use new method FNodeOutPort::storeData
 * changed method to compute total nb of data items
 *
 * Revision 1.13  2009/05/27 08:49:43  bisnard
 * - modified condition output: new IF_THEN and IF_ELSE port types
 * - implemented MERGE and FILTER workflow nodes
 *
 * Revision 1.12  2009/05/15 11:10:20  bisnard
 * release for workflow conditional structure (if)
 *
 * Revision 1.11  2009/04/17 09:04:07  bisnard
 * initial version for conditional nodes in functional workflows
 *
 * Revision 1.10  2009/04/09 09:56:20  bisnard
 * refactoring due to new class FActivityNode
 *
 * Revision 1.9  2009/04/08 09:34:56  bisnard
 * pending nodes mgmt moved to FWorkflow class
 * FWorkflow and FNode state graph revisited
 * FNodePort instanciation refactoring
 *
 * Revision 1.8  2009/02/24 14:01:05  bisnard
 * added dynamic parameter mgmt for wf processors
 *
 * Revision 1.7  2009/02/20 10:23:54  bisnard
 * use estimation class to reduce the nb of submit requests
 *
 * Revision 1.6  2009/02/06 14:55:08  bisnard
 * setup exceptions
 *
 * Revision 1.5  2009/01/16 16:31:54  bisnard
 * added option to specify data source file name
 *
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

#include "WfNode.hh"
#include "FNodePort.hh"
#include "InputIterator.hh"
#include "WfUtils.hh"
class FWorkflow;

using namespace std;

/*****************************************************************************/
/*                                 FNode                                     */
/*****************************************************************************/

class FNode : public WfNode {

public:
  friend class FNodeInPort;

  virtual ~FNode();

  /**
   * Get the workflow the node belongs to
   */
  FWorkflow *
      getWorkflow();

  /**
   * Create a new port
   */
  virtual WfPort *
      newPort(string portId,
              unsigned int ind,
              WfPort::WfPortType portType,
              WfCst::WfDataType dataType,
              unsigned int depth) throw (WfStructException);

  /**
   * Get the name of the default port (only for interface nodes)
   */
  virtual const string&
      getDefaultPortName() const;

  /**
   * Node interconnections (and type check)
   */
  virtual void
      connectNodePorts() throw (WfStructException);

  /**
   * Node connections to workflow external ports (used for sub-workflows)
   */
  virtual void
      connectToWfPort(FNodePort* port);

  /**
   * Initialization
   * (method called once before starting instanciation)
   * Note: may throw different types of exceptions depending on class
   * - XMLParsingException (sources)
   * - WfStructException (proc / sink)
   */
  virtual void
      initialize();

  /**
   * Instanciation = run the instanciation loop for all available input data items
   * and eventually create dag nodes in the given dag
   * @param dag the current generated dag
   */

  virtual void
      instanciate(Dag* dag) = 0;

  /**
   * Instanciation status: ready to process data
   */
  bool instanciationReady();

  /**
   * Instanciation status: waiting for event from dag engine
   */
  bool instanciationPending();

  /**
   * Instanciation status: waiting to resume instanciation (stopped due
   * to instance nb limitation)
   */
  bool instanciationOnHold();

  /**
   * Instanciation status: stopped (unexpected event: instanciation or
   * execution error)
   */
  bool instanciationStopped();

  /**
   * Instanciation status: no more data to process
   */
  bool instanciationCompleted();

  /**
   * Stop the instanciation (execution error)
   */
  void stopInstanciation();

  /**
   * Resume instanciation
   * Calls the parent workflow to resume its instanciation as well.
   */
  void resumeInstanciation();

protected:
  /**
   * Parent workflow (may be NULL)
   */
  FWorkflow * wf;

  /**
   * Instanciation status type
   */
  typedef enum {
    N_INSTANC_READY,
    N_INSTANC_ONHOLD,
    N_INSTANC_PENDING,    // waiting for event from dag engine
    N_INSTANC_END,
    N_INSTANC_STOPPED } nodeInstStatus_t;

  /**
   * Constructor
   */
  FNode(FWorkflow* wf, const string& id, nodeInstStatus_t initStatus);

  /**
   * Instanciation status
   */
  nodeInstStatus_t  myStatus;

  /**
   * Set instanciation status to ready
   */
  void setStatusReady();

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

  // provides access to protected method (no use of attributes)
  friend class DataSourceHandler;

  public:
    FSourceNode(FWorkflow* wf,
                const string& id,
                WfCst::WfDataType type);
    virtual ~FSourceNode();

    const string&
        getDefaultPortName() const;

    WfCst::WfDataType
        getDataType() const;

    unsigned int
        getDepth() const;

    virtual void
        connectToWfPort(FNodePort* port); // used for sub-workflows

    virtual void
        initialize();

    virtual void
        instanciate(Dag* dag); // parameter is not used

    virtual void
        instanciate(const FDataTag& currTag,
                    const vector<FDataHandle*>& currDataLine); // used for sub-workflows

  protected:
    /**
     * Instanciate a new value for the source (used by parser)
     * @param tag the data tag for the value
     * @param value the value as a string
     */
    void
        instanciate(const FDataTag& tag, const string& value);

  private:
    DataSourceParser * myParser;
    static string outPortName;
    FNodeOutPort* myOutPort;

    FNodeInPort* myConnectedPort; // used for sub-workflows
    bool  isConnected;

}; // end class FSourceNode

/*****************************************************************************/
/*                             FSinkNode                                     */
/*****************************************************************************/

class FSinkNode : public FNode {

   public:
     FSinkNode(FWorkflow* wf,
               const string& id,
               WfCst::WfDataType type,
               unsigned int depth);
     virtual ~FSinkNode();

     virtual const string&
         getDefaultPortName() const;

     virtual void
         connectToWfPort(FNodePort* port); // used for sub-workflows

     virtual void
         initialize();

     virtual void
         instanciate(Dag* dag); // used for sub-workflows

     virtual void
         displayResults(ostream& output);

  private:

    static string inPortName;
    FNodeInPort* myInPort;

    FNodeOutPort* myConnectedPort; // used for sub-workflows
    bool  isConnected;

}; // end class FSinkNode

/*****************************************************************************/
/*                             FProcNode                                     */
/*****************************************************************************/

class FProcNode : public FNode {

  public:
    FProcNode(FWorkflow* wf,
              const string& id);
    virtual ~FProcNode();

    // ******************** INPUT DATA OPERATORS *********************

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
     * @param opId  the id of the root operator (must already be registered)
     */
    void
        setRootInputOperator(const string& opId);

    /**
     * Set the root element of the iteration strategy tree
     * @param newOper  the ref of the root operator (not registered yet)
     */
    void
        setRootInputOperator(InputIterator * newOper);

    /**
     * Set one port of the node (identified by its index) as a constant
     * @param idxPort the index of the port
     * @param dataHdl the ref to the data provided as constant
     */
    void
        setConstantInput(int idxPort, FDataHandle* dataHdl);

    /**
     * Checks if a given port index corresponds to a constant
     * @param idxPort the index of the port
     * @return true if the port has been set as constant
     */
    bool
        isConstantInput(int idxPort);

    // ******************** DYNAMIC PARAMETERS *********************

    /**
     * Setup a dynamic parameter for the node
     * @param paramName string containing the name of the parameter
     * @param paramValue string containing the value of the parameter
     */
    void
        checkDynamicParam(const string& paramName, const string& paramValue);


    // *********************** INSTANCIATION *************************

    /**
     * Initialization - called once before all instanciation calls
     *  - connection to other nodes
     *  - input iterators setup
     */
    virtual void
        initialize();

    /**
     * Instanciation initialization - called for each instanciation call
     */
    virtual void
        initInstanciation();

    virtual bool
        instLimitReached();

    virtual void
        createRealInstance(Dag* dag,
                           const FDataTag& currTag,
                           const vector<FDataHandle*>& currDataLine) = 0;

    virtual void
        createVoidInstance(const FDataTag& currTag,
                           const vector<FDataHandle*>& currDataLine);

    virtual void
        updateInstanciationStatus();

    /**
     * Instanciation of the processor (TEMPLATE METHOD)
     * @param dag ref to the dag that will contain the generated DagNodes
     */
    virtual void
        instanciate(Dag* dag);

  protected:

    // ******************** DYNAMIC PARAMETERS *********************

    /**
     * Create a dynamic parameter
     * @param paramName    name of the parameter
     * @param paramVarName name of the variable
     */
    void
        setDynamicParam(const string& paramName,
                        const string& paramVarName);

    /**
     * Set the value of a dynamic parameter
     * @param paramVarName  name of the variable to set
     * @param paramValue    value of the variable
     */
    void
        setDynamicParamValue(const string& paramVarName,
                             const string& paramValue);

    bool
        isDynamicParam(const string& paramName);

    const string&
        getDynamicParamValue(const string& paramName);

    // ****************** INTERNAL PORT MAPPING  *********************
    /**
     * Check a port name and returns the appropriate pointer
     * @param portName  name of the port
     * @return the correct port ref (cannot return NULL)
     */
    template<class PortType>
    PortType*
        checkAssignPort(const string& portName)
        throw (WfStructException) {
      WfPort* port = getPort(portName); // throws exception
      PortType* FPort = dynamic_cast<PortType*>(port);
      if (!FPort) {
        string errorMsg = string("Invalid port in port assignment ")
                      + "(node=" + getId() + "/port=" + portName + ")";
        throw WfStructException(WfStructException::eOTHER, errorMsg);
      }
      return FPort;
    }

    // ******************** INPUT DATA OPERATORS *********************

    PortInputIterator *
        createPortInputIterator(const string& portId);

    bool
        isIteratorDefined(const string& portId);

    void initDataLine();

    /**
     * The map of all input operators
     */
    map<string,InputIterator*> myIterators;

    /**
     * The root operator
     */
    InputIterator*  myRootIterator;

    /**
     * The template data line used for each instance
     * Contains the constant port values and is copied for each new instance
     */
    vector<FDataHandle*>* cstDataLine;

    /**
     * The dynamic parameters map
     * contains param_Name => param_Variable_Name
     */
    map<string,string> dynParMap;

    /**
     * The variables map
     * contains param_Variable_Name => param_Value
     */
    map<string,string> varMap;

}; // end class FProcNode

#endif // _FNODE_HH_
