/****************************************************************************/
/* Basic Node description                                                   */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.2  2006/07/10 11:07:04  aamar
 * - changing the constructor (adding the problem name as parameter)
 * - adding some helpful function : string getPb(), bool isAnExit(),
 * void nextIsDone()
 *
 * Revision 1.1  2006/04/14 13:48:13  aamar
 * Class representing basic dag node (header).
 *
 ****************************************************************************/

#ifndef _BASIC_NODE_HH_
#define _BASIC_NODE_HH_

#include <string>
#include <vector>
#include <map>

using namespace std;

class BasicNode {
public:
  /*********************************************************************/
  /* public methods                                                    */
  /*********************************************************************/

  BasicNode (string id, string pb_name);

  virtual 
  ~BasicNode();

  /**
   * add an argument to the node *
   * @param n   argument id *
   * @param v   argument value *
   *  TO REMOVE *
   */
  void 
  addArg(string n, string v);

  /**
   * TO REMOVE *
   */
  void
  addDataIn(string s1, string s2);

  /**
   * TO REMOVE *
   */
  void
  addDataOut(string s1, string s2);

  /**
   * add a new previous node id *
   */
  void 
  addPrecId(string id);
  
  /**
   * add a new previous node id and reference *
   */
  void
  addPrec(string s, BasicNode * n);

  /**
   * To get the node id *
   */
  string
  getId();

  /**
   * To get the node pb *
   */
  string
  getPb();
  
  /**
   * Return a string representation of the node *
   */
  string
  toString();

  /**
   * Return the number of the previous nodes *
   */  
  unsigned int 
  prec_ids_nb();
  
  /**
   * Get the previous node id by index *
   */
  string 
  getPrecId(unsigned int n);

  /**
   * Add a next node reference *
   */
  void
  addNext(BasicNode * n);

  /**
   * Called when a previous node execution is done *
   * To move to Node class code *
   */
  void
  prevDone();

  /**
   * called when a next node is done *
   * Empty here, reimplemented in subclass Node *
   */
  virtual void 
  nextIsDone();

  /**
   * add a new previous node *
   */
  void
  addPrevNode();
  
  /**
   * Add <n> new previous nodes *
   */
  void
  addPrevNode(int n);

  /**
   * to link input port with output port *
   * reimplemented in Node subclass *
   */
  virtual void 
  link_i2o(const string in, const string out);

  /**
   * to link output port with input port *
   * reimplemented in Node subclass *
   */
  virtual void 
  link_o2i(const string out, const string in);

  /**
   * to link inoutput port with input port *
   * reimplemented in Node subclass *
   */
  virtual void 
  link_io2i(const string io, const string in);

  /**
   * to link inoutput port with input port *
   * reimplemented in Node subclass *
   */
  virtual void 
  link_io2o(const string io, const string out);

  /**
   * set the node tag value *
   * see the myTag properties *
   */
  void setTag(unsigned int t);

  /**
   * return true if the node is an input node *
   * only the nodes with no previous node are considered as dag input  * 
   */
  bool
  isAnInput();

  /**
   * return true if the node is an input node *
   * only the nodes with no next node are considered as dag exit  * 
   */
  bool
  isAnExit();

protected:
  /*********************************************************************/
  /* protected fields                                                  */
  /*********************************************************************/

  /**
   * Node id *
   */
  string myId;

  /**
   * Node problem
   */
  string myPb;

  /**
   * The following three maps are TO REMOVE*
   */
  map <string, string>  args;
  map <string, string> dataIn;
  map <string, string> dataOut;

  /**
   * the previous nodes ids *
   */
  vector<string> prec_ids;

  /**
   * The previous nods map<id, reference> * 
   */
  map <string, BasicNode*>  prec;

  /**
   * The following nodes reference vector *
   */
  vector<BasicNode *> next;

  /**
   * the number of previous nodes not finished (negative) *
   */
  int prevNodes;
  
  /**
   * indicate if the task is done *
   */
  bool task_done;

  /**
   * Node tag*
   * indicates its level in the Dag *
   */
  unsigned int myTag;
};


#endif   /* not defined _BASIC_NODE_HH */



