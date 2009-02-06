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
 * Revision 1.5  2009/02/06 14:55:08  bisnard
 * setup exceptions
 *
 * Revision 1.4  2009/01/16 13:54:50  bisnard
 * new version of the dag instanciator with iteration strategies for nodes with multiple input ports
 *
 * Revision 1.3  2008/12/09 12:15:59  bisnard
 * pending instanciation handling (uses dag outputs for instanciation
 * of a functional wf)
 *
 * Revision 1.2  2008/12/02 14:18:20  bisnard
 * obsolete attribute myDagName
 *
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
    FDataTag(const FDataTag& parentTag, const FDataTag& childTag);
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
     * Returns true if this is an empty tag
     */
    bool
    isEmpty() const;

    /**
     * Returns an index of the tag
     */
    unsigned int
    getIndex(unsigned int level) const;

    /**
     * Returns the last index of the tag
     */
    unsigned int
    getLastIndex() const;

    /**
     * Returns the flat index of the tag
     * ie the index of the tag in the leaves list
     * (used only if this is the last tag, to determine the total
     * nb of items)
     */
    unsigned int
    getFlatIndex() const;

    /**
     * Returns the top indexes of the tag
     * ie a partial tag containing the indexes from level 1 to the provided int
     * @param maxLevel the level of the last index of the returned tag
     */
    FDataTag *
    getLeftPart(unsigned int maxLevel) const;

    /**
     * Returns the bottom indexes of the tag
     * @param minLevel the level of the first index of the returned tag
     */
    FDataTag *
    getRightPart(unsigned int minLevel) const;

    /**
     * Returns the parent tag
     */
    FDataTag
    getParent(unsigned int level) const;

    /**
     * Converts the tag to a string
     * (used to generate node IDs)
     */
    const string&
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
     * Initializes the string version of the tag (used in constructors)
     */
    void
    initStr();

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

    /**
     * String version of the tag
     */
    string myStr;
};

/*****************************************************************************/
/*                        CLASS WfDataHandleException                          */
/*****************************************************************************/

class WfDataHandleException {
  public:
    enum WfDataHandleErrorType { eBAD_STRUCT,
                               eINVALID_ADAPT };
    WfDataHandleException(WfDataHandleErrorType t,
                          const string& _info,
                          const FDataTag& _tag)
      { this->why = t; this->info = _info; this->tag = _tag; }
    WfDataHandleErrorType Type() { return this->why; }
    const string& Info()       { return this->info; }
    const FDataTag& Tag()      { return this->tag; }
    string ErrorMsg();
  private:
    WfDataHandleErrorType why;
    string info;
    FDataTag tag;
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
    getDepth() const;

    const FDataTag&
    getTag() const;

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
    getCardinal() const;

    /**
     * Returns true if the cardinal is known
     * (if true then calling begin() the first time will create the childs)
     */
    bool
    isCardinalDefined() const;

    /**
     * Returns true if the parent is defined
     */
    bool
    isParentDefined() const;

    /**
     * Get the parent handle
     */
    FDataHandle*
    getParent() const;

    /**
     * Add a data as a child of this data handle
     * This will insert the dataHdl at the right sublevel of the datahandle
     * depending on the TAG of the dataHdl
     * The data level must be HIGHER than the level of this object and the tag
     * prefix should match. For example a tag '2,3,1,4' can be added to the
     * object with tag '2,3'.
     * If there are missing nodes in the tree then they will be created.
     * If the inserted child is a direct child and its tag is marked as last then
     * this handle's cardinal is set as defined
     */
    void
    insertInTree(FDataHandle* dataHdl)
        throw (WfDataHandleException);

    /**
     * Get an iterator on the childs of the data Handle
     * If the cardinal is defined and the childs are not yet generated then this
     * will generate the childs with the correct tags
     */
    map<FDataTag,FDataHandle*>::iterator
    begin() throw (WfDataHandleException);

    map<FDataTag,FDataHandle*>::iterator
    end();

    /**
     * Returns true if the handle has a defined reference to a
     * port (that can be retrieved as a port adapter)
     * This property is true if port is not null or inherited from the
     * parent or built from the same property of the childs.
     */
    bool
    isAdapterDefined() const;

    /**
     * Create the port adapter corresponding to this data handle
     * @param currDagName (optional)  used for external links
     * @return ptr to port adapter (to be deallocated by caller) or NULL if the
     *         adapter is not defined
     */
    WfPortAdapter*
    createPortAdapter(const string& currDagName = "");

    /**
     * Returns true if the handle has a defined value
     */
    bool
    isValueDefined() const;

    /**
     * Returns value
     */
    const string&
    getValue() const;

    /**
     * Returns true if the handle has a defined data ID
     */
    bool
    isDataIDDefined() const;

    /**
     * Returns data ID
     */
    const string&
    getDataID() const;

    /**
     * Set data ID
     */
    void
    setDataID(const string& dataID);

    /**
     * Checks recursively if the adapter is defined (up the tree)
     */
    void checkAdapter();

    /**
     * Returns true if the data contains all its childs at the given level
     * and updates the table with the total nb of childs at each level (up to
     * the given level)
     */
    bool
    checkIfComplete(unsigned int level,
                    vector<unsigned int>& childNbTable);

    /**
     * Display the content of the dataHandle as a parenthezized list
     * @param output  the output stream
     * @param goUp    will display the full tree if the current data is a node
     */
    void
    displayDataAsList(ostream& output);

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
    isLastChild() const;

    bool
    checkIfCompleteRec(unsigned int level, unsigned int& total);

    void
    display(bool goUp = false);

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
     * value (if applicable)
     */
    string myValue;

    /**
     * data ID
     */
    string myDataID;

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
     * flag to check if data is locally complete
     * (ie nb of childs match cardinal)
     */
//     bool complete;

    /**
     * flag to check if adapter is defined
     */
    bool adapterDef;

    /**
     * flag to check if value is defined
     */
    bool valueDef;

    /**
     * flag to check if data ID is defined
     */
    bool dataIDDef;

}; // end class FDataHandle

#endif // _FDATAHANDLE_HH_
