/****************************************************************************/
/* The class representing the nodes of a functional workflow that will      */
/* correspond to tasks in the DAG                                           */
/*                                                                          */
/* Author(s):                                                               */
/* - Benjamin ISNARD (benjamin.isnard@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.5  2009/08/26 10:33:09  bisnard
 * implementation of workflow status & restart
 *
 * Revision 1.4  2009/07/23 12:30:10  bisnard
 * new method finalize() for functional wf nodes
 * removed const on currDataLine parameter for instance creation
 *
 * Revision 1.3  2009/05/15 11:10:20  bisnard
 * release for workflow conditional structure (if)
 *
 * Revision 1.2  2009/04/17 09:04:07  bisnard
 * initial version for conditional nodes in functional workflows
 *
 * Revision 1.1  2009/04/09 09:55:27  bisnard
 * new class
 *
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






