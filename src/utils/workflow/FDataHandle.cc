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


FDataTag::FDataTag() : mySize(0) {
//   cout << "FDataTag default constructor" << endl;
}

FDataTag::FDataTag(const FDataTag& tag) {
//   cout << "start of FDataTag copy constructor" << endl;
  mySize = tag.mySize;
  if (mySize>0) {
    myIdxs = new unsigned int[mySize];
    myLastFlags = new bool[mySize];
    for (int ix=0; ix<mySize ; ++ix) {
      myIdxs[ix] = tag.myIdxs[ix];
      myLastFlags[ix] = tag.myLastFlags[ix];
    }
  }
//   cout << "end of FDataTag copy constructor" << endl;
}
FDataTag::FDataTag(unsigned int index, bool isLast)
  : mySize(1) {
//   cout << "start of FDataTag constructor 1" << endl;
  myIdxs = new unsigned int[1];
  myLastFlags = new bool[1];
  myIdxs[0] = index;
  myLastFlags[0] = isLast;
//   cout << "end of FDataTag constructor 1" << endl;
}
FDataTag::FDataTag(const FDataTag& parentTag, unsigned int index, bool isLast) {
  cout << "creating tag (" << this << ") idx=" << index << "/last=" << isLast << endl;
  mySize = parentTag.mySize + 1;
  myIdxs = new unsigned int[mySize];
  myLastFlags = new bool[mySize];
  for (int ix=0; ix<mySize ; ++ix) {
    myIdxs[ix] = parentTag.myIdxs[ix];
    myLastFlags[ix] = parentTag.myLastFlags[ix];
  }
  myIdxs[mySize-1] = index;
  myLastFlags[mySize-1] = isLast;
//   cout << "end of FDataTag constructor 2" << endl;
}

/**
 * Constructor for making parent tag
 * The provided tables must have at least 'level' elements
 */
FDataTag::FDataTag(unsigned int * idxTab, bool * lastTab, unsigned int level)
  : mySize(level) {
//   cout << "start of FDataTag private constructor" << endl;
  myIdxs = new unsigned int[mySize];
  myLastFlags = new bool[mySize];
  for (int ix=0; ix<mySize; ++ix) {
    myIdxs[ix]      = idxTab[ix];
    myLastFlags[ix] = lastTab[ix];
  }
//   cout << "end of FDataTag private constructor" << endl;
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

unsigned int
FDataTag::getIndex(unsigned int level) const {
  return myIdxs[level-1];
}

FDataTag
FDataTag::getParent(unsigned int level) const {
  FDataTag parTag(myIdxs, myLastFlags, level);
  return parTag;
}

string
FDataTag::toString() const {
  stringstream ss;
  for (int ix=0; ix < mySize; ++ix) {
    ss << "_" << myIdxs[ix];
  }
  return ss.str();
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
/*                         FDataHandle class                                 */
/*****************************************************************************/

/**
 * Constructors
 */

FDataHandle::FDataHandle()
  : myTag(), myDepth(0), myParentHdl(NULL), myPort(NULL),
    myCard(0), cardDef(false), adapterDef(false), valueDef(false),
    myAdapterType(ADAPTER_UNDEFINED) {}

FDataHandle::FDataHandle(const FDataTag& tag,
                         unsigned int depth,
                         FDataHandle* parentHdl,
                         WfPort* port)
  : myTag(tag), myDepth(depth), myParentHdl(parentHdl), myPort(port),
    myCard(0), cardDef(false), adapterDef(false), valueDef(false),
    myAdapterType(ADAPTER_UNDEFINED) {
  cout << "Creating data handle : tag = " << tag.toString()
       << " / depth=" << depth << " / parent=" << parentHdl << " / port=" << port << endl;
  if (myDepth > 0)
    myData = new map<FDataTag,FDataHandle*>();
  if ((parentHdl) && parentHdl->isAdapterDefined())
    myAdapterType = ADAPTER_SIMPLE;  // adapter is inherited from parent
  if (myPort)
    myAdapterType = ADAPTER_DIRECT;  // adapter defined by port reference
}

FDataHandle::FDataHandle(const FDataTag& tag,
                         const string& value,
                         FDataHandle* parentHdl)
  : myTag(tag), myDepth(0), myValue(value), myParentHdl(parentHdl),
    myPort(NULL), myCard(0), cardDef(false), adapterDef(false), valueDef(true),
    myAdapterType(ADAPTER_UNDEFINED) {
  cout << "Creating data handle : tag = " << myTag.toString()
       << " / value=" << value << " / parent=" << parentHdl << endl;
}

FDataHandle::FDataHandle(unsigned int depth)
  : myTag(), myDepth(depth), myValue(), myParentHdl(NULL),
    myPort(NULL), myCard(0), cardDef(false), adapterDef(false), valueDef(false),
    myAdapterType(ADAPTER_UNDEFINED) {
  cout << "Creating data handle : tag = " << myTag.toString()
       << " / depth=" << depth << endl;
  if (myDepth > 0)
    myData = new map<FDataTag,FDataHandle*>();
}

/**
 * Destructor
 */
FDataHandle::~FDataHandle() {
  TRACE_TEXT (TRACE_ALL_STEPS,"~FDataHandle() " << myTag.toString()
                              << " destructor ..." <<  endl);
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
FDataHandle::getDepth() {
  return myDepth;
}

const FDataTag&
FDataHandle::getTag() {
  return myTag;
}

void
FDataHandle::setCardinal(unsigned int card) {
  myCard = card;
  cardDef = true;
}

unsigned int
FDataHandle::getCardinal() {
  return myCard;
}

bool
FDataHandle::isCardinalDefined() {
  return cardDef;
}

bool
FDataHandle::isParentDefined() {
  return myParentHdl != NULL;
}

FDataHandle*
FDataHandle::getParent() {
  return myParentHdl;
}

void
FDataHandle::setParent(FDataHandle* parentHdl) {
  myParentHdl = parentHdl;
}

bool
FDataHandle::isAdapterDefined() {
  return (myAdapterType != ADAPTER_UNDEFINED);
}

/**
 * (RECURSIVE)
 * Insert a data handle inside the tree
 * Note: level 0 is the root level, and level increases in the tree
 */
void
FDataHandle::insertInTree(FDataHandle* dataHdl) {
  cout << "insertInTree (current tag=" << getTag().toString() << ") ..." << endl;
  if (myDepth == 0) {
    INTERNAL_ERROR("Cannot insert into non-container data handle (depth=0)",0);
  }
  const FDataTag& dataTag = dataHdl->getTag();
  unsigned int dataLevel = dataTag.getLevel();
  unsigned int myLevel = myTag.getLevel();
  if (dataLevel <= myLevel) {
    INTERNAL_ERROR("Tried to insert data handle of same or lower level",0);
  }
  if (dataLevel == myLevel+1) {
    // data is a direct child of object ==> insert it as a child
    this->addChild(dataHdl);
  } else {
    // data is a sub-child of object ==> look for its parent and
    // calls recursively insertInTree
    FDataTag parentTag = dataTag.getParent(myLevel+1);
    FDataHandle* parentHdl = NULL;
    map<FDataTag,FDataHandle*>::iterator parIter = myData->find(parentTag);
    if (parIter != myData->end()) {
      // parent exists already
      parentHdl = (FDataHandle*) parIter->second;
    } else {
      // parent does not exist yet => create it
      unsigned int parentDepth = dataHdl->getDepth()
                                  + dataHdl->getTag().getLevel()
                                  - this->getTag().getLevel() - 1;
      parentHdl = new FDataHandle(parentTag, parentDepth, this, NULL);
      // insert the parent as a child of current data
      this->addChild(parentHdl);
    }
    parentHdl->insertInTree(dataHdl);
  }
}

void
FDataHandle::addChild(FDataHandle* dataHdl) {
  cout << "adding data child (tag=" << dataHdl->getTag().toString() << ")" << endl;
  myData->insert(make_pair(dataHdl->getTag(),dataHdl));
  dataHdl->setParent(this);
  if (dataHdl->isLastChild()) {
    TRACE_TEXT (TRACE_ALL_STEPS," LAST item => Updating adapter status" << endl);
    dataHdl->getParent()->checkAdapter();
  }
}

bool
FDataHandle::isLastChild() {
  return getTag().isLastOfBranch();
}

/**
 * (RECURSIVE : goes up the tree if one level is complete)
 * Check on all childs if adapter is defined
 * FIXME does not check if the nb of child correspond to the cardinal!
 */
void
FDataHandle::checkAdapter() {
  cout << "checking adapter for " << getTag().toString() << " (parent = " << myParentHdl << ")" << endl;
  if (!isAdapterDefined()) {
    bool allAdaptersDefined = true;
    bool lastChildFound = false;
    map<FDataTag,FDataHandle*>::iterator childIter = myData->begin();
    while (allAdaptersDefined && childIter != myData->end()) {
      FDataHandle * childData = (FDataHandle*) childIter->second;
      if (!childData->isAdapterDefined()) {
        allAdaptersDefined = false;
      }
      if (childData->isLastChild()) {
        lastChildFound = true;
      }
      ++childIter;
    }
    if (allAdaptersDefined && lastChildFound) {
      this->myAdapterType = ADAPTER_MULTIPLE;
      TRACE_TEXT (TRACE_ALL_STEPS,"**** data " << getTag().toString()
                  << " is complete ****" << endl);
      if (isParentDefined()) {
        myParentHdl->checkAdapter();
      }
    }
  }
}

map<FDataTag,FDataHandle*>::iterator
FDataHandle::begin() {
  // check that data handle is a container
  if (myDepth < 1) {
    INTERNAL_ERROR("Trying to get elements of non-container data handle",0);
  }
  // if data handle linked to an existing dag port (ie adapter defined)
  // and if the cardinal is defined then the first call to begin will
  // create the childs
  if (isAdapterDefined() && isCardinalDefined() && (myData->size() == 0)) {
    TRACE_TEXT (TRACE_ALL_STEPS,"Creating childs of data " << getTag().toString() << endl);
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
FDataHandle::isValueDefined() {
  return valueDef;
}

const string&
FDataHandle::getValue() {
  return myValue;
}
