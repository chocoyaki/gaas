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
FActivityNode::createInstance(Dag* dag,
                              const FDataTag& currTag,
                              const vector<FDataHandle*>& currDataLine) {
  // create a new DagNode
  string   dagNodeId = getId() + currTag.toString();
  TRACE_TEXT (TRACE_ALL_STEPS,"  ## NEW ACTIVITY INSTANCE : " << dagNodeId << endl);
  DagNode* dagNode = dag->createDagNode(dagNodeId);
  if (dagNode == NULL) {
    INTERNAL_ERROR("Could not create dag node " << dagNodeId << endl,0);
  }
  dagNode->setFNode(this);
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
          INTERNAL_ERROR("Input data handle is invalid",1);
        }
        // instanciate port with data handle
        inPort->instanciate(dag, dagNode, dataHdl);
        break;
      case WfPort::PORT_OUT:
        outPort = dynamic_cast<FNodeOutPort*>(port);
        // instanciate port with dagNode and tag
        outPort->instanciate(dag, dagNode, currTag);
        break;
      case WfPort::PORT_INOUT:
        ;
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

