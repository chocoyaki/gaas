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
 * Revision 1.9  2009/05/27 08:54:43  bisnard
 * - modified FDataTag comparison op (lexicographic order)
 * - new FDataHandle copy constructor for condition nodes
 *
 * Revision 1.8  2009/05/15 11:10:20  bisnard
 * release for workflow conditional structure (if)
 *
 * Revision 1.7  2009/04/17 09:04:07  bisnard
 * initial version for conditional nodes in functional workflows
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
  for (int ix=0; ix<mySize-1 ; ++ix) {
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
  if (mySize > 0) {
    myIdxs = new unsigned int[mySize];
    myLastFlags = new bool[mySize];
    for (int ix=0; ix<mySize; ++ix) {
      myIdxs[ix]      = idxTab[ix];
      myLastFlags[ix] = lastTab[ix];
    }
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
  if (isEmpty()) {
    INTERNAL_ERROR("Cannot test if last for empty tag",1);
  }
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
  if (isEmpty()) {
    INTERNAL_ERROR("Cannot get index of empty tag",1);
  }
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
FDataTag::getAncestor(unsigned int level) const {
  FDataTag parTag(myIdxs, myLastFlags, level);
  return parTag;
}

FDataTag&
FDataTag::getParent() {
  if (mySize == 0) {
    INTERNAL_ERROR("Cannot get parent of root tag",1);
  }
  mySize -= 1;
  initStr();
  return *this;
}

FDataTag&
FDataTag::getPredecessor() {
  if (getLastIndex() == 0) {
    INTERNAL_ERROR("Cannot get predecessor of tag",1);
  }
  myIdxs[mySize-1] -= 1;
  initStr();
  return *this;
}

FDataTag&
FDataTag::getSuccessor() {
  if (isLastOfBranch()) {
    INTERNAL_ERROR("Cannot get successor of tag",1);
  }
  myIdxs[mySize-1] += 1;
  initStr();
  return *this;
}

/**
 * Converts the tag to a string
 * Used to make the IDs of node instances
 * The only constraint is that the generated string should be unique
 */
const string&
FDataTag::toString() const {
  return myStr;
}

int
operator<( const FDataTag& tag1, const FDataTag& tag2 ) {
//    cout << "COMPARING TAGS: " << tag1.toString() << " AND " << tag2.toString() << endl;
  int ix = 0;
  while ((ix < tag1.getLevel()) && (ix < tag2.getLevel())) {
    if (tag1.myIdxs[ix] != tag2.myIdxs[ix])
      return (tag1.myIdxs[ix] < tag2.myIdxs[ix]);
    ++ix;
  }
  return (tag1.getLevel() < tag2.getLevel());
}

/*****************************************************************************/
/*                   WfDataHandleException class                             */
/*****************************************************************************/
string
WfDataHandleException::ErrorMsg() {
  string errorMsg;
  switch(Type()) {
    case eBAD_STRUCT:
      errorMsg = "Data Hdl bad structure ("+Info()+")"; break;
    case eINVALID_ADAPT:
      errorMsg = "Invalid adapter ("+Info()+")"; break;
    case eCARD_UNDEF:
      errorMsg = "Cardinal of DH undefined ("+Info()+")"; break;
    case eVALUE_UNDEF:
      errorMsg = "Value of DH undefined ("+Info()+")";; break;
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
  : myTag(), myDepth(0), myParentHdl(NULL), myPort(NULL), myPortLevel(0),
    myCard(0), cardDef(false), adapterDef(false), valueDef(false),
    dataIDDef(false), myAdapterType(ADAPTER_UNDEFINED), myCardList(NULL) {}

FDataHandle::FDataHandle(const FDataTag& tag,
                         unsigned int depth,
                         bool isVoid,
                         FDataHandle* parentHdl,
                         DagNodeOutPort* port,
                         unsigned int portLevel)
  : myTag(tag), myDepth(depth), myParentHdl(parentHdl), myPort(port),
    myPortLevel(portLevel), myCard(0), cardDef(false), adapterDef(false),
    valueDef(false), dataIDDef(false), myAdapterType(ADAPTER_UNDEFINED),
    myCardList(NULL) {
//   TRACE_TEXT (TRACE_ALL_STEPS, "Creating data handle : tag = " << tag.toString()
//        << " / depth=" << depth << " / parent=" << parentHdl << " / port=" << port
//        << " / portLvl=" << portLevel << endl);
  if (myDepth > 0)
    myData = new map<FDataTag,FDataHandle*>();

  if ((parentHdl) && parentHdl->isAdapterDefined()) {
    // adapter is inherited from parent
    if ((parentHdl->myAdapterType == ADAPTER_DIRECT)
        || (parentHdl->myAdapterType == ADAPTER_SIMPLE)) {
//       TRACE_TEXT (TRACE_ALL_STEPS, "   / adapter = simple (inherited)" << endl);
      myAdapterType = ADAPTER_SIMPLE;
      // inherit the port information
      myPort = parentHdl->myPort;
      myPortLevel = parentHdl->myPortLevel;
    } else if (parentHdl->isVoid()) {
      // VOID IS PROPAGATED TO CHILDS AUTOMATICALLY (at construction time only)
      isVoid = true;
    }
  }
  if ((myPort) && (myAdapterType == ADAPTER_UNDEFINED)) {
    if (!portLevel) {
      INTERNAL_ERROR("Error in DH constructor : missing port level",1);
    }
    if (portLevel == tag.getLevel()) {
//       TRACE_TEXT (TRACE_ALL_STEPS, "   / adapter = direct" << endl);
      myAdapterType = ADAPTER_DIRECT;  // adapter defined by port reference
    } else {
//       TRACE_TEXT (TRACE_ALL_STEPS, "   / adapter = simple" << endl);
      myAdapterType = ADAPTER_SIMPLE;
    }
  }
  if (isVoid) {
//     TRACE_TEXT (TRACE_ALL_STEPS, "   / adapter = VOID" << endl);
    myAdapterType = ADAPTER_VOID;
    if (depth > 0)
      setCardinal(1); // by default a VOID container DH has 1 element
  }
}

FDataHandle::FDataHandle(const FDataTag& tag,
                         const string& value)
  : myTag(tag), myDepth(0), myValue(value), myParentHdl(NULL), myPortLevel(0),
    myPort(NULL), myCard(0), cardDef(false), adapterDef(false), valueDef(true),
    dataIDDef(false), myAdapterType(ADAPTER_UNDEFINED), myCardList(NULL) {
//   TRACE_TEXT (TRACE_ALL_STEPS, "Creating data handle : tag = " << myTag.toString()
//        << " / value=" << value << endl);
}

FDataHandle::FDataHandle(unsigned int depth)
  : myTag(), myDepth(depth), myValue(), myParentHdl(NULL), myPortLevel(0),
    myPort(NULL), myCard(0), cardDef(false), adapterDef(false), valueDef(false),
    dataIDDef(false), myAdapterType(ADAPTER_UNDEFINED), myCardList(NULL) {
//   TRACE_TEXT (TRACE_ALL_STEPS, "Creating data handle : tag = " << myTag.toString()
//        << " / depth=" << depth << endl);
  if (myDepth > 0)
    myData = new map<FDataTag,FDataHandle*>();
}

// Copy constructor
// FDataHandle::FDataHandle(const FDataHandle& src)
//   : myTag(src.myTag), myDepth(src.myDepth), myValue(src.myValue), myParentHdl(NULL),
//     myPortLevel(src.myPortLevel), myPort(src.myPort), myCard(src.myCard),
//     cardDef(src.cardDef), adapterDef(src.adapterDef), valueDef(src.valueDef),
//     dataIDDef(src.dataIDDef), myAdapterType(src.myAdapterType), myCardList(NULL),
//     myDataID(src.myDataID) {
// //   TRACE_TEXT (TRACE_ALL_STEPS, "Creating data handle COPY of "
// //                                << src.myTag.toString() << endl);
//   if (myDepth > 0)
//     myData = new map<FDataTag,FDataHandle*>();
//   if (myAdapterType == ADAPTER_MULTIPLE) {
//     if (!cardDef) {
//       INTERNAL_ERROR("Cannot copy a MULTIPLE DH if cardinal not defined",1);
//     }
//     if (src.myCardList) {
//       myCardList = new list<string>(*src.myCardList);
//     }
//     for (map<FDataTag,FDataHandle*>::const_iterator srcEltIter = src.myData->begin();
//          srcEltIter != src.myData->end();
//          ++srcEltIter) {
//       FDataHandle *destElt = new FDataHandle(*((FDataHandle*) srcEltIter->second));
//       myData->insert(pair<FDataTag,FDataHandle*>(destElt->getTag(),destElt));
//     }
//   }
// }

FDataHandle::FDataHandle(const FDataTag& tag, const FDataHandle& src)
  : myTag(tag), myDepth(src.myDepth), myValue(src.myValue), myParentHdl(NULL),
    myPortLevel(src.myPortLevel), myPort(src.myPort), myCard(src.myCard),
    cardDef(src.cardDef), adapterDef(src.adapterDef), valueDef(src.valueDef),
    dataIDDef(src.dataIDDef), myAdapterType(src.myAdapterType), myCardList(NULL),
    myDataID(src.myDataID) {
//   TRACE_TEXT (TRACE_ALL_STEPS, "Creating data handle COPY of "
//                                << src.myTag.toString() << endl);
  if (myDepth > 0)
    myData = new map<FDataTag,FDataHandle*>();
  if (myAdapterType == ADAPTER_MULTIPLE) {
    if (!cardDef) {
      INTERNAL_ERROR("Cannot copy a MULTIPLE DH if cardinal not defined",1);
    }
    if (src.myCardList) {
      myCardList = new list<string>(*src.myCardList);
    }
    for (map<FDataTag,FDataHandle*>::const_iterator srcEltIter = src.myData->begin();
         srcEltIter != src.myData->end();
         ++srcEltIter) {
      FDataHandle *srcElt = (FDataHandle*) srcEltIter->second;
      // Create new child tag based on new tag and last index of the src child
      const FDataTag& srcTag = srcElt->getTag();
      FDataTag  *destTag = new FDataTag(tag, srcTag.getLastIndex(), srcTag.isLast());
      FDataHandle *destElt = new FDataHandle(*destTag, *srcElt);
      // Add new child
      myData->insert(pair<FDataTag,FDataHandle*>(*destTag,destElt));
      delete destTag;
    }
  }
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
    if (myCardList) delete myCardList;
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

//private
void
FDataHandle::setCardinal(unsigned int card) {
  myCard = card;
  cardDef = true;
}

void
FDataHandle::setCardinalList(const list<string>& cardList) {
  if (!cardList.empty()) {
    // if the top element is defined, use it to set the cardinal
    const string& cardStr = cardList.front();
    if (cardStr != "x") {
      setCardinal(atoi(cardStr.c_str()));
    }
    myCardList = new list<string>(cardList);
  }
}

void
FDataHandle::setCardinalList(list<string>::const_iterator& start,
                             list<string>::const_iterator& end) {
  string cardStr = *start;
  if (cardStr != "x") {
      setCardinal(atoi(cardStr.c_str()));
  }
  myCardList = new list<string>(start,end);
}

unsigned int
FDataHandle::getCardinal() const throw (WfDataHandleException) {
  if (isCardinalDefined())
    return myCard;
  else
    throw WfDataHandleException(WfDataHandleException::eCARD_UNDEF,
                                "DH Cardinal undefined - tag="
                                + getTag().toString());
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

bool
FDataHandle::isPortDefined() const {
  return ((myAdapterType == ADAPTER_DIRECT)
            || (myAdapterType == ADAPTER_SIMPLE));
}

DagNodeOutPort*
FDataHandle::getSourcePort() const throw (WfDataHandleException) {
  if (isPortDefined())
    return myPort;
  else
    throw WfDataHandleException(WfDataHandleException::eINVALID_ADAPT,
                                "Cannot get source port - tag="
                                         + getTag().toString());
}

// private
unsigned int
FDataHandle::getPortLevel() const {
  return myPortLevel;
}

// private
void
FDataHandle::setParent(FDataHandle* parentHdl) {
  myParentHdl = parentHdl;
}

bool
FDataHandle::isVoid() const {
  return (myAdapterType == ADAPTER_VOID);
}

// private
void
FDataHandle::setAsVoid() {
  myAdapterType = ADAPTER_VOID;
  if (myDepth > 0)
      setCardinal(1);
}

bool
FDataHandle::isAdapterDefined() const {
  return (myAdapterType != ADAPTER_UNDEFINED);
}

/**
 * (RECURSIVE)
 * Insert a data handle inside the object's data tree
 * Note: level 0 is the root level, and level increases in the tree
 */
void
FDataHandle::insertInTree(FDataHandle* dataHdl)
  throw (WfDataHandleException) {
  if (!dataHdl) {
    INTERNAL_ERROR("FDataHandle::insertInTree : NULL input data handle",1);
  }
  if (myDepth == 0)
    throw WfDataHandleException(WfDataHandleException::eBAD_STRUCT,
                              "Cannot insert into non-container data handle (depth=0)");
  const FDataTag& dataTag = dataHdl->getTag();
  unsigned int dataLevel = dataTag.getLevel();
  unsigned int myLevel = myTag.getLevel();

  if (dataLevel <= myLevel)
    throw WfDataHandleException(WfDataHandleException::eBAD_STRUCT,
                              "Tried to insert data handle of same or lower level");

  if (dataLevel == myLevel+1) { // data is a direct child of current DH

    this->addChild(dataHdl);

  } else {    // data is a descendant of current DH

    // look for its ancestor (direct child of current DH)
    FDataTag ancestorTag = dataTag.getAncestor(myLevel+1);
    FDataHandle* ancestorHdl = NULL;
    map<FDataTag,FDataHandle*>::iterator parIter = myData->find(ancestorTag);

    if (parIter != myData->end()) { // ancestor exists already
      ancestorHdl = (FDataHandle*) parIter->second;

    } else {  // ancestor does not exist yet => create it
      unsigned int    childDepth = dataHdl->getDepth() + dataLevel - myLevel - 1;
      DagNodeOutPort* port = NULL;
      unsigned int    portLevel = 0;

      ancestorHdl = new FDataHandle(ancestorTag, childDepth, false, this, port, portLevel);
      addChild(ancestorHdl);
    }

    // call recursively insertInTree on the ancestor
    ancestorHdl->insertInTree(dataHdl);
  }
}

void
FDataHandle::addChild(FDataHandle* dataHdl) {

  // insert the new DH in my child data map
  myData->insert(pair<FDataTag,FDataHandle*>(dataHdl->getTag(),dataHdl));
  dataHdl->setParent(this);

  // update my depth if element inserted is the first one and depth mismatches
  // (actually happens only for the root DH)
  if (dataHdl->getDepth() != myDepth-1) {
    if (myData->size() == 1) {
      myDepth = dataHdl->getDepth() + 1;
    } else {
      INTERNAL_ERROR("Depth mismatch during data handle child insertion",1);
    }
  }
  // update my cardinal if element inserted is the last one
  if (dataHdl->isLastChild() && !isCardinalDefined()) {
    setCardinal(dataHdl->getTag().getLastIndex() + 1);
  }
  // updates the adapter status (recursively until the root of the tree)
  checkAdapter();
}

bool
FDataHandle::isLastChild() const {
  return getTag().isLastOfBranch();
}

/**
 * (RECURSIVE : goes up the tree if one level is complete)
 * Check on all childs if adapter is defined
 * Updates the adapter if all its childs have a defined adapter
 * IMPORTANT: ONLY METHOD THAT CHANGES myAdapterType after construction
 */
void
FDataHandle::checkAdapter() {

  if (!isAdapterDefined()) {
    if (!isCardinalDefined() || (myData->size() != myCard)) {
//       TRACE_TEXT (TRACE_ALL_STEPS, "cardinal undefined or elements still missing "
//                                    << " ==> adapter is undefined" << endl);
      return;
    }
    bool allAdaptersDefined = true;
    bool allAdaptersVoid = true;
    map<FDataTag,FDataHandle*>::iterator childIter = myData->begin();
    while (allAdaptersDefined && childIter != myData->end()) {
      FDataHandle * childData = (FDataHandle*) childIter->second;
      if (!childData->isAdapterDefined()) {
        allAdaptersDefined = false;
      }
      if (!childData->isVoid()) {
        allAdaptersVoid = false;
      }
      ++childIter;
    }
    if (allAdaptersDefined) {
      if (allAdaptersVoid) {
        myAdapterType = ADAPTER_VOID;
        TRACE_TEXT (TRACE_ALL_STEPS,"**** adapter for " << getTag().toString()
                    << " is VOID ****" << endl);
      } else {
        // this is the only way to set the adapterType to MULTIPLE
        myAdapterType = ADAPTER_MULTIPLE;
        TRACE_TEXT (TRACE_ALL_STEPS,"**** adapter for " << getTag().toString()
                    << " is MULTIPLE ****" << endl);
      }
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
    return true;
  } else {
    return false;
  }
}

/**
 * Recursive check used by preceding method
 * (goes down the data tree to check all levels are complete)
 */
bool
FDataHandle::checkIfCompleteRec(unsigned int level, unsigned int& total) {
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
                          "Tried to get elements of non-container data handle");
  }
  // if the cardinal is defined then the first call to begin will
  // create the childs automatically
  if (isCardinalDefined() && (myData->size() == 0)) {
    for (int ix=0; ix < myCard; ++ix) {
      FDataTag  childTag(getTag(),ix, (ix == myCard-1));
      FDataHandle* childHdl = new FDataHandle(childTag, myDepth-1, false, this);
      this->addChild(childHdl);
      // set the cardinal list (static cardinal info) for childs
      if ((myCardList) && (myCardList->size() > 1)) {
        list<string>::const_iterator cardListStart = ++myCardList->begin();
        list<string>::const_iterator cardListEnd = myCardList->end();
        childHdl->setCardinalList(cardListStart, cardListEnd);
      }
    }
  }
  return myData->begin();
}

map<FDataTag,FDataHandle*>::iterator
FDataHandle::end() {
  return myData->end();
}

/**
 * Port adapter creation (RECURSIVE for MULTIPLE adapters)
 */
WfPortAdapter*
FDataHandle::createPortAdapter(const string& currDagName) {
  WfPortAdapter * myAdapter = NULL;

  if (myAdapterType == ADAPTER_VOID) {
    myAdapter = new WfVoidAdapter();

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

  } else {
    // provide dagName only if different from the dag of this handle's port
    DagNode * endNode = dynamic_cast<DagNode*>(myPort->getParent());
    string dagName;
    string endNodeDagName = endNode->getDag()->getId();
    if (endNodeDagName != currDagName)
      dagName = endNodeDagName;

    if (myAdapterType == ADAPTER_DIRECT) {
      // LINK TO OUTPUT PORT WITHOUT SPLIT
       myAdapter = new WfSimplePortAdapter(myPort,dagName);

    } else if (myAdapterType == ADAPTER_SIMPLE) {
      // LINK TO OUTPUT PORT WITH SPLIT (use indexes)
      list<unsigned int>* adapterIndexes = new list<unsigned int>();
      for (int ix = getPortLevel() + 1; ix <= myTag.getLevel(); ++ix) {
        adapterIndexes->push_back(myTag.getIndex(ix));
      }
      myAdapter = new WfSimplePortAdapter(myPort,*adapterIndexes,dagName);
      delete adapterIndexes;
    }
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

void
FDataHandle::downloadValue() {
  if (isValueDefined()) return;
  TRACE_TEXT (TRACE_ALL_STEPS,"Retrieving value of data..." << endl);
  // first try to update data ID
  try {
    downloadDataID();
  } catch (WfDataException& e) {
    if (e.Type() != WfDataException::eID_UNDEF)
    throw;
  }
  if (isDataIDDefined()) {
    // the following must be called only if the dataID is really defined!
    ostringstream  valStr;
    displayDataAsList(valStr);
    myValue = valStr.str();
    valueDef = true;
    TRACE_TEXT (TRACE_ALL_STEPS,"==> Value is : " << myValue << endl);
  } else if (isVoid()) {
    TRACE_TEXT (TRACE_ALL_STEPS,"==> Value is VOID" << endl);
  } else {
    TRACE_TEXT (TRACE_ALL_STEPS,"==> Value is not available" << endl);
    throw WfDataHandleException(WfDataHandleException::eVALUE_UNDEF,getTag().toString());
  }
}

void
FDataHandle::downloadDataID() {
  if (isDataIDDefined()) return;
  if (isAdapterDefined()) {
    WfPortAdapter *adapter = createPortAdapter();
    try {
      setDataID(adapter->getSourceDataID());
    } catch (WfDataException& e) {
      if (e.Type() == WfDataException::eVOID_DATA)
        setAsVoid();
      else throw;
    }
  }
}

void
FDataHandle::downloadCardinal() {
  if (isCardinalDefined()) return;
  if (myDepth < 1) {
    throw WfDataHandleException(WfDataHandleException::eINVALID_ADAPT,
                                "Cannot get cardinal of 0-depth data - tag="
                                + myTag.toString());
  }
  if (isAdapterDefined()) {
    WfPortAdapter *adapter = createPortAdapter();
    setCardinal(adapter->getSourceDataCardinal());
  } else {
    throw WfDataHandleException(WfDataHandleException::eINVALID_ADAPT,
                                "Cannot get cardinal due to invalid adapter - tag="
                                + myTag.toString());
  }
}

void
FDataHandle::downloadElementDataIDs() {
  downloadDataID();
  downloadCardinal();
  for (map<FDataTag,FDataHandle*>::iterator childIter = myData->begin();
       childIter != myData->end();
       ++childIter) {
    ((FDataHandle*)childIter->second)->downloadDataID();
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

