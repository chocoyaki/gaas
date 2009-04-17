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

    FNodeOutPort(WfNode * parent, const string& _id,
                 WfCst::WfDataType _type, unsigned int _depth, unsigned int _ind);
    virtual ~FNodeOutPort();

    /**
     * Method to setup ports connection on each side
     */
    virtual void
    connectToPort(WfPort* remPort);

    /**
     * Method to send a data handle to the connected ports
     */
    virtual void
    sendData(FDataHandle* dataHdl)
        throw (WfDataHandleException);

    /**
     * Method to re-send a data handle to a given connected port when the data
     * is available (data exists on the platform)
     */
    virtual void
    reSendData(FDataHandle* dataHdl, FNodeInPort* inPort)
        throw (WfDataException, WfDataHandleException);

    /**
     * Instanciate the port as a real output port (used for tasks)
     * @param dag       the current dag being instanciated
     * @param nodeInst  the node instance (parent of the port to create)
     * @param tag       the tag of the instance
     */
    void
    instanciate(Dag* dag, DagNode* nodeInst, const FDataTag& tag);

    /**
     * Instanciate the port only as a new output data
     * @param dataHdl  the output data handle
     */
    void
    instanciate(FDataHandle* dataHdl);

    /**
     * Set as a constant
     * @param dataHdl  the data handle that contains the constant
     */
    void
    setAsConstant(FDataHandle* dataHdl);

  protected:

    void
    checkTotalDataNb(FNodeInPort *inPort,
                     FDataHandle *dataHdl);

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
    vector<unsigned int> myBufferChildNbTable;

}; // end class FNodeOutPort

/*****************************************************************************/
/*                            FNodeInPort                                    */
/*****************************************************************************/

class FNodeInPort : public FNodePort {

  public:

    friend class PortInputIterator;

    FNodeInPort(WfNode * parent, const string& _id,
                WfCst::WfDataType _type, unsigned int _depth, unsigned int _ind);
    virtual ~FNodeInPort();

    /**
     * Set the port as requiring a value for all received DH
     * (used for control structures: if, loops)
     */
    void
        setValueRequired();

    /**
     * Static addData (before node execution)
     * Add a new data item to be used for instanciation
     * @param dataHdl   the data Handle
     * @param dataCard  a list of (integer | 'x') = cardinal of data
     * @exception WfDataHandleException(eCARD_UNDEF) if cardinal missing
     * @exception WfDataHandleException(eBAD_STRUCT) if DH cannot be added
     */
    virtual void
        addData(FDataHandle* dataHdl, const list<string>& dataCard)
        throw (WfDataHandleException);

    /**
     * Dynamic addData (after node execution)
     * Add a new data item to be used for instanciation
     * (when data item is available from the specified port)
     * @param dataHdl     the data Handle
     * @param dagOutPort  the port providing the data
     */
    virtual void
        addData(FDataHandle* dataHdl, DagNodeOutPort* dagOutPort)
        throw (WfDataException, WfDataHandleException);

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
     * @param nodeInst  the node instance already created
     * @param dataHdl   (may be NULL) the data Handle to be used (as source)
     */
    virtual void
        instanciate(Dag* dag, DagNode* nodeInst, FDataHandle* dataHdl);

    /**
     * Display the port current data
     * @param output  output stream
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

    /**
     * Static addData (before node execution)
     * @param dataHdl   the data Handle
     * @param dataCard  a list of (integer | 'x') = cardinal of data
     * @exception WfDataHandleException(eVALUE_UNDEF) if value missing
     */
    virtual void
        addData(FDataHandle* dataHdl, const list<string>& dataCard)
        throw (WfDataHandleException);

    /**
     * Dynamic addData (after node execution)
     * Add a new data item to be used for instanciation
     * (when data item is available from the specified port)
     * @param dataHdl     the data Handle
     * @param dagOutPort  the port providing the data
     */
    virtual void
        addData(FDataHandle* dataHdl, DagNodeOutPort* dagOutPort)
        throw (WfDataException, WfDataHandleException);

  private:
    // not applicable
    virtual void
        instanciate(Dag* dag, DagNode* nodeInst, FDataHandle* dataHdl);

};

/*****************************************************************************/
/*                           FNodePortMap                                    */
/*****************************************************************************/

class FNodePortMap {

  public:

    FNodePortMap();

    /**
     * Map an out port of a node to an in port of the same node
     */
    void
        mapPorts(FNodeOutPort* outPort, FNodeInPort* inPort);

    /**
     * Map an out port of a node to the VOID value
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
