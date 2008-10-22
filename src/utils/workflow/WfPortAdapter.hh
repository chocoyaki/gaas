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
 * Revision 1.6  2008/10/22 09:29:00  bisnard
 * replaced uint by standard type
 *
 * Revision 1.5  2008/10/14 13:31:01  bisnard
 * new class structure for dags (DagNode,DagNodePort)
 *
 * Revision 1.4  2008/10/02 07:35:10  bisnard
 * new constants definitions (matrix order and port type)
 *
 * Revision 1.3  2008/09/30 15:32:53  bisnard
 * - using simple port id instead of composite ones
 * - dag nodes linking refactoring
 * - prevNodes and nextNodes data structures modified
 * - prevNodes initialization by Node::setNodePredecessors
 *
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
class Node;
class NodeSet;
class WfPort;

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
     * @param node      the node containing the port linked to this adapter
     * @param nodeSet   the dag that contains the port
     */
    virtual bool
        setNodePrecedence(Node* node, NodeSet* nodeSet) = 0;

    /**
     * Node linking (used for node execution - step 1)
     * @param port      the port that contains the current adapter
     * @param nodeSet   the dag that contains the port
     */
    virtual void
        connectPorts(WfPort* port, NodeSet* nodeSet) = 0;

    /**
     * Data ID retrieval (used for node execution - step 2)
     * REQUIRED: Node linking required before calling this method
     * REQUIRED: to be used only with adapters of DagNode ports
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

    bool
        setNodePrecedence(Node* node, NodeSet* nodeSet);
    void
        connectPorts(WfPort* port, NodeSet* nodeSet);
    const string&
        getSourceDataID();

  protected:
    const string&
        getPortName() const;
    const string&
        getNodeName() const;
    const string&
        getDagName() const;
    unsigned int
        depth();
    const list<unsigned int>&
        getElementIndexes();

  private:
    string      portName;
    string      nodeName;
    string      dagName;
    string      dataID;
    list<unsigned int>  eltIdxList;
    Node *      nodePtr;
    WfPort *    portPtr;

}; // end class WfSimplePortAdapter

class WfMultiplePortAdapter : public WfPortAdapter {

  public:
    /**
     * Constructor for a multiple port
     * @param strRef the complete port reference
     */
    WfMultiplePortAdapter(const string& strRef);

    bool
        setNodePrecedence(Node* node, NodeSet* nodeSet);
    void
        connectPorts(WfPort* port, NodeSet* nodeSet);
    const string&
        getSourceDataID();

  protected:

  private:
    string      strRef;
    string      containerID;
    list<WfPortAdapter*>  adapters;

    static string errorID;

}; // end class WfMultiplePortAdapter

#endif
