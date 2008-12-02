/****************************************************************************/
/* The class representing the nodes of a functional workflow                */
/*                                                                          */
/* Author(s):                                                               */
/* - Benjamin ISNARD (benjamin.isnard@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.2  2008/12/02 10:09:36  bisnard
 * added instanciation methods
 *
 * Revision 1.1  2008/10/20 08:09:58  bisnard
 * new classes for functional workflows
 *
 */

#include "debug.hh"
#include "FNode.hh"
#include "FWorkflow.hh"

using namespace std;

FNode::FNode(FWorkflow* wf, const string& id, short maxInstances)
  : Node(id),
    wf(wf),
    isFullInst(false),
    isOnHoldFlag(false),
    maxInstNb(maxInstances) {}

FNode::~FNode() {
  TRACE_TEXT (TRACE_ALL_STEPS,"~FNode() " << getId() << " destructor ..." <<  endl);
}

bool
FNode::isOnHold() {
  return this->isOnHoldFlag;
}

void
FNode::resumeInstanciation() {
  this->isOnHoldFlag = false;
}

bool
FNode::isFullyInstantiated() {
  return isFullInst;
}

WfPort *
FNode::newPort(string portId,
               unsigned int ind,
               WfPort::WfPortType portType,
               WfCst::WfDataType dataType,
               unsigned int depth) {
  WfPort * p = NULL;
  switch (portType) {
    case WfPort::PORT_IN:
      p = new FNodeInPort(this, portId, dataType, depth, ind);
      break;
    case WfPort::PORT_INOUT:
      INTERNAL_ERROR("Functional Inout port not implemented yet",0);
      /* p = new FNodeInOutPort(this, portId, dataType, depth, ind); */
      break;
    case WfPort::PORT_OUT:
      p = new FNodeOutPort(this, portId, dataType, depth, ind);
      break;
  }
  this->ports[portId] = p;
  return p;
}

/*****************************************************************************/
/*                           FConstantNode                                   */
/*****************************************************************************/
string FConstantNode::outPortName("out");

FConstantNode::FConstantNode(FWorkflow* wf, const string& id,
                             WfCst::WfDataType type)
  : FNode(wf, id, 0) {
  this->newPort(outPortName,0,WfPort::PORT_OUT,type,0);
}
FConstantNode::~FConstantNode() {}

void
FConstantNode::setValue(const string& strVal) {
  myValue = strVal;
}

void
FConstantNode::instanciate(Dag* dag) {
  if (!isOnHold() && !isFullyInstantiated()) {
    // create a WfDataHandle and set its value
    FDataTag singleTag(0,true);
    FDataHandle* dataHdl = new FDataHandle(singleTag, myValue);
    // set the out port as constant with this dataHdl
    FNodeOutPort* outPort = dynamic_cast<FNodeOutPort*>(ports[outPortName]);
    if (!outPort) {
      INTERNAL_ERROR("Missing out port in constant FNode",0);
    }
    outPort->setAsConstant(dataHdl);
    this->isFullInst = true;
  }
}

/*****************************************************************************/
/*                            FSourceNode                                    */
/*****************************************************************************/
string FSourceNode::outPortName("out");

FSourceNode::FSourceNode(FWorkflow* wf,
                         const string& id,
                         WfCst::WfDataType type,
                         short maxInstances)
  : FNode(wf, id, maxInstances) {
  this->newPort(outPortName,0,WfPort::PORT_OUT,type,0);
}
FSourceNode::~FSourceNode() {}


void
FSourceNode::instanciate(Dag* dag) {
  if (!isOnHold() && !isFullyInstantiated()) {
  // LOOP while file contains data items
    // create data handle with the data item
    // instanciate out port with this data handle
    // if this is the last item, then set the tag as last
  } // end if
}

/*****************************************************************************/
/*                             FProcNode                                     */
/*****************************************************************************/

FProcNode::FProcNode(FWorkflow* wf,
                     const string& id,
                     short maxInstances)
  : FNode(wf, id, maxInstances) {}
FProcNode::~FProcNode() {}

void
FProcNode::setDIETServicePath(const string& path) {
  myPath = path;
}

void
FProcNode::instanciate(Dag* dag) {
  if (!isOnHold() && !isFullyInstantiated()) {
    TRACE_TEXT (TRACE_ALL_STEPS,"#### Processor " << this->getId()
                    << " instanciation START" << endl);
    // LOOP while the iterator provides inputs (tag + map(port=>WfDataHandle))

    // (TEMPORARY: Without iterator)
    // LOOP for each entry in my input port queue
    FNodeInPort* inPort = dynamic_cast<FNodeInPort*>(ports["in"]);
    if (inPort == NULL) {
      INTERNAL_ERROR("Missing 'in' port",0);
    }
    int  nbInstances = 0;
    bool limitInstances = this->maxInstNb != 0;
    if (limitInstances) {
      cout << "########## INSTANCE COUNT LIMIT : " << maxInstNb << endl;
    }

    //
    // LOOP OVER ITEMS in the INPUT QUEUE
    // (until queue is empty OR until limit of max instances is reached)

    map<FDataTag, FDataHandle*>::iterator dataIter = inPort->begin();
    while ((dataIter != inPort->end())
            && (!limitInstances || (++nbInstances <= this->maxInstNb))) {
      // get the data handle
      FDataHandle* dataHdl = (FDataHandle*) dataIter->second;
      ++dataIter;
      // create a new DagNode
      string   dagNodeId = this->getId() + dataHdl->getTag().toString();
      TRACE_TEXT (TRACE_ALL_STEPS,"  ## NEW NODE INSTANCE : " << dagNodeId << endl);
      DagNode* dagNode = dag->createDagNode(dagNodeId);
      dagNode->setPbName(myPath);
      if (dagNode == NULL) {
        INTERNAL_ERROR("Could not create dag node " << dagNodeId << endl,0);
      }
      // LOOP for each port
      for (map<string,WfPort*>::iterator portIter = ports.begin();
           portIter != ports.end();
           ++portIter) {
        WfPort* port = (WfPort*) portIter->second;
        FNodeInPort* inPort;
        FNodeOutPort* outPort;
        switch(port->getPortType()) {
          case WfPort::PORT_IN:
            inPort = dynamic_cast<FNodeInPort*>(port);
            // get the input data handle from the map (if not found set as NULL)

            // instanciate port with data handle
            inPort->instanciate(dag, dagNode, dataHdl);
            break;
          case WfPort::PORT_OUT:
            outPort = dynamic_cast<FNodeOutPort*>(port);
            // instanciate port with dagNode and tag
            outPort->instanciate(dag, dagNode, dataHdl->getTag());
            break;
          case WfPort::PORT_INOUT:
            ;
        }
      } // end for ports
      // check if this is the last item
      if (dataHdl->getTag().isLast()) {
        cout << "########## LAST TAG REACHED" << endl;
        this->isFullInst = true;
      }
    }
    //
    // END LOOP
    //

    cout << "Cleanup the input queue" << endl;
    if (dataIter == inPort->end()) {
      // empty the inPort queue
      inPort->clearQueue();
    } else {
      // remove only the processed items
      inPort->clearQueue(inPort->begin(), dataIter);
      // set as on hold
      cout << "########## SET NODE ON HOLD" << endl;
      this->isOnHoldFlag = true;
    }
  } // end if (!isFullyInstantiated())
}

/*****************************************************************************/
/*                           FConditionNode                                  */
/*****************************************************************************/

FConditionNode::FConditionNode(FWorkflow* wf, const string& id)
  : FNode(wf, id, 0) {}
FConditionNode::~FConditionNode() {}


void
FConditionNode::instanciate(Dag* dag) {
}

