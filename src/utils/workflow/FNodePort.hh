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

class FNodeInPort;

/*****************************************************************************/
/*                         FNodePort (Abstract)                              */
/*****************************************************************************/

class FNodePort : public WfPort {

  public:
    FNodePort(Node * parent, const string& _id, WfPort::WfPortType _portType,
              WfCst::WfDataType _type, unsigned int _depth, unsigned int _ind);
    virtual ~FNodePort();

  protected:
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

    friend class FProcNode; // for pending instanciation

    FNodeOutPort(Node * parent, const string& _id,
                 WfCst::WfDataType _type, unsigned int _depth, unsigned int _ind);
    virtual ~FNodeOutPort();

    /**
     * Method to setup ports connection on each side
     */
    virtual void
    connectToPort(WfPort* remPort);

    /**
     * Instanciate the port as a real output port (used for processors)
     * @param dag       the current dag being instanciated
     * @param nodeInst  the node instance (parent of the port to create)
     * @param tag       the tag of the instance
     */
    void
    instanciate(Dag* dag, DagNode* nodeInst, const FDataTag& tag);

    /**
     * Instanciate the port only as a new output data (used for sources)
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

    /**
     * Methods used by the instanciate methods to submit a data item
     * to an input port
     * (also used by FProcNode for pending data)
     */

    bool
    addDataToInPort(FNodeInPort *inPort,
                    FDataHandle *dataHdl,
                    const list<string>& dataCard);
    bool
    addDataToInPort(FNodeInPort *inPort,
                    FDataHandle *dataHdl,
                    DagNodeOutPort * dagOutPort);
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

    FNodeInPort(Node * parent, const string& _id,
                WfCst::WfDataType _type, unsigned int _depth, unsigned int _ind);
    virtual ~FNodeInPort();

    /**
     * Static addData (before node execution)
     * Add a new data item to be used for instanciation
     * @param dataHdl   the data Handle
     * @param dataCard  a list of (integer | 'x') = cardinal of data
     * @return false if the data cannot be added due to missing cardinal
     */
    bool
    addData(FDataHandle* dataHdl, const list<string>& dataCard);

    /**
     * Dynamic addData (after node execution)
     * Add a new data item to be used for instanciation
     * (when data item is available from the specified port)
     * @param dataHdl     the data Handle
     * @param dagOutPort  the port providing the data
     * @return false if the data cannot be added due to missing data ID
     */
    bool
    addData(FDataHandle* dataHdl, DagNodeOutPort* dagOutPort);

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
    void
    instanciate(Dag* dag, DagNode* nodeInst, FDataHandle* dataHdl);

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

}; // end class FNodeInPort


#endif // _FNODEPORT_HH_
