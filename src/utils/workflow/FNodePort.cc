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
  : FNodePort(parent, _id, WfPort::PORT_IN, _type, _depth, _ind) {
}

FNodeInPort::~FNodeInPort() {
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

void
FNodeOutPort::instanciate(Dag* dag, DagNode* nodeInst, const FDataTag& tag) {
  // create the portInst (DagNodeOutPort) for the nodeInst
  string portId = this->getId();
  TRACE_TEXT (TRACE_ALL_STEPS,"   # Creating new instance of OUT port: " << portId << endl);
  WfPort* portInst = nodeInst->newPort(portId, index, portType, getBaseDataType(), depth);
  // create a FDataHandle corresponding to the new port instance
  //  - its depth equals the depth of the out port
  //  - its tag equals the tag of the node instance
  //  - its port equals the portInst
  TRACE_TEXT (TRACE_ALL_STEPS,"   # Creating new data handle / tag = " << tag.toString() << endl);
  FDataHandle* dataHdl = new FDataHandle(tag, depth, NULL, portInst);
  // insert it under the root data handle  ==> myBuffer.insertInTree()
  // the intermediate nodes (if needed) are created on the fly in status
  // incomplete if they are missing. Their status is set as complete when
  // a direct child is inserted and has a tag marked as last and when all
  // their childs become complete.
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
    FNodeInPort * inPort = (FNodeInPort*) *inPortIter;
    if (!inPort->addData(dataHdl, cardList)) {
      TRACE_TEXT (TRACE_ALL_STEPS,"  - in port (" << inPort->getParent()->getId()
                  << "#" << inPort->getId() << ") insertion pending" << endl);
      FProcNode * parentFNode = dynamic_cast<FProcNode*>(getParent());
      parentFNode->setPendingInstanceInfo(nodeInst,dataHdl,this,inPort);
    }
  }
}

void
FNodeOutPort::instanciate(FDataHandle* dataHdl) {
  // if the dataHandle has a value then set the connected in ports
  // as constant
  cout << "FNodeOutPort::instanciate version NOT IMPLEMENTED!!" << endl;
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
FNodeInPort::addData(FDataHandle* dataHdl, const list<string>& dataCard) {
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
    if (myQueue.find(dataHdl->getTag()) == myQueue.end()) {
      TRACE_TEXT (TRACE_ALL_STEPS,"Adding data in input port queue (tag="
                  << dataHdl->getTag().toString() << ")" << endl);
      myQueue.insert(make_pair(dataHdl->getTag(), dataHdl));
    }
  } else if (dataHdl->getDepth() > depth) {
  // if dataHdl depth > port depth (SPLIT)
    cout << "checking cardinal value" << endl;
    if (!dataCard.empty()) {
      string cardStr = dataCard.front();
      if (cardStr != "x") {
        cout << "cardinal value statically defined" << endl;
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
      // loop over all elements of the data handle
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
FNodeInPort::addData(FDataHandle* dataHdl, DagNodeOutPort* dagOutPort) {
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
      TRACE_TEXT (TRACE_ALL_STEPS,"addData: get ID list from port" << endl);
      diet_container_t* content = dagOutPort->getDataIDList(dataHdl->getDataID());
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
        WARNING("FNodeInPort::addData : Empty container for data ID "
                << dataHdl->getDataID() << endl);
      }
    } else {
      WARNING("FNodeInPort::addData : Missing Data ID in data handle "
              << dataHdl->getTag().toString() << endl);
    }
  }
}

void
FNodeInPort::setAsConstant(FDataHandle* dataHdl) {
  // TEMPORARY (constant is instanciated ONCE for the moment)
  list<string> dummyCard; // FIXME
  this->addData(dataHdl,dummyCard);
}

void
FNodeInPort::instanciate(Dag* dag, DagNode* nodeInst, FDataHandle* dataHdl) {
  // create a DagNodeInPort for the nodeInst
  string portId = this->getId();
  TRACE_TEXT (TRACE_ALL_STEPS,"Creating new instance of IN port: " << portId << endl);
  WfPort* portInst = nodeInst->newPort(portId, index, portType, getBaseDataType(), depth);
  // if the dataHdl has a value then set the value of the port
  if (dataHdl->isValueDefined()) {
    DagNodePort* dagPortInst = dynamic_cast<DagNodePort*>(portInst);
    dagPortInst->setValue(dataHdl->getValue());
  } else if (dataHdl->isAdapterDefined()) {
    // if the dataHdl has a source then set the source of the port
    WfPortAdapter * portAdapter = dataHdl->createPortAdapter(dag->getId());
    portInst->setPortAdapter(portAdapter);
    cout << "ADAPTER = " << portAdapter->getSourceRef() << endl;
  }
}

map<FDataTag, FDataHandle*>::iterator
FNodeInPort::begin() {
  return myQueue.begin();
}

map<FDataTag, FDataHandle*>::iterator
FNodeInPort::end() {
  return myQueue.end();
}

void
FNodeInPort::clearQueue() {
  myQueue.clear();
}

void
FNodeInPort::clearQueue(map<FDataTag, FDataHandle*>::iterator start,
                        map<FDataTag, FDataHandle*>::iterator end) {
  myQueue.erase(start, end);
}
