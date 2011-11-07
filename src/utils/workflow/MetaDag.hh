/**
* @file  MetaDagr.hh
* 
* @brief  A structure containing multiple dags having inter-relationships
* 
* @author  Benjamin ISNARD (Benjamin.Isnard@ens-lyon.fr)
* 
* @section Licence
*   |LICENCE|                                                                
*/


#ifndef _METADAG_HH_
#define _METADAG_HH_

#include <map>
#include <string>
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
  explicit MetaDag(const std::string& id);

  virtual ~MetaDag();

  /**
   * Get the Id
   */
  const std::string&
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
   * @param dagId the dag identifier (std::string)
   */
  Dag *
  getDag(const std::string& dagId) throw(WfStructException);

  /**
   * Remove a dag from the metaDag
   * @param dagId the dag identifier (std::string)
   */
  void
  removeDag(const std::string& dagId) throw(WfStructException);

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
  getNode(const std::string& nodeId) throw(WfStructException);

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
  std::string myId;

  /**
   * Map containing all the dag refs
   */
  std::map<std::string, Dag*> myDags;

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

  void
  lock();

  void
  unlock();

  /**
   * Not applicable to this class
   */
  virtual void
  checkPrec(NodeSet* contextNodeSet) throw(WfStructException);
};

#endif   /* not defined _METADAG_HH. */
