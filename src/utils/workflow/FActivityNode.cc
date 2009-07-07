/****************************************************************************/
/* The class representing the nodes of a functional workflow that will      */
/* correspond to tasks in the DAG
/*                                                                          */
/* Author(s):                                                               */
/* - Benjamin ISNARD (benjamin.isnard@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
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
#include "Dag.hh"

FActivityNode::FActivityNode(FWorkflow* wf,
                             const string& id)
  : FProcNode(wf, id), maxInstNb(0), myPath() {}

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
  TRACE_TEXT (TRACE_ALL_STEPS,"#### Activity " << getId()
                    << " instanciation START" << endl);
  nbInstances = 0;
  if (maxInstNb != 0) {
    TRACE_TEXT (TRACE_ALL_STEPS, "########## INSTANCE COUNT LIMIT : "
          << maxInstNb << endl);
  }
}

bool
FActivityNode::instLimitReached() {
  return !((maxInstNb == 0) || (nbInstances < maxInstNb));
}

void
FActivityNode::createRealInstance(Dag* dag,
                                  const FDataTag& currTag,
                                  const vector<FDataHandle*>& currDataLine) {
  DagNode* dagNode = NULL;
  string   dagNodeId = getId() + currTag.toString();

  // create a new DagNode
  TRACE_TEXT (TRACE_ALL_STEPS,"  ## NEW ACTIVITY INSTANCE : " << dagNodeId << endl);
  dagNode = dag->createDagNode(dagNodeId, wf);
  if (dagNode == NULL) {
    INTERNAL_ERROR("Could not create dag node " << dagNodeId << endl,0);
  }
//   dagNode->setFNode(this);
  ++nbInstances;

  // LOOP for each port
  for (map<string,WfPort*>::iterator portIter = ports.begin();
      portIter != ports.end();
      ++portIter) {
    WfPort* port = (WfPort*) portIter->second;
    FNodeInPort* inPort;
    FNodeOutPort* outPort;
    FDataHandle* dataHdl;
    switch(port->getPortType()) {
      case WfPort::PORT_IN:
        inPort = dynamic_cast<FNodeInPort*>(port);
        // get the input data handle from the map (if not found set as NULL)
        dataHdl = currDataLine[inPort->getIndex()];
        if (dataHdl == NULL) {
          INTERNAL_ERROR("FActivityNode: Input data handle is invalid",1);
        }
        // instanciate port with data handle
        inPort->createRealInstance(dag, dagNode, dataHdl);
        break;
      case WfPort::PORT_OUT:
        outPort = dynamic_cast<FNodeOutPort*>(port);
        // instanciate port with dagNode and tag
        dataHdl = outPort->createRealInstance(dag, dagNode, currTag);
        if (dataHdl == NULL) {
          INTERNAL_ERROR("FActivityNode: Output data handle is invalid",1);
        }
        outPort->storeData(dataHdl);
        outPort->sendData(dataHdl);
        break;
      default:
        INTERNAL_ERROR("Invalid port type for FActivityNode",1);
    }
  } // end for ports

  // define service path
  if (!isDynamicParam("path"))
    dagNode->setPbName(myPath);
  else
    dagNode->setPbName(getDynamicParamValue("path"));
  // define ESTIMATION option
  if (myEstimOption == "constant") {
    dagNode->setEstimationClass(getId());
  }

  TRACE_TEXT (TRACE_ALL_STEPS,"  ## END OF CREATION OF NEW ACTIVITY INSTANCE : "
                              << dagNodeId << endl);
}

void
FActivityNode::updateInstanciationStatus() {
  FProcNode::updateInstanciationStatus();
  // Handle specific case for instance nb limitation
  if (!myRootIterator->isAtEnd()) {
    TRACE_TEXT (TRACE_ALL_STEPS, "########## SET NODE ON HOLD" << endl);
    myStatus = N_INSTANC_ONHOLD;
  }
}

