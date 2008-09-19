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
 * Revision 1.8  2008/09/19 13:11:07  bisnard
 * - added support for containers split/merge in workflows
 * - added support for multiple port references
 * - profile for node execution initialized by port (instead of node)
 * - ports linking managed by ports (instead of dag)
 *
 * Revision 1.7  2008/09/08 09:12:58  bisnard
 * removed obsolete attribute nodes_list, pbs_list, alloc
 *
 * Revision 1.6  2008/06/19 10:17:41  bisnard
 * remove some debug mess
 *
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
  this->myDag = new Dag();
} // end constructor DagWfParser::DagWfParser(int, const char *)

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
      if (!parseNode(child, nodeId, nodePath)) {
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
			string nodeId, string nodePath) {
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
  unsigned lastArg = 0;

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
      } // end if (arg

      if (child_name == "in") {
	if (! parseIn(child_elt)) {
	  ERROR ("In element " << child_name << " malformed", false);
	}
	inVect.push_back(child_elt);
	inVectIndex.push_back(lastArg++);
      } // end if (in

      if (child_name == "inOut") {
	if (! parseInout(child_elt)) {
	  ERROR ("Inout element " << child_name << " malformed", false);
	}
	inoutVect.push_back(child_elt);
	inoutVectIndex.push_back(lastArg++);
      } // end if (inout)

      if (child_name == "out") {
	if (!parseOut(child_elt)) {
	  ERROR ("Out element " << child_name << " malformed", false);
	}
	outVect.push_back(child_elt);
	outVectIndex.push_back(lastArg++);
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

  // last_in last_inout last_out
  int last_in    = inVect.size() + argVect.size() - 1;
  int last_inout = last_in + inoutVect.size();
  int last_out   = last_inout + outVect.size();

  lastArg    = 0;
  TRACE_TEXT (TRACE_ALL_STEPS, "Node ports count:" << "last in " << last_in <<
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
  TRACE_TEXT (TRACE_ALL_STEPS,"Checking arg ports" << endl);
  for (uint ix=0; ix < argVect.size(); ix++) {
    node = argVect[ix];
    string portName  = nodeId + "#" + getAttributeValue("name", node);
    string portType  = getAttributeValue("type", node);
    string value     = getAttributeValue("value", node);
    if (portType != "DIET_MATRIX")
      // check the arg attribute and add its description to the profile
      checkArg (portName, value, portType, profile, argVectIndex[ix], dagNode);
    else
      // check the matrix argument
      checkMatrixArg(portName, node, profile, argVectIndex[ix], dagNode);
    lastArg++;
  }
  //************************************
  // Parse the input parameters once again
  TRACE_TEXT (TRACE_ALL_STEPS,"Checking in ports" << endl);
  for (uint ix=0; ix < inVect.size(); ix++) {
    node = inVect[ix];
    string portName  = nodeId + "#" + getAttributeValue("name", node);
    string portType  = getAttributeValue("type", node);
    string source  = getAttributeValue("source", node);

    if (portType != "DIET_MATRIX")
	// check in attribute and add its description to the profile
      checkIn (portName, portType, source, profile, inVectIndex[ix], dagNode);
    else
	// check a matrix input port
      checkMatrixIn(portName, source, node, profile, inVectIndex[ix], dagNode);
    lastArg++;
  }
  //************************************
  // Parse the Inout parameters once again
  // FIXME
  TRACE_TEXT (TRACE_ALL_STEPS,"Checking inout ports" << endl);
  for (uint ix=0; ix < inoutVect.size(); ix++) {
    node = inoutVect[ix];
    string portName  = nodeId + "#" + getAttributeValue("name", node);
    string portType  = getAttributeValue("type", node);
    string sink  = getAttributeValue("sink", node);

    if (portType != "DIET_MATRIX")
	// check the inout attribute and add its description to the profile
      checkInout(portName, portType, sink, profile, inoutVectIndex[ix], dagNode);
    else
	// check matrix inout port
      checkMatrixInout(portName, node, profile, inoutVectIndex[ix], dagNode);
    lastArg++;
  }
  //************************************
  // Parse the out parameters once again
  TRACE_TEXT (TRACE_ALL_STEPS,"Checking out ports" << endl);
  for (uint ix=0; ix < outVect.size(); ix++) {
    node = outVect[ix];
    string portName  = nodeId + "#" + getAttributeValue("name", node);
    string portType  = getAttributeValue("type", node);
    string sink  = getAttributeValue("sink", node);
    if (portType != "DIET_MATRIX")
	// check the out attribute and add its description to the profile
      checkOut (portName, portType, sink, profile, outVectIndex[ix], dagNode);
    else
	// check matrix output port
      checkMatrixOut(portName, node, profile, outVectIndex[ix], dagNode);
    lastArg++;
  }
  //************************************
  for (uint ix=0; ix< precVect.size(); ix++) {
    node = precVect[ix];
    string prev_id  = getAttributeValue("id", node);
    dagNode->addPrecId(prev_id);
  }

  dagNode->set_pb_desc(profile);
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
		      Node * dagNode) {
  setParam(ARG_PORT, name, type, profile, lastArg, dagNode, &value);
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
		     Node * dagNode) {
  setParam(IN_PORT, name, type, profile, lastArg, dagNode);
  if (dagNode) {
    dagNode->link_i2o(name, source);
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
			Node * dagNode) {
  setParam(INOUT_PORT, name, type, profile, lastArg, dagNode);
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
		      Node * dagNode) {
  setParam(OUT_PORT, name, type, profile, lastArg, dagNode);
  if (dagNode) {
    dagNode->link_o2i(name, sink);
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
		      Node * dagNode,
		      const string * value) {
  bool result = true;

  // Get the base type and the depth of the list structure
  uint typeDepth = 0;
  string baseType = type;
  string::size_type listPos = baseType.find("LIST");
  while (listPos != string::npos) {
    baseType = baseType.substr(listPos+5, baseType.length() - 6);
    listPos = baseType.find("LIST");
    ++typeDepth;
  }
  TRACE_TEXT( TRACE_ALL_STEPS,"Parameter type = " << baseType
             << " & depth = " << typeDepth << endl);

  // Initialize the profile with the appropriate parameter type
  // (and optionnally value: used only for Arg ports)
  string descType = typeDepth == 0 ? baseType : WfCst::DIET_CONTAINER;
  setParamDesc(param_type, name, descType, profile, lastArg, dagNode, value);

  if (baseType == WfCst::DIET_CHAR) {
    if (dagNode) {
      if (value)
	dagNode->newPort(name, lastArg, param_type,
			 string(WfCst::DIET_CHAR),typeDepth,
			 *value);
      else
	dagNode->newPort(name, lastArg, param_type,
			 string(WfCst::DIET_CHAR),typeDepth);
    }
  } // end if DIET_CHAR

  if (baseType == WfCst::DIET_SHORT) {
    if (dagNode) {
      if (value)
	dagNode->newPort(name, lastArg, param_type,
			 string(WfCst::DIET_SHORT),typeDepth,
			 *value);
      else
	dagNode->newPort(name, lastArg, param_type,
			 string(WfCst::DIET_SHORT),typeDepth);
    }
  } // end if DIET_SHORT

  if (baseType == WfCst::DIET_INT) {
    if (dagNode) {
      if (value)
	dagNode->newPort(name, lastArg, param_type,
			 string(WfCst::DIET_INT),typeDepth,
			 *value);
      else
	dagNode->newPort(name, lastArg, param_type,
			 string(WfCst::DIET_INT),typeDepth);
    }
  } // end if DIET_INT

  if (baseType == WfCst::DIET_LONGINT) {
    if (dagNode) {
      if (value)
	dagNode->newPort(name, lastArg, param_type,
			 string(WfCst::DIET_LONGINT),typeDepth,
			 *value);
      else
	dagNode->newPort(name, lastArg, param_type,
			 string(WfCst::DIET_LONGINT),typeDepth);
    }

  } // end if LONG_INT

  if (baseType == WfCst::DIET_FLOAT) {
    if (dagNode) {
      if (value)
	dagNode->newPort(name, lastArg, param_type,
			 string(WfCst::DIET_FLOAT),typeDepth,
			 *value);
      else
	dagNode->newPort(name, lastArg, param_type,
			 string(WfCst::DIET_FLOAT),typeDepth);
    }
  } // end if DIET_FLOAT



  if (baseType == WfCst::DIET_DOUBLE) {
    if (dagNode) {
      if (value)
	dagNode->newPort(name, lastArg, param_type,
			 string(WfCst::DIET_DOUBLE),typeDepth,
			 *value);
      else
	dagNode->newPort(name, lastArg, param_type,
			 string(WfCst::DIET_DOUBLE),typeDepth);
    }
  } // end if DIET_DOUBLE

  if (baseType == WfCst::DIET_STRING) {
    if (dagNode) {
      if (value)
	dagNode->newPort(name, lastArg, param_type,
			 string(WfCst::DIET_STRING),typeDepth,
			 *value);
      else
	dagNode->newPort(name, lastArg, param_type,
			 string(WfCst::DIET_STRING),typeDepth);
    }
  } // end if DIET_STRING

  if (baseType == WfCst::DIET_PARAMSTRING) {
    if (dagNode) {
      if (value)
	dagNode->newPort(name, lastArg, param_type,
			 string(WfCst::DIET_PARAMSTRING),typeDepth,
			 *value);
      else
	dagNode->newPort(name, lastArg, param_type,
			 string(WfCst::DIET_PARAMSTRING),typeDepth);
    }
  } // end if DIET_PARAMSTRING

  if (baseType == WfCst::DIET_FILE) {
    if (dagNode) {
      if (value)
	dagNode->newPort(name, lastArg, param_type,
			 string(WfCst::DIET_FILE),typeDepth,
			 *value);
      else
	dagNode->newPort(name, lastArg, param_type,
			 string(WfCst::DIET_FILE),typeDepth);
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

  if (dagNode) {
    dagNode->link_i2o(id, source);
  }
  return true;
} // end checkMatrixIn

/**
 * parse a matrix inout port
 */
bool
DagWfParser::checkMatrixInout(const string& id, const DOMElement * element,
			      diet_profile_t * profile, unsigned int lastArg,
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

  setMatrixParamDesc(param_type, name, base_type_str,
		       nb_rows_str, nb_cols_str,
		       matrix_order_str,
		       profile, lastArg,
		       dagNode,
		       value);

  if (dagNode) {
    if (value) {
      WfPort * port = (WfPort *)
	(dagNode->newPort(name, lastArg, param_type,
			  string(WfCst::DIET_MATRIX), 0,
			  *value));
      port->setMatParams(nb_rows, nb_cols, matrix_order, base_type);
    }
    else {
      WfPort * port = (WfPort *)
	dagNode->newPort(name, lastArg, param_type,
			 string(WfCst::DIET_MATRIX), 0);
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

/**
 * fill a profile with the appropriate parameter type
 */
bool
DagWfParser::setParamDesc(const wf_port_t param_type,
			  const string& name,
			  const string& type,
			  diet_profile_t * profile,
			  unsigned int lastArg,
			  Node * dagNode,
			  const string * value) {
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

  if (type == WfCst::DIET_CONTAINER) {
    if (value != NULL) {
      diet_container_set(diet_parameter(profile, lastArg),
		    DIET_PERSISTENT);
    } else
      diet_container_set(diet_parameter(profile, lastArg),
		    DIET_PERSISTENT);
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

