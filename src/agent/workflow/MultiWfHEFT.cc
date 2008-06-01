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
 * Revision 1.2  2008/06/01 14:06:57  rbolze
 * replace most ot the cout by adapted function from debug.cc
 * there are some left ...
 *
 * Revision 1.1  2008/04/30 07:37:01  bisnard
 * use relative timestamps for estimated and real completion time
 * make MultiWfScheduler abstract and add HEFT MultiWf scheduler
 *
 */


#include "MultiWfHEFT.hh"


using namespace madag;

/****************************************************************************/
/*                         PUBLIC METHODS                                   */
/****************************************************************************/

MultiWfHEFT::MultiWfHEFT(MaDag_impl* maDag) : MultiWfScheduler(maDag) {
  this->execQueue = new PriorityNodeQueue;
  TRACE_TEXT(TRACE_MAIN_STEPS,"Using HEFT multi-workflow scheduler" << endl);
}

MultiWfHEFT::~MultiWfHEFT() {
  delete dynamic_cast<PriorityNodeQueue *>(execQueue);
}

void
MultiWfHEFT::handlerNodeDone(Node * node) {
  // does nothing
}
