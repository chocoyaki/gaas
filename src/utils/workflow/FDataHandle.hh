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
 * Revision 1.9  2009/05/15 11:10:20  bisnard
 * release for workflow conditional structure (if)
 *
 * Revision 1.8  2009/04/17 09:04:07  bisnard
 * initial version for conditional nodes in functional workflows
 *
 * Revision 1.7  2009/04/08 09:34:56  bisnard
 * pending nodes mgmt moved to FWorkflow class
 * FWorkflow and FNode state graph revisited
 * FNodePort instanciation refactoring
 *
 * Revision 1.6  2009/02/24 14:01:05  bisnard
 * added dynamic parameter mgmt for wf processors
 *
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
#include "DagNodePort.hh"

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
                                 eINVALID_ADAPT,
                                 eCARD_UNDEF,
                                 eVALUE_UNDEF };
    WfDataHandleException(WfDataHandleErrorType t,
                          const string& _info)
      { this->why = t; this->info = _info; }
    WfDataHandleErrorType Type() { return this->why; }
    const string& Info()       { return this->info; }
    string ErrorMsg();
  private:
    WfDataHandleErrorType why;
    string info;
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
     * @param isVoid  set to true to create a VOID data
     * @param parentHdl the parent handle (NULL if root handle or new DH)
     * @param port the port that produces the data (may be NULL)
     * @param portLevel the level of the port (if port is provided)
     */
    FDataHandle(const FDataTag& tag,
                unsigned int depth,
                bool isVoid = false,
                FDataHandle* parentHdl = NULL,
                DagNodeOutPort* port = NULL,
                unsigned int portLevel = 0);

    /**
     * Constructor of a constant data handle (that has a value)
     * @param tag the data tag attached to this handle
     * @param value the value of the data
     */
    FDataHandle(const FDataTag& tag,
                const string& value);

    /**
     * Constructor of a root data handle (used for out ports buffer)
     * @param depth data depth
     */
    FDataHandle(unsigned int depth);

    /**
     * Copy constructor (used for port mappings in control structures)
     */
    FDataHandle(const FDataHandle& src);

    /**
     * Destructor
     * destroys the full data tree
     */
    ~FDataHandle();

    /**
     * Get the depth of this data handle
     * Note: the depth value can be modified by addChild() if the first child inserted
     * as a higher depth than current depth - 1. This is necessary for port buffer
     * (root of the tree) as its actual depth is not known at creation.
     * Note: a strictly positive depth does not necessarily mean the childs exist. They
     * may be 'virtual' until an iterator is created on the data using begin() which
     * will create the childs.
     */
    unsigned int
        getDepth() const;

    /**
     * Get the tag of this data handle
     * (The tag is set at creation and is not modified)
     */
    const FDataTag&
        getTag() const;

    /**
     * Set the predefined cardinal (statically provided before execution)
     * (the list may include 'x' values if cardinal unknown at that level)
     */
    void
        setCardinalList(const list<string>& cardList);
    void
        setCardinalList(list<string>::const_iterator& start,
                        list<string>::const_iterator& end);
    /**
     * Get the cardinal
     * (does not match the nb of childs if not complete)
     * @exception WfDataHandleException(eCARD_UNDEF)
     */
    unsigned int
        getCardinal() const throw (WfDataHandleException);

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
     * Returns true if the port and portLevel attribute are defined
     */
    bool
        isPortDefined() const;

    /**
     * Get the source port (if adapter is direct or simple)
     * @return ptr to port
     * @exception WfDataHandleException(eINVALID_ADAPT)
     */
    DagNodeOutPort*
        getSourcePort() const throw (WfDataHandleException);

    /**
     * Returns true if the data is VOID
     */
    bool
        isVoid() const;

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
     * @param dataHdl the DH to insert in the current DH's tree
     * @exception WfDataHandleException(eBAD_STRUCT) if DH cannot be inserted
     *
     * IMPORTANT: DH takes over memory mgmt of the argument (it will deallocate
     * the pointer when destroyed)
     */
    void
        insertInTree(FDataHandle* dataHdl)
        throw (WfDataHandleException);

    /**
     * Get an iterator on the childs of the data Handle
     * If the cardinal is defined and the childs are not yet generated then this
     * will generate the childs with the correct tags
     * @exception WfDataHandleException(eBAD_STRUCT) if DH depth = 0
     */
    map<FDataTag,FDataHandle*>::iterator
        begin()
        throw (WfDataHandleException);

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
     * Get the value from the dag node (after execution)
     */
    void
        downloadValue();

    /**
     * Get the dataID from the dag node (after execution)
     * @exception WfDataException(eID_UNDEF)
     * @exception WfDataException(eINVALID_CONTAINER)
     */
    void
        downloadDataID();

    /**
     * Get the cardinal from the dag node (after execution)
     */
    void
        downloadCardinal();

    /**
     * Get the content (dataID of elements) from the dag node (after execution)
     */
    void
        downloadElementDataIDs();

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
     */
    void
        displayDataAsList(ostream& output);

  private:

    typedef enum {
      ADAPTER_UNDEFINED,    // initial state
      ADAPTER_DIRECT,       // = linked directly to a port
      ADAPTER_SIMPLE,       // = descendant of direct adapter
      ADAPTER_MULTIPLE,     // = parent of direct/multiple adapters
      ADAPTER_VOID          // = empty value
    } FDataHdlAdapterType;
    // ADAPTER_MULTIPLE status implies that all descendants are defined

    void
        addChild(FDataHandle* dataHdl);

    void
        setParent(FDataHandle* parentHdl);

    bool
        isLastChild() const;

    bool
        checkIfCompleteRec(unsigned int level, unsigned int& total);

    void
        checkAdapter();

    void
        display(bool goUp = false);

    void
        setCardinal(unsigned int card);

    void
        setDataID(const string& dataID);

    void
        setAsVoid();

    unsigned int
        getPortLevel() const;

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
    DagNodeOutPort* myPort;

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
     * port level (tag level of the port providing the data)
     * (defined only for SIMPLE or DIRECT typeAdapter)
     */
    unsigned int myPortLevel;

    /**
     * data cardinal (if it is a container)
     */
    unsigned int myCard;

    /**
     * predefined cardinal info (may contain several levels)
     */
    list<string>* myCardList;

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

    /**
     * flag to check if data ID is defined
     */
    bool dataIDDef;

}; // end class FDataHandle

#endif // _FDATAHANDLE_HH_
