/****************************************************************************/
/* Launcher for client-side dag node                                        */
/*                                                                          */
/* Author(s):                                                               */
/* - Benjamin ISNARD (benjamin.isnard@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.1  2009/09/25 12:36:18  bisnard
 * created new classes DagNodeLauncher & childs
 *
 ****************************************************************************/

#include "CltDagNodeLauncher.hh"
#include "debug.hh"
#include "DIETCall.hh"
#include "Dag.hh"
#include "DagNode.hh"

#define MAX_EXEC_SERVERS 10 // max nb of servers in the agent response

CltDagNodeLauncher::CltDagNodeLauncher(DagNode * parent)
  : DagNodeLauncher(parent)
{
}

void
CltDagNodeLauncher::execNode(bool& successful)
{
  string traceHeader = "[" + myNode->getId() + "] Client Launcher : ";
  TRACE_TEXT (TRACE_ALL_STEPS, traceHeader << "starting... " << endl);
  successful = true;

  // set the reqId in the profile
  if (isSedDefined) {
    myNode->getProfile()->dietReqID = (int) myReqID;
    TRACE_TEXT(TRACE_ALL_STEPS, traceHeader << "setting reqID in profile to #"
      << myReqID << endl);
  }

  // diet call
  try {
    if (!diet_call_common(myNode->getDag()->getExecutionAgent(),
                          myNode->getProfile(),
                          myChosenServer,
                          &myEstimVect,
                          MAX_EXEC_SERVERS)) {
      TRACE_TEXT (TRACE_MAIN_STEPS, traceHeader << "diet call DONE reqID=" <<
                  myNode->getProfile()->dietReqID << endl);
      myNode->storeProfileData();
    }
    else {
        WARNING(traceHeader << "diet call FAILED" << endl);
        successful = false;
    }
  } catch(Dagda::DataNotFound& e) {
    WARNING(traceHeader << "Data not found (ID=" << e.dataID << ")");
    successful = false;
  } catch(CORBA::SystemException& e) {
    WARNING(traceHeader << "Got a CORBA " << e._name() << " exception ("
                        << e.NP_minorString() << ")\n") ;
    successful = false;
  }
}
