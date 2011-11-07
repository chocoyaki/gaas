/**
 * @file  CltDagNodeLauncher.hh
 *
 * @brief   Launcher for client-side dag node
 *
 * @author   Benjamin ISNARD (benjamin.isnard@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENCE|
 */


#ifndef _CLTDAGNODELAUNCHER_
#define _CLTDAGNODELAUNCHER_

#include <string>
#include "DagNodeLauncher.hh"

class CltDagNodeLauncher : public DagNodeLauncher {
public:
  explicit
  CltDagNodeLauncher(DagNode *parent);

  virtual std::string
  toString() const;

  virtual void
  execNode();

  virtual void
  finishNode();
};

#endif /* ifndef _CLTDAGNODELAUNCHER_ */
