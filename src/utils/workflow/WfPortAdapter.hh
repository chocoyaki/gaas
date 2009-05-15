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
 * Revision 1.10  2009/05/15 11:10:20  bisnard
 * release for workflow conditional structure (if)
 *
 * Revision 1.9  2009/04/17 09:02:15  bisnard
 * container empty elements (added WfVoidAdapter class)
 *
 * Revision 1.8  2009/02/06 14:54:43  bisnard
 * - setup exceptions
 * - added data type & depth check
 *
 * Revision 1.7  2008/12/02 10:14:51  bisnard
 * modified nodes links mgmt to handle inter-dags links
 *
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
#include "WfPort.hh"
class DagNodeOutPort;

using namespace std;

class WfPortAdapter {

  public:

    virtual ~WfPortAdapter();

    /**
     * Static factory method for adapters
     * @param strRef  the complete port reference (ie content of source
     *                attribute of the port, for ex: node1#port0[2][3])
     */
    static WfPortAdapter*
        createAdapter(const string& strRef);

    /**
     * Node precedence analysis
     * @param node      the node containing the port linked to this adapter
     * @param nodeSet   the node set that contains the linked nodes
     */
    virtual void
        setNodePrecedence(WfNode* node, NodeSet* nodeSet)
        throw (WfStructException) = 0;

    /**
     * Node linking (used for node execution - step 1)
     * Checks the compatibility between both ports (data type and depth)
     * Requires setNodePrecedence to be called before
     * @param port         the port that contains the current adapter
     * @param adapterLevel the level of the current adapter within the port
     */
    virtual void
        connectPorts(WfPort* port, unsigned int adapterLevel)
        throw (WfStructException) = 0;

    /**
     * Returns the string reference (used for generating xml)
     */
    virtual string
        getSourceRef() const = 0;

    /**
     * Data ID retrieval (used for node execution - step 2)
     * REQUIRED: Node linking required before calling this method
     * REQUIRED: to be used only with adapters of DagNode ports
     * @return a string containing the data ID of the toplevel data item
     */
    virtual const string&
        getSourceDataID() = 0;

    /**
     * Cardinal of data retrieval (used for node execution - step 2)
     * REQUIRED: idem as getSourceDataID()
     * REQUIRED: to be used only with adapters pointing to container data
     * @return the size of the container corresponding to the adapter
     */
    virtual unsigned int
        getSourceDataCardinal() = 0;

    /**
     * Data value retrieval and display (used for sink nodes)
     * @param output  the output stream
     */
    virtual void
        displayDataAsList(ostream& output) = 0;

};

class WfSimplePortAdapter : public WfPortAdapter {

  public:
    /**
     * Destructor
     */
    ~WfSimplePortAdapter();

    /**
     * Constructor for a simple port
     * @param strRef the complete port reference
     */
    WfSimplePortAdapter(const string& strRef);

    /**
     * Constructor for a simple port
     * @param port        the port to which the adapter points
     * @param portDagName (optional) the name of dag that contains that port
     *                    (for external links => adds a prefix to the ref)
     */
    WfSimplePortAdapter(WfPort* port, const string& portDagName = "");

    /**
     * Constructor for a simple port
     * @param port        the port to which the adapter points
     * @param indexes     the list of indexes of the element of the port
     * @param portDagName (optional) the name of dag that contains that port
     *                    (for external links => adds a prefix to the ref)
     */
    WfSimplePortAdapter(WfPort* port,
                        const list<unsigned int>& indexes,
                        const string& portDagName = "");

    /**
     * Constructor for a simple port
     * @param parentAdapter the parent of this adapter
     * @param index         the index of the element within the parent cont.
     */
//     WfSimplePortAdapter(WfSimplePortAdapter* parentAdapter,
//                         unsigned int index);

    // virtual base methods

    void
        setNodePrecedence(WfNode* node, NodeSet* nodeSet)
        throw (WfStructException);
    void
        connectPorts(WfPort* port, unsigned int adapterLevel)
        throw (WfStructException);
    string
        getSourceRef() const;

    const string&
        getSourceDataID();

    unsigned int
        getSourceDataCardinal();

    void
        displayDataAsList(ostream& output);

    /**
     * Returns the port ref casted to a dag port
     */
    DagNodeOutPort*
        getSourcePort() const;

  protected:
    const string&
        getPortName() const;
    const string&
        getNodeName() const;
    const string&
        getDagName() const;
    unsigned int
        getDepth() const;
    const list<unsigned int>&
        getElementIndexes();

  private:
    string      portName;
    string      nodeName;
    string      dagName;
    string      dataID;
    list<unsigned int>  eltIdxList;
    WfNode *      nodePtr;
    WfPort *    portPtr;

}; // end class WfSimplePortAdapter

class WfMultiplePortAdapter : public WfPortAdapter {

  public:
    /**
     * Destructor
     */
    ~WfMultiplePortAdapter();

    /**
     * Separator and Parenthesis characters definition
     */
    static string parLeftChar;
    static string parRightChar;
    static string separatorChar;

    /**
     * Constructor for a multiple port
     * @param strRef the complete port reference
     */
    WfMultiplePortAdapter(const string& strRef);

    WfMultiplePortAdapter();

    void
        addSubAdapter(WfPortAdapter* subAdapter);
    void
        setNodePrecedence(WfNode* node, NodeSet* nodeSet)
        throw (WfStructException);
    void
        connectPorts(WfPort* port, unsigned int adapterLevel)
        throw (WfStructException);
    string
        getSourceRef() const;
    const string&
        getSourceDataID();
    unsigned int
        getSourceDataCardinal();
    void
        displayDataAsList(ostream& output);

  protected:

    WfMultiplePortAdapter(const WfMultiplePortAdapter& mpa);

    void parse(const string& strRef, string::size_type& startPos);

  private:
    string      strRef;
    string      containerID;
    list<WfPortAdapter*>  adapters;

    static string errorID;

}; // end class WfMultiplePortAdapter

class WfVoidAdapter : public WfPortAdapter {

  public:

    static string voidRef;    // appears in dag
    static string voidDataID; // appears in data ID field in the profile

    WfVoidAdapter();
    ~WfVoidAdapter();

    // virtual base methods

    void
        setNodePrecedence(WfNode* node, NodeSet* nodeSet)
        throw (WfStructException);
    void
        connectPorts(WfPort* port, unsigned int adapterLevel)
        throw (WfStructException);
    string
        getSourceRef() const;
    const string&
        getSourceDataID();
    unsigned int
        getSourceDataCardinal();
    void
        displayDataAsList(ostream& output);
};

#endif
