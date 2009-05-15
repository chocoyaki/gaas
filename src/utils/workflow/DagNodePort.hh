/****************************************************************************/
/* The port classes used for dag execution                                  */
/* The main classes are DagNodePort, DagNodeOutPort and DagNodeInPort       */
/*                                                                          */
/* Author(s):                                                               */
/* - Benjamin ISNARD (benjamin.isnard@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.10  2009/05/15 11:15:07  bisnard
 * container initialization adapted to standard notation
 * added method to retrieve container cardinal
 *
 * Revision 1.9  2009/04/17 09:02:15  bisnard
 * container empty elements (added WfVoidAdapter class)
 *
 * Revision 1.8  2009/02/06 14:53:09  bisnard
 * - setup exceptions
 * - display methods for container results
 *
 * Revision 1.7  2009/01/16 13:55:36  bisnard
 * changes in dag event handling methods
 *
 * Revision 1.6  2008/12/09 12:15:59  bisnard
 * pending instanciation handling (uses dag outputs for instanciation
 * of a functional wf)
 *
 * Revision 1.5  2008/12/02 10:14:51  bisnard
 * modified nodes links mgmt to handle inter-dags links
 *
 * Revision 1.4  2008/10/30 14:33:01  bisnard
 * added recursive container initialization
 *
 * Revision 1.3  2008/10/22 09:29:00  bisnard
 * replaced uint by standard type
 *
 * Revision 1.2  2008/10/20 07:59:29  bisnard
 * file header creation
 *
 */

#ifndef _DAGNODEPORT_HH_
#define _DAGNODEPORT_HH_

#include <string>
#include <list>

#include "DIET_data.h"
#include "WfPortAdapter.hh"
#include "WfUtils.hh"
#include "WfPort.hh"
#include "DagNode.hh"

using namespace std;

/*******************************/
/* class declarations          */
/*******************************/

class DagNodePort : public WfPort {

public:

  DagNodePort(DagNode * parent,
              string _id,
              WfPort::WfPortType _portType,
              WfCst::WfDataType _type,
              unsigned int _depth,
              unsigned int _ind) ;

  /**
   * Separator used for initializing or displaying container elements
   */
  static string containerSeparator;

  /**
   * Set the value of the data for the port
   */
  void
  setValue(const string& value);

  /**
   * Initialize the profile before node submission
   */
  virtual void
  initProfileSubmit();

  /**
   * Initialize the profile before node execution
   */
  virtual void
  initProfileExec() throw (WfDataException);

  /**
   * Return the XML description of the port
   */
  virtual string
  toXML() const;

  /**
   * Return the profile of the node
   * used by WfPortAdapter::getSourceDataID
   */
  diet_profile_t *
  profile();

  /**
   * Return the data ID of the port
   * @exception WfDataException(eID_UNDEF) if the data ID is not defined
   */
  const string&
  getDataID() throw (WfDataException);

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
   * Initializes the profile when a value is provided
   */
  void
  setProfileWithValue();

  /**
   * Converts the string value to an allocated buffer for a matrix
   */
  void
  initMatrixValue(void **buffer, const string& value);

  /**
   * Converts the string value to a data handle for a container
   * Examples of input: 3;4;5  or ((toto;titi);(tata))
   * @param value container value (uses parenthesis & semicolumns)
   */
  void
  initContainerValue(const string& value);

  /**
   * Stores a data defined by a value string into the data manager
   * @param valStr  the value
   * @param valID   will contain the ID of the data
   */
  void
  initContainerElementValue(const char* valStr, char** valIDPtr);

  /**
   * The port data ID
   */
  string dataID;

  /**
   * The value of the data (if present)
   */
  string value;

  /**
   * The reference of port node
   */
  DagNode * myParent;

private:

  /**
   * Container initialization from string (recursive)
   * @param contIDPtr will contain the ID of the created container
   * @param contStr   string to parse (already parsed content is removed)
   * @param contDepth depth of the current container (must be >= 1)
   */
  void
  initContainerValueRec(char** contIDPtr,
                        string& contStr,
                        unsigned int contDepth);

}; // end DagNodePort

class DagNodeOutPort : virtual public DagNodePort {
public:
  DagNodeOutPort(DagNode * parent,
                 string _id,
                 WfCst::WfDataType _type,
                 unsigned int _depth,
                 unsigned int _ind);

  ~DagNodeOutPort();

  /**
   * Store the data IDs from the profile
   */
  void
  storeProfileData();

  /**
   * Free the persistent data
   */
  void
  freeProfileData();

  /**
   * Return the XML description of the port
   */
  virtual string
  toXML() const;

  /**
   * Get the cardinal of the content of the output data (when it's a container)
   * (for any ID within the container tree of the output)
   * @param dataID  the ID of the data container
   * @return  the size of the container
   */
  unsigned int
  getDataIDCardinal(const string& dataID) throw (WfDataException);

  /**
   * Get the ID of an element of the output data (when it's a container)
   * @param eltIdx a list of indexes for the element
   * @return  the data ID (may be empty if the element is NULL)
   * @exception WfDataException(eID_UNDEF) if the data ID is not defined
   */
  string
  getElementDataID(const list<unsigned int>& eltIdx) throw (WfDataException);

  /**
   * Display the output data in a stream
   * If the data is a container then it's displayed as a parenthezized
   * comma-separated list
   * @param output  the output stream
   */
  void
  displayDataAsList(ostream& output) throw (WfDataException);

  /**
   * Display an element of the output data (when it's a container)
   * @param output  the output stream
   * @param eltIdx a list of indexes for the element
   */
  void
  displayDataElementAsList(ostream& output, const list<unsigned int>& idxList)
      throw (WfDataException);

protected:

  /**
   * Get the content (list of data IDs) of the output data
   * (the element can be the root data generated by this port or any
   * sub-element of this root data if this is a container)
   * Note: uses a cache to avoid calling DAGDA each time the same value
   * is requested (happens in instanciation)
   * @param dataID  the ID of the data element
   * @return  container content structure (no deallocation needed)
   */
#if HAVE_DAGDA
  diet_container_t*
  getDataIDList(const string& dataID) throw (WfDataException);
#endif

  /**
   * Display method used only when port type is container
   * (recursive)
   */
  void
  displayContainerAsList(ostream& output,
                         const char* containerID,
                         unsigned int depth) throw (WfDataException);

  /**
   * Display method used for container elements (dagda API)
   */
  void
  displayContainerData(ostream& output, const char* eltID);

  /**
   * Returns the persistence mode for this port
   */
  diet_persistence_mode_t
      getPersistenceMode();

  /**
   * Cache for container content
   */
  map<string,diet_container_t*> myCache;

};

class DagNodeInPort : virtual public DagNodePort {
public:
  DagNodeInPort(DagNode * parent,
                string _id,
                WfCst::WfDataType _type,
                unsigned int _depth,
                unsigned int _ind);

  /**
   * Initialize the profile before node execution
   */
  virtual void
  initProfileExec() throw (WfDataException);

  /**
   * Return the XML description of the port
   */
  virtual string
  toXML() const;

protected:
  /**
   * Source Data ID retrieval (used for node execution)
   * Initialize the profile corresponding to this port with the data
   * provided by the source port (uses data IDs because all output
   * data in a workflow is persistent)
   * @return false if source data cannot be found
   */
  virtual void
  initSourceData() throw (WfDataException) ;

  /**
   * Returns the persistence mode for this port
   */
  diet_persistence_mode_t
  getPersistenceMode();

}; // end DagNodeInPort

class DagNodeInOutPort : virtual public DagNodeInPort,
                           virtual public DagNodeOutPort {
public:
  DagNodeInOutPort(DagNode * parent,
                   string _id,
                   WfCst::WfDataType _type,
                   unsigned int _depth,
                   unsigned int _ind);

  /**
   * Return the XML description of the port
   */
  virtual string
  toXML() const;

protected:
  /**
   * Returns the persistence mode for this port
   */
  diet_persistence_mode_t
      getPersistenceMode();

}; // end DagNodeInOutPort

#endif // _DAGNODEPORT_HH_
