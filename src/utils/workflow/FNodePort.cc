/****************************************************************************/
/* The class representing the ports of nodes of a functional workflow       */
/*                                                                          */
/* Author(s):                                                               */
/* - Benjamin ISNARD (benjamin.isnard@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.8  2009/04/17 09:04:07  bisnard
 * initial version for conditional nodes in functional workflows
 *
 * Revision 1.7  2009/04/09 09:56:20  bisnard
 * refactoring due to new class FActivityNode
 *
 * Revision 1.6  2009/04/08 09:34:56  bisnard
 * pending nodes mgmt moved to FWorkflow class
 * FWorkflow and FNode state graph revisited
 * FNodePort instanciation refactoring
 *
 * Revision 1.5  2009/02/24 14:01:05  bisnard
 * added dynamic parameter mgmt for wf processors
 *
 * Revision 1.4  2009/02/06 14:55:08  bisnard
 * setup exceptions
 *
 * Revision 1.3  2009/01/16 13:54:50  bisnard
 * new version of the dag instanciator with iteration strategies for nodes with multiple input ports
 *
 * Revision 1.2  2008/12/09 12:15:59  bisnard
 * pending instanciation handling (uses dag outputs for instanciation
 * of a functional wf)
 *
 * Revision 1.1  2008/12/02 10:07:07  bisnard
 * new classes for functional workflow instanciation and execution
 *
 */

#include "debug.hh"
#include "DIET_data.h"
#include "FNodePort.hh"
#include "FNode.hh"
#include "FWorkflow.hh"
#include "DagNodePort.hh"
#include "Dag.hh"

/**
 * Constructors & destructors
 */

FNodePort::FNodePort (WfNode * parent,
                      const string& _id,
                      WfPort::WfPortType _portType,
                      WfCst::WfDataType _type,
                      unsigned int _depth,
                      unsigned int _ind)
  : WfPort(parent, _id, _portType, _type, _depth, _ind) {
}

FNodePort::~FNodePort() {
}

FNodeOutPort::FNodeOutPort(WfNode * parent,
                           const string& _id,
                           WfCst::WfDataType _type,
                           unsigned int _depth,
                           unsigned int _ind)
  : FNodePort(parent, _id, WfPort::PORT_OUT, _type, _depth, _ind)
    , myBuffer(_depth+1) {
}

FNodeOutPort::~FNodeOutPort() {
}

FNodeInPort::FNodeInPort(WfNode * parent,
                           const string& _id,
                           WfCst::WfDataType _type,
                           unsigned int _depth,
                           unsigned int _ind)
  : FNodePort(parent, _id, WfPort::PORT_IN, _type, _depth, _ind),
    dataTotalNb(0), totalDef(false), valueRequired(false) { }

FNodeInPort::~FNodeInPort() {
}

FNodeParamPort::FNodeParamPort(WfNode * parent,
                           const string& _id,
                           WfCst::WfDataType _type,
                           unsigned int _ind)
  : FNodeInPort(parent, _id, _type, 0, _ind) {
  portType = WfPort::PORT_PARAM;
}

FNodeParamPort::~FNodeParamPort() {
}


FNode*
FNodePort::getParentFNode() {
  FNode* parent = dynamic_cast<FNode*>(getParent());
  return parent;
}

FProcNode*
FNodePort::getParentProcNode() {
  FProcNode* parent = dynamic_cast<FProcNode*>(getParent());
  if (parent == NULL) {
    INTERNAL_ERROR("Invalid type of parent node for FNodePort " << getId() << endl,1);
  }
  return parent;
}

/**
 * Ports connection setup method
 * Called by WfSimpleAdapter on both sides to connect node ports
 */
void
FNodeOutPort::connectToPort(WfPort* remPort) {
  // set connected flag
  WfPort::connectToPort(remPort);
  // cast & add to list of connected in ports
  FNodeInPort* srcPort = dynamic_cast<FNodeInPort*>(remPort);
  if (srcPort == NULL) {
    INTERNAL_ERROR("Wrong connected node type in functional node OUT port", 0);
  }
  myConnectedPorts.push_back(srcPort);
}

/**
 * Static sendData
 */
void
FNodeOutPort::sendData(FDataHandle* dataHdl) throw (WfDataHandleException) {
  TRACE_TEXT (TRACE_ALL_STEPS,"   # Insert data into buffer" << endl);
  myBuffer.insertInTree(dataHdl);
  // use cardinal info if data is container (depth > 0) and cardinal is known
  list<string>  cardList;
  if ((depth > 0) && (card != NULL)) {
    cardList = *card;
  } else {
    cardList.push_back("x");
  }
  // add the new dataHandle to all the connected in ports ==> inPort.addData
  // all the inPort(s) that return false must be included in the FNode pending list
  TRACE_TEXT (TRACE_ALL_STEPS,"   # Insert data into connected ports" << endl);
  for (list<FNodeInPort*>::iterator inPortIter = myConnectedPorts.begin();
       inPortIter != myConnectedPorts.end();
       ++inPortIter) {
    FNodeInPort* inPort = (FNodeInPort*) *inPortIter;
    try {
      inPort->addData(dataHdl, cardList);
      checkTotalDataNb(inPort, dataHdl);
    } catch (WfDataHandleException& e) {
      if ((e.Type() == WfDataHandleException::eCARD_UNDEF)
          || (e.Type() == WfDataHandleException::eVALUE_UNDEF)) {
        TRACE_TEXT (TRACE_ALL_STEPS,"  - in port (" << inPort->getParent()->getId()
                    << "#" << inPort->getId() << ") insertion pending" << endl);
        FWorkflow* wf = getParentProcNode()->getWorkflow();
        // THE dataHdl supplied here ALWAYS contains a pointer to a DagNodeOutPort!!
        // BECAUSE if cardinal or value is not known it means this is a dynamic data
        // that is necessarily produced by a dagNode
        DagNodeOutPort* dagOutPort = dynamic_cast<DagNodeOutPort*>(dataHdl->getSourcePort());
        DagNode* dagNode = dynamic_cast<DagNode*>(dagOutPort->getParent());
        wf->setPendingInstanceInfo(dagNode,dataHdl,this,inPort);
      } else throw;
    }
  }
}

/**
 * Dynamic sendData (if data available)
 * Used by workflow when processing pending instances
 */
void
FNodeOutPort::reSendData(FDataHandle* dataHdl, FNodeInPort* inPort)
    throw (WfDataException, WfDataHandleException) {
  DagNodeOutPort* dagOutPort = dynamic_cast<DagNodeOutPort*>(dataHdl->getSourcePort());
  inPort->addData(dataHdl, dagOutPort);
  checkTotalDataNb(inPort, dataHdl);
}

void
FNodeOutPort::instanciate(Dag* dag, DagNode* nodeInst, const FDataTag& tag) {
  // create the portInst (DagNodeOutPort) for the nodeInst
  string portId = this->getId();
  TRACE_TEXT (TRACE_ALL_STEPS,"   # Creating new instance of OUT port: " << portId << endl);
  WfPort* portInst = nodeInst->newPort(portId,
                                       nodeInst->getPortNb(),
                                       portType,
                                       getBaseDataType(),
                                       depth);
  // create a FDataHandle corresponding to the new port instance
  //  - its depth equals the depth of the out port
  //  - its tag equals the tag of the node instance
  //  - its port equals the portInst
  TRACE_TEXT (TRACE_ALL_STEPS,"   # Creating new data handle / tag = " << tag.toString() << endl);
  FDataHandle* dataHdl = new FDataHandle(tag, depth, NULL, portInst);
  // send the data to the connected ports
  sendData(dataHdl);
}

void
FNodeOutPort::instanciate(FDataHandle* dataHdl) {
  sendData(dataHdl);
}

void
FNodeOutPort::checkTotalDataNb(FNodeInPort *inPort,
                               FDataHandle *dataHdl) {
  // determine the level corresponding to data items of the input port
  unsigned int inLevel = dataHdl->getTag().getLevel() + this->depth
                         - inPort->getDepth();
  // check if the tree is complete at this level
  if (myBuffer.checkIfComplete(inLevel, myBufferChildNbTable)) {
    // if yes then use the childNbTable to set the total items for the
    // connected port
    inPort->setTotalDataNb(myBufferChildNbTable[inLevel]);
  }
}

void
FNodeOutPort::setAsConstant(FDataHandle* dataHdl) {
  // set the connected in ports as constant with this handle
  for (list<FNodeInPort*>::iterator inPortIter = myConnectedPorts.begin();
       inPortIter != myConnectedPorts.end();
       ++inPortIter) {
    ((FNodeInPort*) *inPortIter)->setAsConstant(dataHdl);
  }
}

/*****************************************************************************/
/*                            FNodeInPort                                    */
/*****************************************************************************/

void
FNodeInPort::setValueRequired() {
  valueRequired = true;
}

/**
 * Static addData (RECURSIVE)
 */
void
FNodeInPort::addData(FDataHandle* dataHdl, const list<string>& dataCard)
   throw (WfDataHandleException)
{
  TRACE_TEXT (TRACE_ALL_STEPS,"     # Calling addData (port " << getParent()->getId()
                              << "#" << getId()
                              << ") for data: " << dataHdl->getTag().toString() << endl);
  // if dataHdl is not complete then do nothing (happens due to recursive call to parent)
  if (!dataHdl->isAdapterDefined() && !dataHdl->isValueDefined()) {
    TRACE_TEXT (TRACE_ALL_STEPS,"Cannot add data to inPort (no value and no adapter)" << endl);
    return;
  }
  // check if value is required
  if (valueRequired && !dataHdl->isValueDefined()) {
    TRACE_TEXT (TRACE_ALL_STEPS,"Cannot add data without value" << endl);
    throw WfDataHandleException(WfDataHandleException::eVALUE_UNDEF,dataHdl->getTag().toString());
  }
  // if dataHdl depth = port depth
    // if dataHdl not already in the queue
      // add to my queue
  if (dataHdl->getDepth() == depth) {
    const FDataTag& dataTag = dataHdl->getTag();
    if (myQueue.find(dataTag) == myQueue.end()) {
      TRACE_TEXT (TRACE_ALL_STEPS,"Adding data in input port queue (tag="
                  << dataTag.toString() << ")" << endl);
      myQueue.insert(pair<FDataTag,FDataHandle*>(dataTag, dataHdl));
      // updates node status
      getParentFNode()->setStatusReady();
    }
  } else if (dataHdl->getDepth() > depth) {
  // if dataHdl depth > port depth (SPLIT)
    if (!dataCard.empty()) {
      string cardStr = dataCard.front();
      if (cardStr != "x") {
        // if dataHdl cardinal is defined, recursively call addData on all the elements
        // of dataHdl (depth of dataHdl will decrease at each step)
        unsigned int cardInt = atoi(cardStr.c_str());
        dataHdl->setCardinal(cardInt);
      }
    }
    if (dataHdl->isCardinalDefined()) {
      TRACE_TEXT (TRACE_ALL_STEPS,"Adding data elements (SPLIT)" << endl);
      // remove first element of the cardinal list
      list<string> childCard = dataCard;
      childCard.pop_front();
      // loop over all elements of the data handle (begin may throw exception)
      for (map<FDataTag,FDataHandle*>::iterator eltIter = dataHdl->begin();
           eltIter != dataHdl->end();
           ++eltIter) {
        this->addData((FDataHandle*) eltIter->second, childCard);
      }
    } else {
      TRACE_TEXT (TRACE_ALL_STEPS,"Cannot split data (cardinal unknown)" << endl);
      throw WfDataHandleException(WfDataHandleException::eCARD_UNDEF,dataHdl->getTag().toString());
    }
  // if dataHdl depth < port depth (MERGE)
  } else {
    if (dataHdl->isParentDefined()) {
      TRACE_TEXT (TRACE_ALL_STEPS,"Trying to add parent (MERGE)" << endl);
      // recursively call addData on the parent of dataHdl
      list<string> dummyCard; // FIXME
      this->addData(dataHdl->getParent(),dummyCard);
    } else {
      INTERNAL_ERROR("Missing data handle parent",0);
    }
  }
}

/**
 * Dynamic addData
 */
void
FNodeInPort::addData(FDataHandle* dataHdl, DagNodeOutPort* dagOutPort)
    throw (WfDataException, WfDataHandleException) {
  TRACE_TEXT (TRACE_ALL_STEPS,"     # Calling addData (port " << getParent()->getId()
                              << "#" << getId()
                              << ") for data: " << dataHdl->getTag().toString() << endl);
  // initialization: if data handle does not contain ID (toplevel call to addData)
  if (!dataHdl->isDataIDDefined()) {
    const string& portDataID = dagOutPort->getDataID();
    dataHdl->setDataID(portDataID);
    TRACE_TEXT (TRACE_ALL_STEPS,"addData: uses port data ID = " << portDataID << endl);
  }
  // final addData step ==> call the static addData method
  if (dataHdl->getDepth() == depth) {
    TRACE_TEXT (TRACE_ALL_STEPS,"addData: level match => call static addData" << endl);
    list<string> dummyCard;
    addData(dataHdl,dummyCard);
  } else {
    if (dataHdl->isDataIDDefined()) {
#if HAVE_DAGDA
      TRACE_TEXT (TRACE_ALL_STEPS,"addData: get ID list from port" << endl);
      diet_container_t* content = NULL;
      // retrieve container element ID list (may throw exception)
      content = dagOutPort->getDataIDList(dataHdl->getDataID());
      if (content->size > 0) {
        TRACE_TEXT (TRACE_ALL_STEPS,"addData: cardinal = " << content->size << endl);
        dataHdl->setCardinal(content->size);
        TRACE_TEXT (TRACE_ALL_STEPS,"Adding data elements (SPLIT)" << endl);
        int ix = 0;
        for (map<FDataTag,FDataHandle*>::iterator eltIter = dataHdl->begin();
             eltIter != dataHdl->end();
             ++eltIter) {
          FDataHandle * childHdl = (FDataHandle*) eltIter->second;
          // set the data ID of the child data using content provided by dag port
          if (content->elt_ids[ix])
            childHdl->setDataID(content->elt_ids[ix]);
          else
            childHdl->setAsVoid();
          ++ix;
          // add the child ID to the input port queue
          addData(childHdl, dagOutPort);
        }
      } else {
        WARNING("FNodeInPort::addData : Empty container");
        throw (WfDataException(WfDataException::eINVALID_CONTAINER,
               "Empty container " + dataHdl->getDataID()));
      }
#else
      INTERNAL_ERROR("Missing Dagda DIET component to handle containers",1);
#endif
    } else {
      WARNING("FNodeInPort::addData : Missing Data ID");
      throw (WfDataException(WfDataException::eNOTFOUND,
                              "Empty data ID"));
    }
  }
}

void
FNodeInPort::setTotalDataNb(unsigned int total) {
  dataTotalNb = total;
  totalDef = true;
  TRACE_TEXT (TRACE_ALL_STEPS,"Setting total to " << total
               << " for IN port : " << getId() << endl);
}

void
FNodeInPort::setAsConstant(FDataHandle* dataHdl) {
  FProcNode* node = getParentProcNode();
  node->setConstantInput(getIndex(), dataHdl);
  node->setStatusReady();
}

void
FNodeInPort::instanciate(Dag* dag, DagNode* nodeInst, FDataHandle* dataHdl) {
  // create a DagNodeInPort for the nodeInst
  string portId = this->getId();
  TRACE_TEXT (TRACE_ALL_STEPS,"Creating new instance of IN port: " << portId << endl);
  WfPort* portInst = nodeInst->newPort(portId,
                                       nodeInst->getPortNb(),
                                       portType,
                                       getBaseDataType(),
                                       depth);
  // if the dataHdl has a value then set the value of the port
  if (dataHdl->isValueDefined()) {
    TRACE_TEXT (TRACE_ALL_STEPS," IN port ==> VALUE : " << dataHdl->getValue() << endl);
    DagNodePort* dagPortInst = dynamic_cast<DagNodePort*>(portInst);
    dagPortInst->setValue(dataHdl->getValue());
  } else {
    // if the dataHdl has a source then set the source of the port
    WfPortAdapter * portAdapter = dataHdl->createPortAdapter(dag->getId());
    portInst->setPortAdapter(portAdapter);
    TRACE_TEXT (TRACE_ALL_STEPS," IN port ==> ADAPTER = " << portAdapter->getSourceRef() << endl);
  }
}

void
FNodeInPort::displayData(ostream& output) {
  if (totalDef)
    output << "TOTAL NB OF RESULTS : " << dataTotalNb << endl;
  else
    output << "Warning: undefined total nb of results (error during instanciation)" << endl;
  int ix = 0;
  for (map<FDataTag,FDataHandle*>::const_iterator dataIter = myQueue.begin();
       dataIter != myQueue.end();
       ++dataIter) {
    output << "[" << ix++ << "]=";
    try {
      ((FDataHandle*) dataIter->second)->displayDataAsList(output);
    } catch (WfDataException& e) {
      output << "<Error: " << e.ErrorMsg() << ">";
    }
    output << endl;
  }
}

/*****************************************************************************/
/*                          FNodeParamPort                                   */
/*****************************************************************************/
// Note: a param port has necessarily depth=0

void
FNodeParamPort::addData(FDataHandle* dataHdl, const list<string>& dataCard)
    throw (WfDataHandleException)
{
  if (dataHdl->isValueDefined())
    FNodeInPort::addData(dataHdl, dataCard);
  else
    throw WfDataHandleException(WfDataHandleException::eVALUE_UNDEF,
                                dataHdl->getTag().toString());
}

void
FNodeParamPort::addData(FDataHandle* dataHdl, DagNodeOutPort* dagOutPort)
    throw (WfDataException, WfDataHandleException)
{
  // set the data handle value (if no split)
  if ((dataHdl->getDepth() == depth) && !dataHdl->isValueDefined()) {
    dataHdl->downloadValue();
  }
  // call parent method
  FNodeInPort::addData(dataHdl,dagOutPort);
}

void
FNodeParamPort::instanciate(Dag* dag, DagNode* nodeInst, FDataHandle* dataHdl) {
}

/*****************************************************************************/
/*                           FNodePortMap                                    */
/*****************************************************************************/

FNodePortMap::FNodePortMap() {
}

void
FNodePortMap::mapPorts(FNodeOutPort* outPort, FNodeInPort* inPort) {
  myPortMap.insert(pair<FNodeOutPort*,FNodeInPort*>(outPort,inPort));
}

void
FNodePortMap::mapPortToVoid(FNodeOutPort* outPort) {
  myPortMap.insert(pair<FNodeOutPort*,FNodeInPort*>(outPort,NULL));
}

void
FNodePortMap::applyMap(const FDataTag& tag, const vector<FDataHandle*>& dataLine) {
  for (map<FNodeOutPort*,FNodeInPort*>::const_iterator iter = myPortMap.begin();
       iter != myPortMap.end();
       ++iter) {
     FNodeOutPort* outPort = (FNodeOutPort*) iter->first;
     FDataHandle* dataHdl;
     if (iter->second) {
       FNodeInPort* inPort = (FNodeInPort*) iter->second;
       if (inPort->getIndex() < dataLine.size()) {
         // COPY the input DH (with all its child tree)
         dataHdl = new FDataHandle(*dataLine[inPort->getIndex()]);
       } else {
         INTERNAL_ERROR("applyMap : port index out of bound",1);
       }
     } else {
       // CREATE a new VOID datahandle
        dataHdl = new FDataHandle(tag, outPort->getDepth(), NULL, NULL, true);
     }
     // Submit to out port (ie will send it to connected ports)
     outPort->instanciate(dataHdl);
   }
}
