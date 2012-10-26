/**
* @file DagNodePort.hh
* 
* @brief  The port classes used for dag execution 
* 
* @author  Benjamin ISNARD (Benjamin.Isnard@ens-lyon.fr)
* 
* @section Licence
*   |LICENSE|                                                                
*/
/****************************************************************************/
/* The port classes used for dag execution                                  */
/* The main classes are DagNodePort, DagNodeOutPort and DagNodeInPort       */
/****************************************************************************/


#ifndef _DAGNODEPORT_HH_
#define _DAGNODEPORT_HH_

#include <list>
#include <string>

#include "DIET_data.h"
#include "WfPortAdapter.hh"
#include "WfUtils.hh"
#include "WfPort.hh"
#include "DagNode.hh"

/*****************************************************************************/
/*                        DagNodePort (ABSTRACT)                             */
/*****************************************************************************/
class DagNodePort : public WfPort {
public:
  /**
   * Separator used for initializing or displaying container elements
   */
  static std::string containerSeparator;

  /**
   * Event Message types
   */
  enum eventMsg_e {
    DATAID
  };

  /***************************************************/
  /* constructors/destructor                         */
  /***************************************************/
  DagNodePort(DagNode * parent, std::string _id,
              WfPort::WfPortType _portType, WfCst::WfDataType _type,
              unsigned int _depth, unsigned int _ind);

  /***************************************************/
  /*               public methods                    */
  /***************************************************/
  /**
   * Initialize the profile before node submission
   */
  virtual void
  initProfileSubmit();

  /**
   * Initialize the profile before node execution
   */
  virtual void
  initProfileExec() throw(WfDataException) = 0;

  /**
   * Return the object description as a string
   */
  virtual std::string
  toString() const;

  /**
   * Return the XML description of the port
   */
  virtual void
  toXML(std::ostream& output) const;

  /**
   * Return the profile of the node
   * used by WfPortAdapter::getSourceDataID
   */
  diet_profile_t *
  profile();

  /**
   * Set the data ID of the data received/produced by the port
   */
  void
  setDataID(const std::string& dataID);

  /**
   * Return the data ID of the port
   * @exception WfDataException(eID_UNDEF) if the data ID is not defined
   */
  const std::string&
  getDataID() throw(WfDataException);

  bool
  isDataIDAvailable(MasterAgent_var& MA);

  /**
   * Free the persistent data
   */
  virtual void
  freePersistentData(MasterAgent_var& MA) = 0;

protected:
  /**
   * Returns the persistence mode for this port
   */
  virtual diet_persistence_mode_t
  getPersistenceMode() = 0;

  /**
   * Initializes the profile when no value is provided
   */
  void
  setProfileWithoutValue();

  /**
   * The port data ID
   */
  std::string myDataID;

  /**
   * The reference of port node
   */
  DagNode * myParent;
};


/*****************************************************************************/
/*                           DagNodeOutPort                                  */
/*****************************************************************************/

class DagNodeOutPort : virtual public DagNodePort {
public:
  DagNodeOutPort(DagNode * parent, std::string _id,
                 WfCst::WfDataType _type, unsigned int _depth,
                 unsigned int _ind);

  ~DagNodeOutPort();

  /**
   * Initialize the profile before node execution
   */
  virtual void
  initProfileExec() throw(WfDataException);

  /**
   * Store the data IDs from the profile
   */
  void
  storeProfileData();

  /**
   * Free the persistent data
   */
  void
  freePersistentData(MasterAgent_var& MA);

  /**
   * Return the object description as a string
   */
  virtual std::string
  toString() const;

  /**
   * Return the XML description of the port
   */
  virtual void
  toXML(std::ostream& output) const;

  /**
   * Get the ID of an element of the output data (when it's a container)
   * @param eltIdx a list of indexes for the element
   * @return  the data ID (may be empty if the element is NULL)
   * @exception WfDataException(eID_UNDEF) if the data ID is not defined
   */
  std::string
  getElementDataID(const std::list<unsigned int>& eltIdx)
    throw(WfDataException);

  /**
   * Write the output data in a stream
   * If the data is a container then it's displayed as a parenthezized
   * comma-separated list
   * @param dataWriter  the data output formatter
   */
  void
  writeData(WfDataWriter* dataWriter) throw(WfDataException);

  /**
   * Write an element of the output data (when it's a container)
   * @param dataWriter  the data output formatter
   * @param idxList a list of indexes for the element
   */
  void
  writeDataElement(WfDataWriter* dataWriter,
                   const std::list<unsigned int>& idxList)
    throw(WfDataException);

protected:
  /**
   * Display method used only when port type is container
   * (recursive)
   */
  void
  writeContainer(WfDataWriter* dataWriter, const std::string& containerID,
                 unsigned int depth) throw(WfDataException);
  /**
   * Display method used for container elements (dagda API)
   */
  void
  writeContainerData(WfDataWriter* dataWriter, const std::string& eltID);

  /**
   * Returns the persistence mode for this port
   */
  diet_persistence_mode_t
  getPersistenceMode();
};


/*****************************************************************************/
/*                           DagNodeInPort                                   */
/*****************************************************************************/
class DagNodeInPort : virtual public DagNodePort {
public:
  DagNodeInPort(DagNode * parent, std::string _id,
                WfCst::WfDataType _type, unsigned int _depth,
                unsigned int _ind);

  /**
   * Event Message types
   */
  enum eventMsg_e {
    DEPEND
  };

  /**
   * Set the source of the input port (when dag built by parsing XML)
   * @param strRef The source port (output port) reference
   */
  virtual void
  setConnectionRef(const std::string& strRef);

  /**
   * Set the source of the input port (when dag built by wf instanciator)
   * @param adapter The source port (output port) reference
   */
  virtual void
  setPortAdapter(WfPortAdapter* adapter);

  /**
   * Initialize the profile before node execution
   */
  virtual void
  initProfileExec() throw(WfDataException);

  /**
   * Return the object description as a string
   */
  virtual std::string
  toString() const;

  /**
   * Return the XML description of the port
   */
  virtual void
  toXML(std::ostream& output) const;

  /**
   * Free the persistent data
   */
  void
  freePersistentData(MasterAgent_var& MA);

protected:
  /**
   * Source Data ID retrieval (used for node execution)
   * Initialize the profile corresponding to this port with the data
   * provided by the source port (uses data IDs because all output
   * data in a workflow is persistent)
   * @return false if source data cannot be found
   */
  virtual void
  initSourceData() throw(WfDataException);

  /**
   * Returns the persistence mode for this port
   */
  diet_persistence_mode_t
  getPersistenceMode();
};  // end DagNodeInPort


/*****************************************************************************/
/*                         DagNodeInOutPort                                  */
/*****************************************************************************/

class DagNodeInOutPort : virtual public DagNodeInPort,
                         virtual public DagNodeOutPort {
public:
  DagNodeInOutPort(DagNode * parent, std::string _id,
                   WfCst::WfDataType _type, unsigned int _depth,
                   unsigned int _ind);
  /**
   * Initialize the profile before node execution
   */
  virtual void
  initProfileExec() throw(WfDataException);

  /**
   * Return the object description as a string
   */
  virtual std::string
  toString() const;

  /**
   * Return the XML description of the port
   */
  virtual void
  toXML(std::ostream& output) const;

  /**
   * Free the persistent data
   */
  void
  freePersistentData(MasterAgent_var& MA);

protected:
  /**
   * Returns the persistence mode for this port
   */
  diet_persistence_mode_t
  getPersistenceMode();
};


/*****************************************************************************/
/*                           DagNodeArgPort                                  */
/*****************************************************************************/

class DagNodeArgPort : virtual public DagNodePort {
public:
  DagNodeArgPort(DagNode * parent, std::string _id,
                 WfCst::WfDataType _type, unsigned int _depth,
                 unsigned int _ind);

  /**
   * Set the value of the data for the port
   */
  void
  setValue(const std::string& val);

  /**
   * Initialize the profile before node submission
   */
  virtual void
  initProfileSubmit();

  /**
   * Initialize the profile before node execution
   */
  virtual void
  initProfileExec() throw(WfDataException);

  /**
   * Return the object description as a string
   */
  virtual std::string
  toString() const;

  /**
   * Return the XML description of the port
   */
  virtual void
  toXML(std::ostream& output) const;

  /**
   * Free the persistent data
   */
  virtual void
  freePersistentData(MasterAgent_var& MA);

protected:

  /**
   * Initializes the profile when a value is provided
   */
  void
  setProfileWithValue();

  /**
   * Converts the string value to an allocated buffer for a matrix
   */
  void
  initMatrixValue(void **buffer);

  /**
   * Converts the string value to a data handle for a container
   * Examples of input: (3;4;5)  or ((toto;titi);(tata))
   */
  void
  initContainerValue();

  /**
   * Container initialization from string (recursive)
   * @param contIDPtr will contain the ID of the created container
   * @param contStr   string to parse (already parsed content is removed)
   * @param contDepth depth of the current container (must be >= 1)
   */
  void
  initContainerValueRec(char** contIDPtr, std::string& contStr,
                        unsigned int contDepth);
  /**
   * Stores a data defined by a value string into the data manager
   * @param valStr  the value
   * @param valIDPtr   will contain the ID of the data
   */
  void
  initContainerElementValue(const char* valStr, char** valIDPtr);

  /**
   * Returns the persistence mode for this port
   */
  diet_persistence_mode_t
  getPersistenceMode();

  /**
   * The value of the data
   */
  std::string value;
};  // end DagNodeArgPort


#endif  // _DAGNODEPORT_HH_
