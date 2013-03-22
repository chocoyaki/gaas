/**
 * @file  CltDagNodeLauncher.cc
 *
 * @brief   Launcher for client-side dag node
 *
 * @author   Benjamin ISNARD (benjamin.isnard@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


#include "CltDagNodeLauncher.hh"
#include "debug.hh"
#include "DIETCall.hh"
#include "Dag.hh"
#include "DagNode.hh"
#include "EventTypes.hh"
#include "ORBMgr.hh"

using namespace events;

#define MAX_EXEC_SERVERS 10 \
  // max nb of servers in the agent response

CltDagNodeLauncher::CltDagNodeLauncher(DagNode *parent)
  : DagNodeLauncher(parent) {
}

std::string
CltDagNodeLauncher::toString() const {
  return "Client " + DagNodeLauncher::toString();
}


void
CltDagNodeLauncher::execNode() {
  std::string traceHeader = "[" + myNode->getId() + "] Client Launcher : ";
  TRACE_TEXT(TRACE_ALL_STEPS, traceHeader << "starting... \n");

  sendEventFrom<DagNodeLauncher,
                DagNode::START>(this,
                                "Start client launcher",
                                "",
                                EventBase::INFO);

  // set the reqId in the profile
  if (isSeDDefinedFlag) {
    myNode->getProfile()->dietReqID = (int) myReqID;
    TRACE_TEXT(TRACE_ALL_STEPS, traceHeader << "setting reqID in profile to #"
                                            << myReqID << "\n");
  }

  // diet call
  try {
    SeD_var sed;
    if (myChosenServer == NULL) {
      sed = SeD::_nil();
    } else {
      sed = ORBMgr::getMgr()->resolve<SeD, SeD_var>(SEDCTXT, myChosenServer);
    }

    if (!diet_call_common(myNode->getDag()->getExecutionAgent(),
                          myNode->getProfile(),
                          sed,
                          &myEstimVect,
                          MAX_EXEC_SERVERS)) {
      isSuccessfulFlag = true;
      TRACE_TEXT(TRACE_MAIN_STEPS, traceHeader << "diet call DONE reqID=" <<
                 myNode->getProfile()->dietReqID << "\n");
      myNode->storeProfileData();
    } else {
      WARNING(traceHeader << "diet call FAILED\n");
      isSuccessfulFlag = false;
    }
  } catch (Dagda::DataNotFound &e) {
    WARNING(traceHeader << "Data not found (ID=" << e.dataID << ")");
    isSuccessfulFlag = false;
  } catch (Dagda::ReadError &e) {
    WARNING(traceHeader << "Data read error");
    isSuccessfulFlag = false;
  } catch (Dagda::InvalidPathName &e) {
    WARNING(traceHeader << "Invalid path name");
    isSuccessfulFlag = false;
  } catch (Dagda::UnreachableFile &e) {
    WARNING(traceHeader << "Unreachable file");
    isSuccessfulFlag = false;
  } catch (Dagda::PermissionDenied &e) {
    WARNING(traceHeader << "Permission denied");
    isSuccessfulFlag = false;
  } catch (Dagda::UnavailableData &e) {
    WARNING(traceHeader << "Unavailable data");
    isSuccessfulFlag = false;
  } catch (CORBA::SystemException &e) {
    WARNING(traceHeader << "Got a CORBA " << e._name() << " exception ("
                        << e.NP_minorString() << ")");
    isSuccessfulFlag = false;
  }
} // execNode

void
CltDagNodeLauncher::finishNode() {
  if (isSuccessfulFlag) {
    sendEventFrom<DagNodeLauncher, DagNode::FINISH>(this,
                                                    "Node exec successful", "",
                                                    EventBase::INFO);
  } else {
    sendEventFrom<DagNodeLauncher, DagNode::FAILED>(this, "Node exec failed",
                                                    "",
                                                    EventBase::NOTICE);
  }
}
