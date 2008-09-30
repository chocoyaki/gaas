/****************************************************************************/
/* Port adapter classes used to split or merge container content            */
/*                                                                          */
/*                                                                          */
/* Author(s):                                                               */
/* - Benjamin Isnard (benjamin.isnard@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.2  2008/09/30 09:23:29  bisnard
 * removed diet profile initialization from DagWfParser and replaced by node methods initProfileSubmit and initProfileExec
 *
 * Revision 1.1  2008/09/19 13:16:07  bisnard
 * New class to manage containers split/merge in workflows
 *
 */

#ifndef _WFPORTADAPTER_HH_
#define _WFPORTADAPTER_HH_

#include <list>

using namespace std;
class Dag;
class Node;
class WfInPort;
class WfOutPort;

class WfPortAdapter {

  public:
    /**
     * Static factory method for adapters
     * @param strRef  the complete port reference (ie content of source
     *                attribute of the port, for ex: node1#port0[2][3])
     */
    static WfPortAdapter*
        createAdapter(const string& strRef);

    /**
     * Node precedence analysis (used for dag scheduling)
     * @param node  the node containing the port linked to this adapter
     * @param dag   the dag that contains the port
     */
    virtual void
        setNodePredecessors(Node* node, Dag* dag) = 0;

    /**
     * Node linking (used for node execution - step 1)
     * @param port  the port that contains the current adapter
     * @param dag   the dag that contains the port
     */
    virtual void
        setPortDataLinks(WfInPort* inPort, Dag* dag) = 0;

    /**
     * Data ID retrieval (used for node execution - step 2)
     * (Node linking required before calling this method)
     * @return a string containing the data ID of the toplevel data item
     */
    virtual const string&
        getSourceDataID() = 0;

};

class WfSimplePortAdapter : public WfPortAdapter {

  public:
    /**
     * Constructor for a simple port
     * @param strRef the complete port reference
     */
    WfSimplePortAdapter(const string& strRef);

    void
        setNodePredecessors(Node* node, Dag* dag);
    void
        setPortDataLinks(WfInPort* inPort, Dag* dag);
    const string&
        getSourceDataID();

  protected:
    const string&
        getPortName() const;
    const string&
        getNodeName() const;
    const string&
        getDagName() const;
    uint
        depth();
    const list<uint>&
        getElementIndexes();

  private:
    string      portName;
    string      nodeName;
    string      dagName;
    string      dataID;
    list<uint>  eltIdxList;
    Node *      nodePtr;
    WfOutPort * portPtr;

}; // end class WfSimplePortAdapter

class WfMultiplePortAdapter : public WfPortAdapter {

  public:
    /**
     * Constructor for a multiple port
     * @param strRef the complete port reference
     */
    WfMultiplePortAdapter(const string& strRef);

    void
        setNodePredecessors(Node* node, Dag* dag);
    void
        setPortDataLinks(WfInPort* inPort, Dag* dag);
    const string&
        getSourceDataID();

  protected:

  private:
    string      strRef;
    string      containerID;
    list<WfPortAdapter*>  adapters;

}; // end class WfMultiplePortAdapter

#endif
