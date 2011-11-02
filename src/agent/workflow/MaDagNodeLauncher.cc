/**
* @file MaDagNodeLauncher.cc
* 
* @brief  The specialized launcher class used for dagNode scheduling 
* 
* @author - Benjamin Isnard (Benjamin.Isnard@ens-lyon.fr)
* 
* @section Licence
*   |LICENSE|                                                                
*/
/* $Id$
 * $Log$
 * Revision 1.2  2010/07/20 08:59:36  bisnard
 * Added event generation
 *
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
MaDagNodeLauncher::execNode()
{
  Dag *  dag    = myNode->getDag();
  std::string dagId  = dag->getId();
  std::string nodeId = myNode->getId();
  std::string nodePx = "[" + myNode->getCompleteId() + "] : ";
  bool clientFailure = false;
  CORBA::Long res;

  try
  {
    if (isSeDDefinedFlag)
    {
      TRACE_TEXT(TRACE_ALL_STEPS, nodePx << "call client (sed defined) - request #"
                  << this->myReqID << "\n");
      res = myCltMgr->execNodeOnSed(nodeId.c_str(),
                                    dagId.c_str(),
                                    myChosenServer,
                                    (CORBA::ULong) myReqID,
                                    myEstimVect);
    } else {
      TRACE_TEXT(TRACE_ALL_STEPS, nodePx << "call client (sed not defined)\n");
      res = myCltMgr->execNode(nodeId.c_str(),
                               dagId.c_str());
    }
    TRACE_TEXT(TRACE_MAIN_STEPS, nodePx << "call client DONE\n");
  } catch (CORBA::COMM_FAILURE& e) {
    WARNING(nodePx << "Client call had connection problems\n");
    clientFailure = true;
  } catch (CORBA::SystemException& e) {
    WARNING(nodePx << "Client call got a CORBA " << e._name() << " exception ("
            << e.NP_minorString() << ")\n");
    clientFailure = true;
  } catch (...) {
    WARNING(nodePx << "Client call got unknown exception!\n");
    clientFailure = true;
  }

  isSuccessfulFlag = !clientFailure && !res;
}

void
MaDagNodeLauncher::finishNode()
{
  myScheduler->wakeUp(false, myNode);  // scheduler main thread will join my thread
}
