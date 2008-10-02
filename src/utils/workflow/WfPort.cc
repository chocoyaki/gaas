/****************************************************************************/
/* The port classes used in workflow node                                   */
/* The main classes are WfPort, WfOutPort and WfInPort                      */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/* - Benjamin ISNARD (benjamin.isnard@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.6  2008/10/02 07:35:10  bisnard
 * new constants definitions (matrix order and port type)
 *
 * Revision 1.5  2008/09/30 15:32:53  bisnard
 * - using simple port id instead of composite ones
 * - dag nodes linking refactoring
 * - prevNodes and nextNodes data structures modified
 * - prevNodes initialization by Node::setNodePredecessors
 *
 * Revision 1.4  2008/09/30 09:23:29  bisnard
 * removed diet profile initialization from DagWfParser and replaced by node methods initProfileSubmit and initProfileExec
 *
 * Revision 1.3  2008/09/19 14:01:30  bisnard
 * allow compile wf support with or without DAGDA
 *
 * Revision 1.2  2008/09/19 13:11:07  bisnard
 * - added support for containers split/merge in workflows
 * - added support for multiple port references
 * - profile for node execution initialized by port (instead of node)
 * - ports linking managed by ports (instead of dag)
 *
 * Revision 1.1  2008/04/10 08:38:50  bisnard
 * New version of the MaDag where workflow node execution is triggered by the MaDag agent and done by a new CORBA object CltWfMgr located in the client
 *
 * Revision 1.2  2006/11/27 10:15:12  aamar
 * Correct headers of files used in workflow support.
 *
 ****************************************************************************/

#include "WfPort.hh"
#include "Node.hh"
#include "debug.hh"
#if HAVE_DAGDA
extern "C" {
#include "DIET_Dagda.h"
}
#endif

/**
 * WfPort class (ABSTRACT)
 */

WfPort::WfPort(Node * parent, string _id, WfCst::WfDataType _type, uint _depth, uint _ind,
	       const string& v) :
  myParent(parent), id(_id),type(_type), depth(_depth), index(_ind),value(v) {
  if (_depth > 0) {
    eltType = _type; // store the elements type
    type = WfCst::TYPE_CONTAINER;
  }
  this->nb_r = 0;
  this->nb_c = 0;
}

void
WfPort::setProfileWithoutValue() {
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
WfPort::setProfileWithValue() {
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
WfPort::initProfileSubmit() {
  // set the value for scalar & paramstring types
  if ((value != "") &&
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
} // end initProfileSubmit

/**
 * set the port profile for execution
 */
bool
WfPort::initProfileExec() {
  if (value != "")
    setProfileWithValue();
  else
    setProfileWithoutValue();
  return true;
} // end initProfileExec


void
WfPort::setMatParams(long nbr, long nbc,
		    WfCst::WfMatrixOrder o,
		    WfCst::WfDataType bt) {
  this->nb_r = nbr;
  this->nb_c = nbc;
  this->order = o;
  this->eltType = bt;
}

void
WfPort::initMatrixValue(void **buffer, const string& value) {
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
WfPort::initContainerValue(const string& value) {
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
WfPort::profile() {
  return myParent->getProfile();
}

string
WfPort::getId() {
  return this->id;
}

unsigned int
WfPort::getIndex() {
  return this->index;
}

unsigned int
WfPort::getDepth() {
  return this->depth;
}

const string&
WfPort::getDataID() {
  return this->dataID;
}

/**
 * WfOutPort class (inherits from WfPort)
 */

WfOutPort::WfOutPort(Node * parent, string _id, WfCst::WfDataType _type, uint _depth,
                     uint _ind, const string& v) :
  WfPort(parent, _id, _type, _depth, _ind, v) {
  sink_port = NULL;
}

void
WfOutPort::setSink(WfInPort * _sink_port) {
  this->sink_port = _sink_port;
}

void
WfOutPort::setSink(const string& s) {
  this->sink_port_id = s;
}

string
WfOutPort::getSinkId() {
  return this->sink_port_id;
}

WfInPort *
WfOutPort::getSink() {
  return this->sink_port;
}

bool
WfOutPort::isResult() {
  return (sink_port == NULL);
}

void
WfOutPort::storeProfileData() {
  dataID = profile()->parameters[getIndex()].desc.id;
}

diet_persistence_mode_t
WfOutPort::getPersistenceMode() {
  if (isResult()) return DIET_PERSISTENT_RETURN;
  else return DIET_PERSISTENT;
}

/**
 * WfInPort class (inherits from WfPort)
 */
WfInPort::WfInPort(Node * parent, string _id, WfCst::WfDataType _type, uint _depth,
                   uint _ind, const string& v) :
  WfPort(parent, _id, _type, _depth, _ind, v) {
  this->adapter = NULL;
}

/**
 * Set the source of the input port (PARSING ONLY - NO OBJECT CHECK)
 * The existence of the linked port(s) is not checked at this stage.
 * (It is checked by Dag object when linking ports)
 */
// used by Node::link_i2o
void
WfInPort::setSourceRef(const string& strRef) {
  // create the appropriate adapter (simple or split) depending on ref parsing
  this->adapter = WfPortAdapter::createAdapter(strRef);
}

bool
WfInPort::isInput() {
  return (adapter == NULL);
}

// used by Dag::checkPrec()
bool
WfInPort::setNodePredecessors(Dag* dag) {
  if (!isInput()) { // in case this method is called on an argument port
    if (!adapter->setNodePredecessors(myParent, dag))
      return false;
  }
  return true;
}

// used by Dag::linkNodePorts
void
WfInPort::setPortDataLinks(Dag* dag) {
  if (!isInput()) { // this method may be called on an argument port
    adapter->setPortDataLinks(this,dag);
  }
}

// used by Node::initProfileExec
bool
WfInPort::initProfileExec() {
  if (WfPort::initProfileExec()) {
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
WfInPort::initSourceData() {
  if (!isInput()) {
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
WfInPort::getPersistenceMode() {
#if HAVE_DAGDA
  return DIET_PERSISTENT;
#else
  if (isInput()) return DIET_VOLATILE;
  else return DIET_PERSISTENT;
#endif
}

/**
 * WfInOutPort class
 */
WfInOutPort::WfInOutPort(Node * parent, string _id, WfCst::WfDataType _type, uint _depth,
                         uint _ind, const string& v) :
  WfPort(parent, _id, _type, _depth, _ind, v),
  WfInPort(parent, _id, _type, _depth, _ind, v),
  WfOutPort(parent, _id, _type, _depth, _ind, v) {
}

diet_persistence_mode_t
WfInOutPort::getPersistenceMode() {
  if (isResult()) return DIET_PERSISTENT_RETURN;
  else return DIET_PERSISTENT;
}

