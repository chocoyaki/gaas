/**
* @file FActivityNode.cc
* 
* @brief  The nodes of a functional workflow that correspond to tasks in the DAG 
* 
* @author  Benjamin ISNARD (Benjamin.Isnard@ens-lyon.fr)
* 
* @section Licence
*   |LICENCE|                                                                
*/


#include "debug.hh"
#include "FActivityNode.hh"
#include "FWorkflow.hh"
#include "Dag.hh"

FActivityNode::FActivityNode(FWorkflow* wf, const std::string& id)
  : FProcNode(wf, id), myPath(), maxInstNb(0) {
}

FActivityNode::~FActivityNode() {
}

void
FActivityNode::setMaxInstancePerDag(short maxInst) {
  maxInstNb = maxInst;
}

void
FActivityNode::setDIETServicePath(const std::string& path) {
  myPath = path;
}

void
FActivityNode::setDIETEstimationOption(const std::string& estimOption) {
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
                                  std::vector<FDataHandle*>& currDataLine) {
  DagNode* dagNode = NULL;
  std::string dagNodeId = getId() + currTag.toString();

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
    std::map<std::string, WfPort*>::iterator portIter = ports.begin();
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

