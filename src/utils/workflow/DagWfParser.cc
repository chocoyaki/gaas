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
 * Revision 1.16  2008/11/08 19:12:40  bdepardo
 * A few warnings removal
 *
 * Revision 1.15  2008/11/06 08:27:34  bdepardo
 * Added a warning when the 'source' attribute is missing in an input parameter
 *
 * Revision 1.14  2008/10/22 09:29:00  bisnard
 * replaced uint by standard type
 *
 * Revision 1.13  2008/10/20 08:02:57  bisnard
 * new classes XML parser (Dagparser,FWfParser)
 *
 * Revision 1.12  2008/10/14 13:31:01  bisnard
 * new class structure for dags (DagNode,DagNodePort)
 *
 * Revision 1.11  2008/10/02 07:35:09  bisnard
 * new constants definitions (matrix order and port type)
 *
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
#include <string>

#include "debug.hh"
#include "marshalling.hh"
#include "DagWfParser.hh"
#include "WfUtils.hh"

using namespace std;

/**
 * Constructor
 */
DagWfParser::DagWfParser(const char * wf_desc) : content(wf_desc) {
}

DagWfParser::~DagWfParser() {
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
 * Parses the XML and check the structure
 */
bool
DagWfParser::parseAndCheck() throw(XMLParsingException) {
  bool result = parseXml();
  if (! result) return result;
  TRACE_TEXT (TRACE_MAIN_STEPS, "Checking the precedence ..." << endl);
  result = checkPrec();
  TRACE_TEXT (TRACE_MAIN_STEPS, "... checking the precedence done" << endl);
  return result;
}

/**
 * Parse the XML Document
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
    WARNING ("FATAL ERROR : wrong description ?");
    return false;
  }
  DOMNode * root = (DOMNode*)(document->getDocumentElement());

  // Parse the root element
  parseRoot(root);

  TRACE_TEXT (TRACE_MAIN_STEPS, "parsing xml content successful " <<endl);
  return true;
}

/**
 * parse a node element
 */
void
DagWfParser::parseNode (const DOMElement * element, const string& nodeEltName) {
  // parse the node start element and its attributes
  Node * newNode = this->createNode(element, nodeEltName);
  // parse the node sub-elements
  DOMNode * child = element->getFirstChild();
  unsigned lastArg = 0;
  while (child != NULL) {
    if (child->getNodeType() == DOMNode::ELEMENT_NODE) {
      DOMElement * child_elt = (DOMElement*) child;
      char *child_name_str = XMLString::transcode(child_elt->getNodeName());
      string child_name(child_name_str);
      XMLString::release(&child_name_str);

      if (child_name == "arg") {
	parseArg (child_elt, lastArg++, *newNode);
      } else
      if (child_name == "in") {
	parseIn(child_elt, lastArg++, *newNode);
      } else
      if (child_name == "inOut") {
	parseInout(child_elt, lastArg++, *newNode);
      } else
      if (child_name == "out") {
	parseOut(child_elt, lastArg++, *newNode);
      } else
        parseOtherNodeSubElt(child_elt, child_name, *newNode);
    }
    child = child->getNextSibling();
  } // end while
}

/**
 * Parse an argument element
 */
void
DagWfParser::parseArg(DOMElement * child_elt, unsigned int lastArg,
                      Node& node) {
  string name  = getAttributeValue("name", child_elt);
  string value = getAttributeValue("value", child_elt);
  string type  = getAttributeValue("type", child_elt);
  if ((name == "") || (value == "")) {
    throw XMLParsingException(XMLParsingException::eEMPTY_ATTR,
                              "Argument " + name + " malformed");
  }
  if (child_elt->getFirstChild() != NULL) {
    throw XMLParsingException(XMLParsingException::eBAD_STRUCT,
          "Argument element doesn't accept a child element. \
          May be a </arg> is forgotten");
  }
  WfPort *port;
  if (!WfCst::isMatrixType(type)) {
    port = setParam(WfPort::PORT_IN, name, type, lastArg, node, &value);
  } else {
    port = setMatrixParam(child_elt, WfPort::PORT_IN, name, lastArg, node, &value);
  }
}

/**
 * Parse an input port element
 */
void
DagWfParser::parseIn(DOMElement * child_elt, unsigned int lastArg,
                     Node& node) {
  string name    = getAttributeValue("name", child_elt);
  string type    = getAttributeValue("type", child_elt);
  string source  = getAttributeValue("source", child_elt);
  if (child_elt->getFirstChild() != NULL) {
    throw XMLParsingException(XMLParsingException::eBAD_STRUCT,
            "input port element doesn't accept a child element. \
            May be a </in> is forgotten");
  }
  WfPort *port;
  if (!WfCst::isMatrixType(type)) {
    port = setParam(WfPort::PORT_IN, name, type, lastArg, node);
  } else {
    port = setMatrixParam(child_elt, WfPort::PORT_IN, name, lastArg, node);
  }
  if (!source.empty())
    port->setConnectionRef(source);
  else {
    WARNING("No source attribute for input port " << node.getId());
  }
}

/**
 * parse InOut port element
 */
void
DagWfParser::parseInout(DOMElement * child_elt, unsigned int lastArg,
                        Node& node) {
  string name    = getAttributeValue("name", child_elt);
  string type    = getAttributeValue("type", child_elt);
  string source  = getAttributeValue("source", child_elt);
  if (child_elt->getFirstChild()!=NULL) {
    throw XMLParsingException(XMLParsingException::eBAD_STRUCT,
          "inOut port element doesn't accept a child element. \
           May be a </inOut> is forgotten");
  }
  WfPort *port;
  if (!WfCst::isMatrixType(type)) {
    port = setParam(WfPort::PORT_INOUT, name, type, lastArg, node);
  } else {
    port = setMatrixParam(child_elt, WfPort::PORT_INOUT, name, lastArg, node);
  }
  if (!source.empty())
    port->setConnectionRef(source);
}

/**
 * Parse Out port element
 */
void
DagWfParser::parseOut(DOMElement * child_elt, unsigned int lastArg,
                      Node& node) {
  string name  = getAttributeValue("name", child_elt);
  string type  = getAttributeValue("type", child_elt);
  string sink  = getAttributeValue("sink", child_elt);
  if (child_elt->getFirstChild() != NULL) {
    throw XMLParsingException(XMLParsingException::eBAD_STRUCT,
          "output port element doesn't accept a child element. \
	  May be a </out> is forgotten");
  }
  WfPort *port;
  if (!WfCst::isMatrixType(type)) {
    port = setParam(WfPort::PORT_OUT, name, type, lastArg, node);
  } else {
    port = setMatrixParam(child_elt, WfPort::PORT_OUT, name, lastArg, node);
  }
  if (!sink.empty())
    port->setConnectionRef(sink);
}

/**
 * Parse a prec element
 */
void
DagParser::parsePrec(const DOMElement * child_elt, Node& node) {
  string precNodeId = getAttributeValue("id", child_elt);
  if (precNodeId == "") {
    throw XMLParsingException(XMLParsingException::eEMPTY_ATTR,
            "Precedence element malformed");
  }
  if (child_elt->getFirstChild() != NULL) {
    throw XMLParsingException(XMLParsingException::eBAD_STRUCT,
          "Precedence element doesn't accept a child element. \
           May be a </prec> is forgotten");
  }
  node.addPrevId(precNodeId);
}

/**
 * Create a port
 */
WfPort *
DagWfParser::setParam(const WfPort::WfPortType param_type,
		      const string& name,
		      const string& type,
		      unsigned int lastArg,
		      Node&  node,
		      const string * value) {
  // Get the base type and the depth of the list structure
  unsigned int typeDepth = 0;
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
//   short descType = (typeDepth == 0) ? baseType : (short) WfCst::TYPE_CONTAINER;

  WfPort *port = node.newPort(name, lastArg, param_type, (WfCst::WfDataType) baseType, typeDepth);
  if (value) {
    DagNodePort * dagPort = dynamic_cast<DagNodePort*>(port);
    if (dagPort) {
      dagPort->setValue(*value);
    } else {
      throw XMLParsingException(XMLParsingException::eBAD_STRUCT,
              "Cannot assign value to a port in a non-dag node");
    }
  }
  return port;
}

/**
 * create a port with matrix parameter type
 */
WfPort *
DagWfParser::setMatrixParam(const DOMElement * element,
                            const WfPort::WfPortType param_type,
			    const string& name,
			    unsigned int lastArg,
			    Node& node,
			    const string * value) {
  string elt_type_str = getAttributeValue("base_type", element);
  string nb_rows_str = getAttributeValue("nb_rows", element);
  string nb_cols_str = getAttributeValue("nb_cols", element);
  string matrix_order_str = getAttributeValue("matrix_order", element);

  if ((elt_type_str == "") || (nb_rows_str == "")  ||
      (nb_cols_str == "") || (matrix_order_str == "")) {
    ERROR ("Matrix attribute malformed", false);
  }

  // get the base type of matrix element
  short elt_type = WfCst::cvtStrToWfType(elt_type_str);
  // get the number of rows
  size_t nb_rows = atoi(nb_rows_str.c_str());
  // get the column numbers
  size_t nb_cols = atoi(nb_cols_str.c_str());
  // get the matrix order
  short matrix_order = WfCst::cvtStrToWfMatrixOrder(matrix_order_str);
  // create port
  WfPort * port = node.newPort(name, lastArg, param_type,
                               WfCst::TYPE_MATRIX, 0);
  port->setMatParams(nb_rows, nb_cols,
                     (WfCst::WfMatrixOrder) matrix_order,
                     (WfCst::WfDataType) elt_type);
  if (value) {
    DagNodePort * dagPort = dynamic_cast<DagNodePort*>(port);
    if (dagPort) {
      dagPort->setValue(*value);
    } else {
      ERROR("Cannot assign value to a port in a non-dag node",0);
    }
  }
  return port;
}

/*****************************************************************************/
/*                         CLASS DagParser                                   */
/*****************************************************************************/

DagParser::DagParser(Dag& dag, const char* content)
  : DagWfParser(content), dag(dag) {
}
DagParser::~DagParser() {
}

/**
 * check the precedence of the dag
 */
bool
DagParser::checkPrec() {
  return dag.checkPrec();
}

/**
 * parse the root node (dag) and its elements (nodes)
 */
void
DagParser::parseRoot(DOMNode* root) {
  // Check root element
  char * _rootNodeName = XMLString::transcode(root->getNodeName());
  if (strcmp (_rootNodeName, "dag")) {
    throw XMLParsingException(XMLParsingException::eUNKNOWN_TAG,
                              "XML for DAG should begin with <dag>");
  }
  XMLString::release(&_rootNodeName);
  DOMNode * child = root->getFirstChild();
  while ((child != NULL)) {
    // Parse all the <node> elements
    if (child->getNodeType() == DOMNode::ELEMENT_NODE) {
      DOMElement * child_elt = (DOMElement*)child;
      char * _nodeName = XMLString::transcode(child_elt->getNodeName());
      string nodeName(_nodeName);
      XMLString::release(&_nodeName);
      TRACE_TEXT (TRACE_ALL_STEPS,
		  "Parsing the element " << nodeName << endl );
      if (nodeName != "node") {
	throw XMLParsingException(XMLParsingException::eUNKNOWN_TAG,
                                  "A dag should only contain <node> elements");
      }
      this->parseNode(child_elt, nodeName);
    }
    child = child->getNextSibling();
  } // end while
}

Node *
DagParser::createNode(const DOMElement* element, const string& nodeEltName) {
  string nodeId(getAttributeValue("id", element));
  string pbName = getAttributeValue("path", element);
  if (nodeId.empty() || pbName.empty()) {
        throw XMLParsingException(XMLParsingException::eEMPTY_ATTR,
                                  "Node without Id or Path");
  }
  DagNode* node = dag.createDagNode(nodeId);
  node->setPbName(pbName);
  return node;
}

void
DagParser::parseOtherNodeSubElt(const DOMElement * childElt,
                                const string& childName,
                                Node& node) {
  if (childName == "prec") {
    parsePrec(childElt, node);
  } else
    throw XMLParsingException(XMLParsingException::eUNKNOWN_TAG,
                              "Invalid tag within node (" + childName + ")");
}

/*****************************************************************************/
/*                         CLASS FWfParser                                   */
/*****************************************************************************/

FWfParser::FWfParser(FWorkflow& workflow, const char * content)
  : DagWfParser(content), workflow(workflow) {
}
FWfParser::~FWfParser() {
}

/**
 * check the precedence of the dag
 */
bool
FWfParser::checkPrec() {
  return true;
}

/**
 * parse the root node (workflow) and its elements (interface and processor)
 */
void
FWfParser::parseRoot(DOMNode* root) {
  // Check the root node
  char * _rootNodeName = XMLString::transcode(root->getNodeName());
  if (strcmp (_rootNodeName, "workflow")) {
    throw XMLParsingException(XMLParsingException::eUNKNOWN_TAG,
                  "XML for Functional Workflow should begin with <workflow>");
  }
  XMLString::release(&_rootNodeName);
  // Parse the content
  DOMNode * child = root->getFirstChild();
  while ((child != NULL)) {
    if (child->getNodeType() == DOMNode::ELEMENT_NODE) {
      DOMElement * child_elt = (DOMElement*)child;
      char * _nodeName = XMLString::transcode(child_elt->getNodeName());
      string nodeName(_nodeName);
      XMLString::release(&_nodeName);
      TRACE_TEXT (TRACE_ALL_STEPS,
		  "Parsing the element " << nodeName << endl );
      if (nodeName == "interface") {
        // Parse the interface
        DOMNode * child = root->getFirstChild();
        while ((child != NULL)) {
          if (child->getNodeType() == DOMNode::ELEMENT_NODE) {
            DOMElement * child_elt = (DOMElement*)child;
            char * _nodeName = XMLString::transcode(child_elt->getNodeName());
            string nodeName(_nodeName);
            XMLString::release(&_nodeName);
            TRACE_TEXT (TRACE_ALL_STEPS,
                        "Parsing the element " << nodeName << endl );
            parseNode(child_elt, nodeName);
          }
          child = child->getNextSibling();
        } // end while
      } else if (nodeName == "processors") {
        // Parse the processors
        DOMNode * child = root->getFirstChild();
        while ((child != NULL)) {
          if (child->getNodeType() == DOMNode::ELEMENT_NODE) {
            DOMElement * child_elt = (DOMElement*)child;
            char * _nodeName = XMLString::transcode(child_elt->getNodeName());
            string nodeName(_nodeName);
            XMLString::release(&_nodeName);
            TRACE_TEXT (TRACE_ALL_STEPS,
                        "Parsing the element " << nodeName << endl );
            parseNode(child_elt, nodeName);
          }
          child = child->getNextSibling();
        } // end while
      } else {
	throw XMLParsingException(XMLParsingException::eUNKNOWN_TAG,
               "Invalid element within <workflow> element (" + nodeName + ")");
      }
    }
    child = child->getNextSibling();
  } // end while
}

Node *
FWfParser::createNode(const DOMElement* element, const string& nodeEltName) {
  string name = getAttributeValue("name", element);
  string type = getAttributeValue("type", element);
  if (name.empty() || type.empty()) {
    throw XMLParsingException(XMLParsingException::eEMPTY_ATTR,
                              "Element " + nodeEltName + " without name or type");
  }
  if (nodeEltName == "source") {
    return (Node*) workflow.createSource(name);
  } else if (nodeEltName == "constant") {
    return (Node*) workflow.createConstant(name);
  } else if (nodeEltName == "processor") {
    return (Node*) workflow.createProcessor(name);
  } else {
    throw XMLParsingException(XMLParsingException::eUNKNOWN_TAG,
                              "Invalid element : " + nodeEltName);
  }
}

void
FWfParser::parseOtherNodeSubElt(const DOMElement * child_elt,
                                const string& childName,
                                Node& node) {
  if (childName == "diet") {

  }
}
