/**
* @file FDataHandle.hh
* 
* @brief  Represent a data produced consumed by  a functional workflow node
* 
* @author  Benjamin ISNARD (Benjamin.Isnard@ens-lyon.fr)
* 
* @section Licence
*   |LICENCE|                                                                
*/
/****************************************************************************/
/* The class used to represent a data produced or consumed by an instance   */
/* of a functional workflow node                                            */
/****************************************************************************/


#ifndef _FDATAHANDLE_HH_
#define _FDATAHANDLE_HH_

#include <list>
#include <map>
#include <string>
#include "debug.hh"
#include "DagNodePort.hh"
#include "WfDataWriter.hh"

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

  FDataTag(unsigned int * idxTab, bool * lastTab, unsigned int level);

  ~FDataTag();

  friend int operator<(const FDataTag& tag1, const FDataTag& tag2);

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
   * Returns an ancestor's tag
   * @param level the level of the ancestor
   */
  FDataTag
  getAncestor(unsigned int level) const;

  /**
   * Get direct parent's tag (current tag is modified)
   * Returns ref on itself
   */
  FDataTag&
  getParent();

  /**
   * Get predecessor's tag (current tag is modified)
   * Returns ref on itself
   */
  FDataTag&
  getPredecessor();

  /**
   * Get successor's tag (current tag is modified)
   * (the 'last' flag is set by default to false)
   * Returns ref on itself
   */
  FDataTag&
  getSuccessor();

  /**
   * Get tag with same indexes but 'last' flag set (current tag is modified)
   * The 'last' flag is set to true at the tag's level only (not on lower levels)
   * Returns ref on itself
   */
  FDataTag&
  getTagAsLastOfBranch();

  void
  updateLastFlags(const FDataTag& parentTag);

  /**
   * Converts the tag to a string
   * (used to generate node IDs)
   */
  const std::string&
  toString() const;

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
  std::string myStr;
};

/*****************************************************************************/
/*                        CLASS WfDataHandleException                        */
/*****************************************************************************/

class WfDataHandleException {
public:
  enum WfDataHandleErrorType { eBAD_STRUCT,
                               eINVALID_ADAPT,
                               eCARD_UNDEF,
                               eVALUE_UNDEF,
                               eADAPT_UNDEF };

  WfDataHandleException(WfDataHandleErrorType t, const std::string& _info) {
    this->why = t;
    this->info = _info;
  }

  WfDataHandleErrorType
  Type() {
    return this->why;
  }

  const std::string&
  Info() {
    return this->info;
  }

  std::string
  ErrorMsg();

private:
  WfDataHandleErrorType why;
  std::string info;
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
   * @param port the port that produces the data (NULL if void data)
   */
  FDataHandle(const FDataTag& tag, unsigned int depth,
              bool isVoid = false, DagNodeOutPort* port = NULL);

  /**
   * Constructor of a constant data handle (depth = 0: scalar value)
   * @param tag the data tag attached to this handle
   * @param valueType the type of the data
   * @param value the value of the data
   */
  FDataHandle(const FDataTag& tag, WfCst::WfDataType valueType,
              const std::string& value);

  /**
   * Constructor of a data handle referencing a data ID (from data manager)
   * @param tag       the data tag attached to this handle
   * @param dataType  the type of the data
   * @param dataDepth the data depth (>0 if a data is a container)
   * @param dataId    the data ID string
   * @param isOwner   if true, DH is allowed to free the data (remove from platform)
   */
  FDataHandle(const FDataTag& tag, WfCst::WfDataType dataType,
              unsigned int dataDepth, const std::string& dataId,
              bool isOwner = false);

  /**
   * Constructor of a data handle (used for buffer or data with dataID)
   * @param tag the data tag attached to this handle
   * @param dataType data type (type of leaves)
   * @param dataDepth data depth
   */
  FDataHandle(const FDataTag& tag, WfCst::WfDataType dataType,
              unsigned int dataDepth);

  /**
   * Special copy constructor (used for port mappings in control structures)
   * Copy the whole data tree but changes the tags
   */
  FDataHandle(const FDataTag& tag, const FDataHandle& src);

  /**
   * Copy constructor
   * Copy the whole data tree
   */
  FDataHandle(const FDataHandle& src);

  /**
   * Destructor
   * destroys the full data tree
   */
  ~FDataHandle();

  /**
   * Get the depth of this data handle
   * (depth is always defined in the data tree as it is required in all constructors)
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
   * Get a string description of the handle
   */
  std::string
  toString() const;

  /**
   * Set the data tree properties (cardinal and boundaries)
   * This is done recursively until the leaves of DH's tree.
   * (used when data tree is built from parsing and cardinal is not known in advance)
   */
  void
  updateTreeCardinal();

  /**
   * Check the data Ids of the data tree (recursively until the leaves)
   * If no data Id but value available, get a new data Id
   * If a data Id is invalid then try to recover a new one (if value available)
   * @param MA  the execution agent that manages the data
   * @exception WfDataHandleException(eVALUE_UNDEF) if no value and dataID invalid
   */
  void
  uploadTreeData(MasterAgent_var& MA) throw(WfDataHandleException);

  /**
   * Set the predefined cardinal (statically provided before execution)
   * (the list may include 'x' values if cardinal unknown at that level)
   */
  void
  setCardinalList(const std::list<std::string>& cardList);

  void
  setCardinalList(std::list<std::string>::const_iterator& start,
                  std::list<std::string>::const_iterator& end);
  /**
   * Get the cardinal
   * (does not match the nb of childs if not complete)
   * @exception WfDataHandleException(eCARD_UNDEF)
   */
  unsigned int
  getCardinal() const throw(WfDataHandleException);

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
   * Returns true if the data depends from a unique source port
   * (false for data aggregating several elements from different ports
   *  and for data handle defined by value or data ID)
   */
  bool
  isSourcePortDefined() const;

  /**
   * Get the DAG NODE source port (if defined)
   * @return ptr to port
   * @exception WfDataHandleException(eINVALID_ADAPT)
   */
  DagNodeOutPort*
  getSourcePort() const throw(WfDataHandleException);

  /**
   * Returns true if the data is VOID
   */
  bool
  isVoid() const;

  /**
   * Returns true if the data contains no child
   */
  bool
  isEmpty() const;

  /**
   * Add a data as a child of this data handle
   * This will insert the dataHdl at the right sublevel of the datahandle
   * depending on the TAG of the dataHdl
   * The data level must be HIGHER than the level of this object and the tag
   * prefix should match. For example a tag '2, 3, 1, 4' can be added to the
   * object with tag '2, 3'.
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
    throw(WfDataHandleException);

  /**
   * Get an iterator on the childs of the data Handle
   * If the cardinal is defined and the childs are not yet generated then this
   * will generate the childs with the correct tags
   * @exception WfDataHandleException(eBAD_STRUCT) if DH depth = 0
   */
  std::map<FDataTag, FDataHandle*>::iterator
  begin()
    throw(WfDataHandleException);

  std::map<FDataTag, FDataHandle*>::iterator
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
  createPortAdapter(const std::string& currDagName = "");

  /**
   * Returns true if the handle has a defined value
   */
  bool
  isValueDefined() const;

  /**
   * Set the value of the data
   * Does not change the adapter type
   */
  void
  setValue(WfCst::WfDataType valueType,
            const std::string&     value);

  /**
   * Returns value
   */
  const std::string&
  getValue() const;

  /**
   * Returns value type
   */
  WfCst::WfDataType
  getValueType() const;

  /**
   * Set the data ID of the data (if existing on the platform)
   */
  void
  setDataID(const std::string& dataID);

  /**
   * Returns true if the handle has a defined data ID
   */
  bool
  isDataIDDefined() const;

  /**
   * Returns true if the handle's data ID is owned by this DH
   * (ie the data was created by this DH)
   */
  bool
  isDataIDOwner() const;

  /**
   * Returns data ID
   * @return the data ID or an empty string if not available
   */
  const std::string&
  getDataID() const;

  /**
   * Write the value of the dataHandle (does not use the stored value)
   * @param dataWriter  a data writer to format the data
   * @exception WfDataHandleException(eINVALID_ADAPT)
   * @exception WfDataException
   */
  void
  writeValue(WfDataWriter *dataWriter);

  /**
   * Get the value from the dag node (after execution)
   * Does not change the adapter type
   * @exception WfDataHandleException(eINVALID_ADAPT)
   * @exception WfDataHandleException(eVALUE_UNDEF)
   */
  void
  downloadValue() throw(WfDataHandleException);

  /**
   * Get the dataID from the dag node (after execution)
   * Does change the adapter type to DATAID
   * @exception WfDataHandleException(eINVALID_ADAPT)
   * @exception WfDataException(eID_UNDEF)
   */
  void
  downloadDataID() throw(WfDataHandleException, WfDataException);

  /**
   * Get the dataIDs of all tree and values of the leaves (after execution)
   * @exception NO (replaced by warnings)
   */
  void
  downloadTreeData();

  /**
   * Get the cardinal from the dag node (after execution)
   * @exception WfDataHandleException(eINVALID_ADAPT)
   */
  void
  downloadCardinal();

  /**
   * Returns true if the data contains all its childs at the given level
   */
  bool
  checkIfComplete(unsigned int level);

  /**
   * Returns the count of childs at the given level
   */
  unsigned int
  getChildCount(unsigned int level);

  /**
   * Add a new property to the data handle
   * A property is a string key/value pair attached to the data handle.
   * Properties are automatically inherited by childs.
   * If property already defined, value is overriden. //CONFIRM
   */
  void
  addProperty(const std::string& propKey, const std::string& propValue);

  /**
   * Get the value of a property
   * @param propKey the property name
   * @exception FDataHandleException  if property is not defined
   */
  const std::string&
  getProperty(const std::string& propKey) throw(WfDataHandleException);

  /**
   * Write all the data tree in XML format
   * (list/item structure with dataIDs and values if available)
   * @param XMLWriter a XML writer
   */
  void
  toXML(WfXMLDataWriter& XMLWriter);

  /**
   * Remove persistent data created by the DH from the platform
   * (if DH does not own the data - for ex when data was created by a dagNode -
   * then it is not removed here)
   * RECURSIVE: will call itself on all the DH's childs
   */
  void
  freePersistentDataRec(MasterAgent_var& MA);

private:

  typedef enum {
    ADAPTER_UNDEFINED,    // initial state
    ADAPTER_DIRECT,       // = linked directly to a port
    ADAPTER_SIMPLE,       // = descendant of direct adapter
    ADAPTER_MULTIPLE,     // = parent of direct/multiple adapters
    ADAPTER_VOID,         // = void data (non-existent)
    ADAPTER_VALUE,        // = data value only (constant or data source)
    ADAPTER_DATAID,       // = data ID available
    ADAPTER_DELETED       // = data has been deleted from the platform
  } FDataHdlAdapterType;
  // ADAPTER_MULTIPLE status implies that all descendants are defined
  // all adapter types may change to ADAPTER_DATAID when setDataID is called
  // an ADAPTER_DATAID may change to ADAPTER_UNDEFINED or ADAPTER_VALUE if
  //          the data Id is found invalid

  /**
   * Constructor of a data handle
   * @param tag the data tag attached to this handle
   * @param depth the depth of this data (0 if not a container)
   * @param parentHdl the parent handle
   */
  FDataHandle(const FDataTag& tag,
              unsigned int depth,
              FDataHandle* parentHdl);

  void
  addChild(FDataHandle* dataHdl);

  void
  setParent(FDataHandle* parentHdl);

  bool
  isLastChild() const;

  void
  updateAncestors();

  void
  updateTreeCardinalRec(bool isLast, bool parentTagMod = false);

  void
  display(bool goUp = false);

  void
  setCardinal(unsigned int card);

  void
  setAsVoid();

  /**
   * the tag associated with this data handle
   */
  FDataTag  myTag;

  /**
   * the map of sub-data handles
   */
  std::map<FDataTag, FDataHandle*>*  myData;

  /**
   * my parent data handle (if applicable)
   */
  FDataHandle*  myParentHdl;

  /**
   * the port providing the data (if applicable)
   */
  DagNodeOutPort* myPort;

  /**
   * the list of indexes of the port element (if ADAPTER_SIMPLE)
   */
  std::list<unsigned int>* myPortElementIndexes;

  /**
   * type of port adapter
   */
  FDataHdlAdapterType myAdapterType;

  /**
   * value (stored in XML format)
   */
  std::string myValue;

  /**
   * value type (used for ADAPTER_VALUE only)
   */
  WfCst::WfDataType myValueType;

  /**
   * data ID
   */
  std::string myDataID;

  /**
   * data depth (if container)
   */
  unsigned int myDepth;

  /**
   * data cardinal (if it is a container)
   */
  unsigned int myCard;

  /**
   * predefined cardinal info (may contain several levels)
   */
  std::list<std::string>* myCardList;

  /**
   * completion depth (= childs at this depth are all defined)
   */
  unsigned int myCompletionDepth;

  /**
   * flag to check if cardinal is defined
   */
  bool cardDef;

  /**
   * flag to check if value is defined
   */
  bool valueDef;

  /**
   * flag to check if data ID is owned
   */
  bool dataIDOwner;

};  // end class FDataHandle

#endif  // _FDATAHANDLE_HH_
