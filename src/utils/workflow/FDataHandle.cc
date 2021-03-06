/**
* @file FDataHandle.cc
* 
* @brief  Represent a data produced consumed by  a functional workflow node
* 
* @author  Benjamin ISNARD (Benjamin.Isnard@ens-lyon.fr)
* 
* @section Licence
*   |LICENSE|                                                                
*/
/****************************************************************************/
/* The class used to represent a data produced or consumed by an instance   */
/* of a functional workflow node                                            */
/****************************************************************************/


#include <cstring>
#include <sstream>
#include "debug.hh"
#include "FDataHandle.hh"
#include "DagNode.hh"
#include "Dag.hh"
#include "ORBMgr.hh"

/*****************************************************************************/
/*                            FDataTag class                                 */
/*****************************************************************************/


FDataTag::FDataTag() : mySize(0), myStr() {
}

void
FDataTag::initStr() {
  std::ostringstream ss;
  for (unsigned int ix = 0; ix < mySize; ++ix) {
    ss << "_" << myIdxs[ix];
    if (myLastFlags[ix]) {
      ss << "L";
    }
  }
  myStr = ss.str();
}

FDataTag::FDataTag(const FDataTag& tag) {
  mySize = tag.mySize;
  if (mySize>0) {
    myIdxs = new unsigned int[mySize];
    myLastFlags = new bool[mySize];
    for (unsigned int ix = 0; ix < mySize; ++ix) {
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
  for (unsigned int ix = 0; ix < mySize-1; ++ix) {
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
  for (unsigned int ix = 0; ix < parentTag.mySize; ++ix) {
    myIdxs[ix] = parentTag.myIdxs[ix];
    myLastFlags[ix] = parentTag.myLastFlags[ix];
  }
  for (unsigned int ix = 0; ix < childTag.mySize; ++ix) {
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
    for (unsigned int ix = 0; ix < mySize; ++ix) {
      myIdxs[ix] = idxTab[ix];
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
  if (level == mySize) {
    return myLastFlags[level-1];
  } else if (level < mySize) {
    return (myLastFlags[level-1] && isLastRec(level+1));
  } else {
    INTERNAL_ERROR("isLastRec: wrong arguments", 0);
  }
}

bool
FDataTag::isLastOfBranch() const {
  if (isEmpty()) {
    INTERNAL_ERROR("Cannot test if last for empty tag", 1);
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
    INTERNAL_ERROR("Cannot get index of empty tag", 1);
  }
  return myIdxs[mySize-1];
}

unsigned int
FDataTag::getFlatIndex() const {
  int res = 1;
  for (unsigned int ix = 0; ix < mySize; ++ix) {
    res *= (myIdxs[ix] + 1);
  }
  return res-1;
}

FDataTag *
FDataTag::getLeftPart(unsigned int maxLevel) const {
  if (maxLevel > mySize) {
    return NULL;
  }
  return new FDataTag(myIdxs, myLastFlags, maxLevel);
}

FDataTag *
FDataTag::getRightPart(unsigned int minLevel) const {
  if (minLevel > mySize) {
    return NULL;
  }
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
    INTERNAL_ERROR("Cannot get parent of root tag", 1);
  }
  mySize -= 1;
  initStr();
  return *this;
}

FDataTag&
FDataTag::getPredecessor() {
  if (getLastIndex() == 0) {
    INTERNAL_ERROR("Cannot get predecessor of tag", 1);
  }
  myIdxs[mySize-1] -= 1;
  initStr();
  return *this;
}

FDataTag&
FDataTag::getSuccessor() {
  if (isLastOfBranch()) {
    INTERNAL_ERROR("Cannot get successor of tag", 1);
  }
  myIdxs[mySize-1] += 1;
  initStr();
  return *this;
}

FDataTag&
FDataTag::getTagAsLastOfBranch() {
  if (mySize > 0) {
    myLastFlags[mySize-1] = true;
  }
  return *this;
}

void
FDataTag::updateLastFlags(const FDataTag& parentTag) {
  for (unsigned int ix = 0; ix < parentTag.getLevel(); ++ix) {
    myLastFlags[ix] = parentTag.myLastFlags[ix];
  }
}

/**
 * Converts the tag to a string
 * Used to make the IDs of node instances
 * The only constraint is that the generated string should be unique
 */
const std::string&
FDataTag::toString() const {
  return myStr;
}

int
operator<(const FDataTag& tag1, const FDataTag& tag2) {
  unsigned int ix = 0;
  while ((ix < tag1.getLevel()) && (ix < tag2.getLevel())) {
    if (tag1.myIdxs[ix] != tag2.myIdxs[ix]) {
      return (tag1.myIdxs[ix] < tag2.myIdxs[ix]);
    }
    ++ix;
  }
  return (tag1.getLevel() < tag2.getLevel());
}

/*****************************************************************************/
/*                   WfDataHandleException class                             */
/*****************************************************************************/
std::string
WfDataHandleException::ErrorMsg() {
  std::string errorMsg;
  switch (Type()) {
  case eBAD_STRUCT:
    errorMsg = "Data Hdl bad structure (" + Info() + ")";
    break;
  case eINVALID_ADAPT:
    errorMsg = "Invalid adapter (" + Info() + ")";
    break;
  case eCARD_UNDEF:
    errorMsg = "Cardinal of DH undefined (" + Info() + ")";
    break;
  case eVALUE_UNDEF:
    errorMsg = "Value of DH undefined (" + Info() + ")";
    break;
  case eADAPT_UNDEF:
    errorMsg = "DH Adapter undefined (" + Info() + ")";
    break;
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
  : myTag(), myParentHdl(NULL),
    myPort(NULL), myPortElementIndexes(NULL),
    myAdapterType(ADAPTER_UNDEFINED), myDepth(0),
    myCard(0), myCardList(NULL),
    myCompletionDepth(0), cardDef(false),
    valueDef(false), dataIDOwner(false) {
}

FDataHandle::FDataHandle(const FDataTag& tag,
                         unsigned int depth,
                         bool isVoid,
                         DagNodeOutPort* port)
  : myTag(tag), myParentHdl(NULL),
    myPort(port), myPortElementIndexes(NULL),
    myDepth(depth), myCard(0),
    myCardList(NULL), myCompletionDepth(0),
    cardDef(false), valueDef(false),
    dataIDOwner(false) {
  if (depth > 0) {
    myData = new std::map<FDataTag, FDataHandle*>();
  }

  if (port) {
    myAdapterType = ADAPTER_DIRECT;  // adapter defined by port reference
    myValueType = port->getBaseDataType();
  } else if (isVoid) {
    myAdapterType = ADAPTER_VOID;
    if (depth > 0) {
      setCardinal(1);  // by default a VOID container DH has 1 element
    }
  } else {
    INTERNAL_ERROR("Error in DH constructor: NULL port and not void", 1);
  }
}

// Private constructor
FDataHandle::FDataHandle(const FDataTag& tag,
                         unsigned int depth,
                         FDataHandle* parentHdl)
  : myTag(tag), myParentHdl(parentHdl),
    myPort(NULL), myPortElementIndexes(NULL),
    myAdapterType(ADAPTER_UNDEFINED), myValueType(parentHdl->myValueType),
    myDepth(depth), myCard(0),
    myCardList(NULL), myCompletionDepth(0),
    cardDef(false), valueDef(false),
    dataIDOwner(false)  {
  if (myDepth > 0) {
    myData = new std::map<FDataTag, FDataHandle*>();
  }

  if ((parentHdl) && parentHdl->isAdapterDefined()) {
    // adapter is inherited from parent except if parent is MULTIPLE
    if ((parentHdl->myAdapterType == ADAPTER_DIRECT)
        || (parentHdl->myAdapterType == ADAPTER_SIMPLE)) {
      myAdapterType = ADAPTER_SIMPLE;
      // inherit the port information
      myPort = parentHdl->myPort;
      if (parentHdl->myPortElementIndexes) {
        myPortElementIndexes =
          new std::list<unsigned int>(*parentHdl->myPortElementIndexes);
      } else {
        myPortElementIndexes = new std::list<unsigned int>();
      }
      myPortElementIndexes->push_back(tag.getLastIndex());

    } else if (parentHdl->isVoid()) {
      // VOID IS PROPAGATED TO CHILDS AUTOMATICALLY (at construction time only)
      myAdapterType = ADAPTER_VOID;
      if (depth > 0) {
        setCardinal(1);  // by default a VOID container DH has 1 element
      }
    } else if (parentHdl->myAdapterType == ADAPTER_DATAID) {
      INTERNAL_ERROR(__FUNCTION__
                     << "invalid DH constructor params: child of DATAID DH",
                     1);
    }
  } else {
  }
}

FDataHandle::FDataHandle(const FDataTag& tag,
                         WfCst::WfDataType valueType,
                         const std::string& value)
  : myTag(tag), myParentHdl(NULL),
    myPort(NULL), myPortElementIndexes(NULL),
    myAdapterType(ADAPTER_VALUE), myValue(value),
    myValueType(valueType), myDepth(0),
    myCard(0), myCardList(NULL),
    myCompletionDepth(0), cardDef(true),
    valueDef(true), dataIDOwner(false) {
}

FDataHandle::FDataHandle(const FDataTag&   tag,
                         WfCst::WfDataType dataType,
                         unsigned int      dataDepth,
                         const std::string&     dataId,
                         bool              isOwner)
  : myTag(tag), myParentHdl(NULL),
    myPort(NULL), myPortElementIndexes(NULL),
    myAdapterType(ADAPTER_DATAID), myValue(),
    myValueType(dataType), myDataID(dataId),
    myDepth(dataDepth),  myCard(0),
    myCardList(NULL), myCompletionDepth(0),
    cardDef(false), valueDef(false),
    dataIDOwner(isOwner) {
  TRACE_TEXT(TRACE_ALL_STEPS, "Creating data handle : tag = "
             << myTag.toString() << " / dataID = " << dataId
             << " / owner = " << isOwner << "\n");
  if (myDepth > 0) {
    myData = new std::map<FDataTag, FDataHandle*>();
  }
}

FDataHandle::FDataHandle(const FDataTag& tag,
                         WfCst::WfDataType dataType,
                         unsigned int dataDepth)
  : myTag(tag), myParentHdl(NULL),
    myPort(NULL), myPortElementIndexes(NULL),
    myAdapterType(ADAPTER_UNDEFINED), myValue(),
    myValueType(dataType), myDepth(dataDepth),
    myCard(0), myCardList(NULL),
    myCompletionDepth(0), cardDef(false),
    valueDef(false), dataIDOwner(false) {
  if (myDepth > 0) {
    myData = new std::map<FDataTag, FDataHandle*>();
  }
}

/**
 * Special copy constructor with tag modification
 * Used for control structures (IF, WHILE) that use port mappings
 * Tag is modified due to input operators or loop iterations
 */
FDataHandle::FDataHandle(const FDataTag& tag, const FDataHandle& src)
  : myTag(tag), myParentHdl(NULL),
    myPort(src.myPort), myPortElementIndexes(NULL),
    myAdapterType(src.myAdapterType), myValue(src.myValue),
    myValueType(src.myValueType), myDataID(src.myDataID),
    myDepth(src.myDepth), myCard(src.myCard),
    myCardList(NULL), myCompletionDepth(src.myCompletionDepth),
    cardDef(src.cardDef), valueDef(src.valueDef),
    dataIDOwner(false) {
  if (src.myPortElementIndexes) {
    myPortElementIndexes =
      new std::list<unsigned int>(*src.myPortElementIndexes);
  }
  // Copy the childs if applicable
  if (myDepth > 0) {
    myData = new std::map<FDataTag, FDataHandle*>();
    if (!src.myData->empty()) {
      std::map<FDataTag, FDataHandle*>::const_iterator srcEltIter =
        src.myData->begin();
      for (; srcEltIter != src.myData->end(); ++srcEltIter) {
        FDataHandle *srcElt = (FDataHandle*) srcEltIter->second;
        // Create new child tag based on new tag and last index of the src child
        const FDataTag& srcTag = srcElt->getTag();
        FDataTag  *destTag = new FDataTag(tag, srcTag.getLastIndex(),
                                          srcTag.isLastOfBranch());
        FDataHandle *destElt = new FDataHandle(*destTag, *srcElt);
        // Add new child
        myData->insert(std::pair<FDataTag, FDataHandle*>(*destTag, destElt));
        delete destTag;
      }
    }
    if (src.myCardList) {
      myCardList = new std::list<std::string>(*src.myCardList);
    }
  }
}

/**
 * Copy constructor
 * FIXME duplicated code (some differences) with special copy constructor
 */
FDataHandle::FDataHandle(const FDataHandle& src)
  : myTag(src.myTag), myParentHdl(NULL),
    myPort(src.myPort), myPortElementIndexes(NULL),
    myAdapterType(src.myAdapterType), myValue(src.myValue),
    myValueType(src.myValueType), myDataID(src.myDataID),
    myDepth(src.myDepth), myCard(src.myCard),
    myCardList(NULL), myCompletionDepth(src.myCompletionDepth),
    cardDef(src.cardDef), valueDef(src.valueDef),
    dataIDOwner(false) {
  if (src.myPortElementIndexes) {
    myPortElementIndexes =
      new std::list<unsigned int>(*src.myPortElementIndexes);
  }
  // Copy the childs if applicable
  if (myDepth > 0) {
    myData = new std::map<FDataTag, FDataHandle*>();
    if (!src.myData->empty()) {
      std::map<FDataTag, FDataHandle*>::const_iterator srcEltIter =
        src.myData->begin();
      for (; srcEltIter != src.myData->end(); ++srcEltIter) {
        FDataHandle *srcElt = (FDataHandle*) srcEltIter->second;
        // Copy child
        FDataHandle *destElt = new FDataHandle(*srcElt);
        // Add new child
        myData->insert(
          std::pair<FDataTag, FDataHandle*>(destElt->getTag(), destElt));
      }
    }
    if (src.myCardList) {
      myCardList = new std::list<std::string>(*src.myCardList);
    }
  }
}


/**
 * Destructor
 */
FDataHandle::~FDataHandle() {
  // free data
  if (myDepth > 0) {
    while (!myData->empty()) {
      FDataHandle * p = myData->begin()->second;
      myData->erase(myData->begin());
      delete p;
    }
    delete myData;
    if (myCardList) {
      delete myCardList;
    }
  }
  if (myPortElementIndexes) {
    delete myPortElementIndexes;
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

std::string
FDataHandle::toString() const {
  return "Data Handle";
}

// private
void
FDataHandle::setCardinal(unsigned int card) {
  myCard = card;
  cardDef = true;
}

void
FDataHandle::setCardinalList(const std::list<std::string>& cardList) {
  if (!cardList.empty()) {
    // if the top element is defined, use it to set the cardinal
    const std::string& cardStr = cardList.front();
    if (cardStr != "x") {
      setCardinal(atoi(cardStr.c_str()));
    }
    myCardList = new std::list<std::string>(cardList);
  }
}

void
FDataHandle::setCardinalList(std::list<std::string>::const_iterator& start,
                             std::list<std::string>::const_iterator& end) {
  std::string cardStr = *start;
  if (cardStr != "x") {
    setCardinal(atoi(cardStr.c_str()));
  }
  myCardList = new std::list<std::string>(start, end);
}

unsigned int
FDataHandle::getCardinal() const throw(WfDataHandleException) {
  if (isCardinalDefined()) {
    return myCard;
  } else {
    throw WfDataHandleException(WfDataHandleException::eCARD_UNDEF,
                                "DH Cardinal undefined - tag="
                                + getTag().toString());
  }
}

bool
FDataHandle::isCardinalDefined() const {
  return cardDef;
}

bool
FDataHandle::isParentDefined() const {
  return (myParentHdl != NULL);
}

FDataHandle*
FDataHandle::getParent() const {
  return myParentHdl;
}

bool
FDataHandle::isSourcePortDefined() const {
  return (myAdapterType != ADAPTER_MULTIPLE) &&
    ((myPort != NULL) ||
     ((myParentHdl != NULL) &&
      (myParentHdl->isSourcePortDefined())));
}

DagNodeOutPort*
FDataHandle::getSourcePort() const throw(WfDataHandleException) {
  if (myPort != NULL) {
    return myPort;
  //   else if ((myParentHdl != NULL) && (myParentHdl->isSourcePortDefined()))
  //     return myParentHdl->getSourcePort();
  } else {
    throw WfDataHandleException(WfDataHandleException::eINVALID_ADAPT,
                                "Cannot get source port - tag="
                                + getTag().toString());
  }
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

bool
FDataHandle::isEmpty() const {
  return (myDepth > 0) ? myData->empty() : true;
}

// private
void
FDataHandle::setAsVoid() {
  myAdapterType = ADAPTER_VOID;
  if (myDepth > 0) {
    setCardinal(1);
  }
}

bool
FDataHandle::isAdapterDefined() const {
  return ((myAdapterType != ADAPTER_UNDEFINED) &&
          (myAdapterType != ADAPTER_DELETED));
}

/**
 * (RECURSIVE)
 * Insert a data handle inside the object's data tree
 * Note: level 0 is the root level, and level increases in the tree
 */
void
FDataHandle::insertInTree(FDataHandle* dataHdl)
  throw(WfDataHandleException) {
  if (!dataHdl) {
    INTERNAL_ERROR("FDataHandle::insertInTree : NULL input data handle", 1);
  }
  if (myDepth == 0) {
    throw WfDataHandleException(WfDataHandleException::eBAD_STRUCT,
                                "Cannot insert into non-container data handle"
                                "(depth = 0)");
  }
  const FDataTag& dataTag = dataHdl->getTag();
  unsigned int dataLevel = dataTag.getLevel();
  unsigned int myLevel = myTag.getLevel();

  if (dataLevel <= myLevel) {
    throw WfDataHandleException(WfDataHandleException::eBAD_STRUCT,
                                "Tried to insert data handle of same or lower"
                                "level");
  }
  // data is a direct child of current DH
  if (dataLevel == myLevel+1) {
    this->addChild(dataHdl);
  } else {    // data is a descendant of current DH
    // look for its ancestor (direct child of current DH)
    FDataTag ancestorTag = dataTag.getAncestor(myLevel+1);
    FDataHandle* ancestorHdl = NULL;
    std::map<FDataTag, FDataHandle*>::iterator parIter =
      myData->find(ancestorTag);
    // ancestor exists already
    if (parIter != myData->end()) {
      ancestorHdl = (FDataHandle*) parIter->second;
    } else {  // ancestor does not exist yet => create it
      unsigned int childDepth = dataHdl->getDepth() + dataLevel - myLevel - 1;
      ancestorHdl = new FDataHandle(ancestorTag, childDepth, this);
      addChild(ancestorHdl);
    }

    // call recursively insertInTree on the ancestor
    ancestorHdl->insertInTree(dataHdl);
  }
}

void
FDataHandle::addChild(FDataHandle* dataHdl) {
  // insert the new DH in my child data map
  myData->insert(
    std::pair<FDataTag, FDataHandle*>(dataHdl->getTag(), dataHdl));
  dataHdl->setParent(this);

  // update my depth if element inserted is the first one and depth mismatches
  // (actually happens only for the root DH)
  if (dataHdl->getDepth() != (myDepth - 1)) {
    if (myData->size() == 1) {
      myDepth = dataHdl->getDepth() + 1;
    } else {
      INTERNAL_ERROR("Depth mismatch during data handle child insertion", 1);
    }
  }
  // update my cardinal if element inserted is the last one
  if (dataHdl->isLastChild() && !isCardinalDefined()) {
    setCardinal(dataHdl->getTag().getLastIndex() + 1);
  }
  // updates the adapter status (recursively until the root of the tree)
  updateAncestors();
}

bool
FDataHandle::isLastChild() const {
  return getTag().isLastOfBranch();
}

/**
 * (RECURSIVE : goes up the tree if one level is complete)
 * Check on all childs if adapter is defined
 * Updates the adapter if all its childs have a defined adapter
 * Updates the completion depth at the same time
 */
void
FDataHandle::updateAncestors() {
  if (!isCardinalDefined() || (myData->size() != myCard)) {
    return;
  }
  bool allAdaptersDefined = true;
  bool allAdaptersVoid = true;
  unsigned int minChildCompletionDepth = 999;
  if (myData->empty()) {
    minChildCompletionDepth = myDepth-1;
  }
  // LOOP for ALL childs - checking adapters, voids and completionDepth
  std::map<FDataTag, FDataHandle*>::iterator childIter = myData->begin();
  for (; childIter != myData->end(); ++childIter) {
    FDataHandle * childData = (FDataHandle*) childIter->second;
    if (!childData->isAdapterDefined()) {
      allAdaptersDefined = false;
    }

    if (!childData->isVoid()) {
      allAdaptersVoid = false;
    }

    if (childData->myCompletionDepth < minChildCompletionDepth) {
      minChildCompletionDepth = childData->myCompletionDepth;
    }
  }
  bool updateParent = false;
  if ((int) minChildCompletionDepth > (int) myCompletionDepth-1) {
    myCompletionDepth = minChildCompletionDepth+1;
    updateParent = true;
  }
  if (!isAdapterDefined() && allAdaptersDefined) {
    if (allAdaptersVoid) {
      myAdapterType = ADAPTER_VOID;
    } else {
      // this is the only way to set the adapterType to MULTIPLE
      myAdapterType = ADAPTER_MULTIPLE;
    }
    updateParent = true;
  }
  if (updateParent && isParentDefined()) {
    myParentHdl->updateAncestors();
  }
}

/**
 * Check if the tree is complete at a given level
 */
bool
FDataHandle::checkIfComplete(unsigned int level) {
  return (myCompletionDepth >= level);
}

/**
 * Recursive count of childs
 * (note: the 'level' parameter is an absolute value related to the tag level)
 */
unsigned int
FDataHandle::getChildCount(unsigned int level) {
  // special cases (not used normally)
  if ((myDepth == 0) || (level == 0)) {
    return 0;
  }

  unsigned int myLevel = myTag.getLevel();
  unsigned int count = 0;
  bool isParentLevel = (myLevel == level-1);

  std::map<FDataTag, FDataHandle*>::iterator childIter = myData->begin();
  for (; childIter != myData->end(); ++childIter) {
    if (isParentLevel) {
      count += 1;  // recursion stops at level-1
    } else {
      count += ((FDataHandle*) childIter->second)->getChildCount(level);
    }
  }

  return count;
}

void
FDataHandle::addProperty(const std::string& propKey,
                         const std::string& propValue) {
  TRACE_TEXT(TRACE_MAIN_STEPS, __FUNCTION__ << "DH=" << myTag.toString()
             << " : key=" << propKey
             << "/value=" << propValue << "\n");
}

const std::string&
FDataHandle::getProperty(const std::string& propKey)
  throw(WfDataHandleException) {
  return propKey;
}

void
FDataHandle::updateTreeCardinal() {
  updateTreeCardinalRec(false, false);
}

// private
void
FDataHandle::updateTreeCardinalRec(bool isLast, bool parentTagMod) {
  // Modify the tag according to position and parent's tag
  if (parentTagMod) {
    myTag.updateLastFlags(getParent()->getTag());
  }
  if (isLast) {
    myTag.getTagAsLastOfBranch();
  }

  // Update cardinal and call recursively the method for all childs
  if ((myDepth > 0) && (!myData->empty())) {
    // get last child DH
    std::map<FDataTag, FDataHandle*>::reverse_iterator childRIter =
      myData->rbegin();
    FDataHandle*  lastChild = (FDataHandle*)childRIter->second;

    // cardinal update
    setCardinal(lastChild->getTag().getLastIndex() + 1);

    std::map<FDataTag, FDataHandle*>::iterator childIter = myData->begin();
    for (; childIter != myData->end(); ++childIter) {
      FDataHandle* currChild = (FDataHandle*)childIter->second;
      currChild->updateTreeCardinalRec(currChild == lastChild,
                                       parentTagMod || isLast);
    }

  } else {
    // call updateAncestors when on a leave which is the last of its parent
    if (isLast) {
      myParentHdl->updateAncestors();
    }
  }
}

/** RECURSIVE **/
void
FDataHandle::uploadTreeData(MasterAgent_var& MA) throw(WfDataHandleException) {
  // check childs
  if ((myDepth > 0) && (!myData->empty())) {
    for (std::map<FDataTag, FDataHandle*>::iterator childIter = myData->begin();
         childIter != myData->end();
         ++childIter) {
      FDataHandle*  currChild = (FDataHandle*)childIter->second;
      currChild->uploadTreeData(MA);
    }
  }

  // check data ID
  if (isDataIDDefined()) {
    Dagda_var dataManager =
      ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT,
                                                  MA->getDataManager());
    if (!dataManager->pfmIsDataPresent(myDataID.c_str())) {
      if (isValueDefined()) {
        // data ID is obsolete so try to recreate one using value
        myAdapterType = ADAPTER_VALUE;
        downloadDataID();
      } else {
        std::string errorMsg("Missing value and invalid data ID (tag="
                             + getTag().toString() + ")");
        throw WfDataHandleException(WfDataHandleException::eVALUE_UNDEF,
                                    errorMsg);
      }
    }
  } else if (isValueDefined()) {
    // create a copy of the data on the platform using the value
    // will change the adapterType to ADAPTER_DATAID
    downloadDataID();
  }
}

std::map<FDataTag, FDataHandle*>::iterator
FDataHandle::begin() throw(WfDataHandleException) {
  // check that data handle is a container
  if (myDepth < 1) {
    throw WfDataHandleException(WfDataHandleException::eBAD_STRUCT,
                                "Tried to get elements of non-container data"
                                "handle");
  }
  // if the dataID is known then the first call to begin will
  // create the childs automatically with their dataID set
  std::vector<std::string>* childIDVect = NULL;
  if (isDataIDDefined() && (myData->empty())) {
    WfDataIDAdapter* adapterID =
      dynamic_cast<WfDataIDAdapter*>(createPortAdapter());
    childIDVect = new std::vector<std::string>();
    // retrieve the child IDs using the ID adapter
    try {
      adapterID->getElements(*childIDVect);
      delete adapterID;
    } catch (WfDataException& e) {
      delete childIDVect;
      delete adapterID;
      throw WfDataHandleException(WfDataHandleException::eBAD_STRUCT,
                                  "Cannot retrieve element IDs : "
                                  + e.ErrorMsg());
    }
    if (!isCardinalDefined()) {
      setCardinal(childIDVect->size());
    }
  }
  // if the cardinal is defined then the first call to begin will
  // create the childs automatically
  if (isCardinalDefined() && (myData->empty())) {
    for (unsigned int ix = 0; ix < myCard; ++ix) {
      FDataTag childTag(getTag(), ix, (ix == myCard-1));
      // Create new data handle
      FDataHandle* childHdl;
      if (childIDVect) {
        childHdl = new FDataHandle(childTag, myValueType,
                                   myDepth-1, (*childIDVect)[ix], true);
      } else {
        childHdl = new FDataHandle(childTag, myDepth-1, this);
      }
      // Insert the data handle as child of this one
      this->addChild(childHdl);
      // set the cardinal list (static cardinal info) for childs
      if ((myCardList) && (myCardList->size() > 1)) {
        std::list<std::string>::const_iterator cardListStart =
          ++myCardList->begin();
        std::list<std::string>::const_iterator cardListEnd = myCardList->end();
        childHdl->setCardinalList(cardListStart, cardListEnd);
      }
    }
    if (childIDVect) {
      delete childIDVect;
    }
  }
  return myData->begin();
}

std::map<FDataTag, FDataHandle*>::iterator
FDataHandle::end() {
  return myData->end();
}

/**
 * Port adapter creation (RECURSIVE for MULTIPLE adapters)
 */
WfPortAdapter*
FDataHandle::createPortAdapter(const std::string& currDagName) {
  WfPortAdapter * myAdapter = NULL;

  if (myAdapterType == ADAPTER_VOID) {
    myAdapter = new WfVoidAdapter();

  } else if (myAdapterType == ADAPTER_DATAID) {
    myAdapter = new WfDataIDAdapter(getValueType(),
                                    getDepth(),
                                    getDataID());

  } else if (myAdapterType == ADAPTER_VALUE) {
    myAdapter = new WfValueAdapter(getValueType(),
                                   getValue());

  } else if (myAdapterType == ADAPTER_MULTIPLE) {
    // LINK TO SEVERAL OUTPUT PORTS (MERGE)
    // get all my child adapters and create a multiple adapter from them
    WfMultiplePortAdapter* multAdapter = new WfMultiplePortAdapter();
    for (std::map<FDataTag, FDataHandle*>::iterator childIter = myData->begin();
         childIter != myData->end();
         ++childIter) {
      FDataHandle * childHdl = (FDataHandle*) childIter->second;
      multAdapter->addSubAdapter(childHdl->createPortAdapter(currDagName));
    }
    myAdapter = (WfPortAdapter*) multAdapter;

  } else if ((myAdapterType == ADAPTER_DIRECT) ||
             (myAdapterType == ADAPTER_SIMPLE)) {
    // get dag id of source dagNode
    DagNode * endNode = dynamic_cast<DagNode*>(myPort->getParent());
    std::string dagName;
    std::string endNodeDagName = endNode->getDag()->getId();
    if (endNodeDagName != currDagName) {
      dagName = endNodeDagName;
    }
    if (myAdapterType == ADAPTER_DIRECT) {
      // LINK TO OUTPUT PORT WITHOUT SPLIT
      myAdapter = new WfSimplePortAdapter(myPort, dagName);
    } else if (myAdapterType == ADAPTER_SIMPLE) {
      // LINK TO OUTPUT PORT WITH SPLIT (use indexes)
      myAdapter =
        new WfSimplePortAdapter(myPort, *myPortElementIndexes, dagName);
    }
  } else {
    throw WfDataHandleException(WfDataHandleException::eINVALID_ADAPT,
                                "Cannot create adapter for DH - tag="
                                + myTag.toString());
  }
  return myAdapter;
}

bool
FDataHandle::isValueDefined() const {
  return valueDef;
}

void
FDataHandle::setValue(WfCst::WfDataType valueType, const std::string& value) {
  myValueType = valueType;
  myValue = value;
  valueDef = true;
  if (myAdapterType == ADAPTER_UNDEFINED) {
    myAdapterType = ADAPTER_VALUE;
  }
}

const std::string&
FDataHandle::getValue() const {
  return myValue;
}

WfCst::WfDataType
FDataHandle::getValueType() const {
  return myValueType;
}

bool
FDataHandle::isDataIDDefined() const {
  return (myAdapterType == ADAPTER_DATAID);
}

bool
FDataHandle::isDataIDOwner() const {
  return dataIDOwner;
}

const std::string&
FDataHandle::getDataID() const {
  return myDataID;
}

void
FDataHandle::setDataID(const std::string& dataID) {
  if (!dataID.empty()) {
    myDataID = dataID;
    myAdapterType = ADAPTER_DATAID;
  }
}

/**
 * Get the value using the adapter and write it on given DataWriter
 */
void
FDataHandle::writeValue(WfDataWriter *dataWriter) {
  if (isAdapterDefined()) {
    // if adapter is simple => will download the value through DagNodeOutPort
    // if adapter is multiple => will call writeData on sub-adapters
    // if adapter is value => will use the value stored in myValue attribute
    // if adapter is dataID => will download the value through WfDataIDAdapter
    WfPortAdapter *adapter = createPortAdapter();
    adapter->writeDataValue(dataWriter);
    delete adapter;
  } else {
    throw WfDataHandleException(WfDataHandleException::eINVALID_ADAPT,
                                myTag.toString());
  }
}

/**
 * Get the value (in XML format) and store it in myValue
 */
void
FDataHandle::downloadValue() throw(WfDataHandleException) {
  if (isValueDefined()) {
    return;
  }

  std::ostringstream  valStr;
  WfDataWriter  *dataWriter;
  if (myDepth > 0) {
    dataWriter = new WfXMLDataWriter(valStr);  // for containers
  } else {
    dataWriter = new WfListDataWriter(valStr);  // for scalar values
  }
  try {
    writeValue(dataWriter);
  } catch (WfDataHandleException& e) {
    std::string errorMsg = "Cannot get data (" + e.ErrorMsg() + ")";
    delete dataWriter;
    throw WfDataHandleException(WfDataHandleException::eVALUE_UNDEF, errorMsg);
  } catch (WfDataException& e) {
    std::string errorMsg = "Cannot get data (" + e.ErrorMsg() + ")";
    delete dataWriter;
    throw WfDataHandleException(WfDataHandleException::eVALUE_UNDEF, errorMsg);
  } catch (...) {
    WARNING("Uncaught exception in FDataHandle::downloadValue()");
  }
  myValue = valStr.str();  // FIXME use setValue
  valueDef = true;
  delete dataWriter;
  TRACE_TEXT(TRACE_ALL_STEPS, "==> Value is : " << myValue << "\n");
}

void
FDataHandle::downloadDataID() throw(WfDataHandleException, WfDataException) {
  if (!isDataIDDefined() && isAdapterDefined()) {
    // Create an adapter
    WfPortAdapter *adapter = createPortAdapter();

    // Get data ID from adapter
    try {
      adapter->getSourceDataID();
      setDataID(adapter->getSourceDataID());
    } catch (WfDataException& e) {
      if (e.Type() == WfDataException::eVOID_DATA) {
        setAsVoid();
      } else {
        delete adapter;
        throw;
      }
    } catch (...) {
      WARNING("Uncaught exception in FDataHandle::downloadDataID()");
    }
    // Check ownership of dataID
    if (adapter->isDataIDCreator()) {
      dataIDOwner = true;
    }

    delete adapter;
  }
}

void
FDataHandle::downloadTreeData() {
  if (myDepth == 0) {
    try {
      downloadDataID();
      downloadValue();
    } catch (WfDataException& e) {
      WARNING("Data Error (" << e.ErrorMsg() << ")");
    }
  } else {
    for (std::map<FDataTag, FDataHandle*>::iterator childIter = this->begin();
         childIter != this->end();
         ++childIter) {
      ((FDataHandle*)childIter->second)->downloadTreeData();
    }
    try {
      downloadDataID();
    } catch (WfDataException& e) {
      WARNING("Data Error (" << e.ErrorMsg() << ")");
    }
    downloadDataID();
  }
}

void
FDataHandle::downloadCardinal() {
  if (isCardinalDefined()) {
    return;
  }
  if (myDepth < 1) {
    throw WfDataHandleException(WfDataHandleException::eINVALID_ADAPT,
                                "Cannot get cardinal of 0-depth data - tag="
                                + myTag.toString());
  }
  downloadDataID();
  if (isDataIDDefined()) {
    // will create childs and get the cardinal at the same time
    begin();
  } else {
    throw WfDataHandleException(WfDataHandleException::eINVALID_ADAPT,
                                "Cannot get cardinal due to invalid"
                                "adapter - tag=" + myTag.toString());
  }
}

void
FDataHandle::toXML(WfXMLDataWriter& XMLWriter) {
  if (myDepth > 0) {
    XMLWriter.startContainer(getDataID());
    std::map<FDataTag, FDataHandle*>::iterator childIter = this->begin();
    for (; childIter != this->end(); ++childIter) {
      ((FDataHandle*)childIter->second)->toXML(XMLWriter);
    }
    XMLWriter.endContainer();
  } else {
    XMLWriter.itemValue(getValue(), getDataID());
  }
}

void
FDataHandle::freePersistentDataRec(MasterAgent_var& MA) {
  if (isDataIDDefined() && isDataIDOwner()) {
    TRACE_TEXT(TRACE_ALL_STEPS,
               "Deleting DH persistent data: " << getDataID() << "\n");
    const char *dataId = strdup(getDataID().c_str());
    if (MA->diet_free_pdata(dataId) == 0) {
      WARNING("Could not delete persistent data: " << dataId);
    }
    myAdapterType = ADAPTER_DELETED;  // avoid warning in case of double call
  }
  if (myDepth > 0) {
    std::map<FDataTag, FDataHandle*>::iterator childIter = this->begin();
    for (; childIter != this->end(); ++childIter) {
      ((FDataHandle*) childIter->second)->freePersistentDataRec(MA);
    }
  }
}

/**
 * display used for debug
 */
void
FDataHandle::display(bool goUp) {
  if (goUp && myParentHdl) {
    myParentHdl->display(true);
  } else {
    if (getTag().getLevel() == 0) {
      TRACE_TEXT(TRACE_MAIN_STEPS, "-------------------\n");
    }
    TRACE_TEXT(TRACE_MAIN_STEPS, "DATA: " << getTag().toString()
               << " (depth=" << myDepth << ")");

    if (isValueDefined()) {
      TRACE_TEXT(TRACE_MAIN_STEPS, " VALUE=" << myValue);
    }
    TRACE_TEXT(TRACE_MAIN_STEPS, "\n");

    if (myDepth>0) {
      std::map<FDataTag, FDataHandle*>::iterator childIter = myData->begin();
      for (; childIter != myData->end(); ++childIter)
        ((FDataHandle*)childIter->second)->display(false);
    }
    if (getTag().getLevel() == 0) {
      TRACE_TEXT(TRACE_MAIN_STEPS, "-------------------\n");
    }
  }
}

