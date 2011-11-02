/**
* @file  InputIterator.cc
* 
* @brief  Set of classes used to implement data operators for functional workflows
* 
* @author  - Benjamin ISNARD (Benjamin.Isnard@ens-lyon.fr)
* 
* @section Licence
*   |LICENSE|                                                                
*/
/* $Id$
 * $Log$
 * Revision 1.9  2011/02/02 13:56:11  bdepardo
 * Add missing endl in traces
 *
 * Revision 1.8  2011/02/02 11:19:13  bdepardo
 * Be more verbose
 *
 * Revision 1.7  2010/08/26 07:09:37  bisnard
 * 'fixed warning'
 *
 * Revision 1.6  2009/10/23 14:02:45  bisnard
 * removed debug trace messages
 *
 * Revision 1.5  2009/10/02 07:44:56  bisnard
 * new wf data operators MATCH & N-CROSS
 *
 * Revision 1.4  2009/08/26 10:26:29  bisnard
 * added new iterator flatcross
 *
 * Revision 1.3  2009/05/27 08:56:47  bisnard
 * moved id attribute to parent abstract class
 *
 * Revision 1.2  2009/05/15 11:10:20  bisnard
 * release for workflow conditional structure (if)
 *
 * Revision 1.1  2009/01/16 13:52:36  bisnard
 * new class to manage different operators to mix input streams of data
 *
 */

#include "InputIterator.hh"
#include "debug.hh"

/*****************************************************************************/
/*                       InputIterator (ABSTRACT)                            */
/*****************************************************************************/

InputIterator::InputIterator(const std::string& id) : myId(id) {
}

const std::string&
InputIterator::getId() const { return myId; }

std::string
InputIterator::traceId() const {
  return ("[" + myId + "] : ");
}

/*****************************************************************************/
/*                       PortInputIterator                                   */
/*****************************************************************************/

PortInputIterator::PortInputIterator(FNodeInPort * inPort)
  : InputIterator(inPort->getId()), myInPort(inPort), removedItemsCount(0) {}

void
PortInputIterator::begin() {
  TRACE_TEXT(TRACE_ALL_STEPS, traceId() << "begin() : "
             << myInPort->myQueue.size() << " items in the queue\n");
  myQueueIter = myInPort->myQueue.begin();
  if (!isAtEnd()) {
    TRACE_TEXT(TRACE_ALL_STEPS, "  and first item is "
               << ((FDataHandle*) myQueueIter->second)->getTag().toString()
               << "\n");
  }
}

void
PortInputIterator::end() {
  myQueueIter = myInPort->myQueue.end();
}

void
PortInputIterator::next() {
  if (isAtEnd()) {
    INTERNAL_ERROR("Calling next on empty port iterator", 1);
  }
  ++myQueueIter;
}

bool
PortInputIterator::isEmpty() const {
  return (myInPort->myQueue.empty());
}

bool
PortInputIterator::isAtEnd() const {
  return (myQueueIter == myInPort->myQueue.end());
}

bool
PortInputIterator::isDone() const {
  std::string total = isTotalDefined() ? itoa(getTotalItemNb()) : "undef";
  TRACE_TEXT(TRACE_ALL_STEPS, traceId() << "isDone(): total=" << total
             << " / removed=" << removedItemsCount << "\n");
  return (isTotalDefined() && (getTotalItemNb() == removedItemsCount));
}

void
PortInputIterator::removeItem() {
  std::map<FDataTag, FDataHandle*>::iterator toRemove = myQueueIter;
  this->next();
  myInPort->myQueue.erase(toRemove);
  removedItemsCount++;
  //   TRACE_TEXT(TRACE_ALL_STEPS, traceId() << "removeItem(): removed="
  //                                << removedItemsCount << "\n");
}

const FDataTag&
PortInputIterator::getCurrentItem(std::vector<FDataHandle*>& dataLine) {
  FDataHandle * currDataHdl = (FDataHandle*) myQueueIter->second;
  dataLine[myInPort->getIndex()] = currDataHdl;
  return getCurrentTag();
}

const FDataTag&
PortInputIterator::getCurrentTag() {
  return ((FDataHandle*) myQueueIter->second)->getTag();
}

bool
PortInputIterator::find(const FDataTag& tag) {
  myQueueIter = myInPort->myQueue.find(tag);
  return (myQueueIter != myInPort->myQueue.end());
}

bool
PortInputIterator::isTotalDefined() const {
  return (myInPort->totalDef);
}

unsigned int
PortInputIterator::getTotalItemNb() const {
  return (myInPort->dataTotalNb);
}


/*****************************************************************************/
/*                           CrossIterator                                   */
/*****************************************************************************/

CrossIterator::CrossIterator(InputIterator* leftIter,
                             InputIterator* rightIter)
  : InputIterator(createId(leftIter, rightIter)),
    myLeftIter(leftIter), myRightIter(rightIter),
    currTag(NULL), leftTagLength(0) {
}

CrossIterator::~CrossIterator() {
  delete currTag;
}

std::string
CrossIterator::createId(InputIterator* leftIter,
                        InputIterator* rightIter) {
  return ("(" + leftIter->getId() + "_x_" + rightIter->getId() + ")");
}

/**
 * (private)
 * Updates the current tag (used when iterator position changes)
 */
bool
CrossIterator::setTag() {
  clearTag();
  currTag = new FDataTag(myLeftIter->getCurrentTag(),
                         myRightIter->getCurrentTag());
  return true;
}

/**
 * (private)
 * Clears the current tag (used when iterator position is at the end)
 */
void
CrossIterator::clearTag() {
  if (currTag) {
    delete currTag;
    currTag = NULL;
  }
}

/**
 * (private)
 * Check if an element is available
 */
bool
CrossIterator::checkItemAvailable() {
  return (!myLeftIter->isAtEnd() && !myRightIter->isAtEnd());
}

/**
 * (private)
 * Check if the current item was already selected (ie removed)
 */
bool
CrossIterator::isFlagged() {
  std::map<FDataTag, bool>::iterator flagIter = myFlags.find(getCurrentTag());
  return (flagIter != myFlags.end());
}

/**
 * (private)
 * Increment the counter of matched items for an element of the left iterator
 */
int
CrossIterator::incrementMatchCount(const FDataTag& leftTag) {
  int count;
  std::map<FDataTag, int>::iterator countIter = myCounters.find(leftTag);
  if (countIter != myCounters.end()) {
    count = ((int) countIter->second) + 1;
    myCounters[leftTag] = count;
  } else {
    count = 1;
    myCounters[leftTag] = count;
  }
  return count;
}

void
CrossIterator::begin() {
  TRACE_TEXT(TRACE_ALL_STEPS, traceId() << "begin() ...\n");
  myLeftIter->begin();
  myRightIter->begin();
  // if right iterator does not contain one item then this is the end
  if (myRightIter->isAtEnd()) {
    myLeftIter->end();
  }
  // go to next available item if possible
  if (checkItemAvailable()) {
    if (setTag() && isFlagged()) {
      next();
    }
  } else {
    clearTag();
  }
}

void
CrossIterator::end() {
  myLeftIter->end();
}

void
CrossIterator::next() {
  TRACE_TEXT(TRACE_ALL_STEPS, traceId() << "next() ...\n");
  if (isAtEnd()) {
    INTERNAL_ERROR(__FUNCTION__ << " : empty cross iterator", 1);
  }
  // loop until the end of the left iter or a non-flagged item is found
  bool endOfLeft = false, endOfRight = false, nextFound = false;
  while (!endOfLeft && !nextFound) {
    // loop until the end of the right iter or a non-flagged item is found
    while (!endOfRight && !nextFound) {
      myRightIter->next();
      endOfRight = myRightIter->isAtEnd();
      if (!endOfRight && setTag() && !isFlagged()) {
        nextFound = true;
      }
    }
    if (!nextFound) {
      // end of right iter => go to next on left iter and restart right iter
      TRACE_TEXT(TRACE_ALL_STEPS, traceId() << "end of right iter\n");
      myLeftIter->next();
      myRightIter->begin();
      endOfLeft = myLeftIter->isAtEnd();
      if (!endOfLeft && setTag() && !isFlagged()) {
        nextFound = true;
      }
    }
  }
  if (nextFound) {
    TRACE_TEXT(TRACE_ALL_STEPS, traceId() << "next() IS "
               << getCurrentTag().toString() << "\n");
  } else {
    clearTag();
    TRACE_TEXT(TRACE_ALL_STEPS, traceId() << "next() ==> END\n");
  }
}

bool
CrossIterator::isEmpty() const {
  return (myLeftIter->isEmpty() || myRightIter->isEmpty());
}

bool
CrossIterator::isAtEnd() const {
  return myLeftIter->isAtEnd();
}

bool
CrossIterator::isDone() const {
  TRACE_TEXT(TRACE_ALL_STEPS, traceId() << "isDone()\n");
  return myLeftIter->isDone();
}

void
CrossIterator::removeItem() {
  const FDataTag& leftTag  = myLeftIter->getCurrentTag();
  TRACE_TEXT(TRACE_ALL_STEPS, traceId() << "removeItem(): removing "
             << currTag->toString() << "\n");
  // mark the removed item
  myFlags[*currTag] = true;
  // increment the counter of matched items for the left tag
  unsigned int matchCount = incrementMatchCount(leftTag);
  TRACE_TEXT(TRACE_ALL_STEPS, traceId() << "removeItem(): match count of "
             << leftTag.toString() << " is " << matchCount << "\n");
  // check if right iter is completed for the current left item
  if (myRightIter->isTotalDefined() &&
      (matchCount == myRightIter->getTotalItemNb())) {
    // if yes, then remove, go to next left item and reset right iter
    myLeftIter->removeItem();
    myRightIter->begin();
    // then check if current is available and if yes find an unmatched item
    if (checkItemAvailable() && setTag() && isFlagged()) {
      next();
    }
  } else {
    // if no simply go to next unmatched item
    next();
  }
  // if left completed then remove all items in right
  if (myLeftIter->isDone()) {
    myRightIter->begin();
    while (!myRightIter->isAtEnd()) {
      myRightIter->removeItem();
    }
  }
}

const FDataTag&
CrossIterator::getCurrentItem(std::vector<FDataHandle*>& dataLine) {
  if (!checkItemAvailable()) {
    INTERNAL_ERROR(__FUNCTION__ << " : no item available", 1);
  }
  myLeftIter->getCurrentItem(dataLine);
  myRightIter->getCurrentItem(dataLine);
  return getCurrentTag();
}


const FDataTag&
CrossIterator::getCurrentTag() {
  if (!currTag) {
    INTERNAL_ERROR(__FUNCTION__ << " : no item available", 1);
  }
  return *currTag;
}

bool
CrossIterator::splitTag(const FDataTag& tag, FDataTag*& leftTagPtr,
                        FDataTag*& rightTagPtr) {
  leftTagPtr = tag.getLeftPart(leftTagLength);
  rightTagPtr = tag.getRightPart(leftTagLength+1);
  return true;
}

bool
CrossIterator::find(const FDataTag& tag) {
  std::map<FDataTag, bool>::iterator flagIter = myFlags.find(tag);
  if (flagIter != myFlags.end()) {
    return false;
  }
  // initialize left tag's length if not already done
  if (leftTagLength == 0) {
    myLeftIter->begin();
    if (myLeftIter->isAtEnd()) {
      return false;
    }
    leftTagLength = myLeftIter->getCurrentTag().getLevel();
  }
  // split the tag in left and right part (if possible)
  FDataTag *leftTag, *rightTag;
  if (!splitTag(tag, leftTag, rightTag)) {
    return false;
  }
  if ((leftTag == NULL) || (rightTag == NULL)) {
    INTERNAL_ERROR(__FUNCTION__ << " : Fatal Error in tag split", 1);
  }
  // returns true if found in both left and right iterators
  bool found = myLeftIter->find(*leftTag) && myRightIter->find(*rightTag);
  // updates tag
  if (found) {
    setTag();
  } else {
    clearTag();
  }
  // free memory for temporary left and right part of the tag
  delete leftTag;
  delete rightTag;
  return found;
}

bool
CrossIterator::isTotalDefined() const {
  return (myLeftIter->isTotalDefined() && myRightIter->isTotalDefined());
}

unsigned int
CrossIterator::getTotalItemNb() const {
  return (myLeftIter->getTotalItemNb() * myRightIter->getTotalItemNb());
}


/*****************************************************************************/
/*                         FlatCrossIterator                                 */
/*****************************************************************************/

FlatCrossIterator::FlatCrossIterator(InputIterator* leftIter,
                                     InputIterator* rightIter)
  : CrossIterator(leftIter, rightIter) {
  myId = createId(leftIter, rightIter);
}

FlatCrossIterator::~FlatCrossIterator() {
}

std::string
FlatCrossIterator::createId(InputIterator* leftIter,
                            InputIterator* rightIter) {
  return ("(" + leftIter->getId() + "_Fx_" + rightIter->getId() + ")");
}

/**
 * Updates the current tag (used when iterator position changes)
 */
bool
FlatCrossIterator::setTag() {
  if (!isIndexReady()) {
    INTERNAL_ERROR(__FUNCTION__
                   << "Invalid call: index cannot be computed\n", 1);
  }
  clearTag();
  // check if right tag is of level 1
  // (flat cross does not handle level > 1 on the right)
  if (myRightIter->getCurrentTag().getLevel() > 1) {
    WARNING("Operator " << getId() << " cannot handle this data depth\n");
  }
  // parent tag
  FDataTag tmpTag = myLeftIter->getCurrentTag();
  tmpTag.getParent();
  // index
  unsigned int index =
    myLeftIter->getCurrentTag().getLastIndex() * myRightIter->getTotalItemNb()
    + myRightIter->getCurrentTag().getLastIndex();
  // last
  bool isLast = myLeftIter->getCurrentTag().isLastOfBranch()
    && myRightIter->getCurrentTag().isLastOfBranch();

  currTag = new FDataTag(tmpTag, index, isLast);
  return true;
}

/**
 * Check if the current item can NOT be selected (either already selected or
 * index not available)
 */
bool
FlatCrossIterator::isFlagged() {
  return CrossIterator::isFlagged() || !isIndexReady();
}

/**
 * (private)
 * Test if the index can be computed
 */
bool
FlatCrossIterator::isIndexReady() {
  return myRightIter->isTotalDefined()
    || (myLeftIter->getCurrentTag().getLastIndex() == 0);
}

bool
FlatCrossIterator::splitTag(const FDataTag& tag, FDataTag*& leftTagPtr,
                            FDataTag*& rightTagPtr) {
  if (!myRightIter->isTotalDefined()) {
    return false;
  }
  // should be a copy of original tag
  FDataTag* tmpTagPtr = tag.getLeftPart(leftTagLength);
  unsigned int origIndex = tmpTagPtr->getLastIndex();
  tmpTagPtr->getParent();
  unsigned int leftIndex = origIndex / myRightIter->getTotalItemNb();
  unsigned int rightIndex = origIndex % myRightIter->getTotalItemNb();
  leftTagPtr = new FDataTag(*tmpTagPtr, leftIndex, false);
  rightTagPtr = new FDataTag(rightIndex, false);
  delete tmpTagPtr;
  return true;
}

/*****************************************************************************/
/*                           MatchIterator                                   */
/*****************************************************************************/

MatchIterator::MatchIterator(InputIterator* leftIter,
                             InputIterator* rightIter)
  : CrossIterator(leftIter, rightIter) {
  myId = createId(leftIter, rightIter);
}

MatchIterator::~MatchIterator() {
}

std::string
MatchIterator::createId(InputIterator* leftIter,
                        InputIterator* rightIter) {
  return ("(" + leftIter->getId() + "_M_" + rightIter->getId() + ")");
}

/**
 * Updates the current tag (used when iterator position changes)
 * TODO this copy of right tag could be avoided ie avoid using currTag
 */
bool
MatchIterator::setTag() {
  clearTag();
  currTag = new FDataTag(myRightIter->getCurrentTag());
  return true;
}

/**
 * Check if the current item can NOT be selected (either already selected or
 * not selectable)
 */
bool
MatchIterator::isFlagged() {
  return !isMatched();
}

bool
MatchIterator::isMatched() {
  const FDataTag& leftTag = myLeftIter->getCurrentTag();
  const FDataTag& rightTag = myRightIter->getCurrentTag();
  if (leftTag.getLevel() > rightTag.getLevel()) {
    WARNING("Operator " << getId() << " : invalid input depths\n");
    return false;
  }
  // truncate the right tag
  FDataTag rightPrefix = rightTag.getAncestor(leftTag.getLevel());
  if (!(rightPrefix < leftTag) && !(leftTag < rightPrefix)) {
    return true;
  } else {
    return false;
  }
}

bool
MatchIterator::splitTag(const FDataTag& tag, FDataTag*& leftTagPtr,
                        FDataTag*& rightTagPtr) {
  leftTagPtr = tag.getLeftPart(leftTagLength);
  rightTagPtr = new FDataTag(tag);  // TODO: who owns this ?
  return true;
}

void
MatchIterator::removeItem() {
  const FDataTag& leftTag  = myLeftIter->getCurrentTag();
  // mark the removed item
  myFlags[*currTag] = true;
  // increment the counter of matched items for the left tag
  incrementMatchCount(leftTag);
  // remove the matched right item (it cannot be used by other left items)
  myRightIter->removeItem();
  // if right iter is at the end then go to next left item
  if (myRightIter->isAtEnd()) {
    myLeftIter->next();
  } else {
    if (!isMatched()) {
      next();
    } else {
      setTag();
    }
  }
}

bool
MatchIterator::isDone() const {
  return myRightIter->isDone();
}

bool
MatchIterator::isTotalDefined() const {
  return myRightIter->isDone();  // all right items processed
}

unsigned int
MatchIterator::getTotalItemNb() const {
  // make sum of all matches for each left item
  int sum = 0;
  for (std::map<FDataTag, int>::const_iterator countIter = myCounters.begin();
       countIter != myCounters.end();
       ++countIter) {
    sum += (int) countIter->second;
  }
  if (sum < 0) {
    INTERNAL_ERROR(__FUNCTION__ << "Invalid sum of counters\n", 1);
  }
  return (unsigned int) sum;
}

/*****************************************************************************/
/*                           DotIterator                                   */
/*****************************************************************************/

DotIterator::DotIterator(const std::vector<InputIterator*>& iterTable)
  : InputIterator(createId(iterTable)), myInputs(iterTable) {
}

std::string
DotIterator::createId(const std::vector<InputIterator*>& iterTable) {
  std::string id = "(";
  std::vector<InputIterator*>::const_iterator inputIter = iterTable.begin();
  // concatenate the IDs of all the inputs to make this operator's id
  while (inputIter != iterTable.end()) {
    id += ((InputIterator*) *inputIter)->getId();
    ++inputIter;
    if (inputIter != iterTable.end()) {
      id += "_._";
    }
  }
  return id + ")";
}

const std::string&
DotIterator::getId() const {
  return myId;
}

// private
InputIterator*
DotIterator::getFirstInput() const {
  return myInputs[0];
}

// private
bool
DotIterator::isMatched() {
  InputIterator *firstInput = getFirstInput();
  // get the current tag (from first input iterator)
  const FDataTag& firstTag = firstInput->getCurrentTag();
  TRACE_TEXT(TRACE_ALL_STEPS, traceId()
             << " matching tag " << firstTag.toString() << "\n");
  // initialize the input loop on second input
  std::vector<InputIterator*>::iterator inputIter = myInputs.begin();
  ++inputIter;
  // loop until one right input does not match or
  // all right inputs match the left tag
  bool allMatched = true;
  while (allMatched && !(inputIter == myInputs.end())) {
    InputIterator *currInput = (InputIterator*) *(inputIter++);
    if (!(currInput->find(firstTag))) {
      allMatched = false;
      TRACE_TEXT(TRACE_ALL_STEPS, traceId() << "match failed for input : "
                 << currInput->getId() << "\n");
    }
  }
  if (allMatched) {
    TRACE_TEXT(TRACE_ALL_STEPS, traceId() << "Match OK\n");
  }
  return allMatched;
}

void
DotIterator::begin() {
  TRACE_TEXT(TRACE_ALL_STEPS, traceId() << "begin() ...\n");
  if (isEmpty()) {
    getFirstInput()->end();
  } else {
    getFirstInput()->begin();
    if (!isAtEnd() && !isMatched()) {
      next();
    }
  }
}

void
DotIterator::end() {
  getFirstInput()->end();
}

void
DotIterator::next() {
  TRACE_TEXT(TRACE_ALL_STEPS, traceId() << "next() ...\n");
  if (isAtEnd()) {
    INTERNAL_ERROR("Calling next on empty match iterator", 1);
  }
  InputIterator *firstInput = getFirstInput();
  bool endOfFirst = false, nextFound = false;

  while (!endOfFirst && !nextFound) {
    firstInput->next();
    endOfFirst = firstInput->isAtEnd();
    if (!endOfFirst && isMatched()) {
      nextFound = true;
    }
  }
}

bool
DotIterator::isEmpty() const {
  bool oneInputEmpty = false;
  std::vector<InputIterator*>::const_iterator inputIter = myInputs.begin();
  while (!oneInputEmpty && !(inputIter == myInputs.end())) {
    oneInputEmpty = ((InputIterator*) *(inputIter++))->isEmpty();
  }
  return oneInputEmpty;
}

bool
DotIterator::isAtEnd() const {
  return getFirstInput()->isAtEnd();
}

bool
DotIterator::isDone() const {
  return getFirstInput()->isDone();
}

void
DotIterator::removeItem() {
  // remove current item from all inputs (and go to next as well)
  for (std::vector<InputIterator*>::iterator inputIter = myInputs.begin();
       inputIter != myInputs.end();
       ++inputIter) {
    ((InputIterator*) *inputIter)->removeItem();
  }
  // go to next matched item
  if (!isAtEnd() && !isMatched()) {
    next();
  }
}

const FDataTag&
DotIterator::getCurrentItem(std::vector<FDataHandle*>& dataLine) {
  if (isAtEnd()) {
    INTERNAL_ERROR(__FUNCTION__ << " : no item available", 1);
  }
  for (std::vector<InputIterator*>::iterator inputIter = myInputs.begin();
       inputIter != myInputs.end();
       ++inputIter) {
    ((InputIterator*) *inputIter)->getCurrentItem(dataLine);
  }

  return getCurrentTag();
}

const FDataTag&
DotIterator::getCurrentTag() {
  return getFirstInput()->getCurrentTag();
}

bool
DotIterator::find(const FDataTag& tag) {
  return (getFirstInput()->find(tag) && isMatched());
}

bool
DotIterator::isTotalDefined() const {
  bool totalDef = false;
  std::vector<InputIterator*>::const_iterator inputIter = myInputs.begin();
  while (!totalDef && !(inputIter == myInputs.end())) {
    totalDef = ((InputIterator*) *inputIter)->isTotalDefined();
  }
  return totalDef;
}

unsigned int
DotIterator::getTotalItemNb() const {
  bool totalDef = false;
  unsigned int total = 0;
  std::vector<InputIterator*>::const_iterator inputIter = myInputs.begin();
  while (!totalDef && !(inputIter == myInputs.end())) {
    totalDef = ((InputIterator*) *inputIter)->isTotalDefined();
    if (totalDef) {
      total = ((InputIterator*) *inputIter)->getTotalItemNb();
    }
  }
  return total;
}

