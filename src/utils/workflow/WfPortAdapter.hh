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
     *                attribute of the port)
     */
    static WfPortAdapter*
        createAdapter(const string& strRef);

    /**
     * Node precedence analysis (used for dag scheduling)
     * @param node  the node containing the port linked to this adapter
     * @param dag   the dag that contains the linked ports
     */
    virtual void
        setNodePredecessors(Node* node, Dag* dag) = 0;

    /**
     * Node linking (used for node execution - step 1)
     * @param port  the port that contains the current adapter
     * @param dag   the dag that contains the linked ports
     */
    virtual void
        setPortDataLinks(WfInPort* inPort, Dag* dag) = 0;

    /**
     * Data ID retrieval (used for node execution - step 2)
     * (Node linking required before calling this method)
     * @return a char* containing the data ID of the toplevel data item
     */
    virtual char*
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
    char*
        getSourceDataID();

  protected:
    const string&
        getPortName() const;
    const string&
        getNodeName() const;
    uint
        depth();
    const list<uint>&
        getElementIndexes();

  private:
    string      portName;
    string      nodeName;
    list<uint>  eltIdxList;
    WfOutPort*  portPtr;

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
    char*
        getSourceDataID();

  protected:

  private:
    string      strRef;
    list<WfPortAdapter*>  adapters;

}; // end class WfMultiplePortAdapter

#endif
