/****************************************************************************/
/* The class representing the nodes of a functional workflow that will      */
/* correspond to tasks in the DAG                                           */
/*                                                                          */
/* Author(s):                                                               */
/* - Benjamin ISNARD (benjamin.isnard@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.10  2011/02/24 16:50:06  bdepardo
 * Code cleanup.
 * Use TRACE_TEXT instead of cout
 *
 * Revision 1.9  2010/07/20 09:20:11  bisnard
 * integration with eclipse gui and with dietForwarder
 *
 * Revision 1.8  2009/10/02 07:43:51  bisnard
 * modified trace verbosity
 *
 * Revision 1.7  2009/08/26 10:33:09  bisnard
 * implementation of workflow status & restart
 *
 * Revision 1.6  2009/07/23 12:30:10  bisnard
 * new method finalize() for functional wf nodes
 * removed const on currDataLine parameter for instance creation
 *
 * Revision 1.5  2009/07/07 09:03:22  bisnard
 * changes for sub-workflows (FWorkflow class now inherits from FProcNode)
 *
 * Revision 1.4  2009/06/15 12:11:12  bisnard
 * use new XML Parser (SAX) for data source file
 * use new class WfValueAdapter to avoid data duplication
 * use new method FNodeOutPort::storeData
 * changed method to compute total nb of data items
 *
 * Revision 1.3  2009/05/15 11:10:20  bisnard
 * release for workflow conditional structure (if)
 *
 * Revision 1.2  2009/04/17 09:04:07  bisnard
 * initial version for conditional nodes in functional workflows
 *
 * Revision 1.1  2009/04/09 09:55:27  bisnard
 * new class
 *
 */

#include "debug.hh"
#include "FActivityNode.hh"
#include "FWorkflow.hh"
#include "Dag.hh"

FActivityNode::FActivityNode(FWorkflow* wf,
                             const string& id)
  : FProcNode(wf, id), myPath(), maxInstNb(0) {}

FActivityNode::~FActivityNode() {}

void
FActivityNode::setMaxInstancePerDag(short maxInst) {
  maxInstNb = maxInst;
}

void
FActivityNode::setDIETServicePath(const string& path) {
  myPath = path;
}

void
FActivityNode::setDIETEstimationOption(const string& estimOption) {
  myEstimOption = estimOption;
}

void
FActivityNode::initialize() {
  FProcNode::initialize();
}

void
FActivityNode::initInstanciation() {
  TRACE_TEXT(TRACE_ALL_STEPS, "#### Activity " << getId()
             << " instanciation START\n");
  nbInstances = 0;
  if (maxInstNb != 0) {
    TRACE_TEXT(TRACE_ALL_STEPS, "########## INSTANCE COUNT LIMIT : "
               << maxInstNb << "\n");
  }
}

bool
FActivityNode::instLimitReached() {
  return !((maxInstNb == 0) || (nbInstances < maxInstNb));
}

void
FActivityNode::createRealInstance(Dag* dag,
                                  const FDataTag& currTag,
                                  vector<FDataHandle*>& currDataLine) {
  DagNode* dagNode = NULL;
  string   dagNodeId = getId() + currTag.toString();

  // check if dagNode id is not available in the execution transcript
  // (used in case of workflow re-start)
  bool isAlreadyExecuted = false;
  bool isOutputDataAvailable = false;
  getWorkflow()->findDagNodeTranscript(dagNodeId, dagNode, isAlreadyExecuted);

  if (isAlreadyExecuted) {
    // LOOP for each port - check if data available
    isOutputDataAvailable = true;
    std::map<std::string, WfPort*>::iterator portIter = ports.begin();
    for (; portIter != ports.end(); ++portIter) {
      WfPort* FPort = (WfPort*) portIter->second;
      if (FPort->getPortType() == WfPort::PORT_OUT) {
        // TODO manage exceptions
        WfPort *_DPort = dagNode->getPort(FPort->getId());
        DagNodePort *DPort = dynamic_cast<DagNodePort*>(_DPort);

        if (!DPort->isDataIDAvailable(dag->getExecutionAgent())) {
          TRACE_TEXT(TRACE_MAIN_STEPS,  "DATA ID NOT AVAILABLE!\n");
          isOutputDataAvailable = false;
          break;
        }
      }
    }
  }

  if (isOutputDataAvailable) {
    TRACE_TEXT(TRACE_MAIN_STEPS,
               "  ## RE-USE ACTIVITY INSTANCE : " << dagNodeId << "\n");
    // LOOP for each port - send data
    std::map<string, WfPort*>::iterator portIter = ports.begin();
    for (; portIter != ports.end(); ++portIter) {
      WfPort* FPort = (WfPort*) portIter->second;
      if (FPort->getPortType() == WfPort::PORT_OUT) {
        // TODO manage exceptions
        WfPort *_DPort = dagNode->getPort(FPort->getId());
        DagNodePort *DPort = dynamic_cast<DagNodePort*>(_DPort);

        FDataHandle* dataHdl = new FDataHandle(currTag,
                                               FPort->getBaseDataType(),
                                               FPort->getDepth(),
                                               DPort->getDataID());

        TRACE_TEXT(TRACE_ALL_STEPS, "Port " << DPort->getId() << " data ID = "
                   << dataHdl->getDataID() << "\n");

        FNodeOutPort *outPort = dynamic_cast<FNodeOutPort*>(FPort);
        outPort->storeData(dataHdl);
        outPort->sendData(dataHdl);
      }
    }
    TRACE_TEXT(TRACE_ALL_STEPS, "  ## END OF RE-USE OF ACTIVITY INSTANCE : "
               << dagNodeId << "\n");
  } else {
    // USUAL BEHAVIOUR (no previous execution)
    // create a new DagNode
    TRACE_TEXT(TRACE_MAIN_STEPS,
               "  ## NEW ACTIVITY INSTANCE : " << dagNodeId << "\n");
    dagNode = dag->createDagNode(dagNodeId, wf);
    if (dagNode == NULL) {
      INTERNAL_ERROR("Could not create dag node " << dagNodeId << "\n", 0);
    }

    ++nbInstances;

    // LOOP for each port
    std::map<std::string, WfPort*>::iterator portIter = ports.begin();
    for (; portIter != ports.end(); ++portIter) {
      WfPort* port = (WfPort*) portIter->second;
      switch (port->getPortType()) {
      case WfPort::PORT_IN:
      {
        FNodeInPort* inPort = dynamic_cast<FNodeInPort*>(port);
        // get the input data handle from the map (if not found set as NULL)
        FDataHandle* dataHdl = currDataLine[inPort->getIndex()];
        if (dataHdl == NULL) {
          INTERNAL_ERROR("FActivityNode: Input data handle is invalid", 1);
        }
        // instanciate port with data handle
        inPort->createRealInstance(dag, dagNode, dataHdl);
        break;
      }
      case WfPort::PORT_OUT:
      {
        FNodeOutPort* outPort = dynamic_cast<FNodeOutPort*>(port);
        // instanciate port with dagNode and tag
        FDataHandle* dataHdl =
          outPort->createRealInstance(dag, dagNode, currTag);
        if (dataHdl == NULL) {
          INTERNAL_ERROR("FActivityNode: Output data handle is invalid", 1);
        }
        outPort->storeData(dataHdl);
        outPort->sendData(dataHdl);
        break;
      }
      case WfPort::PORT_INOUT:
      {
        FNodeInOutPort* inOutPort = dynamic_cast<FNodeInOutPort*>(port);
        FDataHandle* inDataHdl = currDataLine[inOutPort->getIndex()];
        if (inDataHdl == NULL) {
          INTERNAL_ERROR("FActivityNode: Input data handle is invalid", 1);
        }
        FDataHandle* outDataHdl =
          inOutPort->createRealInstance(dag, dagNode, currTag, inDataHdl);
        if (outDataHdl == NULL) {
          INTERNAL_ERROR("FActivityNode: Output data handle is invalid", 1);
        }
        inOutPort->storeData(outDataHdl);
        inOutPort->sendData(outDataHdl);
        break;
      }
      default:
        INTERNAL_ERROR("Invalid port type for FActivityNode", 1);
      }
    }

    // define service path
    if (!isDynamicParam("path")) {
      dagNode->setPbName(myPath);
    } else {
      dagNode->setPbName(getDynamicParamValue("path"));
      // define ESTIMATION option
      if (myEstimOption == "constant") {
        dagNode->setEstimationClass(getId());
      }

      TRACE_TEXT(TRACE_ALL_STEPS,
                 "  ## END OF CREATION OF NEW ACTIVITY INSTANCE : "
                 << dagNodeId << "\n");
    }
  }
}


void
FActivityNode::updateInstanciationStatus() {
  FProcNode::updateInstanciationStatus();
  // Handle specific case for instance nb limitation
  if (!myRootIterator->isAtEnd()) {
    TRACE_TEXT(TRACE_MAIN_STEPS, "########## SET NODE ON HOLD\n");
    myStatus = N_INSTANC_ONHOLD;
  }
}

