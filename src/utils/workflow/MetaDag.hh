/****************************************************************************/
/* MetaDag is a structure containing multiple dags having                   */
/* inter-relationships                                                      */
/*                                                                          */
/* Author(s):                                                               */
/* - Benjamin Isnard (benjamin.isnard@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.1  2008/12/02 10:07:07  bisnard
 * new classes for functional workflow instanciation and execution
 *
 */

#ifndef _METADAG_HH_
#define _METADAG_HH_

#include "Dag.hh"


/*****************************************************************************/
/*                            METADAG CLASS                                  */
/*****************************************************************************/

/**
 * MetaDag class
 * Used to manage inter-dag relationships
 */

class MetaDag : public NodeSet {
  public:

    MetaDag(const string& id);

    virtual ~MetaDag();

    /**
     * Get the Id
     */
    const string&
        getId();

    /**
     * Add a dag to the metaDag
     * (the dag is supposed not yet executed)
     * @param dag the dag ref
     */
    void
        addDag(Dag * dag);

    /**
     * Set the current dag used when parsing node references
     * (node references without a dag prefix are found using this default dag)
     * @param dag the default dag ref
     */
    void
        setCurrentDag(Dag * dag);

    /**
     * Search a node reference among the nodes of the metadag's dags
     * @param nodeId  the node reference ('nodeId' or 'dagId:nodeId')
     */
    virtual Node*
        getNode(const string& nodeId);

    /**
     * NOT APPLICABLE
     */
    virtual void
        checkPrec(NodeSet* contextNodeSet) throw (WfStructException);

    /**
     * Manages dag termination
     * (when dag terminates the scheduler calls this handler and does NOT delete dag)
     * @param dag the ref of the terminated dag
     */
    virtual void
        handlerDagDone(Dag * dag);

  protected:

    /**
     * Metadag ID
     */
    string myId;

    /**
     * Map containing all the dag refs
     */
    map<string,Dag*> myDags;

    /**
     * Pointer to the current dag (default dag used when parsing refs to nodes)
     */
    Dag * currDag;

    /**
     * Counter of not finished dags
     */
    int dagTodoCount;
};

#endif   /* not defined _METADAG_HH. */
