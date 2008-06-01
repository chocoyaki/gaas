/****************************************************************************/
/* DAG Workflow description Reader class implementation                     */
/* This class read a textual representation of DAG workflow and return the  */
/* corresponding DAG object                                                 */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/* - Benjamin ISNARD (benjamin.isnard@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.5  2008/06/01 14:06:56  rbolze
 * replace most ot the cout by adapted function from debug.cc
 * there are some left ...
 *
 * Revision 1.4  2008/05/28 20:53:33  rbolze
 * now DIET_PARAMSTRING type can be use in DAG.
 *
 * Revision 1.3  2008/05/20 12:41:17  bisnard
 * corrected bug of bad profile desc initialization for scalars
 *
 * Revision 1.2  2008/04/28 12:06:28  bisnard
 * changed constructor for Node (new param wfReqId)
 *
 * Revision 1.1  2008/04/21 14:35:50  bisnard
 * added NodeQueue and renamed WfParser as DagWfParser
 *
 ****************************************************************************/

#include <iostream>

#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/framework/Wrapper4InputSource.hpp>
#include <xercesc/util/XMLString.hpp>

#include "debug.hh"
#include "marshalling.hh"
#include "DagWfParser.hh"
#include "WfUtils.hh"


using namespace std;

/**
 * Constructor
 */
DagWfParser::DagWfParser(int wfReqId, const char * wf_desc) :
  wfReqId(wfReqId), content(wf_desc) {
  this->dagSize = 0;
  this->myDag = new Dag();
  this->alloc = false;
} // end constructor DagWfParser::DagWfParser(int, const char *)

DagWfParser::DagWfParser(int wfReqId, const char * wf_desc, bool alloc) :
  wfReqId(wfReqId), content(wf_desc) {
  this->dagSize = 0;
  this->myDag = new Dag();
  this->alloc = alloc;
} // end constructor DagWfParser::DagWfParser(int, const char *, bool)

DagWfParser::~DagWfParser() {
  // dag is kept alive after parser destruction
} // end destructor

/**
 * Get the attribute value of a DOM element
 */
string
DagWfParser::getAttributeValue(const char * attr_name,
			       const DOMElement * elt) {
  XMLCh * attr = XMLString::transcode(attr_name);
  const XMLCh * value   = elt->getAttribute(attr);
  char  * value_str = XMLString::transcode(value);

  string result(value_str);

  XMLString::release(&value_str);
  XMLString::release(&attr);

  return result;
} // end getAttributeValue

/**
 * Initialize the processing
 */
bool
DagWfParser::setup() {
  return initXml();
} // end setup

/**
 * Parse the XML
 */
bool
DagWfParser::parseXml() {
  const XMLCh gLS[] = { chLatin_L, chLatin_S, chNull };

  DOMImplementation *impl = DOMImplementationRegistry::getDOMImplementation(gLS);


  DOMBuilder  *parser =
    ((DOMImplementationLS*)impl)->createDOMBuilder(DOMImplementationLS::MODE_SYNCHRONOUS, 0);


  static const char * content_id = "workflow_description";

  MemBufInputSource* memBufIS = new MemBufInputSource
    (
     (const XMLByte*)(this->content.c_str())
     , content.size()
     , content_id
     , false
    );

  Wrapper4InputSource * wrapper = new Wrapper4InputSource(memBufIS);

  TRACE_TEXT (TRACE_ALL_STEPS, "Xml wrapper created" << endl);

  this->document = parser->parse(*wrapper);

  if (document == NULL) {
    WARNING ("FATL ERROR : wrong description ?");
    return false;
  }

  DOMNode * root = (DOMNode*)(document->getDocumentElement());

  char * _rootNodeName = XMLString::transcode(root->getNodeName());
  if (strcmp (_rootNodeName, "dag")) {
    ERROR ("The file is not a valid dag description ", false);
  }
  XMLString::release(&_rootNodeName);

  DOMNode * child = root->getFirstChild();
  while ((child != NULL)) {
    if (child->getNodeType() == DOMNode::ELEMENT_NODE) {
      DOMElement * child_elt = (DOMElement*)child;

      char * _nodeName = XMLString::transcode(child_elt->getNodeName());
      string nodeName(_nodeName);
      XMLString::release(&_nodeName);

      TRACE_TEXT (TRACE_ALL_STEPS,
		  "Parsing the element " << nodeName << endl );

      if (nodeName != "node") {
	WARNING ("Founding an element different from node" << nodeName);
	return false;
      }

      // Node path
      string nodePath(getAttributeValue("path", child_elt));
      // Node ID
      string nodeId(getAttributeValue("id", child_elt));

      if ((nodePath == "") ||
	  (nodeId   == "")) {
	WARNING ("FATAL ERROR : Node without Path or Id");
	return false;
      }

      if (nodeId[0] == '$') {
	nodeId.erase(0,1);
	// this is parametrable node
	string nodeInterval(getAttributeValue("interval", child_elt));
	long interval = atoi(nodeInterval.c_str());
	string base_id(nodeId);
	for (long lx=0; lx < interval; lx ++) {
	  nodeId = base_id + itoa(lx).c_str();
	  if (!parseNode(child, nodeId, nodePath, lx)) {
	    return false;
	  }
	} // end for lx
      } // end if == $
      else if (!parseNode(child, nodeId, nodePath)) {
	return false;
      }
    }
    child = child->getNextSibling();
  } // end while

  TRACE_TEXT (TRACE_MAIN_STEPS, "parsing xml content successful " <<endl);
  return true;
} // end parseXml


/**
 * Init the XML processing
 */
bool
DagWfParser::initXml() {
  bool result = parseXml();
  if (! result) return result;
  TRACE_TEXT (TRACE_MAIN_STEPS, "Checking the precedence ..." << endl);
  result = myDag->checkPrec();
  TRACE_TEXT (TRACE_MAIN_STEPS, "... checking the precedence done" << endl);
  return result;
} // end initXml

/**
 * get a reference to Dag structure
 */
Dag *
DagWfParser::getDag() {
  return this->myDag;
} // end getDag

/**
 * parse a node element
 */
bool
DagWfParser::parseNode (const DOMNode * element,
			string nodeId, string nodePath,
			long int var_node) {
  diet_profile_t * profile;
  corba_pb_desc_t * corba_profile = new corba_pb_desc_t;
  vector <DOMElement *> argVect;
  vector <unsigned int> argVectIndex;
  vector <DOMElement *> inVect;
  vector <unsigned int> inVectIndex;
  vector <DOMElement *> inoutVect;
  vector <unsigned int> inoutVectIndex;
  vector <DOMElement *> outVect;
  vector <unsigned int> outVectIndex;
  vector <DOMElement *> precVect;
  DOMElement * node;
  long int mul_arg =0, mul_in = 0, mul_inout = 0, mul_out = 0;
  unsigned lastArg = 0;

  this->dagSize++;

  DOMNode * child = element->getFirstChild();
  while (child != NULL) {
    if (child->getNodeType() == DOMNode::ELEMENT_NODE) {
      DOMElement * child_elt = (DOMElement*)child;

      char *child_name_str = XMLString::transcode(child_elt->getNodeName());
      string child_name(child_name_str);
      XMLString::release(&child_name_str);

      if (child_name == "arg") {
	if (! parseArg (child_elt)) {
	  ERROR ("Argument "<< child_name <<" malformed", false);
	}

	argVect.push_back(child_elt);
	argVectIndex.push_back(lastArg++);

	string mul_attr = getAttributeValue("mul", child_elt);
	if (mul_attr != "") {
	  mul_arg += atoi(mul_attr.c_str());
	}
      } // end if (arg

      if (child_name == "in") {
	if (! parseIn(child_elt)) {
	  ERROR ("In element " << child_name << " malformed", false);
	}
	inVect.push_back(child_elt);
	inVectIndex.push_back(lastArg++);

	string mul_attr = getAttributeValue("mul", child_elt);
	if (mul_attr != "") {
	  mul_in += atoi(mul_attr.c_str());
	}
      } // end if (in

      if (child_name == "inOut") {
	if (! parseInout(child_elt)) {
	  ERROR ("Inout element " << child_name << " malformed", false);
	}
	inoutVect.push_back(child_elt);
	inoutVectIndex.push_back(lastArg++);

	string mul_attr = getAttributeValue("mul", child_elt);
	if (mul_attr != "") {
	  mul_inout += atoi(mul_attr.c_str());
	}
      } // end if (inout)

      if (child_name == "out") {
	if (!parseOut(child_elt)) {
	  ERROR ("Out element " << child_name << " malformed", false);
	}
	outVect.push_back(child_elt);
	outVectIndex.push_back(lastArg++);

	string mul_attr = getAttributeValue("mul", child_elt);
	if (mul_attr != "") {
	  mul_out += atoi(mul_attr.c_str());
	}
      } // end if (out

      if (child_name == "prec") {
	string prevNodeId = getAttributeValue("id", child_elt);
	if (prevNodeId == "") {
	  ERROR ("Precedence element malformed", false);
	}

	if (child_elt->getFirstChild() != NULL) {
	  ERROR ("Precedence element doesn't accept a child element"
		 << endl << "May be a </prec> is forgotten", false);
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

  lastArg    = 0;
  TRACE_TEXT (TRACE_ALL_STEPS, "\t" << "last in " << last_in <<
	      ", last_inout " << last_inout <<
	      ", last_out " << last_out << endl);

  // Now that we now the node path and the service argument number we can
  // create the node object and its profile
  Node * dagNode = new Node(this->wfReqId, nodeId, nodePath,
			    last_in, last_inout, last_out);

  profile = diet_profile_alloc((char*)nodePath.c_str(),
				last_in,
				last_inout,
				last_out);

  //************************************
  //** Parse the arguments once again
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
	  checkArg (name, value, type, profile, argVectIndex[ix],
		    var_node, var_port, dagNode);
	else
	  // check the matrix argument
	  checkMatrixArg(name, node, profile, argVectIndex[ix],
			 var_node, var_port, dagNode);
	lastArg++;
      } // end for
    }
    else {
      // single port
      name = name + getAttributeValue("name", node);
      if (type != "DIET_MATRIX")
	// check the arg attribute and add its description to the profile
	checkArg (name, value, type, profile, argVectIndex[ix],
		  var_node, -1,
		  dagNode);
      else
	// check the matrix argument
	checkMatrixArg(name, node, profile, argVectIndex[ix],
		       var_node, -1,
		       dagNode);
      lastArg++;
    }
  }
  //************************************
  // Parse the input parameters once again
  for (uint ix=0; ix < inVect.size(); ix++) {
    node = inVect[ix];

    string name("");
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
	  checkIn (name, type, source, profile, inVectIndex[ix],
		   var_node, var_port,
		   dagNode);
	else
	  // check a matrix input port
	  checkMatrixIn(name, source, node, profile, inVectIndex[ix],
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
	checkIn (name, type, source, profile, inVectIndex[ix],
		 var_node, -1,
		 dagNode);
      else
	// check a matrix input port
	checkMatrixIn(name, source, node, profile, inVectIndex[ix],
		      var_node, -1,
		      dagNode);
      lastArg++;
    }
  }
  //************************************
  // Parse the Inout parameters once again
  // FIXME
  for (uint ix=0; ix < inoutVect.size(); ix++) {
    node = inoutVect[ix];

    string name("");
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
	  checkInout(name, type, sink, profile, inoutVectIndex[ix],
		     var_node, var_port, dagNode);
	else
	  // check matrix inout port
	  checkMatrixInout(name, node, profile, inoutVectIndex[ix],
			   var_node, var_port, dagNode);

	lastArg++;
      }
    }
    else {
      // single port
      name = nodeId + "#" + getAttributeValue("name", node);
      if (type != "DIET_MATRIX")
	// check the inout attribute and add its description to the profile
	checkInout(name, type, sink, profile, inoutVectIndex[ix],
		   var_node, -1,
		   dagNode);
      else
	// check matrix inout port
	checkMatrixInout(name, node, profile, inoutVectIndex[ix],
			 var_node, -1,
			 dagNode);

      lastArg++;
    }
  }
  //************************************
  // Parse the out parameters once again
  for (uint ix=0; ix < outVect.size(); ix++) {
    node = outVect[ix];

    string name("");
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
	  checkOut (name, type, sink, profile, outVectIndex[ix],
		    var_node, var_port,
		    dagNode);
	else
	  // check matrix output port
	  checkMatrixOut(name, node, profile, outVectIndex[ix],
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
	checkOut (name, type, sink, profile, outVectIndex[ix],
		  var_node, -1,
		  dagNode);
      else
	// check matrix output port
	checkMatrixOut(name, node, profile, outVectIndex[ix],
		       var_node, -1,
		       dagNode);
      lastArg++;
    }
  }
  //************************************
  for (uint ix=0; ix< precVect.size(); ix++) {
    node = precVect[ix];
    string prev_id  = getAttributeValue("id", node);
    dagNode ->addPrecId(prev_id);
  }

  dagNode->set_pb_desc(profile);

  if (! this->alloc) {
    mrsh_pb_desc(corba_profile, profile);
    nodes_list[nodeId] = (*corba_profile);
    this->dagSize = nodes_list.size();

    // add the corba profile to problem list if it is a new one
    //if (! pbAlreadyRegistred(*corba_profile) ) {
      pbs_list.push_back(*corba_profile);
      //}
  } // end if (! alloc)

  myDag->addNode(nodeId, dagNode);
  dagNode->setDag(myDag);

  return true;
} // end parseNode

/**
 * Parse an argument element
 */
bool
DagWfParser::parseArg(DOMElement * child_elt) {
  string argName = getAttributeValue("name", child_elt);
  string argValue = getAttributeValue("value", child_elt);
  if ((argName == "") || (argValue == "")) {
    ERROR ("Argument "<< argName <<" malformed", false);
  }
  if (child_elt->getFirstChild() != NULL) {
    ERROR ("Argument element doesn't accept a child element" << endl <<
      "May be a </arg> is forgotten", false);
  }
  return true;
} // end parseArg

/**
 * Parse an input port element
 */
bool
DagWfParser::parseIn(DOMElement * child_elt) {
  if (child_elt->getFirstChild() != NULL) {
    ERROR ("input port element doesn't accept a child element" << endl <<
      "May be a </in> is forgotten", false);
  }
  return true;
} // end parseIn

/**
 * parse InOut port element
 */
bool
DagWfParser::parseInout(DOMElement * child_elt) {
  if (child_elt->getFirstChild()!=NULL) {
    ERROR ("inOut port element doesn't accept a child element" << endl <<
      "May be a </inOut> is forgotten", false);
  }
  return true;
} // end parseInout

/**
 * Parse Out port element
 */
bool
DagWfParser::parseOut(DOMElement * child_elt) {
  if (child_elt->getFirstChild() != NULL) {
    ERROR ("output port element doesn't accept a child element" << endl <<
	   "May be a </out> is forgotten", false);
  }
  return true;
} // end parseOut

/**
 * parse an argument element
 */
bool
DagWfParser::checkArg(const string& name,
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
} // end checkArg

/**
 * parse an input port element
 */
bool
DagWfParser::checkIn(const string& name,
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
} // end checkIn

/**
 * parse an input/output port element
 */
bool
DagWfParser::checkInout(const string& name,
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
} // end checkInout

/**
 * parse an output port element
 */
bool
DagWfParser::checkOut(const string& name,
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
} // end checkOut

/**
 * fill a profile with the appropriate parameter type and create the
 * node ports
 */
bool
DagWfParser::setParam(const wf_port_t param_type,
		      const string& name,
		      const string& type,
		      diet_profile_t* profile,
		      unsigned int lastArg,
		      long int var_node,
		      long int var_port,
		      Node * dagNode,
		      const string * value) {
  bool result = true;

  if (! this->alloc) {
    setParamDesc(param_type, name, type, profile, lastArg, -1, dagNode, value);
  }

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
  } // end if DIET_FLOAT



  if (type == WfCst::DIET_DOUBLE) {
    if (dagNode) {
      if (value)
	dagNode->newPort(name, lastArg, param_type,
			 string(WfCst::DIET_DOUBLE),
			 *value);
      else
	dagNode->newPort(name, lastArg, param_type,
			 string(WfCst::DIET_DOUBLE));
    }
  } // end if DIET_DOUBLE

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

  if (type == WfCst::DIET_PARAMSTRING) {
    if (dagNode) {
      if (value)
	dagNode->newPort(name, lastArg, param_type,
			 string(WfCst::DIET_PARAMSTRING),
			 *value);
      else
	dagNode->newPort(name, lastArg, param_type,
			 string(WfCst::DIET_PARAMSTRING));
    }
  } // end if DIET_PARAMSTRING

  if (type == WfCst::DIET_FILE) {
    if (dagNode) {
      if (value)
	dagNode->newPort(name, lastArg, param_type,
			 string(WfCst::DIET_FILE),
			 *value);
      else
	dagNode->newPort(name, lastArg, param_type,
			 string(WfCst::DIET_FILE));
    }
  } // end if DIET_FILE

  return result;
} // end setParam

/**
 * parse a matrix argument
 */
bool
DagWfParser::checkMatrixArg(const string& id, const DOMElement * element,
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
    ERROR ("Matrix argument without value", true);
  }

  setMatrixParam(ARG_PORT, id, base_type, nb_rows, nb_cols, matrix_order,
		 profile, lastArg, dagNode, &value);
  return true;
} // end checkMatrixArg

/**
 * parse a matrix input port
 */
bool
DagWfParser::checkMatrixIn(const string& id, const string& source,
			   const DOMElement * element,
			   diet_profile_t * profile, unsigned int lastArg,
			   long int var_node,
			   long int var_port,
			   Node * dagNode) {
  string real_source_name(source);
  string base_type("");
  string nb_rows("");
  string nb_cols("");
  string matrix_order("");

  if (!checkMatrixCommun(id, element,
			 base_type, nb_rows, nb_cols, matrix_order))
    return false;

  setMatrixParam(IN_PORT, id, base_type, nb_rows, nb_cols, matrix_order,
		 profile, lastArg, dagNode);

  if (dagNode) {
    getRealName(real_source_name, var_node, var_port);
    dagNode->link_i2o(id, real_source_name);
  }

  return true;
} // end checkMatrixIn

/**
 * parse a matrix inout port
 */
bool
DagWfParser::checkMatrixInout(const string& id, const DOMElement * element,
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
} // end checkMatrixInout

/**
 * parse a matrix output port
  */
bool
DagWfParser::checkMatrixOut(const string& id, const DOMElement * element,
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
} // end checkMatrixOut

/**
 * parse a matrix argument.
 * Check only the commun attributes of In, Inout and Out ports
 */
bool
DagWfParser::checkMatrixCommun(const string& id, const DOMElement * element,
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
    ERROR ("Matrix attribute malformed", false);
  }
  return true;
} // end checkMatrixCommun

/**
 * fill a profile with matrix parameter type
 * The data are NULL
 */
bool
DagWfParser::setMatrixParam(const wf_port_t param_type,
			    const string& name,
			    const string& base_type_str,
			    const string& nb_rows_str,
			    const string& nb_cols_str,
			    const string& matrix_order_str,
			    diet_profile_t * profile,
			    unsigned int lastArg,
			    Node * dagNode,
			    const string * value) {
  // get the base type of matrix element
  diet_base_type_t base_type = getBaseType(base_type_str);
  // get the number of rows
  size_t nb_rows = atoi(nb_rows_str.c_str());
  // get the column numbers
  size_t nb_cols = atoi(nb_cols_str.c_str());
  // get the matrix order
  diet_matrix_order_t matrix_order =
    getMatrixOrder(matrix_order_str);

  if (! this->alloc) {
    setMatrixParamDesc(param_type, name, base_type_str,
		       nb_rows_str, nb_cols_str,
		       matrix_order_str,
		       profile, lastArg,
		       dagNode,
		       value);
  }


  if (dagNode) {
    if (value) {
      WfPort * port = (WfPort *)
	(dagNode->newPort(name, lastArg, param_type,
			  string(WfCst::DIET_MATRIX),
			  *value));
      port->setMatParams(nb_rows, nb_cols, matrix_order, base_type);
    }
    else {
      WfPort * port = (WfPort *)
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
  }
  */

  return true;
} // end setMatrixParam

string
DagWfParser::getExpr(string& id) {
  string expr = "";
  string::size_type loc = id.find('{');
  if ((id[id.size()-1] == '}') &&
      (loc != string::npos)) {
    expr = id.substr(loc+1, id.size()-loc-2);
  }
  return expr;
} // end getExpr

/**
 * return the name of source/sink after expansion
 */
string
DagWfParser::getRealName(string& real_name,
			 long int var_node,
			 long int var_port) {
  if (real_name == "")
    return real_name;
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
  return real_name;
} // end getRealName

/**
 * fill a profile with the appropriate parameter type
 */
bool
DagWfParser::setParamDesc(const wf_port_t param_type,
			  const string& name,
			  const string& type,
			  diet_profile_t * profile,
			  unsigned int lastArg,
			  long int var,
			  Node * dagNode,
			  const string * value) {
  TRACE_TEXT (TRACE_ALL_STEPS, " type : "<< type << endl);
  if (type == WfCst::DIET_CHAR) {
    if (value != NULL)
      diet_scalar_set(diet_parameter(profile, lastArg),
		    dagNode->newChar(*value),
		    DIET_VOLATILE,
		    DIET_CHAR);
    else
      diet_scalar_set(diet_parameter(profile, lastArg),
		    NULL,
		    DIET_VOLATILE,
		    DIET_CHAR);
  }
  if (type == WfCst::DIET_SHORT) {
    if (value != NULL)
      diet_scalar_set(diet_parameter(profile, lastArg),
		    dagNode->newShort(*value),
		    DIET_VOLATILE,
		    DIET_SHORT);
    else
      diet_scalar_set(diet_parameter(profile, lastArg),
		    NULL,
		    DIET_VOLATILE,
		    DIET_SHORT);
  }
  if (type == WfCst::DIET_INT) {
    if (value != NULL)
      diet_scalar_set(diet_parameter(profile, lastArg),
		    dagNode->newInt(*value),
		    DIET_VOLATILE,
		    DIET_INT);
    else
      diet_scalar_set(diet_parameter(profile, lastArg),
		    NULL,
		    DIET_VOLATILE,
		    DIET_INT);
  }
  if (type == WfCst::DIET_LONGINT) {
    if (value != NULL)
      diet_scalar_set(diet_parameter(profile, lastArg),
		      dagNode->newLong(*value),
		      DIET_VOLATILE,
		      DIET_LONGINT);
    else
      diet_scalar_set(diet_parameter(profile, lastArg),
		      NULL,
		      DIET_VOLATILE,
		      DIET_LONGINT);
  }
  if (type == WfCst::DIET_STRING) {
    if (value != NULL)
      diet_string_set(diet_parameter(profile, lastArg),
		    dagNode->newString(*value),
		    DIET_VOLATILE);
    else
      diet_string_set(diet_parameter(profile, lastArg),
		    new char[1024],
		    DIET_VOLATILE);
  }
  if (type == WfCst::DIET_PARAMSTRING) {
    if (value != NULL)
      diet_paramstring_set(diet_parameter(profile, lastArg),
		    dagNode->newString(*value),
		    DIET_VOLATILE);
    else
      diet_paramstring_set(diet_parameter(profile, lastArg),
		    new char[1024],
		    DIET_VOLATILE);
  }
  if (type == WfCst::DIET_FILE) {
    if (value != NULL)
      diet_file_set(diet_parameter(profile, lastArg),
		  DIET_VOLATILE,
		  dagNode->newFile(*value));
    else
      diet_file_set(diet_parameter(profile, lastArg),
		  DIET_VOLATILE,
		  NULL);
  }

  if (type == WfCst::DIET_FLOAT) {
    if (value != NULL)
      diet_scalar_set(diet_parameter(profile, lastArg),
		    dagNode->newFloat(*value),
		    DIET_VOLATILE,
		    DIET_FLOAT);
    else
      diet_scalar_set(diet_parameter(profile, lastArg),
		    NULL,
		    DIET_VOLATILE,
		    DIET_FLOAT);
  }

  if (type == WfCst::DIET_DOUBLE) {
    if (value != NULL)
      diet_scalar_set(diet_parameter(profile, lastArg),
		    dagNode->newDouble(*value),
		    DIET_VOLATILE,
		    DIET_DOUBLE);
    else
      diet_scalar_set(diet_parameter(profile, lastArg),
		    NULL,
		    DIET_VOLATILE,
		    DIET_DOUBLE);
  } 
  return true;
} // end setParamDesc


/**
 * fill a profile with matrix parameter type
 * The data are NULL
 */
bool
DagWfParser::setMatrixParamDesc(const wf_port_t param_type,
				const string& name,
				const string& base_type_str,
				const string& nb_rows_str,
				const string& nb_cols_str,
				const string& matrix_order_str,
				diet_profile_t * profile,
				unsigned int lastArg,
				Node * dagNode,
				const string * value) {
  // get the base type of matrix element
  diet_base_type_t base_type = getBaseType(base_type_str);
  // get the number of rows
  size_t nb_rows = atoi(nb_rows_str.c_str());
  // get the column numbers
  size_t nb_cols = atoi(nb_cols_str.c_str());
  // get the matrix order
  diet_matrix_order_t matrix_order =
    getMatrixOrder(matrix_order_str);

  // set the parameter as a matrix
  diet_matrix_set(diet_parameter(profile,lastArg),
		  NULL, DIET_VOLATILE,
		  base_type, nb_rows, nb_cols, matrix_order);


  return true;
} // end setMatrixParamDesc

/**
 * Get the dag size (number of nodes)
 */
unsigned int
DagWfParser::getDagSize() {
  //  return dagSize;
  return nodes_list.size();
} // end getDagSize


/**
 * return the index of the provided problem in pbs_list vector
 */
unsigned int
DagWfParser::indexOfPb(corba_pb_desc_t& pb) {
  unsigned int index = pbs_list.size() + 1;
  unsigned int len = pbs_list.size();
  for (unsigned int ix=0; ix < len; ix++) {
    if (pbs_list[ix] == pb) {
      index = ix;
      return index;
    }

  }
  return index;
} // end indexOfPb


/**
 * check if the profile is already in the problems list
 */
bool
DagWfParser::pbAlreadyRegistred(corba_pb_desc_t& pb_desc) {
  for (unsigned int ix=0; ix<pbs_list.size(); ix++) {
    if (pbs_list[ix] == pb_desc)
      return true;
  }
  return false;
} // end pbAlreadyRegistred

/*************************************/
bool operator != (corba_data_id_t& id1,
		  corba_data_id_t& id2) {
  if (strcmp(id1.idNumber, id2.idNumber) ||
      (id1.dataCopy != id2.dataCopy) ||
      (id1.state != id2.state)
      )
    return true;
  return false;
}

bool operator != (corba_data_desc_t& d1,
		  corba_data_desc_t& d2) {
  if ((d1.mode      != d2.mode) ||
      (d1.base_type != d2.base_type) ||
      //      (d1.specific  != d2.specific) ||
      (d1.id        != d2.id)
      )
    return true;
  return false;
}

bool operator != (SeqCorbaDataDesc_t& s1,
		  SeqCorbaDataDesc_t& s2) {
  if (s1.length() != s2.length())
    return true;
  for (unsigned int ix=0; ix<s1.length(); ix++) {
    if (s1[ix] != s2[ix])
      return true;
  }
  return false;
}

bool operator == (corba_pb_desc_t& a,   corba_pb_desc_t& b) {
  if (strcmp(a.path, b.path) ||
      (a.last_in    != b.last_in) ||
      (a.last_inout != b.last_inout) ||
      (a.last_out   != b.last_out) ||
      (a.param_desc != b.param_desc)
      )
    return false;
  return true;
}

