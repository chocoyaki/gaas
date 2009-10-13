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
 * Revision 1.2  2009/10/13 15:08:54  bisnard
 * added Dagda exceptions handling
 *
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
  } catch(Dagda::ReadError& e) {
    WARNING(traceHeader << "Data read error");
    successful = false;
  } catch(Dagda::InvalidPathName& e) {
    WARNING(traceHeader << "Invalid path name");
    successful = false;
  } catch(Dagda::UnreachableFile& e) {
    WARNING(traceHeader << "Unreachable file");
    successful = false;
  } catch(Dagda::PermissionDenied& e) {
    WARNING(traceHeader << "Permission denied");
    successful = false;
  } catch(Dagda::UnavailableData& e) {
    WARNING(traceHeader << "Unavailable data");
    successful = false;
  } catch(CORBA::SystemException& e) {
    WARNING(traceHeader << "Got a CORBA " << e._name() << " exception ("
                        << e.NP_minorString() << ")\n") ;
    successful = false;
  }
}
