/**
* @file  FNodePort.hh
*
* @brief  The classes representing the ports the nodes of functional workflow
*
* @author  Benjamin ISNARD (Benjamin.Isnard@ens-lyon.fr)
*
* @section Licence
*   |LICENCE|
*/


#ifndef _FNODEPORT_HH_
#define _FNODEPORT_HH_

#include <list>
#include <map>
#include <string>
#include <vector>
#include "WfPort.hh"
#include "FDataHandle.hh"
#include "DagNode.hh"
#include "DagNodePort.hh"

class FNode;
class FNodeInPort;

/*****************************************************************************/
/*                         FNodePort (Abstract)                              */
/*****************************************************************************/

class FNodePort : public WfPort {
public:
  FNodePort(WfNode * parent, const std::string& _id,
            WfPort::WfPortType _portType, WfCst::WfDataType _type,
            unsigned int _depth, unsigned int _ind);
  virtual ~FNodePort();

  /**
   * Get a description of the node
   */
  virtual std::string
  toString() const;

protected:

  /**
   * Get the parent node
   * @return ref to FNode
   */
  FNode*
  getParentFNode();

  /**
   * Get the parent node (applicable only if port of processor node)
   * @return  ref to ProcNode (will exit if ref is NULL)
   */
  FProcNode*
  getParentProcNode();
};

/*****************************************************************************/
/*                           FNodeOutPort                                    */
/*****************************************************************************/

class FNodeOutPort : public virtual FNodePort {
public:
  FNodeOutPort(WfNode * parent, const std::string& _id,
               WfPort::WfPortType _portType, WfCst::WfDataType _type,
               unsigned int _depth, unsigned int _ind);

  virtual ~FNodeOutPort();

  /**
   * Method to setup ports connection on each side
   */
  virtual void
  connectToPort(WfPort* remPort);

  /**
   * Method to store a new data handle in the buffer
   * (must be called before sending data)
   */
  virtual void
  storeData(FDataHandle* dataHdl);

  /**
   * Method to send a data handle to the connected ports
   */
  virtual void
  sendData(FDataHandle* dataHdl);

  /**
   * Method to re-send a data handle to a given connected port when the first
   * attempt (sendData) was unsuccessful and therefore data transfer was set
   * as pending
   */
  virtual void
  reSendData(FDataHandle* dataHdl, FNodeInPort* inPort);

  /**
   * Send all the data contained in the buffer
   */
  virtual void
  sendAllData();

  /**
   * Instanciate the port as a real output port (used for tasks)
   * @param dag       the current dag being instanciated
   * @param nodeInst  the node instance
   * @param tag       the tag of the instance
   */
  FDataHandle*
  createRealInstance(Dag* dag, DagNode* nodeInst, const FDataTag& tag);

  /**
   * Instanciate the port as a void port
   * @param tag     the tag of the instance
   */
  FDataHandle*
  createVoidInstance(const FDataTag& tag);

  /**
   * Set as a constant
   * @param dataHdl  the data handle that contains the constant
   */
  void
  setAsConstant(FDataHandle* dataHdl);

  /**
   * Checks if the port did not send any data and if yes set the dataTotalNb
   * of connected ports to O
   */
  void
  checkIfEmptyOutput();

  /**
   * Get data IDs for data with value or verify existing data ID
   * @param MA        the execution agent that manages the data
   */
  void
  uploadAllData(MasterAgent_var& MA);

  /**
   * Get data IDs for all data in the tree and update value of leaves
   */
  void
  downloadAllData();

  /**
   * Set the data boundaries (ie cardinal & last flags) when the data set is complete
   * (used for data sources only)
   */
  void
  updateAllDataCardinal();

  /**
   * Write all data contained in the buffer as XML code
   * @param output    the output stream
   */
  void
  writeAllDataAsXML(std::ostream& output);

  /**
   * Write all data contained in the buffer in list format (for display)
   * @param output    the output stream
   */
  void
  writeAllDataAsList(std::ostream& output);

  /**
   * Get the data handle of the buffer
   */
  FDataHandle*
  getBufferRootDH();

  /**
   * Get the container containing all the data in the buffer
   * (assumes downloadAllData has been called before)
   * @return  the data ID of the container
   * @exception WfDataHandleException(eINVALID_ADAPT) if buffer is not completed
   */
  const std::string&
  getBufferContainerID();

  /**
   * Free the persistent data
   */
  virtual void
  freePersistentData(MasterAgent_var& MA);

protected:
  void
  setPendingDataTransfer(FDataHandle* dataHdl, FNodeInPort* inPort)
    throw(WfDataHandleException);

  void
  checkTotalDataNb(FNodeInPort *inPort);

  /**
   * The list of in ports connected
   */
  std::list<FNodeInPort*>  myConnectedPorts;

  /**
   * The root data handle used to store all generated data handles
   */
  FDataHandle myBuffer;
};

/*****************************************************************************/
/*                            FNodeInPort                                    */
/*****************************************************************************/

class FNodeInPort : public virtual FNodePort {
public:
  friend class PortInputIterator;

  FNodeInPort(WfNode * parent, const std::string& _id,
              WfPort::WfPortType _portType,
              WfCst::WfDataType _type, unsigned int _depth,
              unsigned int _ind);

  virtual ~FNodeInPort();

  /**
   * Set the port as requiring a value for all received DH
   * (used for control structures: if, loops)
   */
  virtual void
  setValueRequired();

  virtual bool
  isValueRequired();

  /**
   * addData
   * Add a new data item to be used for instanciation
   * @param dataHdl   the data Handle
   * @exception WfDataHandleException(eCARD_UNDEF) if cardinal missing
   * @exception WfDataHandleException(eBAD_STRUCT) if DH cannot be added
   */
  virtual void
  addData(FDataHandle* dataHdl);

  /**
   * Get the tag level of data currently stored in the input queue
   * (cannot be called before at least one item is stored)
   * @return  tag level
   */
  unsigned int
  getDataLevel();

  /**
   * Set the total nb of data items
   * This information is used by input operators to determine when
   * the instanciation is finished.
   * Called by connected OUT port when total can be determined.
   */
  void
  setTotalDataNb(unsigned int total);

  /**
   * Set as a constant
   * @param dataHdl  the data handle that contains the constant
   */
  void
  setAsConstant(FDataHandle* dataHdl);

  /**
   * Instanciate the port as a real input port
   * @param dag       the dag being instanciated
   * @param nodeInst  the node instance - NOT NULL
   * @param dataHdl   the data Handle to be used (as source) - NOT NULL
   */
  void
  createRealInstance(Dag* dag, DagNode* nodeInst, FDataHandle* dataHdl);

protected:
  std::map<FDataTag, FDataHandle*> myQueue;

  /**
   * The total nb of data items that the node will receive
   * This information is used by input operators to determine when
   * the instanciation is finished.
   */
  unsigned int dataTotalNb;

  /**
   * Flag to check if total has been defined
   */
  bool totalDef;

  /**
   * Flag for value required
   */
  bool valueRequired;
};


/*****************************************************************************/
/*                          FNodeInOutPort                                   */
/*****************************************************************************/

class FNodeInOutPort : public FNodeInPort, public FNodeOutPort {
public:
  FNodeInOutPort(WfNode* parent, const std::string& _id,
                 WfCst::WfDataType _type, unsigned int _depth,
                 unsigned int _ind);

  /**
   * Method to setup ports connection on each side
   */
  virtual void
  connectToPort(WfPort* remPort);

  /**
   * Instanciate the port as a real input/output port
   * @param dag       the dag being instanciated
   * @param nodeInst  the node instance - NOT NULL
   * @param tag       the tag of the instance
   * @param dataHdl   the data Handle to be used (as source) - NOT NULL
   */
  FDataHandle*
  createRealInstance(Dag* dag, DagNode* nodeInst,
                     const FDataTag& tag, FDataHandle* dataHdl);
};

/*****************************************************************************/
/*                          FNodeParamPort                                   */
/*****************************************************************************/
// notes:
// 1/ a param port always has depth = 0
// 2/ a param port must be defined

class FNodeParamPort : public FNodeInPort {
public:
  FNodeParamPort(WfNode * parent, const std::string& _id,
                 WfCst::WfDataType _type, unsigned int _ind);

  virtual ~FNodeParamPort();
};

/*****************************************************************************/
/*                           FNodePortMap                                    */
/*****************************************************************************/

class FNodePortMap {
public:
  FNodePortMap();

  /**
   * Map an out port of a node to an in port of the same node
   * (will replace existing mapping for outPort if any)
   */
  void
  mapPorts(FNodeOutPort* outPort, FNodeInPort* inPort);

  /**
   * Map an out port of a node to the VOID value
   * (will replace existing mapping for outPort if any)
   */
  void
  mapPortToVoid(FNodeOutPort* outPort);

  /**
   * Apply mapping to a given dataline
   */
  void
  applyMap(const FDataTag& tag, const std::vector<FDataHandle*>& dataLine);

private:
  std::map<FNodeOutPort*, FNodeInPort*> myPortMap;
};


#endif  // _FNODEPORT_HH_
