/****************************************************************************/
/* The specialized launcher class used for dagNode scheduling               */
/*                                                                          */
/*                                                                          */
/* Author(s):                                                               */
/* - Benjamin ISNARD (benjamin.isnard@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.1  2009/09/25 12:35:50  bisnard
 * created new classes DagNodeLauncher & childs
 *
 ****************************************************************************/

#ifndef _MADAGNODELAUNCHER_
#define _MADAGNODELAUNCHER_

#include "DagNodeLauncher.hh"
#include "MultiWfScheduler.hh"
#include "CltMan.hh"

namespace madag {

  class MaDagNodeLauncher : public DagNodeLauncher
  {

    public:
      MaDagNodeLauncher( DagNode * parent,
                         MultiWfScheduler *  scheduler,
                         CltMan_ptr clientMgr);

      virtual void
          execNode(bool& successful);

      virtual void
          finishNode();

    protected:

      CltMan_ptr          myCltMgr;
      MultiWfScheduler *  myScheduler;

  };

}

#endif // ifndef _MADAGNODELAUNCHER_
