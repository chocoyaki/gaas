/**
* @file DagNodePort.cc
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

#include "debug.hh"
#include "DagNodePort.hh"
#include "Dag.hh"
#include "WfUtils.hh"
extern "C" {
#include "DIET_Dagda.h"
}
#include "DagdaFactory.hh"
#include "DagdaImpl.hh"
#include "EventTypes.hh"
#include "ORBMgr.hh"

using namespace std;
using namespace events;

/*****************************************************************************/
/*                        DagNodePort (ABSTRACT)                             */
/*****************************************************************************/

DagNodePort::DagNodePort(DagNode * parent,
                         string _id,
                         WfPort::WfPortType _portType,
                         WfCst::WfDataType _type,
                         unsigned int _depth,
                         unsigned int _ind)
  : WfPort(parent, _id, _portType, _type, _depth, _ind),
    myParent(parent) {
    }

string DagNodePort::containerSeparator = ";";

void
DagNodePort::setProfileWithoutValue() {
  diet_profile_t* profile = myParent->getProfile();
  diet_persistence_mode_t mode = this->getPersistenceMode();
  switch (getDataType()) {
  case WfCst::TYPE_PARAMSTRING :
    diet_paramstring_set(diet_parameter(profile, index),
                         myParent->newString(), mode);
    break;
  case WfCst::TYPE_STRING :
    diet_string_set(diet_parameter(profile, index),
                    NULL, mode);
    break;
  case WfCst::TYPE_FILE :
    diet_file_set(diet_parameter(profile, index), NULL,
                  mode);
    break;
  case WfCst::TYPE_MATRIX :
    diet_matrix_set(diet_parameter(profile, index), NULL, mode,
                    (diet_base_type_t) WfCst::cvtWfToDietType(eltType),
                    nb_r, nb_c,
                    (diet_matrix_order_t) WfCst::cvtWfToDietMatrixOrder(order));
    break;
  case WfCst::TYPE_CONTAINER :
    diet_container_set(diet_parameter(profile, index), DIET_PERSISTENT);
    break;
  default:  // all scalar types
    diet_scalar_set(diet_parameter(profile, index),
                    NULL, mode, (diet_base_type_t) WfCst::cvtWfToDietType(type));
  }
}

/**
 * Profile for submit (only used on MaDag side except with basic scheduler)
 */
void
DagNodePort::initProfileSubmit() {
  setProfileWithoutValue();
}

diet_profile_t *
DagNodePort::profile() {
  return myParent->getProfile();
}

const string&
DagNodePort::getDataID() throw(WfDataException) {
  if (myDataID.empty()) {
    string baseErrorMsg = "PORT=" + getParent()->getId() + "#" + getId();
    throw(WfDataException(WfDataException::eID_UNDEF, baseErrorMsg));
  }
  return myDataID;
}

void
DagNodePort::setDataID(const string& dataID) {
  myDataID = dataID;
  sendEventFrom<DagNodePort, DagNodePort::DATAID>(this, "Set data ID", dataID, EventBase::INFO);
}

bool
DagNodePort::isDataIDAvailable(MasterAgent_var& MA) {
  Dagda_var dataManager = ORBMgr::getMgr()->resolve<Dagda, Dagda_var>(DAGDACTXT, MA->getDataManager());
  return dataManager->pfmIsDataPresent(myDataID.c_str());
}

void
DagNodePort::toXML(ostream& output) const {
  output << "name=\"" << id << "\" type=\"" << WfCst::cvtWfToStrType(getBaseDataType()) << "\" ";
  if (depth > 0) {
    output << " depth=\"" << depth << "\" ";
  }
  if (!myDataID.empty()) {
    output << " dataId=\"" << myDataID <<"\" ";
  }
  if (type == WfCst::TYPE_MATRIX) {
    output << "base_type=\"" << WfCst::cvtWfToStrType(eltType) << "\" "
           << "nb_rows=\"" << nb_r << "\" "
           << "nb_cols=\"" << nb_c << "\" "
           << "matrix_order=\"" << WfCst::cvtWfToStrMatrixOrder(order) << "\" ";
  }
}

string
DagNodePort::toString() const {
  return "PORT " + id + "[NODE=" + myParent->getId() + ";DAG=" + myParent->getDag()->getId() + "]";
}

/*****************************************************************************/
/*                           DagNodeOutPort                                  */
/*****************************************************************************/

DagNodeOutPort::DagNodeOutPort(DagNode * parent,
                               string _id,
                               WfCst::WfDataType _type,
                               unsigned int _depth,
                               unsigned int _ind)
  : DagNodePort(parent, _id, WfPort::PORT_OUT, _type, _depth, _ind) {
}

DagNodeOutPort::~DagNodeOutPort() {
}

void
DagNodeOutPort::initProfileExec() throw(WfDataException) {
  setProfileWithoutValue();
}

void
DagNodeOutPort::storeProfileData() {
  if (profile()) {
    setDataID(profile()->parameters[getIndex()].desc.id);
  }
}

void
DagNodeOutPort::freePersistentData(MasterAgent_var& MA) {
  if (profile()) {
    char* dataId = profile()->parameters[getIndex()].desc.id;
    if (dataId) {
      if (MA->diet_free_pdata(dataId)==0) {
        sendEventFrom<DagNodeOutPort, 0>(this, "Delete persistent data failed", dataId , EventBase::WARNING);
      }
    }
  }
}

string
DagNodeOutPort::toString() const {
  return "OUT_" + DagNodePort::toString();
}

void
DagNodeOutPort::toXML(ostream& output) const {
  output << "\t<out ";
  DagNodePort::toXML(output);
  output << "/>" << endl;
}

// unsigned int
// DagNodeOutPort::getDataIDCardinal(const string& dataID) throw(WfDataException) {
//   WfDataIDAdapter    adapterID(dataID);
//   vector<string>     vectID;
//   adapterID.getElements(vectID);
//   cerr << __FUNCTION__ << " calling getElements" << endl;
//   return vectID.size();
// }

string
DagNodeOutPort::getElementDataID(const list<unsigned int>& eltIdx)
  throw(WfDataException)
{
  string eltID = "";
  string baseErrorMsg = "PORT=" + getParent()->getId() + "#" + getId();
  // initialize with port's data ID (ie container root)
  eltID = getDataID();
  if (eltID.empty())
    throw(WfDataException(WfDataException::eID_UNDEF, baseErrorMsg));
  // loop until the last container level
  for (list<unsigned int>::const_iterator idxIter = eltIdx.begin();
       idxIter != eltIdx.end();
       ++idxIter) {
    baseErrorMsg += "[" + itoa(*idxIter) + "]";
    if (eltID.empty())
      throw(WfDataException(WfDataException::eID_UNDEF, baseErrorMsg));
    WfDataIDAdapter     adapterID(eltID);
    vector<string>      vectID;
    adapterID.getElements(vectID);
    if (vectID.size() >= *idxIter) {
      if (!vectID[*idxIter].empty())
        eltID = vectID[*idxIter];
      else
        throw(WfDataException(WfDataException::eVOID_DATA, baseErrorMsg));
    } else {
      string errorMsg = "Container size smaller than element index (container ID="
        + eltID + "/" + baseErrorMsg + ")";
      throw(WfDataException(WfDataException::eINVALID_CONTAINER, errorMsg));
    }
  }
  return eltID;
}

/**
 * Display for standard data
 * (uses DIET API except for containers)
 */
void
DagNodeOutPort::writeData(WfDataWriter* dataWriter) throw(WfDataException) {
  short dataType = getDataType();
  if (!profile()) {
    throw WfDataException(WfDataException::eNOTFOUND, "No profile available for node "
                          + getCompleteId());
  }
  if (!myParent->isDone()) {
    throw WfDataException(WfDataException::eNOTAVAIL, "Node execution not complete "
                          + getCompleteId());
  }
  diet_arg_t* dietParam = diet_parameter(profile(), getIndex());

  if (dataType == WfCst::TYPE_CONTAINER) {
    const char *contID = (*dietParam).desc.id;
    writeContainer(dataWriter, contID, getDepth());
  }
  // USE DIET API FOR ALL OTHER TYPES
  else if (dataType == WfCst::TYPE_FILE) {
    size_t size;
    char * path;
    diet_file_get(dietParam, &path, NULL, &size);
    dataWriter->itemValue(path,(WfCst::WfDataType) dataType);
  }
  else if (dataType == WfCst::TYPE_DOUBLE) {
    double * value = NULL;
    diet_scalar_get(dietParam, &value, NULL);
    dataWriter->itemValue(value, (WfCst::WfDataType) dataType);
  }
  else if (dataType == WfCst::TYPE_FLOAT) {
    float * value = NULL;
    diet_scalar_get(dietParam, &value, NULL);
    dataWriter->itemValue(value, (WfCst::WfDataType) dataType);
  }
  else if ((dataType == WfCst::TYPE_CHAR) ||
            (dataType == WfCst::TYPE_SHORT) ||
            (dataType == WfCst::TYPE_INT) ||
            (dataType == WfCst::TYPE_LONGINT)) {
    long * value = NULL;
    diet_scalar_get(dietParam,&value, NULL);
    dataWriter->itemValue(value, (WfCst::WfDataType) dataType);
  }
  else if (dataType == WfCst::TYPE_STRING) {
    char * value;
    diet_string_get(dietParam, &value, NULL);
    dataWriter->itemValue(value, (WfCst::WfDataType) dataType);
  }
  else if (dataType == WfCst::TYPE_PARAMSTRING) {
    char * value;
    diet_paramstring_get(dietParam, &value, NULL);
    dataWriter->itemValue(value, (WfCst::WfDataType) dataType);
  }
  else if (dataType == WfCst::TYPE_MATRIX) {
    size_t nb_rows, nb_cols;
    diet_matrix_order_t order;
    short baseType = getEltDataType();

    if (baseType == WfCst::TYPE_DOUBLE) {
      double * value;
      diet_matrix_get(dietParam, &value, NULL,
                      &nb_rows, &nb_cols, &order);
      dataWriter->matrix(value, (WfCst::WfDataType) dataType,
                         nb_rows, nb_cols, order);
    }
    else if (baseType == WfCst::TYPE_FLOAT) {
      float * value;
      diet_matrix_get(dietParam, &value, NULL,
                      &nb_rows, &nb_cols, &order);
      dataWriter->matrix(value, (WfCst::WfDataType) dataType,
                         nb_rows, nb_cols, order);
    }
    else if (baseType == WfCst::TYPE_CHAR) {
      char * value;
      diet_matrix_get(dietParam, &value, NULL,
                      &nb_rows, &nb_cols, &order);
      dataWriter->matrix(value, (WfCst::WfDataType) dataType,
                         nb_rows, nb_cols, order);
    }
    else if (baseType == WfCst::TYPE_SHORT) {
      short * value;
      diet_matrix_get(dietParam, &value, NULL,
                      &nb_rows, &nb_cols, &order);
      dataWriter->matrix(value, (WfCst::WfDataType) dataType,
                         nb_rows, nb_cols, order);
    }
    else if (baseType == WfCst::TYPE_INT) {
      int * value;
      diet_matrix_get(dietParam, &value, NULL,
                      &nb_rows, &nb_cols, &order);
      dataWriter->matrix(value, (WfCst::WfDataType) dataType,
                         nb_rows, nb_cols, order);
    }
    else if (baseType == WfCst::TYPE_LONGINT) {
      long * value;
      diet_matrix_get(dietParam, &value, NULL,
                      &nb_rows, &nb_cols, &order);
      dataWriter->matrix(value, (WfCst::WfDataType) dataType,
                         nb_rows, nb_cols, order);
    }
  } // end if TYPE_MATRIX
}

/** Display for containers
 * (recursive)
 */
void
DagNodeOutPort::writeContainer(WfDataWriter* dataWriter,
                               const string& containerID,
                               unsigned int depth) throw(WfDataException) {
  dataWriter->startContainer();
  WfDataIDAdapter       adapterID(containerID);
  vector<string>        vectID;
  adapterID.getElements(vectID);
  for (vector<string>::iterator eltIter = vectID.begin();
       eltIter != vectID.end();
       ++eltIter) {
    string eltID = *eltIter;
    if (eltID.empty()) {
      dataWriter->voidElement();
    } else if (depth > 1) {
      writeContainer(dataWriter, eltID, depth-1);
    } else if (depth == 1) {
      writeContainerData(dataWriter, eltID);
    } else { // wrong depth
      dataWriter->error();
    }
  } // end for
  dataWriter->endContainer();
}

/**
 * Specific display for container data
 * because we use DAGDA API instead of DIET API in this case
 * TODO simplify
 */
void
DagNodeOutPort::writeContainerData(WfDataWriter* dataWriter,
                                   const string& eltID) {
  short baseType = getBaseDataType();
  try {
    if (baseType == WfCst::TYPE_DOUBLE) {
      double * value;
      dagda_get_scalar(eltID.c_str(),&value, NULL);
      dataWriter->itemValue(value, (WfCst::WfDataType) baseType);
    } else if (baseType == WfCst::TYPE_INT) {
      int *value;
      dagda_get_scalar(eltID.c_str(),&value, NULL);
      dataWriter->itemValue(value, (WfCst::WfDataType) baseType);
    } else if (baseType == WfCst::TYPE_LONGINT) {
      long *value;
      dagda_get_scalar(eltID.c_str(),&value, NULL);
      dataWriter->itemValue(value, (WfCst::WfDataType) baseType);
    } else if (baseType == WfCst::TYPE_FLOAT) {
      float *value;
      dagda_get_scalar(eltID.c_str(),&value, NULL);
      dataWriter->itemValue(value, (WfCst::WfDataType) baseType);
    } else if (baseType == WfCst::TYPE_CHAR) {
      char *value;
      dagda_get_scalar(eltID.c_str(),&value, NULL);
      dataWriter->itemValue(value, (WfCst::WfDataType) baseType);
    } else if (baseType == WfCst::TYPE_SHORT) {
      short *value;
      dagda_get_scalar(eltID.c_str(),&value, NULL);
      dataWriter->itemValue(value, (WfCst::WfDataType) baseType);
    } else if (baseType == WfCst::TYPE_PARAMSTRING) {
      char *value;
      dagda_get_paramstring(eltID.c_str(),&value);
      dataWriter->itemValue(value, (WfCst::WfDataType) baseType);
    } else if (baseType == WfCst::TYPE_STRING) {
      char *value;
      dagda_get_string(eltID.c_str(),&value);
      dataWriter->itemValue(value, (WfCst::WfDataType) baseType);
    } else if (baseType == WfCst::TYPE_FILE) {
      char *path;
      dagda_get_file(eltID.c_str(),&path);
      dataWriter->itemValue(path, (WfCst::WfDataType) baseType);
    }
  } catch (Dagda::DataNotFound& e) {
    string errorMsg = "Data ID = " + eltID;
    throw WfDataException(WfDataException::eNOTFOUND, errorMsg);
  }
}

/**
 * Display for a container sub-element (that may be a container itself)
 */
void
DagNodeOutPort::writeDataElement(WfDataWriter* dataWriter,
                                 const list<unsigned int>& idxList)
  throw(WfDataException) {
  string eltID = getElementDataID(idxList);
  if (!eltID.empty()) {
    if (idxList.size() == getDepth())
      writeContainerData(dataWriter, eltID);
    else
      writeContainer(dataWriter, eltID, getDepth() - idxList.size());
  } else {
    dataWriter->voidElement();
  }
}

diet_persistence_mode_t
DagNodeOutPort::getPersistenceMode() {
  if (!isConnected()) return DIET_PERSISTENT_RETURN;  // WARNING not applied to containers!!
  else return DIET_PERSISTENT;
}

/*****************************************************************************/
/*                           DagNodeInPort                                   */
/*****************************************************************************/

DagNodeInPort::DagNodeInPort(DagNode * parent,
                             string _id,
                             WfCst::WfDataType _type,
                             unsigned int _depth,
                             unsigned int _ind)
  : DagNodePort(parent, _id, WfPort::PORT_IN, _type, _depth, _ind) {
}

void
DagNodeInPort::setConnectionRef(const string& strRef) {
  WfPort::setConnectionRef(strRef);
  sendEventFrom<DagNodeInPort, DagNodeInPort::DEPEND>(this, "Set input reference", strRef, EventBase::INFO);
}

void
DagNodeInPort::setPortAdapter(WfPortAdapter* adapter) {
  WfPort::setPortAdapter(adapter);
  string strRef = adapter->getSourceRef();
  sendEventFrom<DagNodeInPort, DagNodeInPort::DEPEND>(this, "Set input reference", strRef, EventBase::INFO);
}

void
DagNodeInPort::initProfileExec() throw(WfDataException) {
  initSourceData();
}

void
DagNodeInPort::initSourceData() throw(WfDataException) {

  setDataID(adapter->getSourceDataID());

  // Check types are matching
  // FIXME this is using static type which has already been checked so unuseful ?
  // if (adapter->getSourceDataType() != getDataType()) {
  //   string errorMsg = "port " + getId() + " of node " + getParent()->getId()
  //                     + " got data with wrong type ("
  //                     + WfCst::cvtWfToStrType(adapter->getSourceDataType()) + ")";
  //   throw(WfDataException(WfDataException::eWRONGTYPE, errorMsg));
  // }
  // Store ID in profile
  if (profile()) {
    if (!myDataID.empty()) {
      diet_use_data(diet_parameter(this->profile(), index), myParent->newString(myDataID));
    } else {
      throw(WfDataException(WfDataException::eNOTFOUND, ""));
    }
  }
}

string
DagNodeInPort::toString() const {
  return "IN_" + DagNodePort::toString();
}

void
DagNodeInPort::toXML(ostream& output) const {
  output << "\t<in ";
  DagNodePort::toXML(output);
  output << "source=\"" << adapter->getSourceRef()  << "\" />" << endl;
}

diet_persistence_mode_t
DagNodeInPort::getPersistenceMode() {
  return DIET_PERSISTENT;
}

void
DagNodeInPort::freePersistentData(MasterAgent_var& MA) {
  adapter->freeAdapterPersistentData(MA);
}

/*****************************************************************************/
/*                         DagNodeInOutPort                                  */
/*****************************************************************************/

DagNodeInOutPort::DagNodeInOutPort(DagNode * parent,
                                   string _id,
                                   WfCst::WfDataType _type,
                                   unsigned int _depth,
                                   unsigned int _ind)
  : DagNodePort(parent, _id, WfPort::PORT_INOUT, _type, _depth, _ind),
    DagNodeInPort(parent, _id, _type, _depth, _ind),
    DagNodeOutPort(parent, _id, _type, _depth, _ind) {
}


void
DagNodeInOutPort::initProfileExec() throw(WfDataException) {
  DagNodeInPort::initProfileExec();
}

diet_persistence_mode_t
DagNodeInOutPort::getPersistenceMode() {
  return DagNodeOutPort::getPersistenceMode();
}

string
DagNodeInOutPort::toString() const {
  return "INOUT_" + DagNodePort::toString();
}

void
DagNodeInOutPort::toXML(ostream& output) const {
  output << "\t<inOut ";
  DagNodePort::toXML(output);
  output << "source=\"" << adapter->getSourceRef()  << "\" />" << endl;
}

void
DagNodeInOutPort::freePersistentData(MasterAgent_var& MA) {
  DagNodeOutPort::freePersistentData(MA);
}

/*****************************************************************************/
/*                           DagNodeArgPort                                  */
/*****************************************************************************/

DagNodeArgPort::DagNodeArgPort(DagNode * parent,
                               string _id,
                               WfCst::WfDataType _type,
                               unsigned int _depth,
                               unsigned int _ind)
  : DagNodePort(parent, _id, WfPort::PORT_ARG, _type, _depth, _ind) {
}

void
DagNodeArgPort::setValue(const string& val) {
  this->value = val;
}

/**
 * set the port profile for submission
 */
void
DagNodeArgPort::initProfileSubmit() {
  // set the value for scalar & paramstring types
  if ((type == WfCst::TYPE_CHAR)
      || (type == WfCst::TYPE_SHORT)
      || (type == WfCst::TYPE_INT)
      || (type == WfCst::TYPE_LONGINT)
      || (type == WfCst::TYPE_FLOAT)
      || (type == WfCst::TYPE_DOUBLE)
      || (type == WfCst::TYPE_PARAMSTRING))
    setProfileWithValue();
  else
    setProfileWithoutValue();
}

void
DagNodeArgPort::initProfileExec() throw(WfDataException) {
  setProfileWithValue();
}

string
DagNodeArgPort::toString() const {
  return "ARG_" + DagNodePort::toString();
}

void
DagNodeArgPort::toXML(ostream& output) const {
  output << "\t<arg ";
  DagNodePort::toXML(output);
  output << "value=\"" << value << "\" />" << endl;
}

diet_persistence_mode_t
DagNodeArgPort::getPersistenceMode() {
  return DIET_PERSISTENT;
}

// protected
void
DagNodeArgPort::setProfileWithValue() {
  diet_profile_t* profile = myParent->getProfile();
  diet_persistence_mode_t mode = this->getPersistenceMode();
  void * mat = NULL;
  switch (type) {
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
    diet_file_set(diet_parameter(profile, index), myParent->newFile(value),
                  mode);
    break;
  case WfCst::TYPE_MATRIX :
    initMatrixValue(&mat);
    diet_matrix_set(diet_parameter(profile, index), mat, mode,
                    (diet_base_type_t) WfCst::cvtWfToDietType(eltType),
                    nb_r, nb_c,
                    (diet_matrix_order_t) WfCst::cvtWfToDietMatrixOrder(order));
    break;
  case WfCst::TYPE_CONTAINER :
    initContainerValue();
    diet_container_set(diet_parameter(profile, index), DIET_PERSISTENT);
    break;
  case WfCst::TYPE_UNKNOWN:
    throw WfDataException(WfDataException::eWRONGTYPE, string(__FUNCTION__)
                          + string(": type not defined"));
  }
}

void
DagNodeArgPort::initMatrixValue(void **buffer) {
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
    TRACE_TEXT(TRACE_ALL_STEPS,
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
    TRACE_TEXT(TRACE_ALL_STEPS,
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

void
DagNodeArgPort::initContainerValue() {
  TRACE_TEXT(TRACE_ALL_STEPS, " # Initializing container for port " << getId() << endl);
  // init container and link it to profile
  char* contID;
  // fill-in the container with the values (recursive)
  string contVal = value;
  initContainerValueRec(&contID, contVal, depth);
  // attach to profile
  diet_use_data(diet_parameter(myParent->getProfile(), index), contID);
  TRACE_TEXT(TRACE_ALL_STEPS, " # END OF container initialization for port " << getId() << endl);
}

/**
 * Initialize a port value for a container (recursive)
 */
void
DagNodeArgPort::initContainerValueRec(char** contIDPtr,
                                      string& contStr,
                                      unsigned int contDepth) {
  if (contDepth < 1)  {
    string errorMsg = "Too many parenthesis in '" + contStr + "'";
    throw WfDataException(WfDataException::eINVALID_VALUE, errorMsg);
  }
  // init container
  dagda_create_container(contIDPtr);
  // parse values
  char* valID;
  int   valIdx = 0;
  bool  valEnd = false;
  bool  strEnd = false;
  // remove first and last characters (open and close parenthesis)
  contStr.erase(0, 1);
  contStr.erase(contStr.length()-1, 1);

  while (!(valEnd || contStr.empty())) {
    string::size_type parLeft = contStr.find("(");
    string::size_type valSepRight = contStr.find(containerSeparator);
    string::size_type parRight = contStr.find(")");
    // define the right end of the value (as valSepRight-1)
    if ((parRight != string::npos)
        && ((valSepRight == string::npos) || (parRight < valSepRight))) {
      valEnd = true;
      valSepRight = parRight;
    }
    if (valSepRight == string::npos) {  // end of the string
      valEnd = true;
      strEnd = true;
      valSepRight = contStr.length();
    }
    if (valSepRight == 0) {
      contStr.erase(0, 1);
      continue;
    }
    if ((parLeft == string::npos) || (parLeft > valSepRight)) {
      // if no left parenthesis before the separator then parse the value
      const char *valStr = contStr.substr(0, valSepRight).c_str();  // leaf value
      // store value in Dagda
      initContainerElementValue(valStr, &valID);
      // update the parsed string
      if (!strEnd)
        contStr = contStr.substr(valSepRight+1, contStr.length() - valSepRight - 1);
      else
        contStr.clear();
    } else {
      initContainerValueRec(&valID, contStr, depth-1);
    }
    dagda_add_container_element(*contIDPtr, valID, valIdx++);
    CORBA::string_free(valID);
  } // end while
}

void
DagNodeArgPort::initContainerElementValue(const char* valStr, char** valIDPtr) {
  switch (eltType) {
  case WfCst::TYPE_CHAR:
    dagda_put_scalar(myParent->newChar(valStr), DIET_CHAR, DIET_PERSISTENT, valIDPtr);
    break;
  case WfCst::TYPE_SHORT:
    dagda_put_scalar(myParent->newShort(valStr), DIET_SHORT, DIET_PERSISTENT, valIDPtr);
    break;
  case WfCst::TYPE_INT:
    dagda_put_scalar(myParent->newInt(valStr), DIET_INT, DIET_PERSISTENT, valIDPtr);
    break;
  case WfCst::TYPE_LONGINT:
    dagda_put_scalar(myParent->newLong(valStr), DIET_LONGINT, DIET_PERSISTENT, valIDPtr);
    break;
  case WfCst::TYPE_FLOAT:
    dagda_put_scalar(myParent->newFloat(valStr), DIET_FLOAT, DIET_PERSISTENT, valIDPtr);
    break;
  case WfCst::TYPE_DOUBLE:
    dagda_put_scalar(myParent->newDouble(valStr), DIET_DOUBLE, DIET_PERSISTENT, valIDPtr);
    break;
  case WfCst::TYPE_PARAMSTRING:
    dagda_put_paramstring(myParent->newString(valStr), DIET_PERSISTENT, valIDPtr);
    break;
  case WfCst::TYPE_STRING:
    dagda_put_string(myParent->newString(valStr), DIET_PERSISTENT, valIDPtr);
    break;
  case WfCst::TYPE_FILE:
    dagda_put_file(myParent->newFile(valStr), DIET_PERSISTENT, valIDPtr);
    break;
  case WfCst::TYPE_MATRIX:
  default:
    INTERNAL_ERROR("Type not managed in container initialization", 0);
  } // end (switch)
}

void
DagNodeArgPort::freePersistentData(MasterAgent_var& MA) {
  // TODO
}

