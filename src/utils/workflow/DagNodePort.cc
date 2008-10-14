

#include "debug.hh"
#include "DagNodePort.hh"
#if HAVE_DAGDA
extern "C" {
#include "DIET_Dagda.h"
}
#endif

using namespace std;


DagNodePort::DagNodePort(DagNode * parent,
                         string _id,
                         WfPort::WfPortType _portType,
                         WfCst::WfDataType _type,
                         uint _depth,
                         uint _ind)
  : WfPort(parent, _id, _portType, _type, _depth, _ind),
    myParent(parent) {
}

void
DagNodePort::setProfileWithoutValue() {
  diet_profile_t* profile = myParent->getProfile();
  diet_persistence_mode_t mode = this->getPersistenceMode();
  if ((type == WfCst::TYPE_CHAR)
       || (type == WfCst::TYPE_SHORT)
       || (type == WfCst::TYPE_INT)
       || (type == WfCst::TYPE_LONGINT)
       || (type == WfCst::TYPE_FLOAT)
       || (type == WfCst::TYPE_DOUBLE)) {
    diet_scalar_set(diet_parameter(profile, index),
                    NULL, mode, (diet_base_type_t) WfCst::cvtWfToDietType(type));
  } else {
    switch (type) {
      case WfCst::TYPE_PARAMSTRING :
        diet_paramstring_set(diet_parameter(profile, index),
                             NULL, mode);
        break;
      case WfCst::TYPE_STRING :
        diet_string_set(diet_parameter(profile, index),
                        NULL, mode);
        break;
      case WfCst::TYPE_FILE :
        diet_file_set(diet_parameter(profile, index), mode,
                      NULL);
        break;
      case WfCst::TYPE_MATRIX :
        diet_matrix_set(diet_parameter(profile,index), NULL, mode,
                        (diet_base_type_t) WfCst::cvtWfToDietType(eltType),
                        nb_r, nb_c,
                        (diet_matrix_order_t) WfCst::cvtWfToDietMatrixOrder(order));
        break;
      case WfCst::TYPE_CONTAINER :
        diet_container_set(diet_parameter(profile, index), DIET_PERSISTENT);
        break;
    }
  }
}

void
DagNodePort::setProfileWithValue() {
  diet_profile_t* profile = myParent->getProfile();
  diet_persistence_mode_t mode = this->getPersistenceMode();
  void * mat = NULL;
  switch(type) {
    case WfCst::TYPE_CHAR :
      diet_scalar_set(diet_parameter(profile, index),
                      myParent->newChar(value), mode, DIET_CHAR);
      break;
    case WfCst::TYPE_SHORT :
      diet_scalar_set(diet_parameter(profile, index),
                      myParent->newShort(value), mode, DIET_SHORT);
      break;
    case WfCst::TYPE_INT :
      diet_scalar_set(diet_parameter(profile, index),
                      myParent->newInt(value), mode, DIET_INT);
      break;
    case WfCst::TYPE_LONGINT :
      diet_scalar_set(diet_parameter(profile, index),
                      myParent->newLong(value), mode, DIET_LONGINT);
      break;
    case WfCst::TYPE_FLOAT :
      diet_scalar_set(diet_parameter(profile, index),
                      myParent->newFloat(value), mode, DIET_FLOAT);
      break;
    case WfCst::TYPE_DOUBLE :
      diet_scalar_set(diet_parameter(profile, index),
                      myParent->newDouble(value), mode, DIET_DOUBLE);
      break;
    case WfCst::TYPE_PARAMSTRING :
      diet_paramstring_set(diet_parameter(profile, index),
                           myParent->newString(value), mode);
      break;
    case WfCst::TYPE_STRING :
      diet_string_set(diet_parameter(profile, index),
                      myParent->newString(value), mode);
      break;
    case WfCst::TYPE_FILE :
      diet_file_set(diet_parameter(profile, index), mode,
                    myParent->newFile(value));
      break;
    case WfCst::TYPE_MATRIX :
      initMatrixValue(&mat, value);
      diet_matrix_set(diet_parameter(profile,index), mat, mode,
                      (diet_base_type_t) WfCst::cvtWfToDietType(eltType),
                      nb_r, nb_c,
                      (diet_matrix_order_t) WfCst::cvtWfToDietMatrixOrder(order));
      break;
    case WfCst::TYPE_CONTAINER :
      initContainerValue(value);
      diet_container_set(diet_parameter(profile, index), DIET_PERSISTENT);
      break;
  }
}

/**
 * set the port profile for submission
 */
bool
DagNodePort::initProfileSubmit() {
  // set the value for scalar & paramstring types
  if ((!value.empty()) &&
       ((type == WfCst::TYPE_CHAR)
       || (type == WfCst::TYPE_SHORT)
       || (type == WfCst::TYPE_INT)
       || (type == WfCst::TYPE_LONGINT)
       || (type == WfCst::TYPE_FLOAT)
       || (type == WfCst::TYPE_DOUBLE)
       || (type == WfCst::TYPE_PARAMSTRING)))
    setProfileWithValue();
  else
    setProfileWithoutValue();
  return true;
}

/**
 * set the port profile for execution
 */
bool
DagNodePort::initProfileExec() {
  if (!value.empty())
    setProfileWithValue();
  else
    setProfileWithoutValue();
  return true;
}

void
DagNodePort::setValue(const string& value) {
  this->value = value;
}

void
DagNodePort::initMatrixValue(void **buffer, const string& value) {
  switch (eltType) {
    case WfCst::TYPE_CHAR:
      *buffer = new char[nb_r*nb_c];
      break;
    case WfCst::TYPE_SHORT:
      *buffer = new int[nb_r*nb_c];
      break;
    case WfCst::TYPE_INT:
      *buffer = new int[nb_r*nb_c];
      break;
    case WfCst::TYPE_LONGINT:
      *buffer = new long[nb_r*nb_c];
      break;
    case WfCst::TYPE_FLOAT:
      *buffer = new float[nb_r*nb_c];
      break;
    case WfCst::TYPE_DOUBLE:
      *buffer = new double[nb_r*nb_c];
      break;
    default:
      break;
  } // end (switch)
  // VALUE IS PROVIDED IN A FILE
  if (value.substr(0, (string("file->")).size()) == "file->") {
    string dataFileName = value.substr((string("file->")).size());
    unsigned len = nb_r*nb_c;
    TRACE_TEXT (TRACE_ALL_STEPS,
                "reading the matrix data file" << endl);
    switch (eltType) {
      case WfCst::TYPE_CHAR:
        WfCst::readChar(dataFileName.c_str(), (char*)(*buffer), len);
        break;
      case WfCst::TYPE_SHORT:
        WfCst::readShort(dataFileName.c_str(), (short*)(*buffer), len);
        break;
      case WfCst::TYPE_INT:
        WfCst::readInt(dataFileName.c_str(), (int*)(*buffer), len);
        break;
      case WfCst::TYPE_LONGINT:
        WfCst::readLong(dataFileName.c_str(), (long*)(*buffer), len);
        break;
      case WfCst::TYPE_FLOAT:
        WfCst::readFloat(dataFileName.c_str(), (float*)(*buffer), len);
        break;
      case WfCst::TYPE_DOUBLE:
        WfCst::readDouble(dataFileName.c_str(), (double*)(*buffer), len);
        break;
      default:
        break;
    } // end switch
  }
  // VALUE IS PROVIDED DIRECTLY IN THE XML WORKFLOW
  else {
    vector<string> v = getStringToken(value);
    unsigned int len = v.size();
	// fill the matrix with the given data
    TRACE_TEXT (TRACE_ALL_STEPS,
                "filling the matrix with the data (" << len << ")" << endl);
    char  * ptr1(NULL);
    short * ptr2(NULL);
    int   * ptr3(NULL);
    long  * ptr4(NULL);
    float * ptr5(NULL);
    double * ptr6(NULL);
    switch (eltType) {
      case WfCst::TYPE_CHAR:
        ptr1 = (char*)(*buffer);
        for (unsigned int ix = 0; ix<len; ix++)
          ptr1[ix] = v[ix][0];
        break;
      case WfCst::TYPE_SHORT:
        ptr2 = (short*)(*buffer);
        for (unsigned int ix = 0; ix<len; ix++)
          ptr2[ix] = atoi(v[ix].c_str());
        break;
      case WfCst::TYPE_INT:
        ptr3 = (int*)(*buffer);
        for (unsigned int ix = 0; ix<len; ix++)
          ptr3[ix] = atoi(v[ix].c_str());
        break;
      case WfCst::TYPE_LONGINT:
        ptr4 = (long*)(*buffer);
        for (unsigned int ix = 0; ix<len; ix++)
          ptr4[ix] = atoi(v[ix].c_str());
        break;
      case WfCst::TYPE_FLOAT:
        ptr5 = (float*)(*buffer);
        for (unsigned int ix = 0; ix<len; ix++)
          ptr5[ix] = atof(v[ix].c_str());
        break;
      case WfCst::TYPE_DOUBLE:
        ptr6 = (double*)(*buffer);
        for (unsigned int ix = 0; ix<len; ix++)
          ptr6[ix] = atof(v[ix].c_str());
        break;
      default:
        break;
    } // end switch
  } // end else
}

/**
 * Initialize a port value for a container
 * IMPORTANT:  Currently limited to 1 level and to TYPE_LONGINT values
 */
void
DagNodePort::initContainerValue(const string& value) {
#if HAVE_DAGDA
  TRACE_TEXT (TRACE_ALL_STEPS,"Using value (" << value
      << ") to initialize container" << endl);
  // init container
  char* contID;
  diet_profile_t* profile = myParent->getProfile();
  dagda_create_container(&contID);
  diet_use_data(diet_parameter(profile, index),contID);
  // parse values
  char* valID;
  int ix=0;
  string contVal = value;
  string::size_type valSepLeft  = -1;
  while ((ix == 0) || (valSepLeft!=string::npos)) {
    string::size_type valSepRight = contVal.find(";",valSepLeft+1);
    long val = atoi(contVal.substr(
                    valSepLeft+1,
                    (valSepRight == string::npos ? contVal.length() : valSepRight)-valSepLeft-1
                                  ).c_str());
    valSepLeft = valSepRight;
        // store value and add it to container
    dagda_put_scalar(&val, DIET_LONGINT, DIET_PERSISTENT, &valID);
    dagda_add_container_element(contID,valID,ix++);
    CORBA::string_free(valID);
  }
#else
  WARNING("Cannot use containers without Dagda" << endl);
#endif
}

diet_profile_t *
DagNodePort::profile() {
  return myParent->getProfile();
}

const string&
DagNodePort::getDataID() {
  return this->dataID;
}

/**
 * DagNodeOutPort class
 */

DagNodeOutPort::DagNodeOutPort(DagNode * parent,
                               string _id,
                               WfCst::WfDataType _type,
                               uint _depth,
                               uint _ind)
  : DagNodePort(parent, _id, WfPort::PORT_OUT, _type, _depth, _ind) {
}

void
DagNodeOutPort::storeProfileData() {
  dataID = profile()->parameters[getIndex()].desc.id;
}

void
DagNodeOutPort::freeProfileData() {
#if ! HAVE_DAGDA
  diet_free_persistent_data(profile()->parameters[getIndex()].desc.id);
#endif
}

diet_persistence_mode_t
DagNodeOutPort::getPersistenceMode() {
  if (!isConnected()) return DIET_PERSISTENT_RETURN;
  else return DIET_PERSISTENT;
}

/**
 * DagNodeInPort class
 */

DagNodeInPort::DagNodeInPort(DagNode * parent,
                             string _id,
                             WfCst::WfDataType _type,
                             uint _depth,
                             uint _ind)
  : DagNodePort(parent, _id, WfPort::PORT_IN, _type, _depth, _ind) {
}

bool
DagNodeInPort::initProfileExec() {
  if (DagNodePort::initProfileExec()) {
    if (initSourceData()) {
      return true;
    } else {
      ERROR("WfInPort: data init failed" << endl, 0);
    }
  } else {
    ERROR("WfInPort: profile init failed" << endl, 0);
  }
  return true;
}

bool
DagNodeInPort::initSourceData() {
  if (isConnected()) {
    TRACE_TEXT (TRACE_ALL_STEPS,
 		"## using persistent data for " << index << endl);
    dataID = adapter->getSourceDataID();
    if (!dataID.empty()) {
      TRACE_TEXT (TRACE_ALL_STEPS,
                  "##  ==> data ID is " << dataID << endl);
      diet_use_data(diet_parameter(this->profile(), index),
                    const_cast<char*>(dataID.c_str()));
    } else {
      TRACE_TEXT (TRACE_ALL_STEPS,
                  "##  ERROR ==> data ID not found" << endl);
      return false;
    }
  }
  return true;
}

diet_persistence_mode_t
DagNodeInPort::getPersistenceMode() {
#if HAVE_DAGDA
  return DIET_PERSISTENT;
#else
  if (!isConnected()) return DIET_VOLATILE;
  else return DIET_PERSISTENT;
#endif
}

/**
 * DagNodeInOutPort class (inherits from DagNodeInPort and DagNodeOutPort )
 */

DagNodeInOutPort::DagNodeInOutPort(DagNode * parent,
                                   string _id,
                                   WfCst::WfDataType _type,
                                   uint _depth,
                                   uint _ind)
  : DagNodePort(parent, _id, WfPort::PORT_INOUT, _type, _depth, _ind),
    DagNodeInPort(parent,_id,_type,_depth,_ind),
    DagNodeOutPort(parent,_id,_type,_depth,_ind) {
}

diet_persistence_mode_t
DagNodeInOutPort::getPersistenceMode() {
  if (!isConnected()) return DIET_PERSISTENT_RETURN;
  else return DIET_PERSISTENT;
}
