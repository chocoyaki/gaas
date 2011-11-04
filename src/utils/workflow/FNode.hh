/**
* @file  FNode.hh
* 
* @brief  The classes representing the node functional workflow
* 
* @author  Benjamin ISNARD (Benjamin.Isnard@ens-lyon.fr)
* 
* @section Licence
*   |LICENSE|                                                                
*/

#ifndef _FNODE_HH_
#define _FNODE_HH_

#include <map>
#include <string>
#include <vector>

#include "WfNode.hh"
#include "FNodePort.hh"
#include "InputIterator.hh"
#include "WfUtils.hh"
class FWorkflow;

/*****************************************************************************/
/*                                 FNode                                     */
/*****************************************************************************/
class FNode : public WfNode {
public:
  friend class FNodeInPort;

  virtual ~FNode();

  /**
   * Get the workflow the node belongs to
   * @return workflow ptr or NULL if no parent wf
   */
  FWorkflow *
  getWorkflow() const;

  /**
   * Get the root workflow (ie not a subwf) the node belongs to
   * @return workflow ptr or NULL if no parent wf
   */
  virtual FWorkflow*
  getRootWorkflow() const;

  /**
   * Create a new port
   */
  virtual WfPort *
  newPort(std::string portId, unsigned int ind,
          WfPort::WfPortType portType, WfCst::WfDataType dataType,
          unsigned int depth) throw(WfStructException);

  /**
   * Get the name of the default port (only for interface nodes)
   */
  virtual const std::string&
  getDefaultPortName() const;

  /**
   * Node interconnections (and type check)
   */
  virtual void
  connectNodePorts() throw(WfStructException);

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
   * Finalize - called once after instanciation is completed
   *  - check if output ports have 0 items and updates connected ports if yes
   */
  virtual void
  finalize();

  /**
   * Instanciation status: ready to process data
   */
  bool
  instanciationReady();

  /**
   * Instanciation status: waiting for event from dag engine
   */
  bool
  instanciationPending();

  /**
   * Instanciation status: waiting to resume instanciation (stopped due
   * to instance nb limitation)
   */
  bool
  instanciationOnHold();

  /**
   * Instanciation status: stopped (unexpected event: instanciation or
   * execution error)
   */
  bool
  instanciationStopped();

  /**
   * Instanciation status: no more data to process
   */
  bool
  instanciationCompleted();

  /**
   * Stop the instanciation (execution error)
   */
  void
  stopInstanciation();

  /**
   * Resume instanciation
   * Calls the parent workflow to resume its instanciation as well.
   */
  void
  resumeInstanciation();

  /**
   * Set instanciation as complete
   * (used when node cannot update its status itself)
   */
  void
  setInstanciationCompleted();

  /**
   * Remove data created by the node on the platform
   */
  virtual void
  freeNodePersistentData(MasterAgent_var& MA);

  /**
   * Write a text description of the node
   */
  virtual std::string
  toString() const;

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
  FNode(FWorkflow* wf, const std::string& id, nodeInstStatus_t initStatus);

  /**
   * Instanciation status
   */
  nodeInstStatus_t myStatus;

  /**
   * Set instanciation status to ready
   */
  void
  setStatusReady();

  /**
   * Trace header
   */
  std::string
  traceId();
};

/*****************************************************************************/
/*                           FConstantNode                                   */
/*****************************************************************************/

class FConstantNode : public FNode {
public:
  FConstantNode(FWorkflow* wf,
                const std::string& id,
                WfCst::WfDataType type);
  virtual ~FConstantNode();

  /**
   * Get the name of the default port
   */
  virtual const std::string&
  getDefaultPortName() const;

  /**
   * Set the value of the constant
   */
  void
  setValue(const std::string& strVal);

  /**
   * Set the data ID of the constant
   */
  void
  setDataID(const std::string& dataID);

  /**
   * Initialization of the constant node
   */
  virtual void
  initialize();

  /**
   * Instanciate a constant node (does nothing)
   * @param dag should be NULL
   */
  virtual void
  instanciate(Dag* dag);

private:
  /**
   * The default name of the output port
   * (a default port is automatically created to allow connections from
   *  other nodes input ports)
   */
  static std::string outPortName;
  FNodeOutPort* myOutPort;

  std::string myValue;
  std::string myDataID;
  FDataHandle* myDH;
};

/*****************************************************************************/
/*                            FSourceNode                                    */
/*****************************************************************************/

class DataSourceParser;  // used to parse the data source XML file

/**
 * Class FSourceNode
 * This type of functional workflow node is used to provide a data stream to
 * the inputs of processor nodes of the workflow. The items sent through this
 * stream are described in a XML file
 */
class FSourceNode : public FNode {
public:
  FSourceNode(FWorkflow* wf, const std::string& id, WfCst::WfDataType type);

  virtual ~FSourceNode();

  /***************************************************/
  /* event message types                             */
  /***************************************************/

  enum eventMsg_e {
    DATATREE
  };

  const std::string&
  getDefaultPortName() const;

  WfCst::WfDataType
  getDataType() const;

  unsigned int
  getDepth() const;

  void
  connectToWfPort(FNodePort* port);  // used for sub-workflows

  bool
  isConnectedToWfPort();

  /**
   * One-shot instanciate using workflow's data file (XML)
   * Creates all data items and send them to connected ports
   * @param dag the current dag (assumed not NULL)
   */
  virtual void
  instanciate(Dag* dag);

  /**
   * Create one data item for this source (used for SUB-WORKFLOWS)
   * Uses the connected port index to choose data within data line
   * @param currTag   tag of current DH
   * @param currDataLine      the data line provided by the workflow
   */
  virtual void
  createInstance(const FDataTag& currTag,
                 std::vector<FDataHandle*>& currDataLine);

  /**
   * Write the XML description of the source
   * with data IDs (if available)
   */
  virtual void
  toXML(std::ostream& output);

protected:
  /**
   * Instanciate a new data for the source (used by parser)
   * @param tag the data tag
   * @return ref on the data
   */
  FDataHandle *
  createData(const FDataTag& tag);

  /**
   * Instanciate a new list for the source (used by parser)
   * @param tag the data tag
   * @return ref on the data
   */
  FDataHandle *
  createList(const FDataTag& tag);

  /**
   * Set the value of the data
   * @param DH ref on the data
   * @param value string containing the value
   */
  void
  setDataValue(FDataHandle* DH, const std::string& value);

  /**
   * Set the data ID of the data
   * @param DH ref on the data
   * @param dataID  string containing the data ID
   */
  void
  setDataID(FDataHandle* DH, const std::string& dataID);

  /**
   * Set a property of the data
   * @param DH ref on the data
   * @param propKey key of the property
   * @param propValue value of the property
   */
  void
  setDataProperty(FDataHandle* DH, const std::string& propKey,
                  const std::string& propValue);
  /**
   * Store the new data for the source (used by parser)
   * @param newDH the ref to the data (created by createData)
   */
  void
  insertData(FDataHandle* newDH);

private:
  // provides access to protected method (no use of attributes)
  friend class DataSourceHandler;

  DataSourceParser * myParser;
  static std::string outPortName;
  FNodeOutPort* myOutPort;

  FNodeInPort* myConnectedPort;  // used for sub-workflows
  bool  isConnected;
};

/*****************************************************************************/
/*                             FSinkNode                                     */
/*****************************************************************************/

class FSinkNode : public FNode {
public:
  FSinkNode(FWorkflow* wf, const std::string& id,
            WfCst::WfDataType type, unsigned int depth);

  virtual ~FSinkNode();

  /***************************************************/
  /* event message types                             */
  /***************************************************/

  enum eventMsg_e {
    DATAID
  };

  virtual const std::string&
  getDefaultPortName() const;

  virtual void
  connectToWfPort(FNodePort* port);  // used for sub-workflows

  bool
  isConnectedToWfPort();

  virtual void
  instanciate(Dag* dag);  // used for sub-workflows

  virtual void
  finalize();  // does nothing

  /**
   * Method called once after instanciation is completed
   * Will download data IDs and values
   */
  virtual void
  downloadResults();

  /**
   * Write sink data in a readable way
   */
  virtual void
  displayResults(std::ostream& output);

  /**
   * Write the XML description of sink data
   * with data IDs
   */
  virtual void
  toXML(std::ostream& output);

  /**
   * Get the data ID of a container that contains all sink data
   * @param  containerID string that will contain the dataID
   */
  virtual void
  getResultsInContainer(std::string& containerID);

private:

  static std::string inPortName;
  FNodeInPort* myInPort;
  FNodeOutPort* myOutPort;  // used as a buffer only

  // used for sub-workflows
  FNodeOutPort* myConnectedPort;
  bool  isConnected;
  PortInputIterator*  myIterator;
};

/*****************************************************************************/
/*                             FProcNode                                     */
/*****************************************************************************/

class FProcNode : public FNode {
public:
  FProcNode(FWorkflow* wf, const std::string& id);

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
    OPER_CROSS,
    OPER_FLATCROSS };

  /**
   * Create the iteration strategy tree nodes
   * @param opType  the type of operator
   * @param inputIds  the ids of the inputs of the operator
   * @return the id of the operator created
   */
  const std::string&
  createInputOperator(inputOperator_t opType,
                      const std::vector<std::string>& inputIds);

  /**
   * Set the root element of the iteration strategy tree
   * @param opId  the id of the root operator (must already be registered)
   */
  void
  setRootInputOperator(const std::string& opId);

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
  checkDynamicParam(const std::string& paramName,
                    const std::string& paramValue);

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
                     std::vector<FDataHandle*>& currDataLine) = 0;

  virtual void
  createVoidInstance(const FDataTag& currTag,
                     std::vector<FDataHandle*>& currDataLine);

  virtual void
  updateInstanciationStatus();

  /**
   * Instanciation of the processor (TEMPLATE METHOD)
   * @param dag ref to the dag that will contain the generated DagNodes
   */
  virtual void
  instanciate(Dag* dag);

  virtual bool
  hasDataToProcess();

protected:

  // ******************** DYNAMIC PARAMETERS *********************

  /**
   * Create a dynamic parameter
   * @param paramName    name of the parameter
   * @param paramVarName name of the variable
   */
  void
  setDynamicParam(const std::string& paramName,
                  const std::string& paramVarName);

  /**
   * Set the value of a dynamic parameter
   * @param paramVarName  name of the variable to set
   * @param paramValue    value of the variable
   */
  void
  setDynamicParamValue(const std::string& paramVarName,
                       const std::string& paramValue);

  bool
  isDynamicParam(const std::string& paramName);

  const std::string&
  getDynamicParamValue(const std::string& paramName);

  // ****************** INTERNAL PORT MAPPING  *********************
  /**
   * Check a port name and returns the appropriate pointer
   * @param portName  name of the port
   * @return the correct port ref (cannot return NULL)
   */
  template<class PortType>
  PortType*
  checkAssignPort(const std::string& portName)
    throw(WfStructException) {
    WfPort* port = getPort(portName);  // throws exception
    PortType* FPort = dynamic_cast<PortType*>(port);
    if (!FPort) {
      std::string errorMsg = std::string("Invalid port in port assignment ")
        + "(node=" + getId() + "/port=" + portName + ")";
      throw WfStructException(WfStructException::eOTHER, errorMsg);
    }
    return FPort;
  }

  // ******************** INPUT DATA OPERATORS *********************

  PortInputIterator *
  createPortInputIterator(const std::string& portId);

  bool
  isIteratorDefined(const std::string& portId);

  void
  initDataLine();

  /**
   * The map of all input operators
   */
  std::map<std::string, InputIterator*> myIterators;

  /**
   * The root operator
   */
  InputIterator*  myRootIterator;

  /**
   * The template data line used for each instance
   * Contains the constant port values and is copied for each new instance
   */
  std::vector<FDataHandle*>* cstDataLine;

  /**
   * The dynamic parameters map
   * contains param_Name => param_Variable_Name
   */
  std::map<std::string, std::string> dynParMap;

  /**
   * The variables map
   * contains param_Variable_Name => param_Value
   */
  std::map<std::string, std::string> varMap;
};

#endif  // _FNODE_HH_
