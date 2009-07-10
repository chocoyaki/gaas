/****************************************************************************/
/* The class representing the ports of nodes of a functional workflow       */
/*                                                                          */
/* Author(s):                                                               */
/* - Benjamin ISNARD (benjamin.isnard@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.12  2009/07/10 12:52:08  bisnard
 * standardized FNodeInPort constructor
 *
 * Revision 1.11  2009/06/15 12:11:13  bisnard
 * use new XML Parser (SAX) for data source file
 * use new class WfValueAdapter to avoid data duplication
 * use new method FNodeOutPort::storeData
 * changed method to compute total nb of data items
 *
 * Revision 1.10  2009/05/27 08:49:43  bisnard
 * - modified condition output: new IF_THEN and IF_ELSE port types
 * - implemented MERGE and FILTER workflow nodes
 *
 * Revision 1.9  2009/05/15 11:10:20  bisnard
 * release for workflow conditional structure (if)
 *
 * Revision 1.8  2009/04/17 09:04:07  bisnard
 * initial version for conditional nodes in functional workflows
 *
 * Revision 1.7  2009/04/09 09:56:20  bisnard
 * refactoring due to new class FActivityNode
 *
 * Revision 1.6  2009/04/08 09:34:56  bisnard
 * pending nodes mgmt moved to FWorkflow class
 * FWorkflow and FNode state graph revisited
 * FNodePort instanciation refactoring
 *
 * Revision 1.5  2009/02/24 14:01:05  bisnard
 * added dynamic parameter mgmt for wf processors
 *
 * Revision 1.4  2009/02/06 14:55:08  bisnard
 * setup exceptions
 *
 * Revision 1.3  2009/01/16 13:54:50  bisnard
 * new version of the dag instanciator with iteration strategies for nodes with multiple input ports
 *
 * Revision 1.2  2008/12/09 12:15:59  bisnard
 * pending instanciation handling (uses dag outputs for instanciation
 * of a functional wf)
 *
 * Revision 1.1  2008/12/02 10:07:07  bisnard
 * new classes for functional workflow instanciation and execution
 *
 */


#ifndef _FNODEPORT_HH_
#define _FNODEPORT_HH_

#include "WfPort.hh"
#include "FDataHandle.hh"
#include "DagNode.hh"
#include "DagNodePort.hh"

class FNode;
class FNodeInPort;

/*****************************************************************************/
/*                         FNodePort (Abstract)                              */
/*****************************************************************************/

//NOTE this class could be removed (no real common point btw FNodeInPort
// and FNodeOutPort)

class FNodePort : public WfPort {

  public:
    FNodePort(WfNode * parent, const string& _id, WfPort::WfPortType _portType,
              WfCst::WfDataType _type, unsigned int _depth, unsigned int _ind);
    virtual ~FNodePort();

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

}; // end class FNodePort

/*****************************************************************************/
/*                           FNodeOutPort                                    */
/*****************************************************************************/

class FNodeOutPort : public FNodePort {

  public:

    FNodeOutPort(WfNode * parent, const string& _id, WfPort::WfPortType _portType,
                 WfCst::WfDataType _type, unsigned int _depth, unsigned int _ind);
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
        storeData(FDataHandle* dataHdl)
        throw (WfDataHandleException);

    /**
     * Method to send a data handle to the connected ports
     */
    virtual void
        sendData(FDataHandle* dataHdl)
        throw (WfDataHandleException);

    /**
     * Method to re-send a data handle to a given connected port when the first
     * attempt (sendData) was unsuccessful and therefore data transfer was set
     * as pending
     */
    virtual void
        reSendData(FDataHandle* dataHdl, FNodeInPort* inPort)
        throw (WfDataException, WfDataHandleException);

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
     * Set the data boundaries (ie cardinal & last flags) when the data set is complete
     * (used for data sources only)
     */
    void
        updateDataTree();

    /**
     * Send all the data contained in the buffer
     */
    void
        sendAllData() throw (WfDataHandleException);

  protected:

    void
    setPendingDataTransfer(FDataHandle* dataHdl,
                           FNodeInPort* inPort);

    void
    checkTotalDataNb(FNodeInPort *inPort);

    /**
     * The list of in ports connected
     */
    list<FNodeInPort*>  myConnectedPorts;

    /**
     * The root data handle used to store all generated data handles
     */
    FDataHandle myBuffer;

    /**
     * The nb of childs table
     * indexed by the level of the buffer tree (from O to its max level)
     */
//     vector<unsigned int> myBufferChildNbTable;

}; // end class FNodeOutPort

/*****************************************************************************/
/*                            FNodeInPort                                    */
/*****************************************************************************/

class FNodeInPort : public FNodePort {

  public:

    friend class PortInputIterator;

    FNodeInPort(WfNode * parent, const string& _id, WfPort::WfPortType _portType,
                WfCst::WfDataType _type, unsigned int _depth, unsigned int _ind);
    virtual ~FNodeInPort();

    /**
     * Set the port as requiring a value for all received DH
     * (used for control structures: if, loops)
     */
    void
        setValueRequired();

    bool
        isValueRequired();

    /**
     * addData
     * Add a new data item to be used for instanciation
     * @param dataHdl   the data Handle
     * @exception WfDataHandleException(eCARD_UNDEF) if cardinal missing
     * @exception WfDataHandleException(eBAD_STRUCT) if DH cannot be added
     */
    virtual void
        addData(FDataHandle* dataHdl)
        throw (WfDataHandleException, WfDataException);

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
    virtual void
        createRealInstance(Dag* dag, DagNode* nodeInst, FDataHandle* dataHdl);

    /**
     * Display the port current data
     * @param output      output stream
     */
    void
        displayData(ostream& output);

  protected:

    map<FDataTag, FDataHandle*> myQueue;

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

}; // end class FNodeInPort

/*****************************************************************************/
/*                          FNodeParamPort                                   */
/*****************************************************************************/
// notes:
// 1/ a param port always has depth = 0
// 2/ a param port must be defined

class FNodeParamPort : public FNodeInPort {

  public:

    FNodeParamPort(WfNode * parent, const string& _id,
                   WfCst::WfDataType _type, unsigned int _ind);
    virtual ~FNodeParamPort();

  private:
    // not applicable
    virtual void
        createRealInstance(Dag* dag, DagNode* nodeInst, FDataHandle* dataHdl);

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
        applyMap(const FDataTag& tag, const vector<FDataHandle*>& dataLine);

  private:

    map<FNodeOutPort*,FNodeInPort*> myPortMap;

};


#endif // _FNODEPORT_HH_
