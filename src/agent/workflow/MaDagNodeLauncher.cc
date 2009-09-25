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

#include "MaDagNodeLauncher.hh"
#include "debug.hh"
#include "Dag.hh"
#include "DagNode.hh"

using namespace madag;

MaDagNodeLauncher::MaDagNodeLauncher(DagNode * parent,
                                     MultiWfScheduler *  scheduler,
                                     CltMan_ptr clientMgr)
  : DagNodeLauncher(parent, scheduler), myScheduler(scheduler),
    myCltMgr(clientMgr)
{
}

void
MaDagNodeLauncher::execNode(bool& successful)
{
  Dag *  dag    = myNode->getDag();
  string dagId  = dag->getId();
  string nodeId = myNode->getId();
  string nodePx = "[" + myNode->getCompleteId() + "] : ";
  bool clientFailure = false;
  CORBA::Long res;

  try
  {
    if (isSedDefined)
    {
      TRACE_TEXT (TRACE_ALL_STEPS, nodePx << "call client (sed defined) - request #"
          << this->myReqID << endl);
      res = myCltMgr->execNodeOnSed(nodeId.c_str(),
                                    dagId.c_str(),
                                    myChosenServer,
                                    (CORBA::ULong) myReqID,
                                    myEstimVect);
    } else {
      TRACE_TEXT (TRACE_ALL_STEPS, nodePx << "call client (sed not defined)" << endl);
      res = myCltMgr->execNode(nodeId.c_str(),
                                dagId.c_str());
    }
    TRACE_TEXT (TRACE_MAIN_STEPS, nodePx << "call client DONE" << endl);
  } catch (CORBA::COMM_FAILURE& e) {
    WARNING(nodePx << "Client call had connection problems" << endl);
    clientFailure = true;
  } catch (CORBA::SystemException& e) {
    WARNING(nodePx << "Client call got a CORBA " << e._name() << " exception ("
        << e.NP_minorString() << ")" << endl);
    clientFailure = true;
  } catch (...) {
    WARNING(nodePx << "Client call got unknown exception!" << endl);
    clientFailure = true;
  }

  successful = !clientFailure && !res;
}

void
MaDagNodeLauncher::finishNode()
{
  myScheduler->wakeUp(false, myNode);  // scheduler main thread will join my thread
}
