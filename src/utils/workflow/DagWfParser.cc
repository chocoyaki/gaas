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
 * Revision 1.22  2009/02/24 14:01:05  bisnard
 * added dynamic parameter mgmt for wf processors
 *
 * Revision 1.21  2009/02/20 10:23:54  bisnard
 * use estimation class to reduce the nb of submit requests
 *
 * Revision 1.20  2009/02/06 14:55:08  bisnard
 * setup exceptions
 *
 * Revision 1.19  2009/01/16 16:31:54  bisnard
 * added option to specify data source file name
 *
 * Revision 1.18  2009/01/16 13:54:50  bisnard
 * new version of the dag instanciator with iteration strategies for nodes with multiple input ports
 *
 * Revision 1.17  2008/12/02 10:14:51  bisnard
 * modified nodes links mgmt to handle inter-dags links
 *
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

#include <xercesc/framework/LocalFileInputSource.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/framework/Wrapper4InputSource.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XMLStringTokenizer.hpp>
#include <xercesc/dom/DOMText.hpp>
#include <string>
#include <fstream>

#include "debug.hh"
#include "marshalling.hh"
#include "DagWfParser.hh"
#include "WfUtils.hh"
#include "FWorkflow.hh"

using namespace std;

/**
 * XML Parsing errors description
 */
string
XMLParsingException::ErrorMsg() {
  string errorMsg;
  switch(Type()) {
    case eBAD_STRUCT :
      errorMsg = "BAD XML STRUCTURE (" + Info() + ")"; break;
    case eEMPTY_ATTR :
      errorMsg = "MISSING DATA (" + Info() + ")"; break;
    case eUNKNOWN_TAG :
      errorMsg = "UNKNOWN XML TAG ("+ Info() + ")"; break;
    case eUNKNOWN_ATTR :
      errorMsg = "UNKNOWN XML ATTRIBUTE (" + Info() + ")"; break;
    case eINVALID_REF :
      errorMsg = "INVALID REFERENCE (" + Info() + ")"; break;
  }
  return errorMsg;
}

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
 * Parse the XML Document
 */
void
DagWfParser::parseXml() throw (XMLParsingException) {
  const XMLCh gLS[] = { chLatin_L, chLatin_S, chNull };

  TRACE_TEXT(TRACE_ALL_STEPS, "PARSING XML START" << endl);
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
  this->document = parser->parse(*wrapper);
  if (document == NULL) {
    throw XMLParsingException(XMLParsingException::eBAD_STRUCT,
                              "Wrong document description");
  }
  DOMNode * root = (DOMNode*)(document->getDocumentElement());

  // Parse the root element
  parseRoot(root);

  TRACE_TEXT(TRACE_ALL_STEPS, "PARSING XML END" << endl);
}

/**
 * parse a node element
 */
void
DagWfParser::parseNode (const DOMElement * element, const string& elementName) {
  // parse the node start element and its attributes
  Node * newNode = this->createNode(element, elementName);
  // parse the node sub-elements
  DOMNode * child = element->getFirstChild();
  unsigned int lastArg = 0;
  while (child != NULL) {
    if (child->getNodeType() == DOMNode::ELEMENT_NODE) {
      DOMElement * child_elt = (DOMElement*) child;
      char *child_name_str = XMLString::transcode(child_elt->getNodeName());
      string child_name(child_name_str);
      XMLString::release(&child_name_str);

      if (child_name == "arg") {
	parseArg(child_elt, lastArg++, newNode);
      } else
      if (child_name == "in") {
	parseIn(child_elt, lastArg++, newNode);
      } else
      if (child_name == "inOut") {
	parseInOut(child_elt, lastArg++, newNode);
      } else
      if (child_name == "out") {
	parseOut(child_elt, lastArg++, newNode);
      } else
      parseOtherNodeSubElt(child_elt, child_name, lastArg, newNode);
    }
    child = child->getNextSibling();
  } // end while
}

/**
 * Parse an argument element
 */
WfPort *
DagWfParser::parseArg(const DOMElement * element, unsigned int lastArg,
                      Node* node) {
  string name  = getAttributeValue("name", element);
  string value = getAttributeValue("value", element);
  string type  = getAttributeValue("type", element);
  string depth   = getAttributeValue("depth", element);
  if ((name == "") || (value == "")) {
    throw XMLParsingException(XMLParsingException::eEMPTY_ATTR,
                              "Argument " + name + " malformed");
  }
  if (element->getFirstChild() != NULL) {
    throw XMLParsingException(XMLParsingException::eBAD_STRUCT,
          "Argument element doesn't accept a child element. \
          May be a </arg> is forgotten");
  }
  WfPort *port;
  if (!WfCst::isMatrixType(type)) {
    port = setParam(WfPort::PORT_IN, name, type, depth, lastArg, node, &value);
  } else {
    port = setMatrixParam(element, WfPort::PORT_IN, name, lastArg, node, &value);
  }
  return port;
}

/**
 * Parse an input port element
 */
WfPort *
DagWfParser::parseIn(const DOMElement * element, unsigned int lastArg,
                     Node* node) {
  string name    = getAttributeValue("name", element);
  string type    = getAttributeValue("type", element);
  string source  = getAttributeValue("source", element);
  string depth   = getAttributeValue("depth", element);
  if (element->getFirstChild() != NULL) {
    throw XMLParsingException(XMLParsingException::eBAD_STRUCT,
            "input port element doesn't accept a child element. \
            May be a </in> is forgotten");
  }
  WfPort *port;
  if (!WfCst::isMatrixType(type)) {
    port = setParam(WfPort::PORT_IN, name, type, depth, lastArg, node);
  } else {
    port = setMatrixParam(element, WfPort::PORT_IN, name, lastArg, node);
  }
  if (!source.empty())
    port->setConnectionRef(source);
  return port;
}

/**
 * parse InOut port element
 */
WfPort *
DagWfParser::parseInOut(const DOMElement * element, unsigned int lastArg,
                        Node* node) {
  string name    = getAttributeValue("name", element);
  string type    = getAttributeValue("type", element);
  string source  = getAttributeValue("source", element);
  string depth   = getAttributeValue("depth", element);
  if (element->getFirstChild()!=NULL) {
    throw XMLParsingException(XMLParsingException::eBAD_STRUCT,
          "inOut port element doesn't accept a child element. \
           May be a </inOut> is forgotten");
  }
  WfPort *port;
  if (!WfCst::isMatrixType(type)) {
    port = setParam(WfPort::PORT_INOUT, name, type, depth, lastArg, node);
  } else {
    port = setMatrixParam(element, WfPort::PORT_INOUT, name, lastArg, node);
  }
  if (!source.empty())
    port->setConnectionRef(source);
  return port;
}

/**
 * Parse Out port element
 */
WfPort *
DagWfParser::parseOut(const DOMElement * element, unsigned int lastArg,
                      Node* node) {
  string name  = getAttributeValue("name", element);
  string type  = getAttributeValue("type", element);
  string sink  = getAttributeValue("sink", element);
  string depth   = getAttributeValue("depth", element);
  if (element->getFirstChild() != NULL) {
    throw XMLParsingException(XMLParsingException::eBAD_STRUCT,
          "output port element doesn't accept a child element. \
	  May be a </out> is forgotten");
  }
  WfPort *port;
  if (!WfCst::isMatrixType(type)) {
    port = setParam(WfPort::PORT_OUT, name, type, depth, lastArg, node);
  } else {
    port = setMatrixParam(element, WfPort::PORT_OUT, name, lastArg, node);
  }
  if (!sink.empty())
    port->setConnectionRef(sink);
  return port;
}

/**
 * Parse a prec element
 */
void
DagParser::parsePrec(const DOMElement * element, Node* node) {
  string precNodeId = getAttributeValue("id", element);
  if (precNodeId == "") {
    throw XMLParsingException(XMLParsingException::eEMPTY_ATTR,
            "Precedence element malformed");
  }
  if (element->getFirstChild() != NULL) {
    throw XMLParsingException(XMLParsingException::eBAD_STRUCT,
          "Precedence element doesn't accept a child element. \
           May be a </prec> is forgotten");
  }
  node->addNodePredecessor(NULL, precNodeId);
}

/**
 * Create a port
 */
WfPort *
DagWfParser::setParam(const WfPort::WfPortType param_type,
		      const string& name,
		      const string& type,
                      const string& depth,
		      unsigned int lastArg,
		      Node * node,
		      const string * value) {
  // Get the base type and the depth of the list structure (syntax 'LIST(LIST(<basetype>))')
  unsigned int typeDepth = 0;
  string curType = type;
  string::size_type listPos = curType.find("LIST");
  while (listPos != string::npos) {
    curType = curType.substr(listPos+5, curType.length() - 6);
    listPos = curType.find("LIST");
    ++typeDepth;
  }
  short baseType = WfCst::cvtStrToWfType(curType);
  // Use depth attribute (second syntax)
  if (!depth.empty())
    typeDepth = atoi(depth.c_str());
  TRACE_TEXT( TRACE_ALL_STEPS,"Creating new port '" << name
               << "' (base type=" << curType
               << " ,depth=" << typeDepth << " ,idx=" << lastArg << ")" << endl);

  // Initialize the profile with the appropriate parameter type
  // (and optionnally value: used only for Arg ports)

  WfPort *port;
  try {
    port = node->newPort(name, lastArg, param_type, (WfCst::WfDataType) baseType, typeDepth);
  } catch (WfStructException& e) {
    throw XMLParsingException(XMLParsingException::eBAD_STRUCT,
                              "Cannot create port : "+e.ErrorMsg());
  }
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
			    Node * node,
			    const string * value) {
  string elt_type_str = getAttributeValue("base_type", element);
  string nb_rows_str = getAttributeValue("nb_rows", element);
  string nb_cols_str = getAttributeValue("nb_cols", element);
  string matrix_order_str = getAttributeValue("matrix_order", element);

  if ((elt_type_str == "") || (nb_rows_str == "")  ||
      (nb_cols_str == "") || (matrix_order_str == "")) {
    throw XMLParsingException(XMLParsingException::eEMPTY_ATTR,
             "Port with wrong matrix parameters");
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
  WfPort * port;
  try {
    port = node->newPort(name, lastArg, param_type, WfCst::TYPE_MATRIX, 0);
  } catch (WfStructException& e) {
    throw XMLParsingException(XMLParsingException::eBAD_STRUCT,
                              "Cannot create port : "+e.ErrorMsg());
  }
  port->setMatParams(nb_rows, nb_cols,
                     (WfCst::WfMatrixOrder) matrix_order,
                     (WfCst::WfDataType) elt_type);
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

/*****************************************************************************/
/*                         CLASS DagParser                                   */
/*****************************************************************************/

DagParser::DagParser(Dag& dag, const char* content)
  : DagWfParser(content), dag(dag) {
}
DagParser::~DagParser() {
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
DagParser::createNode(const DOMElement* element, const string& elementName) {
  string nodeId(getAttributeValue("id", element));
  string pbName = getAttributeValue("path", element);
  string estClass = getAttributeValue("est-class", element);
  if (nodeId.empty() || pbName.empty()) {
        throw XMLParsingException(XMLParsingException::eEMPTY_ATTR,
                                  "Node without Id or Path");
  }
  DagNode* node = dag.createDagNode(nodeId);
  if (node == NULL)
    throw XMLParsingException(XMLParsingException::eINVALID_REF,
                              "Duplicate node id : " + nodeId);
  node->setPbName(pbName);
  if (!estClass.empty())
    node->setEstimationClass(estClass);
  return node;
}

void
DagParser::parseOtherNodeSubElt(const DOMElement * element,
                                const string& elementName,
                                unsigned int& portIndex,
                                Node * node) {
  if (elementName == "prec") {
    parsePrec(element, node);
  } else
    throw XMLParsingException(XMLParsingException::eUNKNOWN_TAG,
                              "Invalid tag within node (" + elementName + ")");
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
        DOMNode * child2 = child->getFirstChild(); // sub-element of <interface>
        while ((child2 != NULL)) {
          if (child2->getNodeType() == DOMNode::ELEMENT_NODE) {
            DOMElement * child_elt2 = (DOMElement*)child2;
            char * _nodeName2 = XMLString::transcode(child_elt2->getNodeName());
            string nodeName2(_nodeName2);
            XMLString::release(&_nodeName2);
            TRACE_TEXT (TRACE_ALL_STEPS,
                        "Parsing the element " << nodeName2 << endl );
            parseNode(child_elt2, nodeName2);
          }
          child2 = child2->getNextSibling();
        } // end while
      } else if (nodeName == "processors") {
        // Parse the processors
        DOMNode * child2 = child->getFirstChild(); // sub-element of <processors>
        while ((child2 != NULL)) {
          if (child2->getNodeType() == DOMNode::ELEMENT_NODE) {
            DOMElement * child_elt2 = (DOMElement*)child2;
            char * _nodeName2 = XMLString::transcode(child_elt2->getNodeName());
            string nodeName2(_nodeName2);
            XMLString::release(&_nodeName2);
            TRACE_TEXT (TRACE_ALL_STEPS,
                        "Parsing the element " << nodeName2 << endl );
            parseNode(child_elt2, nodeName2);
          }
          child2 = child2->getNextSibling();
        } // end while
      } else if (nodeName == "links") {
        // Parse the links
        DOMNode * child2 = child->getFirstChild(); // sub-element of <links>
        while ((child2 != NULL)) {
          if (child2->getNodeType() == DOMNode::ELEMENT_NODE) {
            DOMElement * child_elt2 = (DOMElement*)child2;
            char * _nodeName2 = XMLString::transcode(child_elt2->getNodeName());
            string nodeName2(_nodeName2);
            XMLString::release(&_nodeName2);
            TRACE_TEXT (TRACE_ALL_STEPS,
                        "Parsing the element " << nodeName2 << endl );
            parseLink(child_elt2);
          }
          child2 = child2->getNextSibling();
        } // end while
      } else {
	throw XMLParsingException(XMLParsingException::eUNKNOWN_TAG,
               "Invalid element within <workflow> element (" + nodeName + ")");
      }
    }
    child = child->getNextSibling();
  } // end while
}

/**
 * Workflow node creation for functional wf
 * TODO manage condition and loop
 */
Node *
FWfParser::createNode(const DOMElement* element, const string& elementName) {
  string name = getAttributeValue("name", element);
  string type = getAttributeValue("type", element);
  string depth = getAttributeValue("depth", element); // for <sink> only
  // Check parameters
  if (name.empty()) {
    throw XMLParsingException(XMLParsingException::eEMPTY_ATTR,
                              "Element " + elementName + " without name");
  }
  if (type.empty() && (elementName != "processor")) {
    throw XMLParsingException(XMLParsingException::eEMPTY_ATTR,
                              "Element " + elementName + " without type");
  }
  // Convert type
  short dataType = 0;
  if (!type.empty()) {
    dataType = WfCst::cvtStrToWfType(type);
  }
  // Convert depth
  unsigned int typeDepth = 0;
  if (!depth.empty())
    typeDepth = atoi(depth.c_str());
  // Create node depending on element name
  Node * node;
  if (elementName == "source") {
    node = workflow.createSource(name,(WfCst::WfDataType) dataType);
  } else if (elementName == "constant") {
    FConstantNode* cstNode = workflow.createConstant(name,(WfCst::WfDataType) dataType);
    string value = getAttributeValue("value", element);
    if (!value.empty())
      cstNode->setValue(value);
    node = (Node*) cstNode;
  } else if (elementName == "sink") {
    node = workflow.createSink(name, (WfCst::WfDataType) dataType, typeDepth);
  } else if (elementName == "processor") {
    node = workflow.createProcessor(name);
  } else {
    throw XMLParsingException(XMLParsingException::eUNKNOWN_TAG,
                              "Invalid element : " + elementName);
  }
  if (node == NULL)
    throw XMLParsingException(XMLParsingException::eINVALID_REF,
                              "Duplicate node id : " + name);
  return node;
}

/**
 * Override methods for port parsing
 */
WfPort*
FWfParser::parseIn(const DOMElement * element,
                   const unsigned int lastArg,
                   Node * node) {
  WfPort *port = DagWfParser::parseIn(element,lastArg,node);
  parseCardAttr(element,port);
  return port;
}

WfPort*
FWfParser::parseOut(const DOMElement * element,
                    const unsigned int lastArg,
                    Node * node) {
  WfPort *port = DagWfParser::parseOut(element,lastArg,node);
  parseCardAttr(element,port);
  return port;
}

WfPort*
FWfParser::parseInOut(const DOMElement * element,
                      const unsigned int lastArg,
                      Node * node) {
  WfPort *port = DagWfParser::parseInOut(element,lastArg,node);
  parseCardAttr(element,port);
  return port;
}

/**
 * Parse Param port element (only for functional wf)
 */
WfPort *
FWfParser::parseParamPort(const DOMElement * element,
                          const unsigned int lastArg,
                          Node* node) {
  string name  = getAttributeValue("name", element);
  string type  = getAttributeValue("type", element);
  if (element->getFirstChild() != NULL)
    throw XMLParsingException(XMLParsingException::eBAD_STRUCT,
          "param port element doesn't accept a child element. \
	  May be a </param> is forgotten");
  if (WfCst::isMatrixType(type))
    throw XMLParsingException(XMLParsingException::eBAD_STRUCT,
          "param port cannot have a matrix type");
  return setParam(WfPort::PORT_PARAM, name, type, "0", lastArg, node);
}

/**
 * Parse the 'card' attribute (the cardinal of containers)
 */
void
FWfParser::parseCardAttr(const DOMElement * element, WfPort* port) {
  const XMLCh * strCard  = element->getAttribute(XMLString::transcode("card"));
  // parse the semi-column separated list
  XMLStringTokenizer *tkizer = new XMLStringTokenizer(strCard, XMLString::transcode(";"));
  list<string> * tklist = new list<string>();
  while (tkizer->hasMoreTokens()) {
    const char *item = XMLString::transcode(tkizer->nextToken());
    tklist->push_back(item);
    TRACE_TEXT (TRACE_ALL_STEPS, "Parsed cardinal item=" << item << endl);
  }
  // set the cardinal of the port
  port->setCardinal(*tklist);
  delete tklist;
  delete tkizer;
}

/**
 * Parse a link
 * This will create the portAdapter object within the TO node port
 * FIXME at this stage we dont know wether the link is a back link or not!
 */
void
FWfParser::parseLink(const DOMElement * element) {
  string from = getAttributeValue("from", element);
  string to = getAttributeValue("to", element);
  string fromNodeName, toNodeName, fromPortName, toPortName;
  FNode *fromNode = parseLinkRef(from, fromNodeName, fromPortName);
  FNode *toNode = parseLinkRef(to, toNodeName, toPortName);
  TRACE_TEXT(TRACE_ALL_STEPS, "Parsing link: from=" << from << " to=" << to << endl);
  // set the port reference
  try {
    WfPort *toPort = toNode->getPort(toPortName);
    toPort->setConnectionRef(fromNodeName + "#" + fromPortName);
  } catch (WfStructException& e) {
    throw XMLParsingException(XMLParsingException::eINVALID_REF,
             "<link> contains an invalid ref: " + e.Info());
  }
}

/**
 * Parses a link attribute (ref to an interface node or to a port)
 * Returns the node reference, and set the node name and the port name
 */
FNode *
FWfParser::parseLinkRef(const string& strRef, string& nodeName, string& portName) {
  FNode *node;
  string::size_type nodeSep = strRef.find(":");
  if (nodeSep != string::npos) { // for processor nodes
    nodeName = strRef.substr(0, nodeSep);
    portName = strRef.substr(nodeSep+1, strRef.length()-nodeSep-1);
    node = workflow.getProcNode(nodeName);
  } else {  // for interface nodes, use the default port
    nodeName = strRef;
    node = workflow.getInterfaceNode(nodeName);
    portName = node->getDefaultPortName();
  }
  if (node == NULL)
    throw XMLParsingException(XMLParsingException::eINVALID_REF,
             "<link> contains an invalid node name : " + nodeName);
  return node;
}

/**
 * Parse the iteration strategy structure (tree)
 * RECURSIVE method
 * returned vector must be deallocated by caller
 */
vector<string>*
FWfParser::parseIterationStrategy(const DOMElement * element,
                                  FProcNode* procNode) {
  const DOMNode * child = element->getFirstChild();
  vector<string>* inputIds = new vector<string>();
  while (child != NULL) {
    if (child->getNodeType() == DOMNode::ELEMENT_NODE) {
      const DOMElement * child_elt = (DOMElement*) child;
      char *child_name_str = XMLString::transcode(child_elt->getNodeName());
      string child_name(child_name_str);
      XMLString::release(&child_name_str);

      if (child_name == "port") {
        string portName = getAttributeValue("name", child_elt);
        if (portName.empty())
          throw XMLParsingException(XMLParsingException::eINVALID_REF,
             "<iterationstrategy> missing port name IN node " + procNode->getId());
	inputIds->push_back(portName);
      } else
      if (child_name == "match") {
        vector<string>* opInputIds = parseIterationStrategy(child_elt, procNode);
        const string& opId = procNode->createInputOperator(FProcNode::OPER_MATCH, *opInputIds);
        inputIds->push_back(opId);
        delete opInputIds;
      } else
      if (child_name == "cross") {
	vector<string>* opInputIds = parseIterationStrategy(child_elt, procNode);
        const string& opId = procNode->createInputOperator(FProcNode::OPER_CROSS, *opInputIds);
        inputIds->push_back(opId);
        delete opInputIds;
      } else
      if (child_name == "dot") {
	vector<string>* opInputIds = parseIterationStrategy(child_elt, procNode);
        const string& opId = procNode->createInputOperator(FProcNode::OPER_DOT, *opInputIds);
        inputIds->push_back(opId);
        delete opInputIds;
      }
    }
    child = child->getNextSibling();
  } // end while
  return inputIds;
}

/**
 * Parse the derived class specific node elements
 */
void
FWfParser::parseOtherNodeSubElt(const DOMElement * element,
                                const string& elementName,
                                unsigned int& portIndex,
                                Node * node) {
  // Diet service
  if (elementName == "diet") {
    FProcNode* procNode = dynamic_cast<FProcNode*>(node);
    if (!procNode)
      throw XMLParsingException(XMLParsingException::eBAD_STRUCT,
                 "<diet> element applied to non-processor element");
    // PATH attribute
    string path = getAttributeValue("path", element);
    if (path.empty())
      throw XMLParsingException(XMLParsingException::eEMPTY_ATTR,
                 "<diet> element must contain a path attribute");
    procNode->checkDynamicParam("path", path);
    procNode->setDIETServicePath(path);
    // MAX-INSTANCES attribute
    string maxInstStr = getAttributeValue("max-instances", element);
    if (!maxInstStr.empty()) {
      procNode->setMaxInstancePerDag(atoi(maxInstStr.c_str()));
    }
    // ESTIMATION attribute
    string estimAttr = getAttributeValue("estimation", element);
    if (!estimAttr.empty()) {
      procNode->setDIETEstimationOption(estimAttr);
    }
  // VALUE of constants (may be replaced by a 'value' attribute within
  //  the <constant> tag)
  } else if (elementName == "value") {
    FConstantNode* cstNode = dynamic_cast<FConstantNode*>(node);
    if (!cstNode)
      throw XMLParsingException(XMLParsingException::eBAD_STRUCT,
                 "<value> element defined outside a <constant>");
    DOMNode * child = element->getFirstChild();
    if (child->getNodeType() == DOMNode::TEXT_NODE) {
      DOMText * child_elt = (DOMText*) child;
      char *child_name_str = XMLString::transcode(child_elt->getData());
      string value(child_name_str);
      XMLString::release(&child_name_str);
      cstNode->setValue(value);
    }
  } else if (elementName == "iterationstrategy") {
    FProcNode* procNode = dynamic_cast<FProcNode*>(node);
    if (!procNode)
      throw XMLParsingException(XMLParsingException::eBAD_STRUCT,
                 "<iterationstrategy> element applied to non-processor element");
    vector<string>* opInputId = parseIterationStrategy(element, procNode);
    if (opInputId->size() != 1)
      throw XMLParsingException(XMLParsingException::eBAD_STRUCT,
                 "<iterationstrategy> contains more than one root operator");
    procNode->setRootInputOperator((*opInputId)[0]);
    delete opInputId;
  } else if (elementName == "param") {
    parseParamPort(element, portIndex++, node);
  } else {
    throw XMLParsingException(XMLParsingException::eUNKNOWN_TAG,
                              "Invalid element : " + elementName);
  }
}

/*****************************************************************************/
/*                    CLASS DataSourceParser                                 */
/*****************************************************************************/

DataSourceParser::DataSourceParser(const string& name)
  : myName(name), currValueNode(NULL) {
}

DataSourceParser::~DataSourceParser() {
  // TODO Free ressources
}

void
DataSourceParser::parseXml(const string& dataFileName) throw (XMLParsingException) {
  const XMLCh gLS[] = { chLatin_L, chLatin_S, chNull };

  TRACE_TEXT(TRACE_ALL_STEPS, "PARSING XML START" << endl);
  DOMImplementation *impl = DOMImplementationRegistry::getDOMImplementation(gLS);
  DOMBuilder  *parser =
    ((DOMImplementationLS*)impl)->createDOMBuilder(DOMImplementationLS::MODE_SYNCHRONOUS, 0);
  const XMLCh * fileName = XMLString::transcode(dataFileName.c_str());
  LocalFileInputSource* locFileIS = new LocalFileInputSource
    (fileName
    );
//   XMLString::release(&fileName);
  Wrapper4InputSource * wrapper = new Wrapper4InputSource(locFileIS);
  this->document = parser->parse(*wrapper);
  if (document == NULL) {
    throw XMLParsingException(XMLParsingException::eBAD_STRUCT,
                              "Data source file not found or wrong document format");
  }
  DOMNode * root = (DOMNode*)(document->getDocumentElement());

  // Parse the root element
  parseRoot(root);

  TRACE_TEXT(TRACE_ALL_STEPS, "PARSING XML END" << endl);
}

void
DataSourceParser::parseRoot(DOMNode* root) throw (XMLParsingException) {
  // Check root element
  char * _rootNodeName = XMLString::transcode(root->getNodeName());
  if (strcmp (_rootNodeName, "data")) {
    throw XMLParsingException(XMLParsingException::eUNKNOWN_TAG,
                              "XML for DATA SOURCE should begin with <data>");
  }
  XMLString::release(&_rootNodeName);
  DOMNode * child = root->getFirstChild();
  while ((child != NULL)) {
    // Parse all the <source> elements
    if (child->getNodeType() == DOMNode::ELEMENT_NODE) {
      DOMElement * child_elt = (DOMElement*)child;
      char * _nodeName = XMLString::transcode(child_elt->getNodeName());
      string nodeName(_nodeName);
      XMLString::release(&_nodeName);
      TRACE_TEXT (TRACE_ALL_STEPS,
		  "Parsing the element " << nodeName << endl );
      if (nodeName != "source") {
	throw XMLParsingException(XMLParsingException::eUNKNOWN_TAG,
                                  "A data source should only contain <source> elements");
      }
      string srcName = DagWfParser::getAttributeValue("name", child_elt);
      if (srcName == myName) {
        TRACE_TEXT (TRACE_ALL_STEPS,
		  "Found the source " << srcName << endl );
        // INITIALIZE THE CURRENT VALUE NODE
        currValueNode = child->getFirstChild();
        findValueNode();
        break;
      }
    }
    child = child->getNextSibling();
  } // end while
  if (currValueNode == NULL)
    throw XMLParsingException(XMLParsingException::eINVALID_REF,
                                  "The data source " + myName + " could not be found");
}

/**
 * Skip the non-element XML nodes until a <value> tag is found
 */
void
DataSourceParser::findValueNode() throw (XMLParsingException) {
  bool valueFound = false;
  while (!valueFound && (currValueNode!=NULL)) {
    if (currValueNode->getNodeType() == DOMNode::ELEMENT_NODE) {
      DOMElement * element = (DOMElement*) currValueNode;
      // get current node name
      char *elt_name_str = XMLString::transcode(element->getNodeName());
      string elt_name(elt_name_str);
      XMLString::release(&elt_name_str);
      // check current node name
      if (elt_name != "value")
         throw XMLParsingException(XMLParsingException::eUNKNOWN_TAG,
                  "Invalid tag within a source (" + elt_name + ")");
      valueFound = true;
    } else {
      // go to next XML node
      currValueNode = currValueNode->getNextSibling();
    }
  }
}

string *
DataSourceParser::getValue() {
  string* value = NULL;
  DOMElement * element = (DOMElement*) currValueNode;
  DOMNode * child = element->getFirstChild();
  if ((child != NULL) && (child->getNodeType() == DOMNode::TEXT_NODE)) {
    DOMText * child_elt = (DOMText*) child;
    char *child_name_str = XMLString::transcode(child_elt->getData());
    value = new string(child_name_str);
    XMLString::release(&child_name_str);
  } else {
    value = new string();
  }
  return value;
}

void
DataSourceParser::goToNextValue() throw (XMLParsingException) {
  currValueNode = currValueNode->getNextSibling();
  // skip the non-value nodes
  findValueNode();
}

bool
DataSourceParser::end() {
  return (currValueNode == NULL);
}
