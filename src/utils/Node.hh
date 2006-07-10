/****************************************************************************/
/* Extended Node description                                                */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.2  2006/07/10 11:07:37  aamar
 * - Adding Matrix data type support
 * - The toXML (for Node and Ports) method that return XML
 * representation
 * - Some function and attributes used for scheduling and
 * rescheduling
 *
 * Revision 1.1  2006/04/14 13:50:16  aamar
 * Class representing a Dag node (header). This is a BasicNode
 * subclass that includes necessary code for execution).
 *
 ****************************************************************************/

#ifndef _NODE_HH_
#define _NODE_HH_

#include <iostream>
#include <string>
#include <map>

#include <sys/time.h>
#include <time.h>

// general DIET headers
#include "DIET_data.h"
#include "DIET_client.h"
#include "SeD.hh"
#include "common_types.hh"
// workflow related headers
#include "BasicNode.hh"
#include "Thread.hh"
#include "WfUtils.hh"


using namespace std;

class Node;
class Dag;


/**
 * This class is responible of node execution *
 */
class RunnableNode : public Thread {
public:
  RunnableNode(Node * parent);
private:
  Node * myParent;

  /**
   * Node execution methods *
   */
  void * 
  run();
};

class Node : public BasicNode {
  /*******************************/
  /* friend classes              */
  friend class RunnableNode;
  friend class Dag;
  friend class WfExtReader;
  friend class AbstractWfSched;
  /*******************************/

  /*******************************/
  /* class declarations          */
  /*******************************/
  class AbstractInP;
  class AbstractOutP;


  /**
   * Input port interface *
   */
  class AbstractInP {
  public:
    virtual 
    ~AbstractInP() {
    }
    virtual void
    set_source(AbstractOutP * p) = 0;
    
    virtual void 
    set_source(const string& id) = 0;
  };

  /**
   * Output port interface *
   */
  class AbstractOutP {
  public:
    virtual 
    ~AbstractOutP() {
    }

    virtual void
    set_sink(AbstractInP * p) = 0;

    virtual void
    set_sink(const string& id) = 0;
  };

  /**
   * Basic Port *
   */
  class WfPort {
  public:
    WfPort(Node * parent, string _id, string _type, uint _ind, 
	   const string& v = "") : 
      myParent(parent), id(_id),type(_type), index(_ind),value(v) {
      this->nb_r = 0;
      this->nb_c = 0;
    }
    Node * myParent;
    string id;
    string type;
    unsigned int index;
    string value;
    // for matrix parameter
    long nb_r, nb_c;
    diet_matrix_order_t order;
    diet_base_type_t base_type;
    void setMatParams(long nbr, long nbc, 
		      diet_matrix_order_t o,
		     diet_base_type_t bt) {
      this->nb_r = nbr;
      this->nb_c = nbc;
      this->order = o;
      this->base_type = bt;
    }

    diet_profile_t * 
    profile() {
      return myParent->profile;
    }

    /**
     * return an XML  representation of the port *
     */
    string 
    toXML() {
      string xml = "";
      xml += "id=\"" + this->id + "\" type=\"" + this->type +"\" ";
      if (this->type == "DIET_MATRIX") {
	/*
	  base_type  = element.attribute("base_type");
	  nb_rows  = element.attribute("nb_rows");
	  nb_cols  = element.attribute("nb_cols");
	  matrix_order  = element.attribute("matrix_order");
	*/
	xml += "base_type=\""+getBaseTypeStr(this->base_type)+"\" ";
	xml += "nb_rows=\"" + itoa(this->nb_r)+"\" ";
	xml += "nb_cols=\"" + itoa(this->nb_c)+"\" ";
	xml += "matrix_order=\"" + getMatrixOrderStr(this->order) + "\" ";
      }
      return xml;
    }

  };

  /**
   * Output port class *
   */
  class WfOutPort : public WfPort, 
		    public AbstractOutP {
  public:
    WfOutPort(Node * parent, string _id, string _type, uint _ind,
	      const string& v) :
      WfPort(parent, _id, _type, _ind, v) {
      sink_port = NULL;
    }
    AbstractInP *sink_port;
    string       sink_port_id;
    
    void 
    set_sink(AbstractInP * _sink_port) {
      this->sink_port = _sink_port;
    }
    
    void 
    set_sink(const string& s) {
      this->sink_port_id = s;
    }
    
    /**
     * return if the output port is a final output port (workflow result) *
     */
    bool
    isResult() {
      return (sink_port == NULL);
    }

    /**
     * return an XML  representation of the output port *
     */
    string 
    toXML() {
      string xml = "\t<out ";
      xml += WfPort::toXML();
      if (this->sink_port != NULL)
	xml += "sink=\""+sink_port_id+"\"";
      
      xml += " />\n";
      return xml;
    }
  };

  /**
   * Input port class *
   */
  class WfInPort : public WfPort,
		   public AbstractInP {
  public:
    WfInPort(Node * parent, string _id, string _type, uint _ind,
	     const string& v) :
      WfPort(parent, _id, _type, _ind, v) {
      source_port = NULL;
    }
    AbstractOutP * source_port;
    string         source_port_id;

    void
    set_source(AbstractOutP * _src) {
      this->source_port = _src;
    }

    void
    set_source(const string& s) {
      this->source_port_id = s;
    }
    
    bool isInput() {
      return (source_port == NULL);
    }

    /**
     * return an XML  representation of the input port *
     * if b = false the source port is not included. used to create the
     * remaining DAG representation
     */
    string 
    toXML(bool b=false) {
      string xml = "";
      if (value != "")
	xml = "\t<arg ";
      else
	xml ="\t<in ";
      xml += WfPort::toXML();
      if (value != "")
	xml += "value=\"" + this->value + "\" "; 
      if ((b) && (this->source_port != NULL))
	xml += "source=\""+source_port_id+"\"";
      
      xml += " />\n";
      return xml;
    }
  };

  /**
   * Input/Output port class *
   */
  class WfInOutPort : public WfPort, AbstractInP, AbstractOutP {
  public:
    WfInOutPort(Node * parent, string _id, string _type, uint _ind,
		const string& v) :
      WfPort(parent, _id, _type, _ind, v) {
    }
    AbstractOutP * source_port;
    string source_port_id;
    AbstractInP  * sink_port;
    string sink_port_id;

    void
    set_source(AbstractOutP * _src) {
      this->source_port = _src;
    }
    void
    set_source(const string& s) {
      this->source_port_id = s;
    }
    void
    set_sink(AbstractInP * _sink) {
      this->sink_port = _sink;
    }
    void
    set_sink(const string& s) {
      this->sink_port_id = s;
    }
  };

public:
  /*********************************************************************/
  /* public methods                                                    */
  /*********************************************************************/
  Node(string id, string pb_name,
       int last_in, int last_inout, int last_out);
  ~Node();

  /**
   * display the textual representation of a node *
   */
  string
  toString();
  
  /**
   * display an XML  representation of a node *
   * if b = false the node representation doesn't include the information
   * about previous nodes (the source ports of input ports)
   */
  string 
  toXML(bool b = false);

  /**
   * set the node profile
   */
  void
  set_pb_desc(diet_profile_t* profile);

  /**
   * start the node execution * 
   */
  void
  start();

  /******************************/
  /* data allocation methods    */
  /******************************/

  /**
   * Allocate a new char *
   */
  char *  
  newChar  (const string value = " ");
  /**
   * Allocate a new short *
   */
  short * 
  newShort (const string value = "0");

  /**
   * Allocate a new int  *
   */
  int *   
  newInt   (const string value = "0");

  /**
   * Allocate a new long *
   */
  long *  
  newLong  (const string value = "0");

  /**
   * Allocate a new string *
   */
  char * 
  newString (const string value = "0");

  /**
   * Allocate a new float  *
   */
  float *   
  newFloat  (const string value = "0");

  /**
   * Allocate a new double  *
   */
  double *   
  newDouble (const string value = "0");

  /**
   * set the node priority *
   */
  void
  setPriority(double priority);

  /**
   * get the node priority *
   */
  double
  getPriority();

  /**
   * get if the node is ready for execution *
   */
  bool
  isReady();

  /**
   * Link input port to output port by id and setting references link *
   */
  virtual void
  link_i2o(const string in, const string out);
  
  /**
   * Link output port to input port by id and setting references link *
   */
  virtual void
  link_o2i(const string out, const string in);
  
  /**
   * Link inoutput port to input port by id and setting references link *
   */
  virtual void
  link_io2i(const string io, const string in);
  
  /**
   * Link inoutput port to output port by id and setting references link *
   */
  virtual void
  link_io2o(const string io, const string out);

  /**
   * create and add a new port to the node *
   */
  WfPort *
  newPort(string id, uint ind, wf_port_t type, string diet_type,
	       const string& v = string(""));
 
  /**
   * set the SeD reference to the node *
   */
  void setSeD(const SeD_var& sed);

  /**
   * return the number of next nodes
   */
  unsigned int 
  nextNb();

  /**
   * return  next node 
   */
  BasicNode *
  getNext(unsigned int n);

  /**
   * return the number of previous nodes
   */
  unsigned int 
  prevNb();

  /**
   * return  next node 
   */
  BasicNode *
  getPrev(unsigned int n);

  /**
   * set the estimated completion time
   */
  void
  setEstCompTime(const long int est_comp_time);

  /**
   * get the estimated completion time
   */
  long int
  getEstCompTime();

  /**
   * set the real completion time
   */
  void
  setRealCompTime(const struct timeval& real_comp_time);

  /** 
   * get the real completion time
   */
  struct timeval
  getRealCompTime();

  /**
   * get the node mark (used for reordering)
   */
  bool
  getMark();

  /**
   * set the node mark 
   */
  void 
  setMark(bool b);

  /**
   * test if the node is running *
   */
  bool
  isRunning();
  
  /**
   * test if the execution is done *
   */
  bool
  isDone();

private:
  /*********************************************************************/
  /* private fields                                                    */
  /*********************************************************************/
  /**
   * problem profile *
   */
  diet_profile_t * profile;

  /**
   * node thread *
   */
  RunnableNode   * myRunnableNode;

  /*************************/
  /* problem parameters    */
  /*************************/
  vector<char*>   charParams;
  vector<short*>  shortParams;
  vector<int*>    intParams;
  vector<long*>   longParams; 
  vector<char *>  stringParams;
  vector<float*>  floatParams;
  vector<double*> doubleParams;
  //  vector<void *>  matrixParams;
  /*************************/

  /**
   * the node priority *
   */
  double priority;

  /**
   * input ports map<id, reference> *
   */
  map<string, WfInPort*> inports;

  /**
   * output ports map<id, reference> *
   */
  map<string, WfOutPort*> outports;

  /**
   * inoutput ports map<id, reference> *
   */
  map<string, WfInOutPort*> inoutports;

  /**
   * node running status *
   */
  bool node_running;

  /**
   * chosen server *
   */
  SeD_var chosenServer;

  /**
   * number of immediate next nodes that have end their execution *
   */
  unsigned int nextDone;

  /**
   * Estimated completion time in second 
   * (time 0 is the beginning of the execution)
   */
  long int EstCompTime;

  /**
   * Real completion time
   */
  struct timeval RealCompTime;

  /**
   * the node mark (used for reordering)
   */
  bool myMark;

  /*********************************************************************/
  /* private methods                                                   */
  /*********************************************************************/

  /**
   * Store the persistent data of the node profile *
   */
  void
  storePersistentData();

  
  /**
   * Get the input port references by id *
   */
  WfInPort*    
  getInPort(string id);

  /**
   * Get the output port reference by id *
   */
  WfOutPort*   
  getOutPort(string id);

  /**
   * Get the input/output port reference by id *
   */
  WfInOutPort* 
  getInOutPort(string id);

  /**
   * create the diet profile associated to the node *
   */
  void
  createProfile();

  /**
   * set the node profile param *
   * @param type    parameter data type * 
   * @param lastArg parameter index *
   * @param value   string representation of parameter value *
   */
  void
  set_profile_param(WfPort * port, 
		    string type, const int lastArg, const string& value,
		    const diet_persistence_mode_t mode);

  /**
   * Set the node as running *
   */
  void
  set_as_running();

  /**
   * called when the node execution is done *
   */
  void
  done();

  /**
   * called when a next node is done *
   */
  virtual void 
  nextIsDone();
};


#endif   /* not defined _NODE_HH */
