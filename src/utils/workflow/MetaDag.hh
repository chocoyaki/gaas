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
 * Revision 1.5  2009/09/25 12:49:11  bisnard
 * avoid deadlocks due to new thread mgmt in DagNodeLauncher
 *
 * Revision 1.4  2009/04/17 08:54:44  bisnard
 * renamed Node class as WfNode
 *
 * Revision 1.3  2009/02/06 14:53:46  bisnard
 * make thread-safe
 *
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

#include "NodeSet.hh"

class Dag;
class DagScheduler;

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
     * Get a dag by ID
     * @param dagId the dag identifier (string)
     */
    Dag *
        getDag(const string& dagId) throw (WfStructException);

    /**
     * Remove a dag from the metaDag
     * @param dagId the dag identifier (string)
     */
    void
        removeDag(const string& dagId) throw (WfStructException);

    /**
     * Nb of dags
     */
    int
        getDagNb();

    /**
     * Set the release flag and returns the metadag completion status
     * TRUE => destroy metadag when last dag is done
     * FALSE => do not destroy metadag when last dag is done
     */
    void
        setReleaseFlag(bool release);
    void
        setReleaseFlag(bool release, bool& isDone);

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
     * @return pointer to node (does not return NULL)
     */
    virtual WfNode*
        getNode(const string& nodeId) throw (WfStructException);

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

    /**
     * Cancel all dags
     */
    void
        cancelAllDags(DagScheduler * scheduler = NULL);

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

    /**
     * Cancelled flag
     */
    bool cancelFlag;

    /**
     * Critical section
     */
    omni_mutex myLock;
    void lock();
    void unlock();

    /**
     * Not applicable to this class
     */
    virtual void
    checkPrec(NodeSet* contextNodeSet) throw (WfStructException);

};

#endif   /* not defined _METADAG_HH. */
