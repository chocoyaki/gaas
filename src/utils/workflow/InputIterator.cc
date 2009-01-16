/****************************************************************************/
/* Set of classes used to implement data operators for functional workflows */
/*                                                                          */
/* Author(s):                                                               */
/* - Benjamin ISNARD (benjamin.isnard@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.1  2009/01/16 13:52:36  bisnard
 * new class to manage different operators to mix input streams of data
 *
 */

#include "InputIterator.hh"
#include "debug.hh"

/*****************************************************************************/
/*                       PortInputIterator                                   */
/*****************************************************************************/

PortInputIterator::PortInputIterator(FNodeInPort * inPort)
  : myInPort(inPort), removedItemsCount(0) {}

const string&
PortInputIterator::getId() const {
  return myInPort->getId();
}

void
PortInputIterator::begin() {
  TRACE_TEXT (TRACE_ALL_STEPS,"[ITER] begin() for port : " << getId() << " : "
                      << myInPort->myQueue.size() << " items in the queue" << endl);
  myQueueIter = myInPort->myQueue.begin();
  if (!isAtEnd()) {
  TRACE_TEXT (TRACE_ALL_STEPS,"  and first item is "
      << ((FDataHandle*) myQueueIter->second)->getTag().toString() << endl);
  }
}

void
PortInputIterator::end() {
  TRACE_TEXT (TRACE_ALL_STEPS,"[ITER] end() for port : " << getId() << endl);
  myQueueIter = myInPort->myQueue.end();
}

void
PortInputIterator::next() {
  TRACE_TEXT(TRACE_ALL_STEPS,"[ITER] next item for port : " << getId());
  if (isAtEnd()) {
    INTERNAL_ERROR("Calling next on empty port iterator",1);
  }
  ++myQueueIter;
  if (isAtEnd()) {
    TRACE_TEXT(TRACE_ALL_STEPS," ==> END" << endl);
  } else {
    TRACE_TEXT(TRACE_ALL_STEPS," ==> " << ((FDataHandle*) myQueueIter->second)->getTag().toString() << endl);
  }
}

bool
PortInputIterator::isEmpty() const {
  return (myInPort->myQueue.empty());
}

bool
PortInputIterator::isAtEnd() const {
//   cout << "in isAtEnd() for port " << getId() << endl;
  return (myQueueIter == myInPort->myQueue.end());
}

bool
PortInputIterator::isDone() const {
  string total = isTotalDefined() ? itoa(getTotalItemNb()) : "undef";
  TRACE_TEXT (TRACE_ALL_STEPS, "[ITER] port " << getId() << " isDone(): total=" << total
       << " / removed=" << removedItemsCount << endl);
  return (isTotalDefined() && (getTotalItemNb() == removedItemsCount));
}

void
PortInputIterator::removeItem() {
  map<FDataTag, FDataHandle*>::iterator toRemove = myQueueIter;
  this->next();
  myInPort->myQueue.erase(toRemove);
  removedItemsCount++;
  TRACE_TEXT (TRACE_ALL_STEPS, "[ITER] port " << getId()
      << " removeItem(): removed=" << removedItemsCount << endl);
}

const FDataTag&
PortInputIterator::getCurrentItem(vector<FDataHandle*>& dataLine) {
  FDataHandle * currDataHdl = (FDataHandle*) myQueueIter->second;
  dataLine[myInPort->getIndex()] = currDataHdl;
  return currDataHdl->getTag();
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
  : myLeftIter(leftIter), myRightIter(rightIter), leftTagLength(0)
    , currTag(NULL) {
  myId = myLeftIter->getId() + "_x_" + myRightIter->getId();
}

CrossIterator::~CrossIterator() {
  if (currTag)
    delete currTag;
}

const string&
CrossIterator::getId() const {
  return myId;
}

/**
 * (private)
 * Updates the current tag (used when iterator position changes)
 */
bool
CrossIterator::setTag() {
  cout << "in CrossIterator::setTag()" << endl;
  if (currTag) delete currTag;
  currTag = new FDataTag(myLeftIter->getCurrentTag(), myRightIter->getCurrentTag());
  return true;
}

/**
 * (private)
 * Clears the current tag (used when iterator position is at the end)
 */
void
CrossIterator::clearTag() {
  cout << "in CrossIterator::clearTag()" << endl;
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
  map<FDataTag,bool>::iterator flagIter = myFlags.find(getCurrentTag());
  return (flagIter != myFlags.end());
}

/**
 * (private)
 * Increment the counter of matched items for an element of the left iterator
 */
int
CrossIterator::incrementMatchCount(const FDataTag& leftTag) {
  int count;
  map<FDataTag,int>::iterator countIter = myCounters.find(leftTag);
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
  TRACE_TEXT (TRACE_ALL_STEPS, "[ITER-CROSS " << myId << "] begin() ..." << endl);
  myLeftIter->begin();
  myRightIter->begin();
  // if right iterator does not contain one item then this is the end
  if (myRightIter->isAtEnd()) myLeftIter->end();
  // go to next available item if possible
  if (checkItemAvailable()) {
    if (setTag() && isFlagged()) next();
  } else clearTag();
}

void
CrossIterator::end() {
  myLeftIter->end();
}

void
CrossIterator::next() {
  TRACE_TEXT (TRACE_ALL_STEPS, "[ITER-CROSS " << myId << "] next() ..." << endl);
  if (isAtEnd()) {
    INTERNAL_ERROR("Calling next on empty cross iterator",1);
  }
  // loop until the end of the left iter or a non-flagged item is found
  bool endOfLeft = false, endOfRight = false, nextFound = false;
  while (!endOfLeft && !nextFound) {
    // loop until the end of the right iter or a non-flagged item is found
    while (!endOfRight && !nextFound) {
      myRightIter->next();
      endOfRight = myRightIter->isAtEnd();
      if (!endOfRight && setTag() && !isFlagged()) nextFound = true;
    }
    if (!nextFound) {
      // end of right iter => go to next on left iter and restart right iter
      TRACE_TEXT (TRACE_ALL_STEPS, "[ITER-CROSS " << myId
                  << "] end of right iter ==> next on left and reset right" << endl);
      myLeftIter->next();
      myRightIter->begin();
      endOfLeft = myLeftIter->isAtEnd();
      if (!endOfLeft && setTag() && !isFlagged()) nextFound = true;
    }
  }
  if (nextFound) {
    TRACE_TEXT (TRACE_ALL_STEPS, "[ITER-CROSS " << myId << "] next() IS "
        << getCurrentTag().toString() << endl);
  } else {
    clearTag();
    TRACE_TEXT (TRACE_ALL_STEPS, "[ITER-CROSS " << myId << "] next() ==> END" << endl);
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
  TRACE_TEXT (TRACE_ALL_STEPS, "[ITER-CROSS " << myId << "] isDone()" << endl);
  return myLeftIter->isDone();
}

void
CrossIterator::removeItem() {
  const FDataTag& leftTag  = myLeftIter->getCurrentTag();
  TRACE_TEXT (TRACE_ALL_STEPS, "[ITER-CROSS " << myId << "] removeItem(): removing "
      << currTag->toString() << endl);
  // mark the removed item
  myFlags[*currTag] = true;
  // increment the counter of matched items for the left tag
  int matchCount = incrementMatchCount(leftTag);
  TRACE_TEXT (TRACE_ALL_STEPS, "[ITER-CROSS " << myId << "] removeItem(): match count of "
      << leftTag.toString() << " is " << matchCount << endl);
  // check if right iter is completed for the current left item
  if (myRightIter->isTotalDefined() && (matchCount == myRightIter->getTotalItemNb())) {
    // if yes, then remove, go to next left item and reset right iter
    myLeftIter->removeItem();
    myRightIter->begin();
    // then check if current is available and if yes find an unmatched item
    if (checkItemAvailable() && setTag() && isFlagged()) next();
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
CrossIterator::getCurrentItem(vector<FDataHandle*>& dataLine) {
  if (!checkItemAvailable()) {
    INTERNAL_ERROR("Tried to get iterator current item although no item available",1);
  }
  myLeftIter->getCurrentItem(dataLine);
  myRightIter->getCurrentItem(dataLine);
  return getCurrentTag();
}


const FDataTag&
CrossIterator::getCurrentTag() {
  if (!currTag) {
    INTERNAL_ERROR("Tried to get iterator current tag although no item available",1);
  }
  return *currTag;
}

bool
CrossIterator::find(const FDataTag& tag) {
  map<FDataTag,bool>::iterator flagIter = myFlags.find(tag);
  if (flagIter != myFlags.end()) {
    return false;
  }
  // initialize left tag's length if not already done
  if (leftTagLength == 0) {
    myLeftIter->begin();
    leftTagLength = myLeftIter->getCurrentTag().getLevel();
  }
  // split the tag in left and right part
  FDataTag *leftTag = tag.getLeftPart(leftTagLength);
  FDataTag *rightTag = tag.getRightPart(leftTagLength+1);
  if ((leftTag == NULL) || (rightTag == NULL)) {
    INTERNAL_ERROR("CrossIterator::find : Fatal Error in tag split",1);
  }
  // returns true if found in both left and right iterators
  bool found = myLeftIter->find(*leftTag) && myRightIter->find(*rightTag);
  // updates tag
  if (found) setTag();
  else clearTag();
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
/*                           DotIterator                                   */
/*****************************************************************************/

DotIterator::DotIterator(const vector<InputIterator*>& iterTable)
  : myInputs(iterTable), myId() {
  vector<InputIterator*>::const_iterator inputIter = myInputs.begin();
  // concatenate the IDs of all the inputs to make this operator's id
  while (inputIter != myInputs.end()) {
    myId += ((InputIterator*) *inputIter)->getId();
    ++inputIter;
    if (inputIter != myInputs.end())
      myId += "_._";
  }
}

const string&
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
  TRACE_TEXT (TRACE_ALL_STEPS, "[ITER-DOT " << myId << "] matching tag "
              << firstTag.toString() << endl);
  // initialize the input loop on second input
  vector<InputIterator*>::iterator inputIter = myInputs.begin();
  ++inputIter;
  // loop until one right input does not match or all right inputs match the left tag
  bool allMatched = true;
  while (allMatched && !(inputIter == myInputs.end())) {
    InputIterator *currInput = (InputIterator*) *(inputIter++);
    if (!(currInput->find(firstTag))) {
        allMatched = false;
        TRACE_TEXT (TRACE_ALL_STEPS, "[ITER-DOT] match failed for input : "
             << currInput->getId() << endl);
    }
  }
  return allMatched;
}

void
DotIterator::begin() {
  TRACE_TEXT (TRACE_ALL_STEPS, "[ITER-DOT " << myId << "] begin() ..." << endl);
  if (isEmpty()) {
    getFirstInput()->end();
  } else {
    getFirstInput()->begin();
    if (!isAtEnd() && !isMatched()) next();
  }
}

void
DotIterator::end() {
  getFirstInput()->end();
}

void
DotIterator::next() {
  TRACE_TEXT (TRACE_ALL_STEPS, "[ITER-DOT " << myId << "] next() ..." << endl);
  if (isAtEnd()) {
    INTERNAL_ERROR("Calling next on empty match iterator",1);
  }
  InputIterator *firstInput = getFirstInput();
  bool endOfFirst = false, nextFound = false;

  while (!endOfFirst && !nextFound) {
    firstInput->next();
    endOfFirst = firstInput->isAtEnd();
    if (!endOfFirst && isMatched()) nextFound = true;
  }
  if (nextFound) {
    TRACE_TEXT (TRACE_ALL_STEPS, "[ITER-DOT " << myId << "] next() IS "
        << getCurrentTag().toString() << endl);
  } else {
    TRACE_TEXT (TRACE_ALL_STEPS, "[ITER-DOT " << myId << "] next() ==> END" << endl);
  }
}

bool DotIterator::isEmpty() const {
  bool oneInputEmpty = false;
  vector<InputIterator*>::const_iterator inputIter = myInputs.begin();
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
  TRACE_TEXT (TRACE_ALL_STEPS, "[ITER-DOT " << myId << "] removeItem() "
      << getCurrentTag().toString() << endl);
  // remove current item from all inputs (and go to next as well)
  for (vector<InputIterator*>::iterator inputIter = myInputs.begin();
       inputIter != myInputs.end();
       ++inputIter)
    ((InputIterator*) *inputIter)->removeItem();
  // go to next matched item
  if (!isAtEnd() && !isMatched()) next();
}

const FDataTag&
DotIterator::getCurrentItem(vector<FDataHandle*>& dataLine) {
  if (isAtEnd()) {
    INTERNAL_ERROR("Tried to get iterator current item although no item available",1);
  }
  for (vector<InputIterator*>::iterator inputIter = myInputs.begin();
       inputIter != myInputs.end();
       ++inputIter)
    ((InputIterator*) *inputIter)->getCurrentItem(dataLine);

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
  vector<InputIterator*>::const_iterator inputIter = myInputs.begin();
  while (!totalDef && !(inputIter == myInputs.end())) {
    totalDef = ((InputIterator*) *inputIter)->isTotalDefined();
  }
  return totalDef;
}

unsigned int
DotIterator::getTotalItemNb() const {
  bool totalDef = false;
  unsigned int total = 0;
  vector<InputIterator*>::const_iterator inputIter = myInputs.begin();
  while (!totalDef && !(inputIter == myInputs.end())) {
    totalDef = ((InputIterator*) *inputIter)->isTotalDefined();
    if (totalDef) total = ((InputIterator*) *inputIter)->getTotalItemNb();
  }
  return total;
}

