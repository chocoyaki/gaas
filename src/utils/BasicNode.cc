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
 * Revision 1.1  2006/04/14 13:48:45  aamar
 * Class representing basic dag node (source).
 *
 ****************************************************************************/

#include <iostream>
#include <sstream>
using namespace std;

#include "BasicNode.hh"

// The manual advise to nose use the ? operator but don't worry about this one
#define MAX( x, y) ( (x) > (y) ? (x) : (y) )

BasicNode::BasicNode(string id, string pb_name) {
  this->myId = id;
  this->myPb = pb_name;
  this->prevNodes = 0;
  this->task_done = false;
  this->myTag = 0;
}
BasicNode::~BasicNode() {
  // clear the arguments map
  args.clear();
  // clear the precedence map
  prec.clear();
  // clear the dataIn map
  dataIn.clear();
  // clear the dataOut map
  dataOut.clear();
}

/**
 * TO REMOVE *
 */
void
BasicNode::addArg(string name, string value) {
  args[name] = value;
}

/**
 * TO REMOVE *
 */
void BasicNode::addDataIn(string s1, string s2) {
  dataIn[s1] = s2;
}
/**
 * TO REMOVE *
 */
void BasicNode::addDataOut(string s1, string s2) {
  dataOut[s1] = s2;
}

/**
 * add a new previous node id *
 */
void BasicNode::addPrecId(string str) {
  prec_ids.push_back(str);
}
/**
 * Add a new previous node id and reference *
 */
void BasicNode::addPrec(string str, BasicNode * node) {
  // add the node as a previous one if not already done
  if (prec.find(str) == prec.end()) {
    cout << "The node " << this->myId << " has a new previous node " << endl;
    prec[str] = node; 
    prevNodes--;
    node->addNext(this);
  }
}

/**
 * To get the node id *
 */
string BasicNode::getId() {
  return this->myId;
}

/**
 * To get the node id *
 */
string BasicNode::getPb() {
  return this->myPb;
}

/**
 * Return a string representation of the node *
 */
string BasicNode::toString() {
  string str;
  str = "--------------------------------------";
  str += "-> Name = " + this->myId + "\n";
  str += "-> Arguments :\n";

  for (map<string, string>::iterator p = args.begin( ); 
       p != args.end( ); 
       ++p ) { 
    string argName = (string) p->first;
    string argValue = (string) p->second;
    str += "    - " + argName + "=" + argValue + "\n";
  }
  str += "-> Input Data :\n";
  for (map<string, string>::iterator p = dataIn.begin( ); 
       p != dataIn.end( ); 
       ++p ) { 
    string inName = (string) p->first;
    string inSource = (string) p->second;
    str += "    - " + inName + "=" + inSource + "\n";
  }
  str += "-> Output Data :\n";
  for (map<string, string>::iterator p = dataOut.begin( ); 
       p != dataOut.end( ); 
       ++p ) { 
    string outName = (string) p->first;
    string outSource = (string) p->second;
    str += "    " + outName + "=" + outSource + "\n";
  }
  str += "-> Precedence :\n";
  for (unsigned int ix = 0; ix< prec_ids.size(); ix++) { 
    str += "    - " + prec_ids[ix] + "\n";
  }
  str += "-> Precedence (with reference, just to check) :\n";

  for (map<string, BasicNode * >::iterator p = prec.begin( ); 
       p != prec.end( ); 
       ++p ) { 
    BasicNode * node = (BasicNode*) p->second;
    str += "    - " + node->getId() + "\n";
  }
  ostringstream ss;
  ss << "-> My tag =" << this->myTag << "\n" << 
    "I have " << next.size() << " next nodes " << endl;
  str += ss.str();
  return str;
}

/**
 * Return the number of the previous nodes *
 */  
unsigned int
BasicNode::prec_ids_nb() {
  return prec_ids.size();
}

/**
 * Get the previous node id by index *
 */
string 
BasicNode::getPrecId(unsigned int n) {
  if (n<prec_ids.size())
    return prec_ids[n];
  else
    return string("");
}

/**
 * Add a next node reference *
 */
void
BasicNode::addNext(BasicNode * n) {
  next.push_back(n);
}

/**
 * Called when a previous node execution is done *
 */
void 
BasicNode::prevDone() {
  prevNodes++;
}

/**
 * called when a next node is done *
 * Empty here, reimplemented in subclass Node *
 */
void
BasicNode::nextIsDone() {
}

/**
 * add a new previous node *
 */
void 
BasicNode::addPrevNode() {
  prevNodes--;
}

/**
 * Add n new previous nodes *
 */
void 
BasicNode::addPrevNode(int n) {
  prevNodes -= n;
}



/**
 * to link input port with output port *
 * reimplemented in Node subclass *
 */
void 
BasicNode::link_i2o(const string in, const string out) {
}

/**
 * to link output port with input port *
 * reimplemented in Node subclass *
 */
void
BasicNode::link_o2i(const string out, const string in) {
}

/**
 * to link inoutput port with input port *
 * reimplemented in Node subclass *
 */
void 
BasicNode::link_io2i(const string io, const string in) {
}

/**
 * to link inoutput port with input port *
 * reimplemented in Node subclass *
 */
void 
BasicNode::link_io2o(const string io, const string out) {
}

/**
 * set the node tag value *
 */
void
BasicNode::setTag(unsigned int t) {
  this->myTag = MAX(this->myTag, t);
  for (unsigned int ix=0; ix < next.size(); ix++) {
    next[ix] -> setTag(1+ (this->myTag));
  }
}

/**
 * return if the node is an input node *
 * only the nodes with no previous node are considered as dag input  * 
 */
bool
BasicNode::isAnInput() {
  return (prec.size()==0);
}

/**
 * return true if the node is an input node *
 * only the nodes with no next node are considered as dag exit  * 
 */
bool
BasicNode::isAnExit() {
  return (next.size() == 0);
}
