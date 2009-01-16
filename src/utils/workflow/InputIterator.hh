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

#ifndef _INPUTITERATOR_HH_
#define _INPUTITERATOR_HH_

#include <vector>
#include <map>

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

    /**
     * Returns the identifier of the iterator
     * (mainly used to create the map of iterators for a node, and for logs)
     */
    virtual const string&
        getId() const = 0;

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

    /**
     * Returns true if the first tag is either the current tag or has been already processed
     * (used by DOT operator to start the iteration)
     */
//     virtual bool
//         hasGotFirstItem() = 0;

};

class PortInputIterator : public InputIterator {

  public:
    PortInputIterator(FNodeInPort * inPort);
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
    FNodeInPort * myInPort;
    map<FDataTag, FDataHandle*>::iterator myQueueIter;
    int removedItemsCount;

};


class CrossIterator : public InputIterator {

  public:
    CrossIterator(InputIterator* leftIter,
                  InputIterator* rightIter);
    ~CrossIterator();
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
    bool find(const FDataTag& tag); // modifies the left iterator
    bool isTotalDefined() const;
    unsigned int getTotalItemNb() const;

  protected:
    bool isFlagged();
    int  incrementMatchCount(const FDataTag& leftTag);
    bool checkItemAvailable();
    bool setTag();
    void clearTag();

  private:
    string myId;
    // TODO following maps could use a hash code from the tag to be faster
    map<FDataTag,bool> myFlags; // keys are the complete tags (left+right)
    map<FDataTag,int> myCounters; // keys are the left tags
    InputIterator* myLeftIter;
    InputIterator* myRightIter;
    FDataTag* currTag;
    unsigned int leftTagLength;  // to be initialized at first find call by calling left.begin
                                 // and use the length of the tag of left.current

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
    InputIterator*
    getFirstInput() const;
    bool isMatched();

    /**
     * The operator's ID (used only for logs)
     */
    string myId;

    /**
     * Table containing all the inputs of the operator
     */
    vector<InputIterator*>  myInputs;
};

/*
class MatchIterator : public InputIterator {
  public:
    MatchIterator(const vector<InputIterator*>& iterTable);
    const string& getId() const;
    void begin(); // begin on all iterators
                  // if one iterator does not have its first item then set isAtEnd=true
    void end();
    void next(); // if all iterators have a following item, then do next on all of them
                 // else set isAtEnd=true
    bool isEmpty() const;
    bool isAtEnd() const; // true if left isAtEnd
    bool isDone() const; // true if one of the iterators is done
    void removeItem(); // if next is available, remove current item on all iterators and go to next
    const FDataTag& getCurrentItem(vector<FDataHandle*>& dataLine);
    const FDataTag& getCurrentTag(); // left.currentTag ???
    bool find(const FDataTag& tag); // UNAPPLICABLE ???
    bool isTotalDefined() const; // true if left.totaldef or right.totaldef
    unsigned int getTotalItemNb() const; // left.total or right.total (should be equal)

  private:
    InputIterator*
    getFirstInput() const;

    string myId;

    vector<InputIterator*>  myInputs;
}; */

#endif
