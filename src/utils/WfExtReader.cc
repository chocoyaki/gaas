/****************************************************************************/
/* Extended workflow description Reader                                     */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $@Id: $
 * $@Log: $
 ****************************************************************************/

#include <iostream>

#include "marshalling.hh"
#include "WfExtReader.hh"
#include "WfUtils.hh"
#include "debug.hh"


using namespace std;

WfExtReader::WfExtReader(const char * content) :
  WfReader(content) {
  myDag = new Dag();
}

WfExtReader::~WfExtReader() {
  if (myDag) {
    delete (myDag);
  }
}

/**
 * Initialize the processing
 */
void 
WfExtReader::setup() {
  initXml();
}

/** 
 * Init the XML processing
 */
bool 
WfExtReader::initXml() {
  bool result = WfReader::initXml();
  cout << "Cheking the precedence ..." << endl;
  myDag->checkPrec();
  cout << myDag->toString() << endl;
  cout << "... done" << endl;  
  return result;
}

/**
 * get a reference to Dag structure
 */
Dag *
WfExtReader::getDag() {
  return myDag;
}

/**
 * parse a node element
 */
bool 
WfExtReader::parseNode (const DOMNode * element, 
			string nodeId, string nodePath,
			long int var_node) {
  diet_profile_t * profile;
  //  corba_pb_desc_t * corba_profile = new corba_pb_desc_t;
  vector <DOMElement *> argVect;
  vector <DOMElement *> inVect;
  vector <DOMElement *> inoutVect;
  vector <DOMElement *> outVect;
  vector <DOMElement *> precVect;
  DOMElement * node;
  long int mul_arg =0, mul_in = 0, mul_inout = 0, mul_out = 0; 

  DOMNode * child = element->getFirstChild();
  while (child != NULL) {
    if (child->getNodeType() == DOMNode::ELEMENT_NODE) {
      DOMElement * child_elt = (DOMElement*)child;

      char *child_name_str = XMLString::transcode(child_elt->getNodeName());
      string child_name(child_name_str);
      XMLString::release(&child_name_str);

      if (child_name == "arg") {
	string argName = getAttributeValue("name", child_elt);
	string argValue = getAttributeValue("value", child_elt);
	if ((argName == "") || (argValue == "")) {
	  ERROR ("Argument "<< child_name <<" malformed", false);
	  return false;
	}
	if (child_elt->getFirstChild() != NULL) {
	  cerr << "Argument element doesn't accept a child element" << endl <<
	    "May be a </arg> is forgotten" << endl;
	  return false;
	}

	argVect.push_back(child_elt);

	string mul_attr = getAttributeValue("mul", child_elt);
	if (mul_attr != "") {
	  mul_arg += atoi(mul_attr.c_str());
	}
      } // end if (arg

      if (child_name == "in") {
	if (child_elt->getFirstChild() != NULL) {
	  cerr << "input port element doesn't accept a child element" << endl <<
	    "May be a </in> is forgotten" << endl;
	  return false;
	}
	inVect.push_back(child_elt);

	string mul_attr = getAttributeValue("mul", child_elt);
	if (mul_attr != "") {
	  mul_in += atoi(mul_attr.c_str());
	}
      } // end if (in

      if (child_name == "inOut") {
	if (child_elt->getFirstChild()!=NULL) {
	  cerr << "inOut port element doesn't accept a child element" << endl <<
	    "May be a </inOut> is forgotten" << endl;
	  return false;
	}

	inoutVect.push_back(child_elt);
	string mul_attr = getAttributeValue("mul", child_elt);
	if (mul_attr != "") {
	  mul_inout += atoi(mul_attr.c_str());
	}
      } // end if (inout)

      if (child_name == "out") {
	if (child_elt->getFirstChild() != NULL) {
	  cerr << "output port element doesn't accept a child element" << endl <<
	    "May be a </out> is forgotten" << endl;
	  return false;
	}
	
	outVect.push_back(child_elt);
	
	string mul_attr = getAttributeValue("mul", child_elt);
	if (mul_attr != "") {
	  mul_out += atoi(mul_attr.c_str());
	} 
      } // end if (out
      
      if (child_name == "prec") {
	string prevNodeId = getAttributeValue("id", child_elt);
	if (prevNodeId == "") {
	  cout  << "Precedence element malformed" << endl;
	  return false;
	}
	
	if (child_elt->getFirstChild() != NULL) {
	  cerr << "Precedence element doesn't accept a child element" << endl <<
	    "May be a </prec> is forgotten" << endl;
	  return false;
	}
	string key = prevNodeId + string("/") + nodeId;
	
	precVect.push_back(child_elt);
	// Is the previous node useful ?
      } // end if (prec      
    } // end if (ELEMENT_NODE
    child = child->getNextSibling();
  } // end while

  // adjust mul_xxx
  if (mul_arg == 0)
    mul_arg = 1;
  if (mul_in == 0)
    mul_in = 1;
  if (mul_inout == 0)  
    mul_inout = 1;
  if (mul_out == 0)  
    mul_out = 1;

  // last_in last_inout last_out
  int last_in    = inVect.size() + argVect.size() - 1 + 
    (mul_arg - 1) + (mul_in - 1);
  int last_inout = last_in + inoutVect.size() + 
    (mul_inout - 1);
  int last_out   = last_inout + outVect.size() +
    (mul_out - 1);

  int lastArg    = 0;
  cout << "\t" << "last in " << last_in <<
    ", last_inout " << last_inout <<
    ", last_out " << last_out << endl;
  Node * dagNode = new Node(nodeId, nodePath,
			    last_in, last_inout, last_out);  

  profile = diet_profile_alloc((char*)nodePath.c_str(), 
				last_in,
				last_inout,
				last_out);

  //************************************  
  //** Arguments
  for (uint ix=0; ix < argVect.size(); ix++) {
    node = argVect[ix];

    string name = nodeId + "#";
    /* not necessary since the node id contains already the number 
    if (var != -1) {
      name = nodeId + itoa(var).c_str() + "#";
    }
    else {
      name = nodeId + "#";
    }
    */
    string value = getAttributeValue("value", node);
    string type  = getAttributeValue("type", node);
    string abs_name = getAttributeValue("name", node);
    if (abs_name[0] == '$') {
      // multiple port
      abs_name = abs_name.erase(0,1);
      string mul_str = getAttributeValue("mul", node);
      long int mul = atoi(mul_str.c_str());
      for (int var_port=0; var_port < mul; var_port++) {
	string real_port_name = abs_name + itoa(var_port);
	name = name + real_port_name.c_str();
	if (type != "DIET_MATRIX")
	  // check the arg attribute and add its description to the profile
	  checkArg (name, value, type, profile, lastArg, 
		    var_node, var_port, dagNode);
	else
	  // check the matrix argument
	  checkMatrixArg(name, node, profile, lastArg, 
			 var_node, var_port, dagNode);
	lastArg++;
      } // end for
    }
    else {
      // single port
      name = name + getAttributeValue("name", node);
      if (type != "DIET_MATRIX")
	// check the arg attribute and add its description to the profile
	checkArg (name, value, type, profile, lastArg, 
		  var_node, -1,
		  dagNode);
      else
	// check the matrix argument
	checkMatrixArg(name, node, profile, lastArg,
		       var_node, -1, 
		       dagNode);
      lastArg++;
    }
  }
  //************************************  
  // In parameters
  for (uint ix=0; ix < inVect.size(); ix++) {
    node = inVect[ix];

    string name("");
    /* not necessary since the node id contains already the number 
    QString name("");
    if (var != -1) {
      name = nodeId + itoa(var).c_str() + "#";
    }
    else {
      name = nodeId + "#";
    } 
    */

    string type  = getAttributeValue("type", node);
    string source  = getAttributeValue("source", node);

    string abs_name = getAttributeValue("name", node);
    if (abs_name[0] == '$') {
      // multiple port
      abs_name = abs_name.erase(0,1);
      string mul_str = getAttributeValue("mul", node);
      long int mul = atoi(mul_str.c_str());
      for (int var_port = 0; var_port < mul; var_port++) {
	string real_port_name = abs_name + itoa(var_port);
	name = nodeId + "#" + real_port_name.c_str();
	if (type != "DIET_MATRIX")
	  // check in attribute and add its description to the profile
	  checkIn (name, type, source, profile, lastArg, 
		   var_node, var_port, 
		   dagNode);
	else
	  // check a matrix input port
	  checkMatrixIn(name, node, profile, lastArg, 
			var_node, var_port, 
			dagNode);
	lastArg++;
      }
    }
    else {
      // single port
      name = nodeId + "#" + getAttributeValue("name", node);
      if (type != "DIET_MATRIX")
	// check in attribute and add its description to the profile
	checkIn (name, type, source, profile, lastArg, 
		 var_node, -1,
		 dagNode);
      else
	// check a matrix input port
	checkMatrixIn(name, node, profile, lastArg, 
		      var_node, -1,
		      dagNode);
      lastArg++;
    }
  }
  //************************************  
  // Inout parameters
  // TO FIX
  for (uint ix=0; ix < inoutVect.size(); ix++) {
    node = inoutVect[ix];

    string name("");
    /* not necessary since the node id contains already the number 
    QString name("");
    if (var != -1) {
      name = nodeId + itoa(var).c_str() + "#";
    }
    else {
      name = nodeId + "#";
    }
    */

    string type  = getAttributeValue("type", node);
    string sink  = getAttributeValue("sink", node);

    string abs_name = getAttributeValue("name", node);
    if (abs_name[0] == '$') {
      // multiple port
      abs_name = abs_name.erase(0,1);
      string mul_str = getAttributeValue("mul", node);
      long int mul = atoi(mul_str.c_str());
      for (int var_port = 0; var_port < mul; var_port++) {
	string real_port_name = abs_name + itoa(var_port);
	name = nodeId + "#" + real_port_name.c_str();
	if (type != "DIET_MATRIX")
	  // check the inout attribute and add its description to the profile
	  checkInout(name, type, sink, profile, lastArg, 
		     var_node, var_port, dagNode);
	else 
	  // check matrix inout port
	  checkMatrixInout(name, node, profile, lastArg, 
			   var_node, var_port, dagNode);

	lastArg++;
      }
    }
    else {
      // single port
      name = nodeId + "#" + getAttributeValue("name", node);
      if (type != "DIET_MATRIX")
	// check the inout attribute and add its description to the profile
	checkInout(name, type, sink, profile, lastArg, 
		   var_node, -1,
		   dagNode);
      else 
	// check matrix inout port
	checkMatrixInout(name, node, profile, lastArg, 
			 var_node, -1,
			 dagNode);
      
      lastArg++;
    }
  }
  //************************************  
  // Out parameters

  for (uint ix=0; ix < outVect.size(); ix++) {
    node = outVect[ix];

    string name("");
    /* not necessary since the node id contains already the number 
    QString name("");
    if (var_node != -1) {
      name = nodeId + itoa(var_node).c_str() + "#";
    }
    else {
      name = nodeId + "#";
    }
    */

    string type  = getAttributeValue("type", node);
    string sink  = getAttributeValue("sink", node); 

    string abs_name = getAttributeValue("name", node);
    if (abs_name[0] == '$') {
      // multiple port
      abs_name = abs_name.erase(0,1);
      string mul_str = getAttributeValue("mul", node);
      long int mul = atoi(mul_str.c_str());
      for (int var_port=0; var_port < mul; var_port++) {
	string real_port_name = abs_name + itoa(var_port);
	name = nodeId + "#" + real_port_name.c_str();
	if (type != "DIET_MATRIX")
	  // check the out attribute and add its description to the profile
	  checkOut (name, type, sink, profile, lastArg, 
		    var_node, var_port,
		    dagNode);
	else
	  // check matrix output port
	  checkMatrixOut(name, node, profile, lastArg, 
			 var_node, var_port, 
			 dagNode);
	lastArg++;
      } // end for
    }
    else {
      // single port
      name = nodeId + "#" + getAttributeValue("name", node);
      if (type != "DIET_MATRIX")
	// check the out attribute and add its description to the profile
	checkOut (name, type, sink, profile, lastArg, 
		  var_node, -1,
		  dagNode);
      else
	// check matrix output port
	checkMatrixOut(name, node, profile, lastArg, 
		       var_node, -1,
		       dagNode);
      lastArg++;
    }
  }
  //************************************  
  cout << "\t" << "---> Adding the precedence ..." << endl;
  for (uint ix=0; ix< precVect.size(); ix++) {
    node = precVect[ix];
    string prev_id  = getAttributeValue("id", node);
    dagNode ->addPrecId(prev_id);
  }
  cout << "\t" << "... done" << endl;

  cout << "\t" << "---> Set the profile to dagNode ..." << endl;
  dagNode->set_pb_desc(profile);
  cout << "\t" << "... done" << endl;


  cout << "\t" << "---> Add a node to the Dag " << endl;
  myDag->addNode(nodeId, dagNode);
  cout << "\t" << "... done" << endl;

  return true;
}

/**
 * parse an argument element
 * @param name     argument id
 * @param value    argument value
 * @param type     argument data type 
 * @param profile  current profile reference
 * @param lastArg  the argument index in the profile
 * @param dagNode  node reference in the Dag structure
 */
bool 
WfExtReader::checkArg(const string& name, 
		      const string& value, 
		      const string& type,
		      diet_profile_t * profile, 
		      unsigned int lastArg,
		      long int var_node,
		      long int var_port,
		      Node * dagNode) {
  setParam(ARG_PORT, name, type, profile, lastArg, 
	   var_node, var_port,
	   dagNode, &value);
  return true;
}

/**
 * parse an input port element
 * @param name     input port id
 * @param type     input port data type 
 * @param source   linked output port id
 * @param profile  current profile reference
 * @param lastArg  the input port index in the profile
 * @param dagNode  node reference in the Dag structure
 */
bool 
WfExtReader::checkIn(const string& name, 
		     const string& type, 
		     const string& source,
		     diet_profile_t * profile,
		     unsigned int lastArg,
		     long int var_node,
		     long int var_port,
		     Node * dagNode) {
  string real_source_name(source);
  setParam(IN_PORT, name, type, profile, lastArg, 
	   var_node, var_port,
	   dagNode);
  if (dagNode) {
    getRealName(real_source_name, var_node, var_port);
    dagNode->link_i2o(name, real_source_name);
  }
  return true;
}

/**
 * parse an input/output port element
 * @param name     inoutput port id
 * @param type     inoutput port data type 
 * @param source   linked output port id
 * @param profile  current profile reference
 * @param lastArg  the inoutput port index in the profile
 * @param dagNode  node reference in the Dag structure
 */  
bool
WfExtReader::checkInout(const string& name, 
			const string& type, 
			const string& source,
			diet_profile_t * profile,
			unsigned int lastArg,
			long int var_node,
			long int var_port,
			Node * dagNode) {
  setParam(INOUT_PORT, name, type, profile, lastArg, 
	   var_node, var_port, dagNode);
  return true;
}

/**
 * parse an output port element
 * @param name     output port id
 * @param type     output port data type 
 * @param sink     linked input port id
 * @param profile  current profile reference
 * @param lastArg  the output port index in the profile
 * @param dagNode  node reference in the Dag structure
 */  
bool
WfExtReader::checkOut(const string& name, 
		      const string& type,
		      const string& sink,
		      diet_profile_t * profile,
		      unsigned int lastArg,
		      long int var_node,
		      long int var_port,
		      Node * dagNode) {
  string real_sink_name(sink);
  setParam(OUT_PORT, name, type, profile, lastArg, 
	   var_node, var_port, dagNode);
  if (dagNode) {
    getRealName(real_sink_name, var_node, var_port);
    dagNode->link_o2i(name, real_sink_name);
  }
  return true;
}

/**
 * fill a profile with the appropriate parameter type and create the 
 * node ports
 */  
bool
WfExtReader::setParam(const wf_port_t param_type,
		      const string& name, 
		      const string& type,
		      diet_profile_t* profile,
		      unsigned int lastArg,
		      long int var_node,
		      long int var_port,
		      Node * dagNode,
		      const string * value) {
  bool result = true;
  cout << "\t" << "the requested type is " << type << endl;
  if (type == WfCst::DIET_CHAR) {
    if (dagNode) {
      if (value) 
	dagNode->newPort(name, lastArg, param_type, 
			 string(WfCst::DIET_CHAR),
			 *value);
      else
	dagNode->newPort(name, lastArg, param_type, 
			 string(WfCst::DIET_CHAR));
    }
  } // end if DIET_CHAR

  if (type == WfCst::DIET_SHORT) {
    if (dagNode) {
      if (value)
	dagNode->newPort(name, lastArg, param_type, 
			 string(WfCst::DIET_SHORT),
			 *value);
      else
	dagNode->newPort(name, lastArg, param_type, 
			 string(WfCst::DIET_SHORT));
    }
  } // end if DIET_SHORT
  if (type == WfCst::DIET_INT) {
    if (dagNode) {
      if (value)
	dagNode->newPort(name, lastArg, param_type, 
			 string(WfCst::DIET_INT),
			 *value);
      else
	dagNode->newPort(name, lastArg, param_type, 
			 string(WfCst::DIET_INT));
    }
  } // end if DIET_INT
  if (type == WfCst::DIET_LONGINT) {
    if (dagNode) {
      if (value)
	dagNode->newPort(name, lastArg, param_type, 
			 string(WfCst::DIET_LONGINT),
			 *value);
      else
	dagNode->newPort(name, lastArg, param_type, 
			 string(WfCst::DIET_LONGINT));
    }

  } // end if LONG_INT

  if (type == WfCst::DIET_FLOAT) {
    if (dagNode) {
      if (value) 
	dagNode->newPort(name, lastArg, param_type, 
			 string(WfCst::DIET_FLOAT),
			 *value);
      else
	dagNode->newPort(name, lastArg, param_type, 
			 string(WfCst::DIET_FLOAT));
    }
  } // end if DIET_CHAR


  if (type == WfCst::DIET_STRING) {
    if (dagNode) {
      if (value)
	dagNode->newPort(name, lastArg, param_type, 
			 string(WfCst::DIET_STRING),
			 *value);
      else
	dagNode->newPort(name, lastArg, param_type, 
			 string(WfCst::DIET_STRING));
    }	
  } // end if DIET_STRING
  return result;
}

/**
 * parse a matrix argument
 */
bool
WfExtReader::checkMatrixArg(const string& id, const DOMElement * element,
			    diet_profile_t * profile, unsigned int lastArg,
			    long int var_node,
			    long int var_port,
			    Node * dagNode) {
  string base_type("");
  string nb_rows("");
  string nb_cols("");
  string matrix_order("");

  if (!checkMatrixCommun(id, element,
			 base_type, nb_rows, nb_cols, matrix_order))
    return false;

  string value = getAttributeValue("value", element);
  if (value == "") {
    cerr << "Matrix argument without value" << endl;
    return false;
  }

  setMatrixParam(ARG_PORT, id, base_type, nb_rows, nb_cols, matrix_order,
		 profile, lastArg, dagNode, &value);
  return true;
}

/**
 * parse a matrix input port
 */
bool
WfExtReader::checkMatrixIn(const string& id, const DOMElement * element,
			   diet_profile_t * profile, unsigned int lastArg,
			   long int var_node,
			   long int var_port,
			   Node * dagNode) {
  string base_type("");
  string nb_rows("");
  string nb_cols("");
  string matrix_order("");

  if (!checkMatrixCommun(id, element, 
			 base_type, nb_rows, nb_cols, matrix_order))
    return false;

  setMatrixParam(IN_PORT, id, base_type, nb_rows, nb_cols, matrix_order,
		 profile, lastArg, dagNode);
  return true;
}

/**
 * parse a matrix inout port
 */
bool
WfExtReader::checkMatrixInout(const string& id, const DOMElement * element,
			      diet_profile_t * profile, unsigned int lastArg,
			      long int var_node,
			      long int var_port,
			      Node * dagNode) {
  string base_type("");
  string nb_rows("");
  string nb_cols("");
  string matrix_order("");

  if (!checkMatrixCommun(id, element, 
			 base_type, nb_rows, nb_cols, matrix_order))
    return false;

  setMatrixParam(INOUT_PORT, id, base_type, nb_rows, nb_cols, matrix_order,
		 profile, lastArg, dagNode);
  return true;
}

/**
 * parse a matrix output port
  */
bool
WfExtReader::checkMatrixOut(const string& id, const DOMElement * element,
			    diet_profile_t * profile, unsigned int lastArg,
			    long int var_node,
			    long int var_port,
			    Node * dagNode) {
  string base_type("");
  string nb_rows("");
  string nb_cols("");
  string matrix_order("");

  if (!checkMatrixCommun(id, element,
			 base_type, nb_rows, nb_cols, matrix_order))
    return false;
  setMatrixParam(OUT_PORT, id, base_type, nb_rows, nb_cols, matrix_order,
		 profile, lastArg, dagNode);
  return true;
}

/**
 * parse a matrix argument. 
 * Check only the commun attributes of In, Inout and Out ports
 */
bool
WfExtReader::checkMatrixCommun(const string& id, const DOMElement * element,
			       string& base_type, 
			       string& nb_rows, 
			       string& nb_cols, 
			       string& matrix_order) {
  // mandatory attributes
  base_type  = getAttributeValue("base_type", element);
  nb_rows  = getAttributeValue("nb_rows", element);
  nb_cols  = getAttributeValue("nb_cols", element);
  matrix_order  = getAttributeValue("matrix_order", element);

  if ((base_type == "") || (nb_rows == "")  ||
      (nb_cols == "") || (matrix_order == "")) {
    cout << "--- Matrix attribute malformed" << endl;
    return false;
  }
  return true;
}

/**
 * fill a profile with matrix parameter type
 * The data are NULL
 */  
bool 
WfExtReader::setMatrixParam(const wf_port_t param_type,
			    const string& name,
			    const string& base_type_str,
			    const string& nb_rows_str,
			    const string& nb_cols_str,
			    const string& matrix_order_str,
			    diet_profile_t * profile,
			    unsigned int lastArg,
			    Node * dagNode,
			    const string * value) {

  cout << "\tsetMatrixParam " << name << ", " <<
    base_type_str << endl;

  // get the base type of matrix element
  diet_base_type_t base_type = getBaseType(base_type_str);
  // get the number of rows
  size_t nb_rows = atoi(nb_rows_str.c_str());
  // get the column numbers
  size_t nb_cols = atoi(nb_cols_str.c_str());
  // get the matrix order  
  diet_matrix_order_t matrix_order = 
    getMatrixOrder(matrix_order_str);


  if (dagNode) {
    if (value) {
      Node::WfPort * port = (Node::WfPort *)
	(dagNode->newPort(name, lastArg, param_type, 
			  string(WfCst::DIET_MATRIX),
			  *value));
      port->setMatParams(nb_rows, nb_cols, matrix_order, base_type);
    }
    else {
      Node::WfPort * port = (Node::WfPort *)
	dagNode->newPort(name, lastArg, param_type, 
			 string(WfCst::DIET_MATRIX));
      port->setMatParams(nb_rows, nb_cols, matrix_order, base_type);
    } // end else
  }
  // set the parameter as a matrix
  /*
  diet_matrix_set(diet_parameter(profile,lastArg),
		  NULL, DIET_VOLATILE, 
		  base_type, nb_rows, nb_cols, matrix_order);
  */

  return true;
}

string
WfExtReader::getExpr(string& id) {
  string expr = "";
  string::size_type loc = id.find('{');
  if ((id[id.size()-1] == '}') &&
      (loc != string::npos)) {
    expr = id.substr(loc+1, id.size()-loc-2);
  }
  return expr;
}

/**
 * return the name of source/sink after expansion
 */
string
WfExtReader::getRealName(string& real_name, 
			 long int var_node,
			 long int var_port) {
  if (real_name == "")
    return real_name;
  cout << "getRealName (" + real_name + ") = ";
  string var_node_str = itoa(var_node);
  string var_port_str = itoa(var_port);
  string linked_nid = real_name;
  // linked port id
  string linked_pid = linked_nid.substr(linked_nid.find('#')+1);
  // linked_nid
  linked_nid = linked_nid.substr(0, linked_nid.find('#'));
  // check if the id contains an expression
  string expr1 = getExpr(linked_nid);
  // replace nv macro by its value
  string::size_type loc = expr1.find("nv");
  while (loc != string::npos) {
    expr1.replace(loc, 2, var_node_str);
    loc = expr1.find("nv");
  }
  // replace np macro by its value
  loc = expr1.find("pv");
  while (loc != string::npos) {
    expr1.replace(loc, 2, var_port_str);
    loc = expr1.find("pv");
  }
  if ((expr1 != "") && 
      ((var_node != -1)||(var_port != -1))) {
    long int value = WfCst::eval_expr(expr1, 0);
    real_name = linked_nid.substr(0, linked_nid.find('{')) +
	itoa(value) + "#";
  }
  else
    real_name = linked_nid + "#";
  
  // check if the port id contains an expression
  string expr2 = getExpr(linked_pid);
  // replace pv macro by its value
  loc = expr2.find("pv");
  while (loc != string::npos) {
    expr2.replace(loc, 2, var_port_str);
    loc = expr2.find("pv");
  }
  // replace nv macro by its value
  loc = expr2.find("nv");
  while (loc != string::npos) {
    expr2.replace(loc, 2, var_node_str);
    loc = expr2.find("nv");
  }
  if ((expr2 != "") && 
      ((var_port != -1)||(var_node != -1))) {
    long int value = WfCst::eval_expr(expr2, 0);
    real_name += linked_pid.substr(0, linked_pid.find('{')) + itoa(value);
  }
  else {
    real_name += linked_pid;
  }
  cout << real_name + "), with var_node = " << var_node << 
    " and var_port = " << var_port << endl;
  return real_name;
}
