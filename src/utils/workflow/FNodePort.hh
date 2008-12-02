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
 * Revision 1.1  2008/12/02 10:07:07  bisnard
 * new classes for functional workflow instanciation and execution
 *
 */


#ifndef _FNODEPORT_HH_
#define _FNODEPORT_HH_

#include "WfPort.hh"
#include "FDataHandle.hh"
#include "DagNode.hh"

class FNodeInPort;

/*****************************************************************************/
/*                         FNodePort (Abstract)                              */
/*****************************************************************************/

class FNodePort : public WfPort {

  public:
    FNodePort(Node * parent, const string& _id, WfPort::WfPortType _portType,
              WfCst::WfDataType _type, unsigned int _depth, unsigned int _ind);
    virtual ~FNodePort();


}; // end class FNodePort

/*****************************************************************************/
/*                           FNodeOutPort                                    */
/*****************************************************************************/

class FNodeOutPort : public FNodePort {

  public:
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
     * The list of in ports connected
     */
    list<FNodeInPort*>  myConnectedPorts;

    /**
     * The root data handle used to store all generated data handles
     */
    FDataHandle myBuffer;

    /**
     * The waiting list (map instance tag => in port)
     * Used when some data handles cannot be directly transferred to in ports
     * during instanciation because their cardinal is not known statically
     * This map is used after node instance has been executed. The functional
     * node uses this map to re-submit the data handle to the in ports (the data
     * handle can be found in the buffer using the instance tag)
     */
    map<FDataTag, FNodeInPort*> myWaitingPorts;

}; // end class FNodeOutPort

/*****************************************************************************/
/*                            FNodeInPort                                    */
/*****************************************************************************/

class FNodeInPort : public FNodePort {

  public:
    FNodeInPort(Node * parent, const string& _id,
                WfCst::WfDataType _type, unsigned int _depth, unsigned int _ind);
    virtual ~FNodeInPort();

    /**
     * Add a new data item to be used for instanciation
     * @param dataHdl   the data Handle
     * @param dataCard  a list of (integer | 'x') = cardinal of data
     * @return true if the data cannot be added due to missing cardinal
     */
    bool
    addData(FDataHandle* dataHdl, const list<string>& dataCard);

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

    /**
     * Get an iterator on the queue of datahandles
     */
    map<FDataTag, FDataHandle*>::iterator
    begin();

    map<FDataTag, FDataHandle*>::iterator
    end();

    /**
     * Clear the queue
     */
    void
    clearQueue();

    /**
     * Remove elements from the queue
     */
    void
    clearQueue(map<FDataTag, FDataHandle*>::iterator start,
               map<FDataTag, FDataHandle*>::iterator end);

  protected:

    map<FDataTag, FDataHandle*> myQueue;

}; // end class FNodeInPort


#endif // _FNODEPORT_HH_
