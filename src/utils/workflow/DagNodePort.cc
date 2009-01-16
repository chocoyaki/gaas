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
 * Revision 1.9  2009/01/16 13:55:36  bisnard
 * changes in dag event handling methods
 *
 * Revision 1.8  2008/12/09 12:15:59  bisnard
 * pending instanciation handling (uses dag outputs for instanciation
 * of a functional wf)
 *
 * Revision 1.7  2008/12/02 10:14:51  bisnard
 * modified nodes links mgmt to handle inter-dags links
 *
 * Revision 1.6  2008/10/30 14:33:01  bisnard
 * added recursive container initialization
 *
 * Revision 1.5  2008/10/29 10:10:40  bisnard
 * avoid warning for incomplete switch
 *
 * Revision 1.4  2008/10/29 08:35:57  bisnard
 * completed container initialization for all data types
 *
 * Revision 1.3  2008/10/22 09:29:00  bisnard
 * replaced uint by standard type
 *
 * Revision 1.2  2008/10/20 07:59:29  bisnard
 * file header creation
 *
 */

#include "debug.hh"
#include "DagNodePort.hh"
#include "WfUtils.hh"
#if HAVE_DAGDA
extern "C" {
  #include "DIET_Dagda.h"
}
#include "DagdaFactory.hh"
#include "DagdaImpl.hh"
#endif

using namespace std;


DagNodePort::DagNodePort(DagNode * parent,
                         string _id,
                         WfPort::WfPortType _portType,
                         WfCst::WfDataType _type,
                         unsigned int _depth,
                         unsigned int _ind)
  : WfPort(parent, _id, _portType, _type, _depth, _ind),
    myParent(parent) {
}

void
DagNodePort::setProfileWithoutValue() {
  diet_profile_t* profile = myParent->getProfile();
  diet_persistence_mode_t mode = this->getPersistenceMode();
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
    default:  // all scalar types
      diet_scalar_set(diet_parameter(profile, index),
                      NULL, mode, (diet_base_type_t) WfCst::cvtWfToDietType(type));
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
 */
void
DagNodePort::initContainerValue(const string& value) {
#if HAVE_DAGDA
  // init container and link it to profile
  char* contID;
  // fill-in the container with the values (recursive)
  string contVal = value; // due to const
  initContainerValueRec(&contID, contVal, depth);
  // attach to profile
  diet_use_data(diet_parameter(myParent->getProfile(), index),contID);
#else
  WARNING("Cannot use containers without Dagda" << endl);
#endif
}

/**
 * Initialize a port value for a container (recursive)
 */
void
DagNodePort::initContainerValueRec(char** contIDPtr,
                                   string& contStr,
                                   unsigned int contDepth) {
  if (contDepth < 1)  {
    cout << "Too many parenthesis in container init (contStr="
         << contStr << ")" << endl;
    exit(0);
  }
#if HAVE_DAGDA
  // init container
//   cout << "creating container" << endl;
  dagda_create_container(contIDPtr);
  // parse values
  char* valID;
  int   valIdx = 0;
  bool  valEnd = false;
  bool  strEnd = false;
  while (!(valEnd || contStr.empty())) {
//     cout << "while loop - str = " << contStr << endl;
    string::size_type parLeft = contStr.find("(");
    string::size_type valSepRight = contStr.find(";");
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
//     cout << " end of current value: " << valSepRight << endl;
    if (valSepRight == 0) {
      contStr.erase(0,1);
      continue;
    }
    if ((parLeft == string::npos) || (parLeft > valSepRight)) {
//       cout << " parsing leaf value" << endl;
      // if no left parenthesis before the separator then parse the value
      const char *valStr = contStr.substr(0, valSepRight).c_str();  // leaf value
      // store value in Dagda
      switch (eltType) {
        case WfCst::TYPE_CHAR:
          dagda_put_scalar(myParent->newChar(valStr), DIET_CHAR, DIET_PERSISTENT, &valID);
          break;
        case WfCst::TYPE_SHORT:
          dagda_put_scalar(myParent->newShort(valStr), DIET_SHORT, DIET_PERSISTENT, &valID);
          break;
        case WfCst::TYPE_INT:
          dagda_put_scalar(myParent->newInt(valStr), DIET_INT, DIET_PERSISTENT, &valID);
          break;
        case WfCst::TYPE_LONGINT:
          dagda_put_scalar(myParent->newLong(valStr), DIET_LONGINT, DIET_PERSISTENT, &valID);
          break;
        case WfCst::TYPE_FLOAT:
          dagda_put_scalar(myParent->newFloat(valStr), DIET_FLOAT, DIET_PERSISTENT, &valID);
          break;
        case WfCst::TYPE_DOUBLE:
          dagda_put_scalar(myParent->newDouble(valStr), DIET_DOUBLE, DIET_PERSISTENT, &valID);
          break;
        case WfCst::TYPE_PARAMSTRING:
          dagda_put_paramstring(myParent->newString(valStr), DIET_PERSISTENT, &valID);
          break;
        case WfCst::TYPE_STRING:
          dagda_put_string(myParent->newString(valStr), DIET_PERSISTENT, &valID);
          break;
        case WfCst::TYPE_FILE:
          dagda_put_file(myParent->newFile(valStr), DIET_PERSISTENT, &valID);
          break;
        case WfCst::TYPE_MATRIX:
        default:
          INTERNAL_ERROR("Type not managed in container initialization",0);
      } // end (switch)
      // update the parsed string
      if (!strEnd)
        contStr = contStr.substr(valSepRight+1, contStr.length() - valSepRight - 1);
      else
        contStr.clear();
    } else {
      contStr.erase(parLeft,1);
      initContainerValueRec(&valID,contStr,depth-1);
    }
//     cout << "adding element to container" << endl;
    dagda_add_container_element(*contIDPtr,valID,valIdx++);
    CORBA::string_free(valID);
  } // end while
//   cout << "end of recursive call" << endl;
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

string
DagNodePort::toXML() {
  string xml = "";
  xml += "name=\"" + id + "\" type=\"" + WfCst::cvtWfToStrType(getBaseDataType()) +"\" ";
  if (depth > 0) {
    xml += " depth=\"" + itoa(depth) +"\" ";
  }
  if (type == WfCst::TYPE_MATRIX) {
    xml += "base_type=\""+ WfCst::cvtWfToStrType(eltType) +"\" ";
    xml += "nb_rows=\"" + itoa(nb_r)+"\" ";
    xml += "nb_cols=\"" + itoa(nb_c)+"\" ";
    xml += "matrix_order=\"" + WfCst::cvtWfToStrMatrixOrder(order) + "\" ";
  }
  return xml;
}

/**
 * DagNodeOutPort class
 */

DagNodeOutPort::DagNodeOutPort(DagNode * parent,
                               string _id,
                               WfCst::WfDataType _type,
                               unsigned int _depth,
                               unsigned int _ind)
  : DagNodePort(parent, _id, WfPort::PORT_OUT, _type, _depth, _ind) {
}

DagNodeOutPort::~DagNodeOutPort() {
  // free cache
  while (! myCache.empty() ) {
    TRACE_TEXT (TRACE_ALL_STEPS, "deleting out port cache entry" << endl);
    diet_container_t * p = myCache.begin()->second;
    myCache.erase( myCache.begin() );
    free(p->elt_ids); // was allocated using malloc
    delete p;
  }
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

string
DagNodeOutPort::toXML() {
  string xml = "\t<out ";
  xml += DagNodePort::toXML();
  xml += "/>\n";
  return xml;
}

diet_container_t*
DagNodeOutPort::getDataIDList(const string& dataID) {
  diet_container_t* content = NULL;
  map<string,diet_container_t*>::iterator cacheIter = myCache.find(dataID);
  if (cacheIter != myCache.end()) {
    TRACE_TEXT (TRACE_ALL_STEPS, "port " << id << ": using container cache for entry "
                                 << dataID << endl);
    content = (diet_container_t*) cacheIter->second;
  } else {
    content = new diet_container_t;
    content->size = 0;
    TRACE_TEXT (TRACE_ALL_STEPS, "port " << id << ": get ID list for container "
                                 <<  dataID << endl);
    dagda_get_container(dataID.c_str());
    dagda_get_container_elements(dataID.c_str(), content);
    myCache[dataID] = content;
  }
  return content;
}

diet_persistence_mode_t
DagNodeOutPort::getPersistenceMode() {
  if (!isConnected()) return DIET_PERSISTENT_RETURN; // WARNING not applied to containers!!
  else return DIET_PERSISTENT;
}

/**
 * DagNodeInPort class
 */

DagNodeInPort::DagNodeInPort(DagNode * parent,
                             string _id,
                             WfCst::WfDataType _type,
                             unsigned int _depth,
                             unsigned int _ind)
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
      diet_use_data(diet_parameter(this->profile(), index), myParent->newString(dataID));
//       char* dataIDStr = new char[dataID.length()+1];
//       strcpy(dataIDStr, dataID.c_str());
//       diet_use_data(diet_parameter(this->profile(), index), dataIDStr);
    } else {
      TRACE_TEXT (TRACE_ALL_STEPS,
                  "##  ERROR ==> data ID not found" << endl);
      return false;
    }
  }
  return true;
}

string
DagNodeInPort::toXML() {
  string xml = "";
  if (!value.empty())
    xml = "\t<arg ";
  else
    xml ="\t<in ";
  xml += DagNodePort::toXML();
  if (!value.empty())
    xml += "value=\"" + this->value + "\" ";
  if (adapter)
    xml += "source=\""+ adapter->getSourceRef() +"\"";
  xml += "/>\n";
  return xml;
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
                                   unsigned int _depth,
                                   unsigned int _ind)
  : DagNodePort(parent, _id, WfPort::PORT_INOUT, _type, _depth, _ind),
    DagNodeInPort(parent,_id,_type,_depth,_ind),
    DagNodeOutPort(parent,_id,_type,_depth,_ind) {
}

diet_persistence_mode_t
DagNodeInOutPort::getPersistenceMode() {
  if (!isConnected()) return DIET_PERSISTENT_RETURN;
  else return DIET_PERSISTENT;
}

string
DagNodeInOutPort::toXML() {
  string xml = "CANNOT BE GENERATED YET!!";
  return xml;
}
