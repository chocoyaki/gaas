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
extern "C" {
#include "DIET_Dagda.h"
}

/**
 * WfPort class (ABSTRACT)
 */

WfPort::WfPort(Node * parent, string _id, string _type, uint _depth, uint _ind,
	       const string& v) :
  myParent(parent), id(_id),type(_type), depth(_depth), index(_ind),value(v) {
  this->nb_r = 0;
  this->nb_c = 0;
}


/**
 * set the port profile for execution
 */
bool
WfPort::initProfileExec() {
  diet_profile_t* profile = myParent->getProfile();
  diet_persistence_mode_t mode = this->getPersistenceMode();
  if (getDepth() == 0) {
    if (type == WfCst::DIET_CHAR) {
      if (value != "")
        diet_scalar_set(diet_parameter(profile, index),
                        myParent->newChar(value),
                                          mode,
                                          DIET_CHAR);
      else
        diet_scalar_set(diet_parameter(profile, index),
                        myParent->newChar(),
                                          mode,
                                          DIET_CHAR);
    }
    if (type == WfCst::DIET_SHORT) {
      if (value != "")
        diet_scalar_set(diet_parameter(profile, index),
                        myParent->newShort(value),
                                           mode,
                                           DIET_SHORT);
      else
        diet_scalar_set(diet_parameter(profile, index),
                        myParent->newShort(),
                                           mode,
                                           DIET_SHORT);
    }
    if (type == WfCst::DIET_INT) {
      if (value != "")
        diet_scalar_set(diet_parameter(profile, index),
                        myParent->newInt(value),
                                         mode,
                                         DIET_INT);
      else
        diet_scalar_set(diet_parameter(profile, index),
                        myParent->newInt(),
                                         mode,
                                         DIET_INT);
    }
    if (type == WfCst::DIET_LONGINT) {
      if (value != "")
        diet_scalar_set(diet_parameter(profile, index),
                        myParent->newLong(value),
                                          mode,
                                          DIET_LONGINT);
      else
        diet_scalar_set(diet_parameter(profile, index),
                        myParent->newLong(),
                                          mode,
                                          DIET_LONGINT);
    }
    if (type == WfCst::DIET_PARAMSTRING) {
      if (value != "")
        diet_paramstring_set(diet_parameter(profile, index),
                             myParent->newString(value),
                                 mode);
      else
        diet_paramstring_set(diet_parameter(profile, index),
                             myParent->newString(),
                                 mode);
    }
    if (type == WfCst::DIET_STRING) {
      if (value != "")
        diet_string_set(diet_parameter(profile, index),
                        myParent->newString(value),
                                            mode);
      else
        diet_string_set(diet_parameter(profile, index),
                        myParent->newString(),
                                            mode);
    }
    if (type == WfCst::DIET_FLOAT) {
      if (value != "")
        diet_scalar_set(diet_parameter(profile, index),
                        myParent->newFloat(value),
                                           mode,
                                           DIET_FLOAT);
      else
        diet_scalar_set(diet_parameter(profile, index),
                        myParent->newFloat(),
                                           mode,
                                           DIET_FLOAT);
    }
    if (type == WfCst::DIET_DOUBLE) {
      if (value != "")
        diet_scalar_set(diet_parameter(profile, index),
                        myParent->newDouble(value),
                                            mode,
                                            DIET_DOUBLE);
      else
        diet_scalar_set(diet_parameter(profile, index),
                        myParent->newDouble(),
                                            mode,
                                            DIET_DOUBLE);
    }
    if (type == WfCst::DIET_FILE) {
      if (value != "")
        diet_file_set(diet_parameter(profile, index),
                      mode,
                      myParent->newFile(value));
      else
        diet_file_set(diet_parameter(profile, index),
                      mode,
                      NULL);
    }
    if (type == WfCst::DIET_MATRIX) {
      void * mat = NULL;
      switch (base_type) {
        case DIET_CHAR:
          mat = new char[nb_r*nb_c];
          break;
        case DIET_SHORT:
          mat = new int[nb_r*nb_c];
          break;
        case DIET_INT:
          mat = new int[nb_r*nb_c];
          break;
        case DIET_LONGINT:
          mat = new long[nb_r*nb_c];
          break;
        case DIET_FLOAT:
          mat = new float[nb_r*nb_c];
          break;
        case DIET_DOUBLE:
          mat = new double[nb_r*nb_c];
          break;
        default:
          return false;
          break;
      } // end (switch)
      if (value != "") {
        if (value.substr(0, (string("file->")).size()) == "file->") {
          string dataFileName = value.substr((string("file->")).size());
          unsigned len = nb_r*nb_c;
          TRACE_TEXT (TRACE_ALL_STEPS,
                      "reading the matrix data file" << endl);
          switch (base_type) {
            case DIET_CHAR:
              WfCst::readChar(dataFileName.c_str(), (char*)(mat), len);
              break;
            case DIET_SHORT:
              WfCst::readShort(dataFileName.c_str(), (short*)(mat), len);
              break;
            case DIET_INT:
              WfCst::readInt(dataFileName.c_str(), (int*)(mat), len);
              break;
            case DIET_LONGINT:
              WfCst::readLong(dataFileName.c_str(), (long*)(mat), len);
              break;
            case DIET_FLOAT:
              WfCst::readFloat(dataFileName.c_str(), (float*)(mat), len);
              break;
            case DIET_DOUBLE:
              WfCst::readDouble(dataFileName.c_str(), (double*)(mat), len);
              break;
            default:
              return false;
              break;
          } // end switch
        }
        else {
	// get the data if included in the XML workflow description
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
          switch (base_type) {
            case DIET_CHAR:
              ptr1 = (char*)(mat);
              for (unsigned int ix = 0; ix<len; ix++)
                ptr1[ix] = v[ix][0];
              break;
            case DIET_SHORT:
              ptr2 = (short*)(mat);
              for (unsigned int ix = 0; ix<len; ix++)
                ptr2[ix] = atoi(v[ix].c_str());
              break;
            case DIET_INT:
              ptr3 = (int*)(mat);
              for (unsigned int ix = 0; ix<len; ix++)
                ptr3[ix] = atoi(v[ix].c_str());
              break;
            case DIET_LONGINT:
              ptr4 = (long*)(mat);
              for (unsigned int ix = 0; ix<len; ix++)
                ptr4[ix] = atoi(v[ix].c_str());
              break;
            case DIET_FLOAT:
              ptr5 = (float*)(mat);
              for (unsigned int ix = 0; ix<len; ix++)
                ptr5[ix] = atof(v[ix].c_str());
              break;
            case DIET_DOUBLE:
              ptr6 = (double*)(mat);
              for (unsigned int ix = 0; ix<len; ix++)
                ptr6[ix] = atof(v[ix].c_str());
              break;
            default:
              return false;
              break;
          } // end switch
        } // end else
      } // end if value
      diet_matrix_set(diet_parameter(profile,index),
                      mat, mode, base_type, nb_r, nb_c,order);
    }
  } else { // depth > 0 (DIET_CONTAINER)
    if (value != "") {
#if HAVE_DAGDA
      // Currently this container initialization is limited to 1 level
      TRACE_TEXT (TRACE_ALL_STEPS,"Using value (" << value
          << ") to initialize container" << endl);
      // init container
      char* contID;
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
      ERROR("Cannot use containers without Dagda" << endl, 0);
#endif
    } else {
      diet_container_set(diet_parameter(profile, index),DIET_PERSISTENT);
    }
  }
  return true;
} // end initProfileExec


void
WfPort::setMatParams(long nbr, long nbc,
		    diet_matrix_order_t o,
		    diet_base_type_t bt) {
  this->nb_r = nbr;
  this->nb_c = nbc;
  this->order = o;
  this->base_type = bt;
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

/**
 * WfOutPort class (inherits from WfPort)
 */

WfOutPort::WfOutPort(Node * parent, string _id, string _type, uint _depth,
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

diet_persistence_mode_t
WfOutPort::getPersistenceMode() {
  if (isResult()) return DIET_PERSISTENT_RETURN;
  else return DIET_PERSISTENT;
}

/**
 * WfInPort class (inherits from WfPort)
 */
WfInPort::WfInPort(Node * parent, string _id, string _type, uint _depth,
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
void
WfInPort::setNodePredecessors(Dag* dag) {
  if (!isInput()) { // this method may be called on an argument port
    adapter->setNodePredecessors(myParent, dag);
  }
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
    char* srcDataID = adapter->getSourceDataID();
    if (srcDataID != NULL) {
      TRACE_TEXT (TRACE_ALL_STEPS,
                  "##  ==> data ID is " << srcDataID << endl);
      diet_use_data(diet_parameter(this->profile(), index), srcDataID);
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
WfInOutPort::WfInOutPort(Node * parent, string _id, string _type, uint _depth,
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

