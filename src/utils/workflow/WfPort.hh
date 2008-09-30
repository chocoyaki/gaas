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
 * Revision 1.4  2008/09/30 15:32:53  bisnard
 * - using simple port id instead of composite ones
 * - dag nodes linking refactoring
 * - prevNodes and nextNodes data structures modified
 * - prevNodes initialization by Node::setNodePredecessors
 *
 * Revision 1.3  2008/09/30 09:23:29  bisnard
 * removed diet profile initialization from DagWfParser and replaced by node methods initProfileSubmit and initProfileExec
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
 * Revision 1.3  2007/04/20 09:43:28  ycaniou
 * Cosmetic changements for less warnings when compiling with doc.
 * Still some errors to correct (parameters to detail) that I cannot do.
 *
 * Revision 1.2  2006/11/27 10:15:12  aamar
 * Correct headers of files used in workflow support.
 *
 ****************************************************************************/

#ifndef _WFPORT_HH_
#define _WFPORT_HH_

#include <string>
#include <list>

#include "DIET_data.h"
#include "WfPortAdapter.hh"
#include "WfUtils.hh"

using namespace std;

/*******************************/
/* class declarations          */
/*******************************/
class Node;
class WfInPort;
class WfOutPort;

/**
 * Basic Port *
 */
class WfPort {
public:
  /**
   * Basic Port constructor *
   * @param parent The node of the port
   * @param _id    The id of the port
   * @param _type  The port data base type
   * @param _depth The depth of the list structure (0 if no list)
   * @param _ind   The index of the parameter in the diet profile
   * @param v      The value of the parameter if it's an argument
   */
  WfPort(Node * parent, string _id, WfCst::WfDataType _type, uint _depth, uint _ind,
	 const string& v = "") ;

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
   * Set the parameter of a matrix argument
   * @param nbr  Row count
   * @param nbc  Column count
   * @param o    Matrix order
   * @param bt   Data type of matrix elements
   */
  void setMatParams(long nbr, long nbc,
		    diet_matrix_order_t o,
		    diet_base_type_t bt);

  /**
   * Return the profile of the node
   * used by WfPortAdapter::getSourceDataID
   */
  diet_profile_t *
  profile();

  /**
   * Return the port id
   */
  string
  getId();

  /**
   * Return the port index (parameter index in diet profile)
   */
  unsigned int
  getIndex();

  /**
   * Return the port depth (container)
   */
  unsigned int
  getDepth();

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

  /*******************************************/
  /* Protected fields                        */
  /*******************************************/

  /**
   * The reference of port node
   */
  Node * myParent;

  /**
   * The complete id of the port ('node id'#'port id')
   */
  string id;

  /**
   * The data type (constants defined in WfCst class)
   */
  WfCst::WfDataType type;

  /**
   * The data type of elements (in case of type=CONTAINER)
   */
  WfCst::WfDataType eltType;

  /**
   * The depth of the list structure (eg if type="LIST(LIST(INT))")
   */
  unsigned int depth;

  /**
   * The index of the port parameter in the diet profile
   */
  unsigned int index;

  /**
   * The port data ID
   */
  string dataID;

  /**
   * The port value as a string
   */
  string value;

  /**
   * The row count (for matrix parameter)
   */
  long nb_r;

  /**
   * The column count (for matrix parameter)
   */
  long nb_c;

  /**
   * The matrix order (for matrix parameter)
   */
  diet_matrix_order_t order;

  /**
   * The data type of matrix elements (for matrix parameter)
   */
  diet_base_type_t base_type;


};

/**
 * Output port class *
 */
class WfOutPort : virtual public WfPort {
public:
  /**
   * Output port constructor *
   * @param parent The node of the port
   * @param _id    The id of the port
   * @param _type  The port data type
   * @param _depth The depth of the list structure (0 if no list)
   * @param _ind   The index of the parameter in the diet profile
   * @param v      The value of the parameter if it's an argument
   */
  WfOutPort(Node * parent, string _id, WfCst::WfDataType _type, uint _depth, uint _ind,
	    const string& v);


  /**
   * return if the output port is a final output port (workflow result) *
   */
  bool
  isResult();

  /**
   * Set the sink of the output port
   * @param _sink_port The sink port (input port) reference
   */
  void
  setSink(WfInPort * _sink_port);

  /**
   * Set the sink id of the output port
   * @param _sink_id The sink port (input port) id
   */
  void
  setSink(const string& _sink_id);

  /**
   * Return the sink port id
   */
  string
  getSinkId();

  /**
   * Return the sink port reference
   */
  WfInPort *
  getSink();

  /**
   * Store the data IDs from the profile
   */
  void
  storeProfileData();

protected:

  /**
   * Returns the persistence mode for this port
   */
  diet_persistence_mode_t
      getPersistenceMode();

  /*******************************************/
  /* Protected fields                        */
  /*******************************************/

  /**
   * The reference of sink port associated to this output port
   */
  WfInPort * sink_port;

  /**
   * The identifier of sink port associated to this output port
   */
  string       sink_port_id;
};

/**
 * Input port class *
 */
class WfInPort : virtual public WfPort {
public:
  /**
   * Input port constructor *
   * @param parent The node of the port
   * @param _id    The id of the port
   * @param _type  The port data type
   * @param _depth The depth of the list structure (0 if no list)
   * @param _ind   The index of the parameter in the diet profile
   * @param v      The value of the parameter if it's an argument
   */

  WfInPort(Node * parent, string _id, WfCst::WfDataType _type, uint _depth, uint _ind,
	   const string& v);

  /**
   * Set the source of the input port (Parsing only)
   * @param s The source port (output port) reference
   */
  virtual void
  setSourceRef(const string& strRef);

  /**
   * Return if the input port is an input of the DAG
   * (ie if there is no source port defined)
   */
  bool
  isInput();

  /**
   * Nodes linking (used for dag scheduling)
   */
  bool
  setNodePredecessors(Dag* dag);

  /**
   * Ports linking (used for node execution)
   * @param dag   the dag that contains the linked ports
   */
  void setPortDataLinks(Dag* dag);

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

  /*******************************************/
  /* Protected fields                        */
  /*******************************************/

  WfPortAdapter* adapter;

}; // end class WfInPort

/**
 * Input/Output port class *
 * This class is not used and need to be completed when inout parameter *
 * support will be integrated *
 */
class WfInOutPort : virtual public WfInPort, virtual public WfOutPort {
public:
  /**
   * Input/Output port constructor *
   * @param parent The node of the port
   * @param _id    The id of the port
   * @param _type  The port data type
   * @param _depth The depth of the list structure (0 if no list)
   * @param _ind   The index of the parameter in the diet profile
   * @param v      The value of the parameter if it's an argument
   */
  WfInOutPort(Node * parent, string _id, WfCst::WfDataType _type, uint _depth, uint _ind,
	      const string& v);

protected:
  /**
   * Returns the persistence mode for this port
   */
  diet_persistence_mode_t
      getPersistenceMode();

}; // end class WfInOutPort

#endif   /* not defined _WFPORT_HH */
