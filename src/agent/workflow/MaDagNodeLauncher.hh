/**
 * @file MaDagNodeLauncher.hh
 *
 * @brief  The specialized launcher class used for dagNode scheduling
 *
 * @author  Benjamin Isnard (Benjamin.Isnard@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENCE|
 */


#ifndef _MADAGNODELAUNCHER_
#define _MADAGNODELAUNCHER_

#include "DagNodeLauncher.hh"
#include "MultiWfScheduler.hh"
#include "CltMan.hh"

namespace madag {
class MaDagNodeLauncher : public DagNodeLauncher {
public:
  MaDagNodeLauncher(DagNode * parent,
                    MultiWfScheduler * scheduler,
                    CltMan_ptr clientMgr);

  virtual void
  execNode();

  virtual void
  finishNode();

protected:
  MultiWfScheduler *myScheduler;
  CltMan_ptr myCltMgr;
};
}

#endif  // ifndef _MADAGNODELAUNCHER_
