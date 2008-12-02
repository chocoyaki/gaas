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

#ifndef _FDATAHANDLE_HH_
#define _FDATAHANDLE_HH_

#include <map>
#include "debug.hh"
#include "WfPort.hh"


using namespace std;

/*****************************************************************************/
/*                            FDataTag class                                 */
/*****************************************************************************/

class FDataTag {

  public:

    FDataTag();
    FDataTag(const FDataTag& tag);
    FDataTag(unsigned int index, bool isLast);
    FDataTag(const FDataTag& parentTag, unsigned int index, bool isLast);
    ~FDataTag();

    friend int operator<( const FDataTag& tag1, const FDataTag& tag2 );

    /**
     * Returns the level of the tag
     * (= number of indexes)
     */
    unsigned int
    getLevel() const;

    /**
     * Returns true if this is the last element at the level of the tag
     */
    bool
    isLastOfBranch() const;

    /**
     * Returns true if this is the last element (at all levels)
     */
    bool
    isLast() const;

    /**
     * Returns an index of the tag
     */
    unsigned int
    getIndex(unsigned int level) const;

    /**
     * Returns the parent tag
     */
    FDataTag
    getParent(unsigned int level) const;

    /**
     * Converts the tag to a string
     * (used to generate node IDs)
     */
    string
    toString() const;

  protected:

    FDataTag(unsigned int * idxTab, bool * lastTab, unsigned int level);

  private:

    /**
     * Returns true if this is the last element at a given level
     * (recursive)
     */
    bool
    isLastRec(unsigned int level) const;

    /**
     * Contains the index values of the tag
     */
    unsigned int * myIdxs;

    /**
     * Contains the 'last' flags of the tag
     */
    bool * myLastFlags;

    /**
     * Size of the tag (= nb of levels = nb of index values)
     */
    unsigned int mySize;
};

/*****************************************************************************/
/*                         FDataHandle class                                 */
/*****************************************************************************/

class FDataHandle {

  public:

    /**
     * Default constructor
     * (builds a data handle with an empty tag)
     */
    FDataHandle();

    /**
     * Constructor of a data handle
     * @param tag the data tag attached to this handle
     * @param depth the depth of this data (0 if not a container)
     * @param parentHdl the parent handle (NULL if root handle)
     * @param port the port that produces the data (NULL if data is a container element)
     */
    FDataHandle(const FDataTag& tag,
                unsigned int depth,
                FDataHandle* parentHdl = NULL,
                WfPort* port = NULL);

    /**
     * Constructor of a constant data handle (that has a value)
     * @param tag the data tag attached to this handle
     * @param value the value of the data
     * @param parentHdl the parent handle (NULL if root handle)
     */
    FDataHandle(const FDataTag& tag,
                const string& value,
                FDataHandle* parentHdl = NULL);

    /**
     * Constructor of a root data handle (that has no tag)
     * @param depth data depth
     */
    FDataHandle(unsigned int depth);

    /**
     * Destructor
     * destroys the full data tree
     */
    ~FDataHandle();

    unsigned int
    getDepth();

    const FDataTag&
    getTag();

    /**
     * Set the cardinal (at this handle's level)
     */
    void
    setCardinal(unsigned int card);

    /**
     * Get the cardinal
     * (does not match the nb of childs if not complete)
     */
    unsigned int
    getCardinal();

    /**
     * Returns true if the cardinal is known
     * (if true then calling begin() the first time will create the childs)
     */
    bool
    isCardinalDefined();

    /**
     * Returns true if the parent is defined
     */
    bool
    isParentDefined();

    /**
     * Get the parent handle
     */
    FDataHandle*
    getParent();

    /**
     * Add a data as a child of this data handle
     * This will insert the dataHdl at the right sublevel of the datahandle
     * depending on the TAG of the dataHdl
     * The data level must be HIGHER than the level of this object and the tag
     * prefix should match. For example a tag '2,3,1,4' can be added to the
     * object with tag '2,3'.
     * If there are missing nodes in the tree then they will be created.
     * If the inserted child is a direct child and its tag is marked as last then
     * this handle's cardinal is set as defined and checkAdapter is called
     */
    void
    insertInTree(FDataHandle* dataHdl);

    /**
     * Get an iterator on the childs of the data Handle
     * If the cardinal is defined and the childs are not yet generated then this
     * will generate the childs with the correct tags
     * TODO optimize begin()
     */
    map<FDataTag,FDataHandle*>::iterator
    begin();

    map<FDataTag,FDataHandle*>::iterator
    end();

    /**
     * Returns true if the handle has a defined reference to a
     * port (that can be retrieved as a port adapter)
     * This property is true if port is not null or inherited from the
     * parent or built from the same property of the childs.
     */
    bool
    isAdapterDefined();

    /**
     * Create the port adapter corresponding to this data handle
     * @param currDagName (optional)  used for external links
     * @return ptr to port adapter (to be deallocated by caller) or NULL if the
     *         adapter is not defined
     */
    WfPortAdapter*
    createPortAdapter(const string& currDagName = "");

    /**
     * Returns true if the handle as a defined value
     */
    bool
    isValueDefined();

    /**
     * Returns value
     */
    const string&
    getValue();

  private:

    typedef enum {
      ADAPTER_UNDEFINED,
      ADAPTER_DIRECT,
      ADAPTER_SIMPLE,
      ADAPTER_MULTIPLE } FDataHdlAdapterType;

    void
    addChild(FDataHandle* dataHdl);

    void
    setParent(FDataHandle* parentHdl);

    bool
    isLastChild();

    void
    checkAdapter();

    /**
     * the tag associated with this data handle
     */
    FDataTag  myTag;

    /**
     * the map of sub-data handles
     */
    map<FDataTag,FDataHandle*>*  myData;

    /**
     * my parent data handle (if applicable)
     */
    FDataHandle*  myParentHdl;

    /**
     * the port providing the data (if applicable)
     */
    WfPort* myPort;

    /**
     * type of port adapter
     */
    FDataHdlAdapterType myAdapterType;

    /**
     * port dag name
     */
    string myDagName;

    /**
     * value (if applicable)
     */
    string myValue;

    /**
     * data depth (if container)
     */
    unsigned int myDepth;

    /**
     * data cardinal (if it is a container)
     */
    unsigned int myCard;

    /**
     * flag to check if cardinal is defined
     */
    bool cardDef;

    /**
     * flag to check if adapter is defined
     */
    bool adapterDef;

    /**
     * flag to check if value is defined
     */
    bool valueDef;

}; // end class FDataHandle

#endif // _FDATAHANDLE_HH_
