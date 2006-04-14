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
    }
    Node * myParent;
    string id;
    string type;
    unsigned int index;
    string value;
    
    diet_profile_t * 
    profile() {
      return myParent->profile;
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
  setPriority(int priority);

  /**
   * get the node priority *
   */
  int  
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
  void
  newPort(string id, uint ind, wf_port_t type, string diet_type,
	       const string& v = string(""));
 
  /**
   * set the SeD reference to the node *
   */
  void setSeD(const SeD_var& sed);
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
  /*************************/

  /**
   * the node priority *
   */
  int priority;

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
  set_profile_param(string type, const int lastArg, const string& value,
		    const diet_persistence_mode_t mode);

  /**
   * Set the node as running *
   */
  void
  set_as_running();

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

  /**
   * called when the node execution is done *
   */
  void
  done();

  /**
   * called when a next node is done *
   */
  void nextIsDone();
};


#endif   /* not defined _NODE_HH */
