/****************************************************************************/
/* The port classes used in workflow node                                   */
/* The main classes are WfPort, WfOutPort and WfInPort                      */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.2  2006/11/27 10:15:12  aamar
 * Correct headers of files used in workflow support.
 *
 ****************************************************************************/

#ifndef _WFPORT_HH_
#define _WFPORT_HH_

#include <string>

#include "DIET_data.h"

using namespace std;
/*******************************/
/* class declarations          */
/*******************************/
class Node;
class AbstractInP;
class AbstractOutP;


/**
 * Input port interface *
 */
class AbstractInP {
  friend class Node;
public:
  /**
   * Input port interface destructor *
   */
  virtual 
  ~AbstractInP();

  virtual void
  set_source(AbstractOutP * p) = 0;
    
  virtual void 
  set_source(const string& id) = 0;
};

/**
 * Output port interface *
 */
class AbstractOutP {
  friend class Node;
public:
  /**
   * Output port interface destructor *
   */
  virtual 
  ~AbstractOutP();

  virtual void
  set_sink(AbstractInP * p) = 0;

  virtual void
  set_sink(const string& id) = 0;
};

/**
 * Basic Port *
 */
class WfPort {
  friend class Node;
public:
  /**
   * Basic Port constructor *
   * @param parent The node of the port
   * @param _id    The id of the port (<node id>#<port id>)
   * @param _type  The port data type
   * @param _ind   The index of the parameter in the diet profile
   * @param v      The value of the parameter if it's an argument
   */
  WfPort(Node * parent, string _id, string _type, uint _ind, 
	 const string& v = "") ;

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
   * Return the XML  representation of the port *
   */
  string 
  toXML();

  /**
   * Return the profile of the node
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
protected:
  /*******************************************/
  /* Protected fields                        */
  /*******************************************/

  /**
   * The reference of port node
   */
  Node * myParent;
  
  /**
   * The complete id of the port (<node id>#<port id>)
   */
  string id;

  /**
   * The data type as a string (DIET_XXXX)
   */
  string type;

  /**
   * The index of the port parameter in the diet profile
   */
  unsigned int index;

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
class WfOutPort : public WfPort, 
		  public AbstractOutP {
  friend class Node;
  friend class RunnableNode;
public:
  /**
   * Output port constructor *
   * @param parent The node of the port
   * @param _id    The id of the port (<node id>#<port id>)
   * @param _type  The port data type
   * @param _ind   The index of the parameter in the diet profile
   * @param v      The value of the parameter if it's an argument
   */
  WfOutPort(Node * parent, string _id, string _type, uint _ind,
	    const string& v);


  /**
   * return if the output port is a final output port (workflow result) *
   */
  bool
  isResult();

  /**
   * return an XML  representation of the output port *
   */
  string 
  toXML();

  /**
   * Set the sink of the output port
   * @param _sink_port The sink port (input port) reference
   */
  void 
  set_sink(AbstractInP * _sink_port);

  /**
   * Set the sink id of the output port
   * @param _sink_id The sink port (input port) id
   */
  void 
  set_sink(const string& _sink_id);

  /**
   * Return the sink port id
   */
  string
  getSinkId();

  /**
   * Return the sink port reference
   */
  AbstractInP *
  getSink();

protected:
  /*******************************************/
  /* Protected fields                        */
  /*******************************************/

  /**
   * The reference of sink port associated to this output port
   */
  AbstractInP *sink_port;

  /**
   * The identifier of sink port associated to this output port
   */
  string       sink_port_id;
};

/**
 * Input port class *
 */
class WfInPort : public WfPort,
		 public AbstractInP {
  friend class Node;
  friend class RunnableNode;
public:
  /**
   * Input port constructor *
   * @param parent The node of the port
   * @param _id    The id of the port (<node id>#<port id>)
   * @param _type  The port data type
   * @param _ind   The index of the parameter in the diet profile
   * @param v      The value of the parameter if it's an argument
   */

  WfInPort(Node * parent, string _id, string _type, uint _ind,
	   const string& v);


  /**
   * Set the source of the input port
   * @param _src_port The source port (output port) reference
   */
  void
  set_source(AbstractOutP * _src_port);

  /**
   * Set the source of the input port
   * @param _src_port The source port (output port) reference
   */
  void
  set_source(const string& s);

  /**
   * Return the source id of the input port
   */
  string
  getSourceId();
    
  /**
   * Return the source port reference of the input port
   */
  AbstractOutP *
  getSource();

  /**
   * Return if the input port is an input of the DAG
   */
  bool
  isInput();

  /**
   * return an XML  representation of the input port *
   * if b = false the source port is not included. used to create the
   * remaining DAG representation
   * if b == true the complete representation is returned
   */
  string 
  toXML(bool b = false);

protected:
  /*******************************************/
  /* Protected fields                        */
  /*******************************************/

  /**
   * The reference of source port associated to this input port
   */
  AbstractOutP * source_port;

  /**
   * The identifier of source port associated to this input port
   */
  string         source_port_id;
};

/**
 * Input/Output port class *
 * This class is not used and need to be completed when inout parameter *
 * support will be integrated *
 */
class WfInOutPort : public WfPort, AbstractInP, AbstractOutP {
  friend class Node;
  friend class RunnableNode;
public:
  /**
   * Input/Output port constructor *
   * @param parent The node of the port
   * @param _id    The id of the port (<node id>#<port id>)
   * @param _type  The port data type
   * @param _ind   The index of the parameter in the diet profile
   * @param v      The value of the parameter if it's an argument
   */
  WfInOutPort(Node * parent, string _id, string _type, uint _ind,
	      const string& v);

  void
  set_source(AbstractOutP * _src);

  void
  set_source(const string& s);

  void
  set_sink(AbstractInP * _sink);

  void
  set_sink(const string& s);
  
protected:
  /*******************************************/
  /* Protected fields                        */
  /*******************************************/
  AbstractOutP * source_port;
  string source_port_id;
  AbstractInP  * sink_port;
  string sink_port_id;
};


#endif   /* not defined _WFPORT_HH */
