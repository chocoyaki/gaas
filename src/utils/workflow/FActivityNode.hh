/**
* @file FActivityNode.hh
* 
* @brief  The nodes of a functional workflow that correspond to tasks in the DAG 
* 
* @author   Benjamin ISNARD (Benjamin.Isnard@ens-lyon.fr)
* 
* @section Licence
*   |LICENSE|                                                                
*/

#ifndef _FACTIVITYNODE_HH_
#define _FACTIVITYNODE_HH_

#include <string>
#include <vector>
#include "FNode.hh"

class FActivityNode : public FProcNode {
public:
  FActivityNode(FWorkflow* wf, const std::string& id);

  virtual ~FActivityNode();

  /**
   * Set the maximum nb of instances of this node inside one dag
   * @param maxInst the max nb of instances
   */
  void
  setMaxInstancePerDag(short maxInst);

  /**
   * Set DIET service name
   * @param path the DIET service name
   */
  void
  setDIETServicePath(const std::string& path);

  /**
   * Set DIET estimation option
   * If estimOption == "constant" the instanciator will use the
   * 'est-class' attribute for all generated dag nodes
   * @param estimOption = ('constant'|...)
   */
  void
  setDIETEstimationOption(const std::string& estimOption);

  /**
   * Initialization
   *  - connection to other nodes
   *  - input iterators setup
   */
  virtual void
  initialize();

  /**
   * Specific action to take before starting data processing loop
   */
  virtual void
  initInstanciation();

  /**
   * Test if the instanciation can continue
   * @return true if instanciation must stop
   */
  virtual bool
  instLimitReached();

  /**
   * Instanciation of the activity as a DagNode (inside data processing loop)
   * @param dag ref to the dag that will contain the DagNode
   * @param currTag data tag of the current dataLine
   * @param currDataLine dataLine used to set input data for the DagNode
   */
  virtual void
  createRealInstance(Dag* dag, const FDataTag& currTag,
                     std::vector<FDataHandle*>& currDataLine);

  /**
   * Update the FNode instanciation status after data processing loop
   */
  virtual void
  updateInstanciationStatus();

protected:
  /**
   * The service path
   */
  std::string myPath;

  /**
   * The estimation option
   */
  std::string myEstimOption;

  /**
   * Max number of instances per dag
   */
  short maxInstNb;

private:
  /**
   * Nb of instances created in current dag
   */
  int nbInstances;
};

#endif  // _FACTIVITYNODE_HH_






