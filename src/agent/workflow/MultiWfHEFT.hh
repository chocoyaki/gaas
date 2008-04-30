/****************************************************************************/
/* Another multi-workflow scheduler                                         */
/* This scheduler is based on HEFT algorithm                                */
/*                                                                          */
/* Author(s):                                                               */
/* - Benjamin ISNARD (benjamin.isnard@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/

/* $Id$
 * $Log$
 * Revision 1.1  2008/04/30 07:37:01  bisnard
 * use relative timestamps for estimated and real completion time
 * make MultiWfScheduler abstract and add HEFT MultiWf scheduler
 *
 */

#ifndef _MULTIWFHEFT_HH_
#define _MULTIWFHEFT_HH_


#include "workflow/Dag.hh"
#include "MultiWfScheduler.hh"

using namespace std;

namespace madag {

  class MultiWfHEFT : public MultiWfScheduler {

  public:

    MultiWfHEFT(MaDag_impl* maDag);
    virtual ~MultiWfHEFT();

    /**
     * Updates scheduler when a node has been executed
     */
    virtual void
        handlerNodeDone(Node * node);

  }; // end class MultiWfHEFT

} // end namespace madag

#endif // _MULTIWFHEFT_HH_
