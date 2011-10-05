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
 * Revision 1.7  2010/12/17 09:48:00  kcoulomb
 * * Set diet to use the new log with forwarders
 * * Fix a CoRI problem
 * * Add library version remove DTM flag from ccmake because deprecated
 *
 * Revision 1.6  2010/07/26 14:27:49  bisnard
 * fixed bug with undefined sed ref
 *
 * Revision 1.5  2010/07/20 09:22:33  bisnard
 * changes for dietForwarder
 *
 * Revision 1.4  2010/07/12 16:14:12  glemahec
 * DIET 2.5 beta 1 - Use the new ORB manager and allow the use of SSH-forwarders for all DIET CORBA objects
 *
 * Revision 1.3  2009/10/23 13:59:24  bisnard
 * replaced \n by std::endl
 *
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
#include "EventTypes.hh"
#include "ORBMgr.hh"

using namespace events;

#define MAX_EXEC_SERVERS 10 // max nb of servers in the agent response

CltDagNodeLauncher::CltDagNodeLauncher(DagNode * parent)
  : DagNodeLauncher(parent)
{
}

string 
CltDagNodeLauncher::toString() const
{
  return "Client " + DagNodeLauncher::toString();
}


void
CltDagNodeLauncher::execNode()
{
  string traceHeader = "[" + myNode->getId() + "] Client Launcher : ";
  TRACE_TEXT (TRACE_ALL_STEPS, traceHeader << "starting... " << endl);
  
  sendEventFrom<DagNodeLauncher, DagNode::START>(this, "Start client launcher", "", EventBase::INFO);

  // set the reqId in the profile
  if (isSeDDefinedFlag) {
    myNode->getProfile()->dietReqID = (int) myReqID;
    TRACE_TEXT(TRACE_ALL_STEPS, traceHeader << "setting reqID in profile to #"
	       << myReqID << endl);
  }

  // diet call
  try {
    SeD_var sed;
    if (myChosenServer == NULL)
      sed = SeD::_nil();
    else 
      sed = ORBMgr::getMgr()->resolve<SeD, SeD_var>(SEDCTXT, myChosenServer);

    if (!diet_call_common(myNode->getDag()->getExecutionAgent(),
                          myNode->getProfile(),
                          sed,
                          &myEstimVect,
                          MAX_EXEC_SERVERS)) {
      isSuccessfulFlag = true;
      TRACE_TEXT (TRACE_MAIN_STEPS, traceHeader << "diet call DONE reqID=" <<
                  myNode->getProfile()->dietReqID << endl);
      myNode->storeProfileData();
    }
    else {
      WARNING(traceHeader << "diet call FAILED" << endl);
      isSuccessfulFlag = false;
    }
  } catch(Dagda::DataNotFound& e) {
    WARNING(traceHeader << "Data not found (ID=" << e.dataID << ")");
    isSuccessfulFlag = false;
  } catch(Dagda::ReadError& e) {
    WARNING(traceHeader << "Data read error");
    isSuccessfulFlag = false;
  } catch(Dagda::InvalidPathName& e) {
    WARNING(traceHeader << "Invalid path name");
    isSuccessfulFlag = false;
  } catch(Dagda::UnreachableFile& e) {
    WARNING(traceHeader << "Unreachable file");
    isSuccessfulFlag = false;
  } catch(Dagda::PermissionDenied& e) {
    WARNING(traceHeader << "Permission denied");
    isSuccessfulFlag = false;
  } catch(Dagda::UnavailableData& e) {
    WARNING(traceHeader << "Unavailable data");
    isSuccessfulFlag = false;
  } catch(CORBA::SystemException& e) {
    WARNING(traceHeader << "Got a CORBA " << e._name() << " exception ("
	    << e.NP_minorString() << ")") ;
    isSuccessfulFlag = false;
  }
}

void 
CltDagNodeLauncher::finishNode()
{
  if (isSuccessfulFlag) {
    sendEventFrom<DagNodeLauncher, DagNode::FINISH>(this, "Node exec successful", "", EventBase::INFO);
  } else {
    sendEventFrom<DagNodeLauncher, DagNode::FAILED>(this, "Node exec failed", "", EventBase::NOTICE);
  }
}

