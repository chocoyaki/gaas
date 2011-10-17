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
 * Revision 1.4  2009/10/02 07:44:56  bisnard
 * new wf data operators MATCH & N-CROSS
 *
 * Revision 1.3  2009/08/26 10:26:29  bisnard
 * added new iterator flatcross
 *
 * Revision 1.2  2009/05/27 08:56:47  bisnard
 * moved id attribute to parent abstract class
 *
 * Revision 1.1  2009/01/16 13:52:36  bisnard
 * new class to manage different operators to mix input streams of data
 *
 */

#ifndef _INPUTITERATOR_HH_
#define _INPUTITERATOR_HH_

#include <map>
#include <string>
#include <vector>

#include "FNodePort.hh"
#include "FDataHandle.hh"

using namespace std;

/**
 * Abstract class: InputIterator
 * Defines the common interface to all iterators
 * (used by the FProcNode class to iterate over input queues)
 */

class InputIterator {

public:

  InputIterator(const string& id);

  /**
   * Returns the identifier of the iterator
   * (mainly used to create the map of iterators for a node, and for logs)
   */
  virtual const string&
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
  getCurrentItem(vector<FDataHandle*>& dataLine) = 0;

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
  string myId;

  /**
   * Prefix for all trace messages
   */
  virtual string traceId() const;

};

class PortInputIterator : public InputIterator {

public:
  PortInputIterator(FNodeInPort * inPort);
  void begin();
  void end();
  void next();
  bool isEmpty() const;
  bool isAtEnd() const;
  //     bool isComplete() const;
  bool isDone() const;
  void removeItem();
  const FDataTag& getCurrentItem(vector<FDataHandle*>& dataLine);
  const FDataTag& getCurrentTag();
  bool find(const FDataTag& tag);
  bool isTotalDefined() const;
  unsigned int getTotalItemNb() const;

protected:
  FNodeInPort * myInPort;
  map<FDataTag, FDataHandle*>::iterator myQueueIter;
  unsigned int removedItemsCount;

};

class CrossIterator : public InputIterator {

public:
  CrossIterator(InputIterator* leftIter,
                InputIterator* rightIter);
  ~CrossIterator();
  virtual void begin();
  virtual void end();
  virtual void next();
  virtual bool isEmpty() const;
  virtual bool isAtEnd() const;
  virtual bool isDone() const;
  virtual void removeItem();
  virtual const FDataTag& getCurrentItem(vector<FDataHandle*>& dataLine);
  virtual const FDataTag& getCurrentTag();
  virtual bool find(const FDataTag& tag);  // modifies the left iterator
  virtual bool isTotalDefined() const;
  virtual unsigned int getTotalItemNb() const;

protected:
  virtual string createId(InputIterator* leftIter,
                          InputIterator* rightIter);
  virtual bool isFlagged();
  int  incrementMatchCount(const FDataTag& leftTag);
  bool checkItemAvailable();
  virtual bool setTag();
  void clearTag();
  virtual bool splitTag(const FDataTag& tag,
                        FDataTag*& leftTagPtr, FDataTag*& rightTagPtr);

  InputIterator* myLeftIter;
  InputIterator* myRightIter;
  FDataTag* currTag;
  unsigned int leftTagLength;  // to be initialized at first find call by calling left.begin
  // and use the length of the tag of left.current

  // TODO following maps could use a hash code from the tag to be faster
  map<FDataTag, bool> myFlags;  // keys are the complete tags (left+right)
  map<FDataTag, int> myCounters;  // keys are the left tags
};


class FlatCrossIterator : public CrossIterator {

public:
  FlatCrossIterator(InputIterator* leftIter,
                    InputIterator* rightIter);
  ~FlatCrossIterator();

protected:
  virtual string createId(InputIterator* leftIter,
                          InputIterator* rightIter);
  virtual bool isFlagged();
  virtual bool setTag();
  virtual bool splitTag(const FDataTag& tag,
                        FDataTag*& leftTagPtr, FDataTag*& rightTagPtr);

private:
  bool isIndexReady();
};

class MatchIterator : public CrossIterator {

public:
  MatchIterator(InputIterator* leftIter,
                InputIterator* rightIter);
  ~MatchIterator();

  virtual void removeItem();
  virtual bool isDone() const;
  virtual bool isTotalDefined() const;
  virtual unsigned int getTotalItemNb() const;

protected:
  virtual string createId(InputIterator* leftIter,
                          InputIterator* rightIter);
  virtual bool isFlagged();
  virtual bool setTag();
  virtual bool splitTag(const FDataTag& tag,
                        FDataTag*& leftTagPtr, FDataTag*& rightTagPtr);
private:
  bool isMatched();

};

class DotIterator : public InputIterator {
public:
  DotIterator(const vector<InputIterator*>& iterTable);
  const string& getId() const;
  void begin();
  void end();
  void next();
  bool isEmpty() const;
  bool isAtEnd() const;
  bool isDone() const;
  void removeItem();
  const FDataTag& getCurrentItem(vector<FDataHandle*>& dataLine);
  const FDataTag& getCurrentTag();
  bool find(const FDataTag& tag);
  bool isTotalDefined() const;
  unsigned int getTotalItemNb() const;

private:
  virtual string createId(const vector<InputIterator*>& iterTable);
  InputIterator* getFirstInput() const;
  bool isMatched();

  /**
   * Table containing all the inputs of the operator
   */
  vector<InputIterator*>  myInputs;
};

#endif
