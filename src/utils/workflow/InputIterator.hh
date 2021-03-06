/**
* @file  InputIterator.hh
* 
* @brief  Set of classes used to implement data operators for functional workflows
* 
* @author  Benjamin ISNARD (Benjamin.Isnard@ens-lyon.fr)
* 
* @section Licence
*   |LICENSE|                                                                
*/


#ifndef _INPUTITERATOR_HH_
#define _INPUTITERATOR_HH_

#include <map>
#include <string>
#include <vector>

#include "FNodePort.hh"
#include "FDataHandle.hh"


/**
 * Abstract class: InputIterator
 * Defines the common interface to all iterators
 * (used by the FProcNode class to iterate over input queues)
 */

class InputIterator {
public:
  explicit InputIterator(const std::string& id);

  /**
   * Returns the identifier of the iterator
   * (mainly used to create the map of iterators for a node, and for logs)
   */
  virtual const std::string&
  getId() const;

  /**
   * Go to the first available element
   */
  virtual void
  begin() = 0;

  /**
   * Go to the position after the last element
   */
  virtual void
  end() = 0;

  /**
   * Go to the next available element
   */
  virtual void
  next() = 0;

  /**
   * Returns true if the iterator contains no element
   */
  virtual bool
  isEmpty() const = 0;

  /**
   * Returns true if no more element is available
   */
  virtual bool
  isAtEnd() const = 0;

  /**
   * Returns true if all elements have been processed
   */
  virtual bool
  isDone() const = 0;

  /**
   * Remove the current element and go to the next available
   */
  virtual void
  removeItem() = 0;

  /**
   * Get the data handles for all the ports under this iterator
   * @param dataLine a vector of datahandles
   */
  virtual const FDataTag&
  getCurrentItem(std::vector<FDataHandle*>& dataLine) = 0;

  /**
   * Get the current tag
   */
  virtual const FDataTag&
  getCurrentTag() = 0;

  /**
   * Find an element with a given tag
   * (used by MATCH operator)
   * @param tag the tag search parameter
   */
  virtual bool
  find(const FDataTag& tag) = 0;

  /**
   * Returns true if the number of elements this iterator will provide is known
   * (used by CROSS operator to determine when iteration is finished)
   */
  virtual bool
  isTotalDefined() const = 0;

  /**
   * Returns the total nb of elements this iterator will provide (when known)
   * (used by CROSS operator to determine when iteration is finished)
   */
  virtual unsigned int
  getTotalItemNb() const = 0;

protected:
  /**
   * The operator's ID
   */
  std::string myId;

  /**
   * Prefix for all trace messages
   */
  virtual std::string
  traceId() const;
};

class PortInputIterator : public InputIterator {
public:
  explicit PortInputIterator(FNodeInPort * inPort);

  void
  begin();

  void
  end();

  void
  next();

  bool
  isEmpty() const;

  bool
  isAtEnd() const;
  //     bool isComplete() const;
  bool
  isDone() const;

  void
  removeItem();

  const FDataTag&
  getCurrentItem(std::vector<FDataHandle*>& dataLine);

  const FDataTag&
  getCurrentTag();

  bool
  find(const FDataTag& tag);

  bool
  isTotalDefined() const;

  unsigned int
  getTotalItemNb() const;

protected:
  FNodeInPort * myInPort;
  std::map<FDataTag, FDataHandle*>::iterator myQueueIter;
  unsigned int removedItemsCount;
};

class CrossIterator : public InputIterator {
public:
  CrossIterator(InputIterator* leftIter, InputIterator* rightIter);

  ~CrossIterator();

  virtual void
  begin();

  virtual void
  end();

  virtual void
  next();

  virtual bool
  isEmpty() const;

  virtual bool
  isAtEnd() const;

  virtual bool
  isDone() const;

  virtual void
  removeItem();

  virtual const FDataTag&
  getCurrentItem(std::vector<FDataHandle*>& dataLine);

  virtual const FDataTag&
  getCurrentTag();

  virtual bool
  find(const FDataTag& tag);  // modifies the left iterator

  virtual bool
  isTotalDefined() const;

  virtual unsigned int
  getTotalItemNb() const;

protected:
  virtual std::string
  createId(InputIterator* leftIter, InputIterator* rightIter);

  virtual bool
  isFlagged();

  int
  incrementMatchCount(const FDataTag& leftTag);

  bool
  checkItemAvailable();

  virtual bool
  setTag();

  void
  clearTag();

  virtual bool
  splitTag(const FDataTag& tag, FDataTag*& leftTagPtr, FDataTag*& rightTagPtr);

  InputIterator* myLeftIter;
  InputIterator* myRightIter;
  FDataTag* currTag;
  unsigned int leftTagLength;  // to be initialized at first find
  // call by calling left.begin and use the length of the tag of left.current
  // TODO following maps could use a hash code from the tag to be faster
  std::map<FDataTag, bool> myFlags;  // keys are the complete tags (left+right)
  std::map<FDataTag, int> myCounters;  // keys are the left tags
};


class FlatCrossIterator : public CrossIterator {
public:
  FlatCrossIterator(InputIterator* leftIter, InputIterator* rightIter);

  ~FlatCrossIterator();

protected:
  virtual std::string
  createId(InputIterator* leftIter, InputIterator* rightIter);

  virtual bool
  isFlagged();

  virtual bool
  setTag();

  virtual bool
  splitTag(const FDataTag& tag, FDataTag*& leftTagPtr, FDataTag*& rightTagPtr);

private:
  bool
  isIndexReady();
};

class MatchIterator : public CrossIterator {
public:
  MatchIterator(InputIterator* leftIter, InputIterator* rightIter);

  ~MatchIterator();

  virtual void
  removeItem();

  virtual bool
  isDone() const;

  virtual bool
  isTotalDefined() const;

  virtual unsigned int
  getTotalItemNb() const;

protected:
  virtual std::string
  createId(InputIterator* leftIter, InputIterator* rightIter);

  virtual bool
  isFlagged();

  virtual bool
  setTag();

  virtual bool
  splitTag(const FDataTag& tag, FDataTag*& leftTagPtr, FDataTag*& rightTagPtr);

private:
  bool
  isMatched();
};

class DotIterator : public InputIterator {
public:
  explicit DotIterator(const std::vector<InputIterator*>& iterTable);

  const std::string&
  getId() const;

  void
  begin();

  void
  end();

  void
  next();

  bool
  isEmpty() const;

  bool
  isAtEnd() const;

  bool
  isDone() const;

  void
  removeItem();

  const FDataTag&
  getCurrentItem(std::vector<FDataHandle*>& dataLine);

  const FDataTag&
  getCurrentTag();

  bool
  find(const FDataTag& tag);

  bool
  isTotalDefined() const;

  unsigned int
  getTotalItemNb() const;

private:
  virtual std::string
  createId(const std::vector<InputIterator*>& iterTable);

  InputIterator*
  getFirstInput() const;

  bool
  isMatched();

  /**
   * Table containing all the inputs of the operator
   */
  std::vector<InputIterator*>  myInputs;
};

#endif
