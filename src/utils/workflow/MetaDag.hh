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
 * Revision 1.2  2008/12/09 12:15:59  bisnard
 * pending instanciation handling (uses dag outputs for instanciation
 * of a functional wf)
 *
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
     * Remove a dag from the metaDag
     * @param dag the dag ref
     */
    bool
        removeDag(Dag * dag);

    /**
     * Nb of dags
     */
    int
        getDagNb();

    /**
     * Set the release flag
     * TRUE => destroy metadag when last dag is done
     * FALSE => do not destroy metadag when last dag is done
     */
    void
        setReleaseFlag(bool release);

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

    /**
     * Check if metadag is completed
     * (for destruction)
     */
    bool
        isDone();

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

    /**
     * Release flag
     */
    bool releaseFlag;
};

#endif   /* not defined _METADAG_HH. */
