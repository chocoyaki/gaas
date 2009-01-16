/****************************************************************************/
/* Abstract interface for dag schedulers to handle execution events         */
/*                                                                          */
/* Author(s):                                                               */
/* - Benjamin ISNARD (benjamin.isnard@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.1  2009/01/16 13:53:09  bisnard
 * new abstract class to simplify dag event handling
 *
 */

#ifndef _DAG_SCHEDULER_HH_
#define _DAG_SCHEDULER_HH_

class Dag;
class DagNode;

class DagScheduler {

  public:

    /**
     * Handle DAG DONE event
     * This event is triggered when all the nodes of the dag have been
     * completed without any error
     * @param dag the dag reference
     */
    virtual void
        handlerDagDone(Dag *dag) = 0;

    /**
     * Handle NODE DONE event
     * This event is triggered when one dag node has been completed
     * without any error
     * @param node  the dag node reference
     */
    virtual void
        handlerNodeDone(DagNode *node) = 0;

    /**
     * Get the current timestamp provided by the scheduler's clock
     * @return double value of the timestamp
     */
    virtual double
        getRelCurrTime() = 0;
};

#endif
