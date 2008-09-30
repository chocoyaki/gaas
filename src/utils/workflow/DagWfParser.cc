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
 * Revision 1.10  2008/09/30 15:29:22  bisnard
 * code refactoring after profile mgmt change
 *
 * Revision 1.9  2008/09/30 09:23:29  bisnard
 * removed diet profile initialization from DagWfParser and replaced by node methods initProfileSubmit and initProfileExec
 *
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
}

DagWfParser::~DagWfParser() {
  // dag is kept alive after parser destruction
}

/**
 * get a reference to Dag structure
 */
Dag *
DagWfParser::getDag() {
  return this->myDag;
}

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
}

/**
 * Initialize the processing
 */
bool
DagWfParser::setup() {
  return initXml();
}

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
}

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
}

/**
 * parse a node element
 */
bool
DagWfParser::parseNode (const DOMNode * element,
			string nodeId, string nodePath) {
  Node * dagNode  = new Node(this->wfReqId, nodeId, nodePath);
  DOMNode * child = element->getFirstChild();
  unsigned lastArg = 0;
  while (child != NULL) {
    if (child->getNodeType() == DOMNode::ELEMENT_NODE) {
      DOMElement * child_elt = (DOMElement*) child;
      char *child_name_str = XMLString::transcode(child_elt->getNodeName());
      string child_name(child_name_str);
      XMLString::release(&child_name_str);

      if (child_name == "arg") {
	if (! parseArg (child_elt, lastArg++, *dagNode)) {
	  ERROR ("Argument "<< child_name <<" malformed", false);
	}
      }
      if (child_name == "in") {
	if (! parseIn(child_elt, lastArg++, *dagNode)) {
	  ERROR ("In element " << child_name << " malformed", false);
	}
      }
      if (child_name == "inOut") {
	if (! parseInout(child_elt, lastArg++, *dagNode)) {
	  ERROR ("Inout element " << child_name << " malformed", false);
	}
      }
      if (child_name == "out") {
	if (!parseOut(child_elt, lastArg++, *dagNode)) {
	  ERROR ("Out element " << child_name << " malformed", false);
	}
      }
      if (child_name == "prec") {
	string precNodeId = getAttributeValue("id", child_elt);
	if (precNodeId == "") {
	  ERROR ("Precedence element malformed", false);
	}
	if (child_elt->getFirstChild() != NULL) {
	  ERROR ("Precedence element doesn't accept a child element"
		 << endl << "May be a </prec> is forgotten", false);
	}
        dagNode->addPrevId(precNodeId);
      }
    }
    child = child->getNextSibling();
  } // end while

  myDag->addNode(nodeId, dagNode);
  dagNode->setDag(myDag);
  return true;
} // end parseNode

/**
 * Parse an argument element
 */
bool
DagWfParser::parseArg(DOMElement * child_elt, unsigned int lastArg,
                      Node& dagNode) {
  string name  = getAttributeValue("name", child_elt);
  string value = getAttributeValue("value", child_elt);
  string type  = getAttributeValue("type", child_elt);
  if ((name == "") || (value == "")) {
    ERROR ("Argument "<< name <<" malformed", false);
  }
  if (child_elt->getFirstChild() != NULL) {
    ERROR ("Argument element doesn't accept a child element" << endl <<
      "May be a </arg> is forgotten", false);
  }
  if (type != "DIET_MATRIX") {
    setParam(ARG_PORT, name, type, lastArg, dagNode, &value);
  } else {
    if (!setMatrixParam(child_elt, ARG_PORT, name, lastArg, dagNode, &value))
      return false;
  }
  return true;
} // end parseArg

/**
 * Parse an input port element
 */
bool
DagWfParser::parseIn(DOMElement * child_elt, unsigned int lastArg,
                     Node& dagNode) {
  string name    = getAttributeValue("name", child_elt);
  string type    = getAttributeValue("type", child_elt);
  string source  = getAttributeValue("source", child_elt);
  if (child_elt->getFirstChild() != NULL) {
    ERROR ("input port element doesn't accept a child element" << endl <<
      "May be a </in> is forgotten", false);
  }
  if (type != "DIET_MATRIX") {
    setParam(IN_PORT, name, type, lastArg, dagNode);
  } else {
    if (!setMatrixParam(child_elt, IN_PORT, name, lastArg, dagNode))
      return false;
  }
  if (!source.empty())
    dagNode.link_i2o(name, source);
  return true;
} // end parseIn

/**
 * parse InOut port element
 */
bool
DagWfParser::parseInout(DOMElement * child_elt, unsigned int lastArg,
                        Node& dagNode) {
  string name    = getAttributeValue("name", child_elt);
  string type    = getAttributeValue("type", child_elt);
  string source  = getAttributeValue("source", child_elt);
  if (child_elt->getFirstChild()!=NULL) {
    ERROR ("inOut port element doesn't accept a child element" << endl <<
      "May be a </inOut> is forgotten", false);
  }
  if (type != "DIET_MATRIX") {
    setParam(INOUT_PORT, name, type, lastArg, dagNode);
  } else {
    if (!setMatrixParam(child_elt, INOUT_PORT, name, lastArg, dagNode))
      return false;
  }
  if (!source.empty())
    dagNode.link_i2o(name, source);
  return true;
} // end parseInout

/**
 * Parse Out port element
 */
bool
DagWfParser::parseOut(DOMElement * child_elt, unsigned int lastArg,
                      Node& dagNode) {
  string name  = getAttributeValue("name", child_elt);
  string type  = getAttributeValue("type", child_elt);
  string sink  = getAttributeValue("sink", child_elt);
  if (child_elt->getFirstChild() != NULL) {
    ERROR ("output port element doesn't accept a child element" << endl <<
	   "May be a </out> is forgotten", false);
  }
  if (type != "DIET_MATRIX") {
    setParam(OUT_PORT, name, type, lastArg, dagNode);
  } else {
    if (!setMatrixParam(child_elt, OUT_PORT, name, lastArg, dagNode))
      return false;
  }
  if (!sink.empty())
    dagNode.link_o2i(name, sink);
  return true;
} // end parseOut

/**
 * Create a port
 */
void
DagWfParser::setParam(const wf_port_t param_type,
		      const string& name,
		      const string& type,
		      unsigned int lastArg,
		      Node&  dagNode,
		      const string * value) {
  // Get the base type and the depth of the list structure
  uint typeDepth = 0;
  string curType = type;
  string::size_type listPos = curType.find("LIST");
  while (listPos != string::npos) {
    curType = curType.substr(listPos+5, curType.length() - 6);
    listPos = curType.find("LIST");
    ++typeDepth;
  }
  short baseType = WfCst::cvtStrToWfType(curType);
  TRACE_TEXT( TRACE_ALL_STEPS,"Parameter base type = " << curType
             << " & depth = " << typeDepth << endl);

  // Initialize the profile with the appropriate parameter type
  // (and optionnally value: used only for Arg ports)
  short descType = (typeDepth == 0) ? baseType : WfCst::TYPE_CONTAINER;

  if (value)
    dagNode.newPort(name, lastArg, param_type, (WfCst::WfDataType) baseType, typeDepth, *value);
  else
    dagNode.newPort(name, lastArg, param_type, (WfCst::WfDataType) baseType, typeDepth);
} // end setParam

/**
 * create a port with matrix parameter type
 */
bool
DagWfParser::setMatrixParam(const DOMElement * element,
                            const wf_port_t param_type,
			    const string& name,
			    unsigned int lastArg,
			    Node& dagNode,
			    const string * value) {
  string base_type_str = getAttributeValue("base_type", element);
  string nb_rows_str = getAttributeValue("nb_rows", element);
  string nb_cols_str = getAttributeValue("nb_cols", element);
  string matrix_order_str = getAttributeValue("matrix_order", element);

  if ((base_type_str == "") || (nb_rows_str == "")  ||
      (nb_cols_str == "") || (matrix_order_str == "")) {
    ERROR ("Matrix attribute malformed", false);
  }

  // get the base type of matrix element
  diet_base_type_t base_type = getBaseType(base_type_str);
  // get the number of rows
  size_t nb_rows = atoi(nb_rows_str.c_str());
  // get the column numbers
  size_t nb_cols = atoi(nb_cols_str.c_str());
  // get the matrix order
  diet_matrix_order_t matrix_order =
    getMatrixOrder(matrix_order_str);

  if (value) {
    WfPort * port = (WfPort *)
        dagNode.newPort(name, lastArg, param_type,
                        WfCst::TYPE_MATRIX, 0, *value);
    port->setMatParams(nb_rows, nb_cols, matrix_order, base_type);
  }
  else {
    WfPort * port = (WfPort *)
        dagNode.newPort(name, lastArg, param_type,
                         WfCst::TYPE_MATRIX, 0);
    port->setMatParams(nb_rows, nb_cols, matrix_order, base_type);
  } // end else

  return true;
} // end setMatrixParam

