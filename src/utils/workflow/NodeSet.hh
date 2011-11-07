/**
* @file  NodeSet.hh
* 
* @brief  Absctract class common parent to dag and workflows 
* 
* @author  Benjamin ISNARD (Benjamin.Isnard@ens-lyon.fr)
* 
* @section Licence
*   |LICENCE|                                                                
*/


#ifndef _NODESET_
#define _NODESET_

#include <string>
#include "WfUtils.hh"
#include "WfNode.hh"

/*****************************************************************************/
/*                    CLASS NodeSet (ABSTRACT)                               */
/*****************************************************************************/

/**
 * NodeSet class
 *
 * Used by the DagWfParser to create either a dag or workflow
 * Used by Node class to manage relationships between nodes (this
 * applies to both dags or workflows)
 */

class NodeSet {
public:
  virtual ~NodeSet() {
  }

  /**
   * Get a node from the nodeset
   * @param nodeId  the identifier (string) of the node
   * @return pointer to node (does not return NULL)
   */
  virtual WfNode*
  getNode(const std::string& nodeId) throw(WfStructException) = 0;

  /**
   * Check that the relationships between nodes are correct
   * @param contextNodeSet the nodeSet used to find nodes referenced in the
   * current nodeSet (can be the current nodeSet itself)
   */
  virtual void
  checkPrec(NodeSet* contextNodeSet) throw(WfStructException) = 0;
};

#endif  // _NODESET_
