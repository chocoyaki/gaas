/****************************************************************************/
/* The class used to represent a data produced or consumed by an instance   */
/* of a functional workflow node
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

#include <sstream>
#include "debug.hh"
#include "FDataHandle.hh"
#include "DagNode.hh"
#include "Dag.hh"


/*****************************************************************************/
/*                            FDataTag class                                 */
/*****************************************************************************/


FDataTag::FDataTag() : mySize(0), myStr() {
//   cout << "FDataTag default constructor" << endl;
}

void
FDataTag::initStr() {
  ostringstream ss;
  for (int ix=0; ix < mySize; ++ix) {
    ss << "_" << myIdxs[ix];
  }
  myStr = ss.str();
}

FDataTag::FDataTag(const FDataTag& tag) {
  mySize = tag.mySize;
  if (mySize>0) {
    myIdxs = new unsigned int[mySize];
    myLastFlags = new bool[mySize];
    for (int ix=0; ix<mySize ; ++ix) {
      myIdxs[ix] = tag.myIdxs[ix];
      myLastFlags[ix] = tag.myLastFlags[ix];
    }
  }
  initStr();
}

FDataTag::FDataTag(unsigned int index, bool isLast)
  : mySize(1) {
  myIdxs = new unsigned int[1];
  myLastFlags = new bool[1];
  myIdxs[0] = index;
  myLastFlags[0] = isLast;
  initStr();
}

FDataTag::FDataTag(const FDataTag& parentTag, unsigned int index, bool isLast) {
  mySize = parentTag.mySize + 1;
  myIdxs = new unsigned int[mySize];
  myLastFlags = new bool[mySize];
  for (int ix=0; ix<mySize ; ++ix) {
    myIdxs[ix] = parentTag.myIdxs[ix];
    myLastFlags[ix] = parentTag.myLastFlags[ix];
  }
  myIdxs[mySize-1] = index;
  myLastFlags[mySize-1] = isLast;
  initStr();
}

FDataTag::FDataTag(const FDataTag& parentTag, const FDataTag& childTag) {
  mySize = parentTag.mySize + childTag.mySize;
  myIdxs = new unsigned int[mySize];
  myLastFlags = new bool[mySize];
  for (int ix=0; ix<parentTag.mySize ; ++ix) {
    myIdxs[ix] = parentTag.myIdxs[ix];
    myLastFlags[ix] = parentTag.myLastFlags[ix];
  }
  for (int ix=0; ix<childTag.mySize ; ++ix) {
    myIdxs[parentTag.mySize + ix] = childTag.myIdxs[ix];
    myLastFlags[parentTag.mySize + ix] = childTag.myLastFlags[ix];
  }
  initStr();
}

/**
 * Constructor for making parent tag
 * The provided tables must have at least 'level' elements
 */
FDataTag::FDataTag(unsigned int * idxTab, bool * lastTab, unsigned int level)
  : mySize(level) {
  myIdxs = new unsigned int[mySize];
  myLastFlags = new bool[mySize];
  for (int ix=0; ix<mySize; ++ix) {
    myIdxs[ix]      = idxTab[ix];
    myLastFlags[ix] = lastTab[ix];
  }
  initStr();
}

FDataTag::~FDataTag() {
  if (mySize > 0) {
    delete [] myIdxs;
    delete [] myLastFlags;
  }
}

unsigned int
FDataTag::getLevel() const {
  return mySize;
}

bool
FDataTag::isLastRec(unsigned int level) const {
  if (level == mySize)
    return myLastFlags[level-1];
  else if (level < mySize)
    return (myLastFlags[level-1] && isLastRec(level+1));
  else {
    INTERNAL_ERROR("isLastRec: wrong arguments",0);
  }
}

bool
FDataTag::isLastOfBranch() const {
  return myLastFlags[mySize-1];
}

bool
FDataTag::isLast() const {
  return isLastRec(1);
}

bool
FDataTag::isEmpty() const {
  return (mySize == 0);
}

unsigned int
FDataTag::getIndex(unsigned int level) const {
  return myIdxs[level-1];
}

unsigned int
FDataTag::getLastIndex() const {
  return myIdxs[mySize-1];
}

unsigned int
FDataTag::getFlatIndex() const {
  int res = 1;
  for (int ix=0; ix < mySize; ++ix) {
    res *= (myIdxs[ix] + 1);
  }
  return res-1;
}

FDataTag *
FDataTag::getLeftPart(unsigned int maxLevel) const {
  if (maxLevel > mySize) return NULL;
  return new FDataTag(myIdxs, myLastFlags, maxLevel);
}

FDataTag *
FDataTag::getRightPart(unsigned int minLevel) const {
  if (minLevel > mySize) return NULL;
  unsigned offset = minLevel - 1;
  return new FDataTag(myIdxs + offset, myLastFlags + offset, mySize - offset);
}

FDataTag
FDataTag::getParent(unsigned int level) const {
  FDataTag parTag(myIdxs, myLastFlags, level);
  return parTag;
}

/**
 * Converts the tag to a string
 * Used to make the IDs of node instances
 * The only constraint is that the generated string should be unique
 */
const string&
FDataTag::toString() const {
// VERSION with unique integers
//   static int toto = 0;
//   return itoa(toto++);
// VERSION with streams ==> BUG (corrupted list)
//   ostringstream ss;
//   for (int ix=0; ix < mySize; ++ix) {
//     ss << "_" << myIdxs[ix];
//   }
//   return ss.str();
// VERSION with itoa
//   string output;
//   for (int ix=0; ix < mySize; ++ix) {
//      output += "_" + itoa(myIdxs[ix]);
//    }
//   return output;
  return myStr;
}

int
operator<( const FDataTag& tag1, const FDataTag& tag2 ) {
//   cout << "COMPARING TAGS: " << tag1.toString() << " AND " << tag2.toString() << endl;
  if (tag1.getLevel() != tag2.getLevel()) {
    INTERNAL_ERROR("Cannot compare tags of different level",0);
  }
  int ix = 0;
  while (ix < tag1.getLevel()) {
    if (tag1.myIdxs[ix] != tag2.myIdxs[ix])
      return (tag1.myIdxs[ix] < tag2.myIdxs[ix]);
    ++ix;
  }
  return 0;
}
/*****************************************************************************/
/*                   WfDataHandleException class                             */
/*****************************************************************************/
string
WfDataHandleException::ErrorMsg() {
  string errorMsg;
  switch(Type()) {
    case eBAD_STRUCT:
      errorMsg = "Data Hdl bad structure (tag="+Tag().toString()+":"+Info(); break;
    case eINVALID_ADAPT:
      errorMsg = "Invalid adapter (tag="+Tag().toString()+":"+Info(); break;
  }
  return errorMsg;
}

/*****************************************************************************/
/*                         FDataHandle class                                 */
/*****************************************************************************/

/**
 * Constructors
 */

FDataHandle::FDataHandle()
  : myTag(), myDepth(0), myParentHdl(NULL), myPort(NULL),
    myCard(0), cardDef(false), adapterDef(false), valueDef(false),
    dataIDDef(false), myAdapterType(ADAPTER_UNDEFINED) {}

FDataHandle::FDataHandle(const FDataTag& tag,
                         unsigned int depth,
                         FDataHandle* parentHdl,
                         WfPort* port)
  : myTag(tag), myDepth(depth), myParentHdl(parentHdl), myPort(port),
    myCard(0), cardDef(false), adapterDef(false), valueDef(false),
    dataIDDef(false), myAdapterType(ADAPTER_UNDEFINED) {
  TRACE_TEXT (TRACE_ALL_STEPS, "Creating data handle : tag = " << tag.toString()
       << " / depth=" << depth << " / parent=" << parentHdl << " / port=" << port << endl);
  if (myDepth > 0)
    myData = new map<FDataTag,FDataHandle*>();
  if ((parentHdl) && parentHdl->isAdapterDefined()) {
    TRACE_TEXT (TRACE_ALL_STEPS, "   / adapter = simple" << endl);
    myAdapterType = ADAPTER_SIMPLE;  // adapter is inherited from parent
  }
  if (myPort) {
    TRACE_TEXT (TRACE_ALL_STEPS, "   / adapter = direct" << endl);
    myAdapterType = ADAPTER_DIRECT;  // adapter defined by port reference
  }
}

FDataHandle::FDataHandle(const FDataTag& tag,
                         const string& value,
                         FDataHandle* parentHdl)
  : myTag(tag), myDepth(0), myValue(value), myParentHdl(parentHdl),
    myPort(NULL), myCard(0), cardDef(false), adapterDef(false), valueDef(true),
    dataIDDef(false), myAdapterType(ADAPTER_UNDEFINED) {
  TRACE_TEXT (TRACE_ALL_STEPS, "Creating data handle : tag = " << myTag.toString()
       << " / value=" << value << " / parent=" << parentHdl << endl);
}

FDataHandle::FDataHandle(unsigned int depth)
  : myTag(), myDepth(depth), myValue(), myParentHdl(NULL),
    myPort(NULL), myCard(0), cardDef(false), adapterDef(false), valueDef(false),
    dataIDDef(false), myAdapterType(ADAPTER_UNDEFINED) {
  TRACE_TEXT (TRACE_ALL_STEPS, "Creating data handle : tag = " << myTag.toString()
       << " / depth=" << depth << endl);
  if (myDepth > 0)
    myData = new map<FDataTag,FDataHandle*>();
}

/**
 * Destructor
 */
FDataHandle::~FDataHandle() {
//   this trace makes sometimes the destructor crash (corrupted linked list)
//   in toString() ...
//   TRACE_TEXT (TRACE_ALL_STEPS,"~FDataHandle() " << myTag.toString()
//                               << " destructor ..." <<  "(" << this << ")" << endl);
  // free data
  if (myDepth > 0) {
    while (! myData->empty() ) {
      FDataHandle * p = myData->begin()->second;
      myData->erase( myData->begin() );
      delete p;
    }
    delete myData;
  }
}

/**
 * Basic get/set
 */

unsigned int
FDataHandle::getDepth() const {
  return myDepth;
}

const FDataTag&
FDataHandle::getTag() const {
  return myTag;
}

void
FDataHandle::setCardinal(unsigned int card) {
  myCard = card;
  cardDef = true;
}

unsigned int
FDataHandle::getCardinal() const {
  return myCard;
}

bool
FDataHandle::isCardinalDefined() const {
  return cardDef;
}

bool
FDataHandle::isParentDefined() const {
  return myParentHdl != NULL;
}

FDataHandle*
FDataHandle::getParent() const {
  return myParentHdl;
}

void
FDataHandle::setParent(FDataHandle* parentHdl) {
  myParentHdl = parentHdl;
}

bool
FDataHandle::isAdapterDefined() const {
  return (myAdapterType != ADAPTER_UNDEFINED);
}

/**
 * (RECURSIVE)
 * Insert a data handle inside the tree
 * Note: level 0 is the root level, and level increases in the tree
 */
void
FDataHandle::insertInTree(FDataHandle* dataHdl)
  throw (WfDataHandleException) {
  TRACE_TEXT (TRACE_ALL_STEPS, "insertInTree (current tag=" << getTag().toString()
       << " / depth=" << myDepth << ") ..." << endl);
  if (myDepth == 0)
    throw WfDataHandleException(WfDataHandleException::eBAD_STRUCT,
                              "Cannot insert into non-container data handle (depth=0)",
                              this->getTag());
  const FDataTag& dataTag = dataHdl->getTag();
  unsigned int dataLevel = dataTag.getLevel();
  unsigned int myLevel = myTag.getLevel();
  if (dataLevel <= myLevel)
    throw WfDataHandleException(WfDataHandleException::eBAD_STRUCT,
                              "Tried to insert data handle of same or lower level",
                              this->getTag());
  if (dataLevel == myLevel+1) {
    // data is a direct child of object ==> insert it as a child
    // if the inserted data is at the completion level then set it as complete
    this->addChild(dataHdl);
  } else {
    // data is a sub-child of object ==> look for the child that is data's parent
    // and calls recursively insertInTree
    FDataTag childTag = dataTag.getParent(myLevel+1);
    FDataHandle* childHdl = NULL;
    map<FDataTag,FDataHandle*>::iterator parIter = myData->find(childTag);
    if (parIter != myData->end()) {
      // child exists already
      childHdl = (FDataHandle*) parIter->second;
    } else {
      // child does not exist yet => create it
      unsigned int childDepth = dataHdl->getDepth()
                                  + dataHdl->getTag().getLevel()
                                  - this->getTag().getLevel() - 1;
      TRACE_TEXT (TRACE_ALL_STEPS, "adding intermediate data" << endl);
      childHdl = new FDataHandle(childTag, childDepth, this, NULL);
      // insert the child of current data (without checking if complete)
      this->addChild(childHdl);
    }
    childHdl->insertInTree(dataHdl);
  }
}

void
FDataHandle::addChild(FDataHandle* dataHdl) {
  TRACE_TEXT (TRACE_ALL_STEPS, "adding data child (tag="
      << dataHdl->getTag().toString() << ")" << endl);
  myData->insert(make_pair(dataHdl->getTag(),dataHdl));
  dataHdl->setParent(this);
  if (dataHdl->isLastChild() && !isCardinalDefined()) {
    unsigned int card = dataHdl->getTag().getLastIndex() + 1;
    TRACE_TEXT (TRACE_ALL_STEPS," LAST child added => Updating cardinal : "
                                << card << endl);
    setCardinal(card);
  }
  // updates the adapter status of parents
  dataHdl->getParent()->checkAdapter();
  // display the tree
//   display(true);
}

bool
FDataHandle::isLastChild() const {
  return getTag().isLastOfBranch();
}

/**
 * (RECURSIVE : goes up the tree if one level is complete)
 * Check on all childs if adapter is defined
 */
void
FDataHandle::checkAdapter() {
  TRACE_TEXT (TRACE_ALL_STEPS, "checking adapter for " << getTag().toString()
      << " (parent = " << myParentHdl << ")" << endl);
  if (!isAdapterDefined()) {
    if (!isCardinalDefined() || (myData->size() != myCard)) {
      TRACE_TEXT (TRACE_ALL_STEPS, "cardinal undefined ==> cannot update adapter" << endl);
      return;
    }
    bool allAdaptersDefined = true;
    map<FDataTag,FDataHandle*>::iterator childIter = myData->begin();
    while (allAdaptersDefined && childIter != myData->end()) {
      FDataHandle * childData = (FDataHandle*) childIter->second;
      if (!childData->isAdapterDefined()) {
        allAdaptersDefined = false;
      }
      ++childIter;
    }
    if (allAdaptersDefined) {
      this->myAdapterType = ADAPTER_MULTIPLE;
      TRACE_TEXT (TRACE_ALL_STEPS,"**** adapter for " << getTag().toString()
                  << " is MULTIPLE ****" << endl);
      if (isParentDefined()) {
        myParentHdl->checkAdapter();
      }
    }
  }
}

/**
 * Check if the tree is complete at a given level (level >= 1)
 */
bool
FDataHandle::checkIfComplete(unsigned int level,
                             vector<unsigned int>& childNbTable) {
  TRACE_TEXT (TRACE_ALL_STEPS, " checking if level " << level
      << " is complete..." << endl);
  // check if current total for given level is already stored
  if (childNbTable.size() > level) {
    return true;
  }
  unsigned int childTotal = 0;
  if (checkIfCompleteRec(level, childTotal)) {
    childNbTable.resize(level+1);
    childNbTable[level] = childTotal;
    TRACE_TEXT (TRACE_ALL_STEPS,"**** buffer " << getTag().toString()
                  << " is complete at level " << level << " ("
                  << childTotal << " childs) ****" << endl);
    // updates the adapter when it is undefined (ie this is necessary a data hdl
    // that merges several ports so the adapter should be multiple)
//     if (!isAdapterDefined())
//       this->myAdapterType = ADAPTER_MULTIPLE;
    return true;
  } else
    return false;
}

/**
 * Recursive check used by preceding method
 */
bool
FDataHandle::checkIfCompleteRec(unsigned int level, unsigned int& total) {
//   TRACE_TEXT (TRACE_ALL_STEPS, "@@@@ REC checkIfComplete (data=" << getTag().toString()
//        << " / total=" << total << ")" << endl);
  // check if current data is not locally complete
  if (!isCardinalDefined() || (myData->size() != myCard)) return false;
  // recursion stops when current data's level is at level-1
  if (this->getTag().getLevel() == level-1) {
    // nb of childs is added to the total
    total += myCard;
    return true;
  }
  // recursion continues by checking all childs
  bool allChildsComplete = true;
  map<FDataTag,FDataHandle*>::iterator childIter = myData->begin();
  while (allChildsComplete && childIter != myData->end()) {
    FDataHandle * childData = (FDataHandle*) childIter->second;
    if (!childData->checkIfCompleteRec(level, total))
      allChildsComplete = false;
    ++childIter;
  }
  return allChildsComplete;
}

map<FDataTag,FDataHandle*>::iterator
FDataHandle::begin() throw (WfDataHandleException) {
  // check that data handle is a container
  if (myDepth < 1) {
    throw WfDataHandleException(WfDataHandleException::eBAD_STRUCT,
                          "Tried to get elements of non-container data handle",myTag);
  }
  // if data handle linked to an existing dag port (ie adapter defined)
  // and if the cardinal is defined then the first call to begin will
  // create the childs
  if (isAdapterDefined() && isCardinalDefined() && (myData->size() == 0)) {
//     TRACE_TEXT (TRACE_ALL_STEPS,"Creating childs of data " << getTag().toString() << endl);
    for (int ix=0; ix < myCard; ++ix) {
      FDataTag  childTag(myTag,ix, (ix == myCard-1));
      FDataHandle* childHdl = new FDataHandle(childTag, myDepth-1, this, NULL);
      this->addChild(childHdl);
    }
  }
  return myData->begin();
}

map<FDataTag,FDataHandle*>::iterator
FDataHandle::end() {
  return myData->end();
}

WfPortAdapter*
FDataHandle::createPortAdapter(const string& currDagName) {
  WfPortAdapter * myAdapter = NULL;
  if (myAdapterType == ADAPTER_DIRECT) {
    // LINK TO OUTPUT PORT WITHOUT SPLIT
    // provide dagName only if different from the dag of this handle's port
    DagNode * endNode = dynamic_cast<DagNode*>(myPort->getParent());
    string dagName;
    string endNodeDagName = endNode->getDag()->getId();
    if (endNodeDagName != currDagName)
      dagName = endNodeDagName;
    myAdapter = new WfSimplePortAdapter(myPort,dagName);

  } else if (myAdapterType == ADAPTER_SIMPLE) {
    // LINK TO OUTPUT PORT WITH SPLIT (use indexes)
    // get parent port adapter that is necessarily a simple port adapter
    // (my parent adapter type can either be DIRECT or SIMPLE but not MULTIPLE)
    if (myParentHdl->myAdapterType == ADAPTER_MULTIPLE) {
      INTERNAL_ERROR("Wrong port adapter structure: multiple parent of simple",1);
    }
    WfPortAdapter * parentAdapterBase = myParentHdl->createPortAdapter(currDagName);
    WfSimplePortAdapter * parentAdapter = dynamic_cast<WfSimplePortAdapter*>(parentAdapterBase);
    if (parentAdapter == NULL) {
      INTERNAL_ERROR("Wrong port adapter structure",0);
    }
    // add the last index of my tag at the end
    myAdapter = new WfSimplePortAdapter(parentAdapter,
                                        myTag.getIndex(myTag.getLevel()));
    delete parentAdapter;

  } else if (myAdapterType == ADAPTER_MULTIPLE) {
    // LINK TO SEVERAL OUTPUT PORTS (MERGE)
    // get all my child adapters and create a multiple adapter from them
    WfMultiplePortAdapter* multAdapter = new WfMultiplePortAdapter();
    for (map<FDataTag,FDataHandle*>::iterator childIter = myData->begin();
         childIter != myData->end();
         ++childIter) {
      FDataHandle * childHdl = (FDataHandle*) childIter->second;
      multAdapter->addSubAdapter(childHdl->createPortAdapter(currDagName));
    }
    myAdapter = (WfPortAdapter*) multAdapter;
  }
  return myAdapter;
}

bool
FDataHandle::isValueDefined() const {
  return valueDef;
}

const string&
FDataHandle::getValue() const {
  return myValue;
}

bool
FDataHandle::isDataIDDefined() const {
  return dataIDDef;
}

const string&
FDataHandle::getDataID() const {
  return myDataID;
}

void
FDataHandle::setDataID(const string& dataID) {
  if (!dataID.empty()) {
    myDataID = dataID;
    dataIDDef = true;
  }
}

/**
 * Method to display data handle value after execution (for sink nodes)
 * Creates a temporary port adapter and calls displayData()
 * on this adapter
 */
void
FDataHandle::displayDataAsList(ostream& output) {
  if (isValueDefined()) {
    output << myValue;
  } else if (isAdapterDefined()) {
    WfPortAdapter *adapter = createPortAdapter();
    adapter->displayDataAsList(output);
    delete adapter;
  } else { // no adapter and no static value
    output << "<error>";
  }
}

/**
 * display used for debug
 */
void
FDataHandle::display(bool goUp) {
  if (goUp && myParentHdl)
    myParentHdl->display(true);
  else {
    if (getTag().getLevel() == 0) cout << "-------------------" << endl;
    cout << "DATA: " << getTag().toString()
        << " (depth=" << myDepth << ")";
//     if (myPort)
//       cout << " PORT=" << myPort->getParent()->getId() << "#" << myPort->getId();
// (PORT COULD BE DESTROYED BEFORE DATAHANDLE POINTING TO IT!!)
    if (isValueDefined())
      cout << " VALUE=" << myValue;
    cout << endl;
    if (myDepth>0) {
      for (map<FDataTag,FDataHandle*>::iterator childIter = myData->begin();
           childIter != myData->end();
           ++childIter)
        ((FDataHandle*)childIter->second)->display(false);
      }
    if (getTag().getLevel() == 0) cout << "-------------------" << endl;
  }
}

