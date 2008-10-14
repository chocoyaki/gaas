

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
              uint _depth,
              uint _ind) ;

  /**
   * Set the value of the data for the port
   */
  void
  setValue(const string& value);

  /**
   * Initialize the profile before node submission
   */
  virtual bool
  initProfileSubmit();

  /**
   * Initialize the profile before node execution
   */
  virtual bool
  initProfileExec();

  /**
   * Return the profile of the node
   * used by WfPortAdapter::getSourceDataID
   */
  diet_profile_t *
  profile();

  /**
   * Return the data ID of the port
   */
  const string&
  getDataID();

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
   */
  void
  initContainerValue(const string& value);

  /**
   * The port data ID
   */
  string dataID;

private:

  /**
   * The reference of port node
   */
  DagNode * myParent;

  /**
   * The value of the data (if present)
   */
  string value;

}; // end DagNodePort

class DagNodeOutPort : virtual public DagNodePort {
public:
  DagNodeOutPort(DagNode * parent,
                 string _id,
                 WfCst::WfDataType _type,
                 uint _depth,
                 uint _ind);

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

protected:

  /**
   * Returns the persistence mode for this port
   */
  diet_persistence_mode_t
      getPersistenceMode();
};

class DagNodeInPort : virtual public DagNodePort {
public:
  DagNodeInPort(DagNode * parent,
                string _id,
                WfCst::WfDataType _type,
                uint _depth,
                uint _ind);

  /**
   * Initialize the profile before node execution
   */
  virtual bool
  initProfileExec();

protected:
  /**
   * Source Data ID retrieval (used for node execution)
   * Initialize the profile corresponding to this port with the data
   * provided by the source port (uses data IDs because all output
   * data in a workflow is persistent)
   * @return false if source data cannot be found
   */
  virtual bool
  initSourceData();

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
                   uint _depth,
                   uint _ind) ;

protected:
  /**
   * Returns the persistence mode for this port
   */
  diet_persistence_mode_t
      getPersistenceMode();

}; // end DagNodeInOutPort

#endif // _DAGNODEPORT_HH_
