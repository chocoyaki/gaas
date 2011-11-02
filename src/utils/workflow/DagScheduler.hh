/**
* @file DagScheduler.hh
* 
* @brief  Abstract interface for dag schedulers to handle execution events 
* 
* @author  - Benjamin ISNARD (Benjamin.Isnard@ens-lyon.fr)
* 
* @section Licence
*   |LICENSE|                                                                
*/
/* $Id$
 * $Log$
 * Revision 1.4  2011/04/05 14:19:08  bdepardo
 * Reverting last commit
 *
 * Revision 1.2  2010/03/08 13:50:48  bisnard
 * handle node ready event (for logging)
 *
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
  virtual ~DagScheduler() {
  }

  /**
   * Handle DAG DONE event
   * This event is triggered when all the nodes of the dag have been
   * completed without any error
   * @param dag the dag reference
   */
  virtual void
  handlerDagDone(Dag *dag) = 0;

  /**
   * Handle NODE READY event
   * This event is triggered when one dag node's dependencies (ie previous
   * nodes) are all completed. This is also triggered if the node has no
   * dependencies when created (input node).
   * @param node  the dag node reference
   */
  virtual void
  handlerNodeReady(DagNode *node) = 0;

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
