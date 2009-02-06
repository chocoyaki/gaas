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
#include "DagNodePort.hh"
#include "Dag.hh"

/**
 * Constructors & destructors
 */

FNodePort::FNodePort (Node * parent,
                      const string& _id,
                      WfPort::WfPortType _portType,
                      WfCst::WfDataType _type,
                      unsigned int _depth,
                      unsigned int _ind)
  : WfPort(parent, _id, _portType, _type, _depth, _ind) {
}

FNodePort::~FNodePort() {
}

FNodeOutPort::FNodeOutPort(Node * parent,
                           const string& _id,
                           WfCst::WfDataType _type,
                           unsigned int _depth,
                           unsigned int _ind)
  : FNodePort(parent, _id, WfPort::PORT_OUT, _type, _depth, _ind)
    , myBuffer(_depth+1) {
}

FNodeOutPort::~FNodeOutPort() {
}

FNodeInPort::FNodeInPort(Node * parent,
                           const string& _id,
                           WfCst::WfDataType _type,
                           unsigned int _depth,
                           unsigned int _ind)
  : FNodePort(parent, _id, WfPort::PORT_IN, _type, _depth, _ind),
    dataTotalNb(0), totalDef(false) { }

FNodeInPort::~FNodeInPort() {
}

/**
 * Ports connection setup method
 * Called by WfSimpleAdapter on both sides to connect node ports
 */

FProcNode*
FNodePort::getParentProcNode() {
  FProcNode* parent = dynamic_cast<FProcNode*>(getParent());
  if (parent == NULL) {
    INTERNAL_ERROR("Invalid type of parent node for FNodePort " << getId() << endl,1);
  }
  return parent;
}

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

void
FNodeOutPort::instanciate(Dag* dag, DagNode* nodeInst, const FDataTag& tag) {
  // create the portInst (DagNodeOutPort) for the nodeInst
  string portId = this->getId();
  TRACE_TEXT (TRACE_ALL_STEPS,"   # Creating new instance of OUT port: " << portId << endl);
  WfPort* portInst = nodeInst->newPort(portId,
                                       index,
                                       portType,
                                       getBaseDataType(),
                                       depth);
  // create a FDataHandle corresponding to the new port instance
  //  - its depth equals the depth of the out port
  //  - its tag equals the tag of the node instance
  //  - its port equals the portInst
  TRACE_TEXT (TRACE_ALL_STEPS,"   # Creating new data handle / tag = " << tag.toString() << endl);
  FDataHandle* dataHdl = new FDataHandle(tag, depth, NULL, portInst);
  // insert it under the root data handle  ==> myBuffer.insertInTree()
  // the intermediate nodes (if needed) are created on the fly if they are missing.
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
    FNodeInPort *inPort = (FNodeInPort*) *inPortIter;
    if (!addDataToInPort(inPort, dataHdl, cardList)) {
      TRACE_TEXT (TRACE_ALL_STEPS,"  - in port (" << inPort->getParent()->getId()
                  << "#" << inPort->getId() << ") insertion pending" << endl);
      FProcNode * parentFNode = getParentProcNode();
      parentFNode->setPendingInstanceInfo(nodeInst,dataHdl,this,inPort);
    }
  }
}

void
FNodeOutPort::instanciate(FDataHandle* dataHdl) {
  // if the dataHandle has a value then add it to the connected in ports
  TRACE_TEXT (TRACE_ALL_STEPS,"   # Insert data into buffer" << endl);
  myBuffer.insertInTree(dataHdl);
  TRACE_TEXT (TRACE_ALL_STEPS,"   # Insert data into connected ports" << endl);
  list<string> dummyCard;
  for (list<FNodeInPort*>::iterator inPortIter = myConnectedPorts.begin();
       inPortIter != myConnectedPorts.end();
       ++inPortIter) {
    if (!addDataToInPort((FNodeInPort*) *inPortIter, dataHdl, dummyCard)) {
      INTERNAL_ERROR("Cannot split source data",1);
    }
  }
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

bool
FNodeOutPort::addDataToInPort(FNodeInPort *inPort,
                              FDataHandle *dataHdl,
                              const list<string>& dataCard)
    throw (WfDataHandleException)
{
  // try to insert the data in the inPort
  // (fails in case of missing cardinal information)
  if (!inPort->addData(dataHdl, dataCard)) {
    return false;
  } else {
    checkTotalDataNb(inPort, dataHdl);
    return true;
  }
}

void
FNodeOutPort::addDataToInPort(FNodeInPort *inPort,
                              FDataHandle *dataHdl,
                              DagNodeOutPort * dagOutPort)
    throw (WfDataException, WfDataHandleException)
{
  inPort->addData(dataHdl, dagOutPort);
  checkTotalDataNb(inPort, dataHdl);
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

/**
 * Static addData
 */
bool
FNodeInPort::addData(FDataHandle* dataHdl, const list<string>& dataCard)
   throw (WfDataHandleException)
{
  TRACE_TEXT (TRACE_ALL_STEPS,"     # Calling addData (port " << getParent()->getId()
                              << "#" << getId()
                              << ") for data: " << dataHdl->getTag().toString() << endl);
  // if dataHdl is not complete then do nothing
  if (!dataHdl->isAdapterDefined() && !dataHdl->isValueDefined()) {
    TRACE_TEXT (TRACE_ALL_STEPS,"Cannot add data to inPort (no value and no adapter)" << endl);
    return true;
  }
  // if dataHdl depth = port depth
    // if dataHdl not already in the queue
      // add to my queue
  if (dataHdl->getDepth() == depth) {
    const FDataTag& dataTag = dataHdl->getTag();
    if (myQueue.find(dataTag) == myQueue.end()) {
      TRACE_TEXT (TRACE_ALL_STEPS,"Adding data in input port queue (tag="
                  << dataTag.toString() << ")" << endl);
      myQueue.insert(make_pair(dataTag, dataHdl));
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
      // loop over all elements of the data handle (may throw exception)
      for (map<FDataTag,FDataHandle*>::iterator eltIter = dataHdl->begin();
           eltIter != dataHdl->end();
           ++eltIter) {
        if (!this->addData((FDataHandle*) eltIter->second, childCard))
          return false;
      }
    } else {
      TRACE_TEXT (TRACE_ALL_STEPS,"Cannot split data (cardinal unknown)" << endl);
      return false;
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
  return true;
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
          childHdl->setDataID(content->elt_ids[ix++]);
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
  getParentProcNode()->setConstantInput(getIndex(), dataHdl);
}

void
FNodeInPort::instanciate(Dag* dag, DagNode* nodeInst, FDataHandle* dataHdl) {
  // create a DagNodeInPort for the nodeInst
  string portId = this->getId();
  TRACE_TEXT (TRACE_ALL_STEPS,"Creating new instance of IN port: " << portId << endl);
  WfPort* portInst = nodeInst->newPort(portId, index, portType, getBaseDataType(), depth);
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


