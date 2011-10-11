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
 * Revision 1.43  2010/08/27 07:30:26  bisnard
 * 'added missing include'
 *
 * Revision 1.42  2010/08/26 11:05:46  bdepardo
 * Check that data file exists before trying to parse it.
 * Throws an XMLParsingException::eFILENOTFOUND exception if it does not exist.
 *
 * Revision 1.41  2010/07/20 09:20:11  bisnard
 * integration with eclipse gui and with dietForwarder
 *
 * Revision 1.40  2010/07/12 16:14:13  glemahec
 * DIET 2.5 beta 1 - Use the new ORB manager and allow the use of SSH-forwarders for all DIET CORBA objects
 *
 * Revision 1.39  2010/06/11 14:10:09  bisnard
 * fixed warning
 *
 * Revision 1.38  2009/11/06 13:06:37  bisnard
 * replaced 'list' tag by 'array' tag in data files
 *
 * Revision 1.37  2009/10/23 13:59:25  bisnard
 * replaced \n by std::endl
 *
 * Revision 1.36  2009/10/02 07:44:56  bisnard
 * new wf data operators MATCH & N-CROSS
 *
 * Revision 1.35  2009/09/25 12:49:45  bisnard
 * handle user data tags
 *
 * Revision 1.34  2009/08/26 10:33:09  bisnard
 * implementation of workflow status & restart
 *
 * Revision 1.33  2009/07/24 15:06:46  bisnard
 * XML validation using DTD for functional workflows
 *
 * Revision 1.32  2009/07/23 12:29:11  bisnard
 * separated sub-workflow parsing and precedence check
 *
 * Revision 1.31  2009/07/10 12:55:59  bisnard
 * implemented while loop workflow node
 *
 * Revision 1.30  2009/07/07 11:25:54  bisnard
 * modified data file parser
 *
 * Revision 1.29  2009/07/07 09:03:22  bisnard
 * changes for sub-workflows (FWorkflow class now inherits from FProcNode)
 *
 * Revision 1.28  2009/06/23 13:08:19  bisnard
 * updated include dependencies
 *
 * Revision 1.27  2009/06/15 12:11:12  bisnard
 * use new XML Parser (SAX) for data source file
 * use new class WfValueAdapter to avoid data duplication
 * use new method FNodeOutPort::storeData
 * changed method to compute total nb of data items
 *
 * Revision 1.26  2009/05/27 08:49:43  bisnard
 * - modified condition output: new IF_THEN and IF_ELSE port types
 * - implemented MERGE and FILTER workflow nodes
 *
 * Revision 1.25  2009/05/15 11:10:20  bisnard
 * release for workflow conditional structure (if)
 *
 * Revision 1.24  2009/04/17 08:54:43  bisnard
 * renamed Node class as WfNode
 *
 * Revision 1.23  2009/04/09 09:56:20  bisnard
 * refactoring due to new class FActivityNode
 *
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
#include <sys/stat.h>

#include <xercesc/framework/LocalFileInputSource.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/framework/Wrapper4InputSource.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XMLStringTokenizer.hpp>
#include <xercesc/dom/DOMText.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/sax2/Attributes.hpp>
#include <xercesc/dom/DOMLSParser.hpp>

#include <string>
#include <fstream>

#include "debug.hh"
#include "marshalling.hh"
#include "DagWfParser.hh"
#include "Dag.hh"
#include "WfUtils.hh"
#include "FWorkflow.hh"
#include "FActivityNode.hh"
#include "FIfNode.hh"
#include "FLoopNode.hh"
#include "FDataHandle.hh"
#include "EventTypes.hh"

#ifndef XTOC
#define XTOC(x) XMLString::transcode(x) // Use iff x is a XMLCh *
#define CTOX(x) XMLString::transcode(x) // Use iff x is a char *
#define XREL(x) XMLString::release(&x)
#endif

using namespace std;
using namespace events;

/**
 * XML Parsing errors description
 */
string
XMLParsingException::ErrorMsg() {
  string errorMsg;
  switch(Type()) {
  case eUNKNOWN:
    errorMsg = "UNDEFINED ERROR (" + Info() + ")"; break;
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
  case eINVALID_DATA :
    errorMsg = "INVALID DATA (" + Info() + ")"; break;
  case eFILENOTFOUND :
    errorMsg = "FILE NOT FOUND (" + Info() + ")"; break;
  }
  return errorMsg;
}

/**
 * Error handler for workflow parser
 */
bool
MyDOMErrorHandler::handleError (const DOMError &domError) {
  char* errorMsg = XTOC(domError.getMessage());
  WARNING("XML Error: " << errorMsg);
  XREL(errorMsg);
  if (domError.getSeverity() == DOMError::DOM_SEVERITY_WARNING)
    return true;
  else  // exceptions cannot be thrown here - parser *should* stop if false
    return false;
}

/**
 * Constructor
 */
DagWfParser::DagWfParser() : content() {
}
DagWfParser::DagWfParser(const char * wf_desc) : content(wf_desc) {
}
DagWfParser::DagWfParser(const string& fileName)
  : content (), myXmlFileName(fileName) {
}

DagWfParser::~DagWfParser() {
}

/**
 * Parse the XML Document using DOM Parser (common part for Dag and Functional WF)
 */
void
DagWfParser::parseXml(bool checkValid) {
  const XMLCh gLS[] = { chLatin_L, chLatin_S, chNull };

  TRACE_TEXT(TRACE_ALL_STEPS, "INITIALIZE XML PARSER" << endl);
  DOMImplementation *impl = DOMImplementationRegistry::getDOMImplementation(gLS);
  DOMLSParser *parser = impl->createLSParser(DOMImplementationLS::MODE_SYNCHRONOUS, 0);
  MyDOMErrorHandler* errHandler = new MyDOMErrorHandler();

  // Validation
  if (parser->getDomConfig()->canSetParameter(XMLUni::fgDOMValidateIfSchema, true)) {
    TRACE_TEXT(TRACE_ALL_STEPS, "Activating XML Validation" << endl);
    parser->getDomConfig()->setParameter(XMLUni::fgDOMValidateIfSchema, true);
    parser->getDomConfig()->setParameter(XMLUni::fgXercesValidationErrorAsFatal, true);
  } else {
    TRACE_TEXT(TRACE_ALL_STEPS, "XML Validation cannot be activated" << endl);
  }

  // Error handler
  if (parser->getDomConfig()->canSetParameter(XMLUni::fgDOMErrorHandler, errHandler)) {
    TRACE_TEXT(TRACE_ALL_STEPS, "Activating XML Error handler" << endl);
    parser->getDomConfig()->setParameter(XMLUni::fgDOMErrorHandler, errHandler);
  } else {
    TRACE_TEXT(TRACE_ALL_STEPS, "XML Error handler cannot be activated" << endl);
  }

  // Wrapper
  Wrapper4InputSource * wrapper;
  string errorMsgPfx;
  if (!content.empty()) {

    // INITIALIZE FROM BUFFER
    static const char * content_id = "workflow_description";
    MemBufInputSource* memBufIS = new MemBufInputSource
      (
        (const XMLByte*)(this->content.c_str())
        , content.size()
        , content_id
        , false
        );
    wrapper = new Wrapper4InputSource(memBufIS);

  } else if (!myXmlFileName.empty()) {

    // INITIALIZE FROM FILE
    TRACE_TEXT(TRACE_ALL_STEPS, "PARSING FROM FILE: " << myXmlFileName << endl);
    errorMsgPfx = "In file " + myXmlFileName + " : ";
    XMLCh* xmlFileName = CTOX(myXmlFileName.c_str());
    LocalFileInputSource * fileBufIS = new LocalFileInputSource(xmlFileName);
    wrapper = new Wrapper4InputSource(fileBufIS);
    //     XREL(xmlFileName);

  } else {
    throw XMLParsingException(XMLParsingException::eUNKNOWN,"Empty XML filename");
  }

  // PARSE
  TRACE_TEXT(TRACE_ALL_STEPS, "PARSING XML START" << endl);
  try {
    this->document = parser->parse((DOMLSInput*) wrapper);
  } catch (...) {
    WARNING(errorMsgPfx << "Unexpected exception during XML Parsing");
    throw XMLParsingException(XMLParsingException::eUNKNOWN,"");
  }

  if (document == NULL)
    throw XMLParsingException(XMLParsingException::eFILENOTFOUND,
                              myXmlFileName);

  // Check if DTD was provided
  if (checkValid && !document->getDoctype()) {
    WARNING(errorMsgPfx << "XML is not validated (no DTD provided)" << endl
            << "Use <!DOCTYPE workflow SYSTEM \"[DIET_INSTALL_DIR]/etc/FWorkflow.dtd\">"
            << " instruction to provide it");
  }

  DOMNode * root = (DOMNode*)(document->getDocumentElement());
  if (root == NULL)
    throw XMLParsingException(XMLParsingException::eUNKNOWN,
                              "No details available");

  // Parse the root element
  parseRoot(root);

  delete errHandler;
  delete parser;
  TRACE_TEXT(TRACE_ALL_STEPS, "PARSING XML END" << endl);
}

/**
 * parse a node element
 */
void
DagWfParser::parseNode (const DOMElement * element, const string& elementName) {
  // parse the node start element and its attributes
  WfNode * newNode = this->createNode(element, elementName);
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
                      WfNode* node) {
  string name  = getAttributeValue("name", element);
  string value = getAttributeValue("value", element);
  string type  = getAttributeValue("type", element);
  string depth   = getAttributeValue("depth", element);
  checkMandatoryAttr("node","name",name);
  checkMandatoryAttr("node","value",value);
  checkLeafElement(element, "arg");
  WfPort *port;
  if (!WfCst::isMatrixType(type)) {
    port = createPort(WfPort::PORT_ARG, name, type, depth, lastArg, node);
  } else {
    port = createMatrixPort(element, WfPort::PORT_ARG, name, lastArg, node);
  }
  // set the value
  DagNodeArgPort * dagPort = dynamic_cast<DagNodeArgPort*>(port);
  if (dagPort)
    dagPort->setValue(value);
  else throw XMLParsingException(XMLParsingException::eBAD_STRUCT,
                                 "value attribute not valid for other ports than <arg> port");

  return port;
}

/**
 * Parse an input port element
 */
WfPort *
DagWfParser::parseIn(const DOMElement * element, unsigned int lastArg,
                     WfNode* node) {
  string name    = getAttributeValue("name", element);
  string type    = getAttributeValue("type", element);
  string source  = getAttributeValue("source", element);
  string interface  = getAttributeValue("interface", element);
  string depth   = getAttributeValue("depth", element);
  checkMandatoryAttr("in","name",name);
  checkMandatoryAttr("in","type",name);
  checkLeafElement(element, "in");
  WfPort *port;
  if (!WfCst::isMatrixType(type)) {
    port = createPort(WfPort::PORT_IN, name, type, depth, lastArg, node);
  } else {
    port = createMatrixPort(element, WfPort::PORT_IN, name, lastArg, node);
  }
  if (!source.empty())
    port->setConnectionRef(source);
  if (!interface.empty())
    port->setInterfaceRef(interface);
  return port;
}

/**
 * parse InOut port element
 */
WfPort *
DagWfParser::parseInOut(const DOMElement * element, unsigned int lastArg,
                        WfNode* node) {
  string name    = getAttributeValue("name", element);
  string type    = getAttributeValue("type", element);
  string source  = getAttributeValue("source", element);
  string depth   = getAttributeValue("depth", element);
  checkMandatoryAttr("inOut","name",name);
  checkMandatoryAttr("inOut","type",name);
  checkLeafElement(element, "inOut");
  WfPort *port;
  if (!WfCst::isMatrixType(type)) {
    port = createPort(WfPort::PORT_INOUT, name, type, depth, lastArg, node);
  } else {
    port = createMatrixPort(element, WfPort::PORT_INOUT, name, lastArg, node);
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
                      WfNode* node) {
  string name  = getAttributeValue("name", element);
  string type  = getAttributeValue("type", element);
  string sink  = getAttributeValue("sink", element);
  string interface  = getAttributeValue("interface", element);
  string depth = getAttributeValue("depth", element);
  string dataId = getAttributeValue("dataId", element);
  checkMandatoryAttr("out","name",name);
  checkMandatoryAttr("out","type",name);
  checkLeafElement(element, "out");
  WfPort *port;
  if (!WfCst::isMatrixType(type)) {
    port = createPort(WfPort::PORT_OUT, name, type, depth, lastArg, node);
  } else {
    port = createMatrixPort(element, WfPort::PORT_OUT, name, lastArg, node);
  }
  if (!sink.empty())
    port->setConnectionRef(sink);
  if (!interface.empty())
    port->setInterfaceRef(interface);
  if (!dataId.empty()) {
    DagNodePort *DPort = dynamic_cast<DagNodePort*>(port);
    DPort->setDataID(dataId);
  }
  return port;
}

/**
 * Create a port
 */
WfPort *
DagWfParser::createPort( const WfPort::WfPortType param_type,
                         const string& name,
                         const string& type,
                         const string& depth,
                         unsigned int lastArg,
                         WfNode * node ) {
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
  if (baseType == WfCst::TYPE_UNKNOWN)
    throw XMLParsingException(XMLParsingException::eINVALID_DATA,
                              "Unknown data type : "+type);
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
  return port;
}

/**
 * create a port with matrix parameter type
 */
WfPort *
DagWfParser::createMatrixPort( const DOMElement * element,
                               const WfPort::WfPortType param_type,
                               const string& name,
                               unsigned int lastArg,
                               WfNode * node ) {
  string elt_type_str = getAttributeValue("base_type", element);
  string nb_rows_str = getAttributeValue("nb_rows", element);
  string nb_cols_str = getAttributeValue("nb_cols", element);
  string matrix_order_str = getAttributeValue("matrix_order", element);
  checkMandatoryAttr("(matrix port)","base_type",elt_type_str);
  checkMandatoryAttr("(matrix port)","nb_rows",nb_rows_str);
  checkMandatoryAttr("(matrix port)","nb_cols",nb_cols_str);
  checkMandatoryAttr("(matrix port)","matrix_order",matrix_order_str);

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
  return port;
}

/*****************************************************************************/
/*                         CLASS DagParser                                   */
/*****************************************************************************/

DagParser::DagParser() : DagWfParser(), myCurrDag(NULL) {
}
DagParser::DagParser(const char* content)
  : DagWfParser(content), myCurrDag(NULL) {
}
DagParser::DagParser(const string& xmlFileName)
  : DagWfParser(xmlFileName), myCurrDag(NULL) {
}
DagParser::~DagParser() {
}

void
DagParser::setCurrentDag(Dag& dag) {
  myCurrDag = &dag;
}

void
DagParser::parseOneDag(DOMNode* root) {
  char * rootNodeName = XTOC(root->getNodeName());
  if ( strcmp(rootNodeName, "dag") )
    throw XMLParsingException(XMLParsingException::eUNKNOWN_TAG,
                              "XML for DAG should begin with <dag>");
  XREL(rootNodeName);
  DOMNode * child = root->getFirstChild();
  while ((child != NULL)) {
    // Parse all the <node> elements
    if (child->getNodeType() == DOMNode::ELEMENT_NODE) {
      DOMElement * child_elt = (DOMElement*)child;
      char * _nodeName = XTOC(child_elt->getNodeName());
      string nodeName(_nodeName);
      XREL(_nodeName);
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

WfNode *
DagParser::createNode(const DOMElement* element, const string& elementName) {
  if (!myCurrDag) {
    INTERNAL_ERROR(__FUNCTION__ << "Null dag pointer" << endl,1);
  }
  string nodeId(getAttributeValue("id", element));
  string pbName = getAttributeValue("path", element);
  string estClass = getAttributeValue("est-class", element);
  string status = getAttributeValue("status", element);
  checkMandatoryAttr("node","id",nodeId);
  checkMandatoryAttr("node","path",pbName);

  DagNode* node = NULL;
  try {
    node = myCurrDag->createDagNode(nodeId);
  } catch (WfStructException& e) {
    throw XMLParsingException(XMLParsingException::eBAD_STRUCT,
                              "Cannot create node : "+e.ErrorMsg());
  }
  node->setPbName(pbName);
  if (!estClass.empty())
    node->setEstimationClass(estClass);
  if (!status.empty())
    node->setStatus(status);
  return node;
}

void
DagParser::parseOtherNodeSubElt(const DOMElement * element,
                                const string& elementName,
                                unsigned int& portIndex,
                                WfNode * node) {
  if (elementName == "prec") {
    parsePrec(element, node);
  } else
    throw XMLParsingException(XMLParsingException::eUNKNOWN_TAG,
                              "Invalid tag within node (" + elementName + ")");
}

void
DagParser::parsePrec(const DOMElement * element, WfNode* node) {
  string precNodeId = getAttributeValue("id", element);
  checkMandatoryAttr("prec","id",precNodeId);
  checkLeafElement(element, "prec");
  node->addNodePredecessor(NULL, precNodeId);
}


/*****************************************************************************/
/*                       CLASS SingleDagParser                               */
/*****************************************************************************/

SingleDagParser::SingleDagParser(Dag& dag, const char* content)
  : DagParser(content) {
  setCurrentDag(dag);
}

SingleDagParser::~SingleDagParser() {
}

void
SingleDagParser::parseRoot(DOMNode* root) {
  parseOneDag(root);
}


/*****************************************************************************/
/*                       CLASS MultiDagParser                               */
/*****************************************************************************/

MultiDagParser::MultiDagParser()
  : DagParser(), myWorkflow(NULL) {
}

MultiDagParser::MultiDagParser( const string& xmlFileName )
  : DagParser(xmlFileName), myWorkflow(NULL) {
}

MultiDagParser::~MultiDagParser() {
  // does not deallocate parsed dags
}

list<Dag*>&
MultiDagParser::getDags() {
  return myDags;
}

void
MultiDagParser::setWorkflow(FWorkflow* wf) {
  myWorkflow = wf;
}

void
MultiDagParser::parseRoot(DOMNode* root) {
  // Check root element
  char * _rootNodeName = XTOC(root->getNodeName());
  if (strcmp (_rootNodeName, "dags")) {
    throw XMLParsingException(XMLParsingException::eUNKNOWN_TAG,
                              "XML for MultiDAG should begin with <dags>");
  }
  XREL(_rootNodeName);
  DOMNode * child = root->getFirstChild();
  int dagCounter = 0;
  while ((child != NULL)) {
    // Parse all the <dag> elements
    if (child->getNodeType() == DOMNode::ELEMENT_NODE) {
      DOMElement * child_elt = (DOMElement*)child;
      char * _childName = XTOC(child_elt->getNodeName());
      string childName(_childName);
      TRACE_TEXT (TRACE_ALL_STEPS,
                  "Parsing the element " << childName << endl );
      if (strcmp(_childName, "dag")) {
        throw XMLParsingException(XMLParsingException::eUNKNOWN_TAG,
                                  "A MultiDag should only contain <dag> elements");
      }
      XREL(_childName);
      string suffix = "." + itoa(dagCounter);
      Dag * currDag = new Dag(myXmlFileName + suffix);
      if (myWorkflow) {
        currDag->setWorkflow(myWorkflow);
        EventManager::getEventMgr()->sendEvent(
          new EventCreateObject<Dag,FWorkflow>(currDag, myWorkflow) );
      }
      myDags.push_back(currDag);
      this->setCurrentDag(*currDag);
      this->parseOneDag(child);
      dagCounter++;
    }
    child = child->getNextSibling();
  }
}

/*****************************************************************************/
/*                         CLASS FWfParser                                   */
/*****************************************************************************/

FWfParser::FWfParser(FWorkflow& workflow, const char * content)
  : DagWfParser(content), workflow(workflow) {
}
FWfParser::FWfParser(FWorkflow& workflow, const string& fileName)
  : DagWfParser(fileName), workflow(workflow) {
}

FWfParser::~FWfParser() {
}

string
FWfParser::getWfClassName(const string& fileName) {
  string::size_type lastSlash = fileName.rfind("/");
  string::size_type start = (lastSlash == string::npos) ? 0 : lastSlash+1;
  string::size_type suffix = fileName.rfind(".xml");
  string::size_type end = (suffix == string::npos) ? 0 : suffix-1;
  return fileName.substr(start, end-start+1);
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
      if (nodeName == "include") {
        string fileName = getAttributeValue("file", child_elt);
        // store class=>filename mapping
        string className = getWfClassName(fileName);
        if (!className.empty()) {
          TRACE_TEXT (TRACE_ALL_STEPS,
                      "Include workflow class : " << className << endl );
          myClassFiles[className] = fileName;
        }
      } else if (nodeName == "interface") {
        // Parse the interface
        DOMNode * child2 = child->getFirstChild();  // sub-element of <interface>
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
        DOMNode * child2 = child->getFirstChild();  // sub-element of <processors>
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
        DOMNode * child2 = child->getFirstChild();  // sub-element of <links>
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
 */
WfNode *
FWfParser::createNode(const DOMElement* element, const string& elementName) {
  string name = getAttributeValue("name", element);
  string type = getAttributeValue("type", element);
  string depth = getAttributeValue("depth", element);
  string value = getAttributeValue("value", element);
  string nclass = getAttributeValue("class", element);
  checkMandatoryAttr(elementName,"name",name);

  // Convert type
  short dataType = 0;
  if (!type.empty()) {
    dataType = WfCst::cvtStrToWfType(type);
    if (dataType == WfCst::TYPE_UNKNOWN)
      throw XMLParsingException(XMLParsingException::eINVALID_DATA,
                                "Unknown data type : "+type);
  }
  // Convert depth
  unsigned int typeDepth = 0;
  if (!depth.empty())
    typeDepth = atoi(depth.c_str());
  // Create node depending on element name
  WfNode * node;
  if (elementName == "source") {
    checkMandatoryAttr(elementName,"type",type);
    node = workflow.createSource(name,(WfCst::WfDataType) dataType);

  } else if (elementName == "constant") {
    string dataID = getAttributeValue("dataId", element);
    checkMandatoryAttr(elementName,"type",type);
    if (!depth.empty())
      throw XMLParsingException(XMLParsingException::eUNKNOWN_ATTR,
                                "Attribute depth is not applicable to constant tag");
    FConstantNode* cstNode = workflow.createConstant(name,(WfCst::WfDataType) dataType);
    if (!value.empty())
      cstNode->setValue(value);
    if (!dataID.empty())
      cstNode->setDataID(dataID);
    node = (WfNode*) cstNode;

  } else if (elementName == "sink") {
    checkMandatoryAttr(elementName,"type",type);
    node = workflow.createSink(name, (WfCst::WfDataType) dataType, typeDepth);

  } else if (elementName == "processor") {
    node = workflow.createActivity(name);

  } else if (elementName == "condition") {
    node = workflow.createIf(name);

  } else if (elementName == "merge") {
    node = workflow.createMerge(name);

  } else if (elementName == "filter") {
    node = workflow.createFilter(name);

  } else if (elementName == "loop") {
    node = workflow.createLoop(name);

  } else if (elementName == "subWorkflow") {
    // initialize workflow from XML file defined in the <include> tag
    checkMandatoryAttr(elementName,"class",nclass);
    map<string,string>::iterator classIter = myClassFiles.find(nclass);
    if (classIter == myClassFiles.end())
      throw XMLParsingException(XMLParsingException::eINVALID_REF,
                                "Unknown workflow class (missing include) : " + nclass);
    // Create sub-workflow and parse the workflow XML
    string xmlWfFileName = (string) classIter->second;
    FWorkflow*  subWf = workflow.createSubWorkflow(name, nclass);
    FWfParser   reader(*subWf, xmlWfFileName);
    reader.parseXml(true);
    // Use the same data file as parent workflow
    subWf->setDataSrcXmlFile(workflow.getDataSrcXmlFile());

    node = (WfNode*) subWf;

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

WfPort *
FWfParser::parsePortCommon(const DOMElement * element,
                           const unsigned int portIndex,
                           WfNode * node,
                           const string& portName,
                           const WfPort::WfPortType portType) {
  string name  = getAttributeValue("name", element);
  string type  = getAttributeValue("type", element);
  string depth = getAttributeValue("depth", element);
  checkMandatoryAttr(portName,"name",name);
  checkMandatoryAttr(portName,"type",name);
  checkLeafElement(element,portName);

  if (!WfCst::isMatrixType(type)) {
    return createPort(portType, name, type, depth, portIndex, node);
  } else {
    return createMatrixPort(element, portType, name, portIndex, node);
  }
}

WfPort*
FWfParser::parseIn(const DOMElement * element,
                   const unsigned int lastArg,
                   WfNode * node) {
  WfPort *port = DagWfParser::parseIn(element,lastArg,node);
  parseCardAttr(element,port);
  return port;
}

WfPort*
FWfParser::parseOut(const DOMElement * element,
                    const unsigned int lastArg,
                    WfNode * node) {
  WfPort *port = DagWfParser::parseOut(element,lastArg,node);
  parseCardAttr(element,port);
  return port;
}

WfPort*
FWfParser::parseInOut(const DOMElement * element,
                      const unsigned int lastArg,
                      WfNode * node) {
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
                          WfNode* node) {
  string name  = getAttributeValue("name", element);
  string type  = getAttributeValue("type", element);
  checkMandatoryAttr("param","name",name);
  checkMandatoryAttr("param","type",name);
  checkLeafElement(element,"param");

  if (WfCst::isMatrixType(type))
    throw XMLParsingException(XMLParsingException::eBAD_STRUCT,
                              "param port cannot have a matrix type");
  return createPort(WfPort::PORT_PARAM, name, type, "0", lastArg, node);
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
 */
void
FWfParser::parseLink(const DOMElement * element) {
  string from = getAttributeValue("from", element);
  string to = getAttributeValue("to", element);
  string fromNodeName, toNodeName, fromPortName, toPortName;
  try {
    FNode *fromNode = parseLinkRef(from, fromNodeName, fromPortName);
    FNode *toNode = parseLinkRef(to, toNodeName, toPortName);
    TRACE_TEXT(TRACE_ALL_STEPS, "Parsing link: from=" << fromNode->getId()
               << " to=" << toNode->getId() << endl);
    // set the port reference
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
        checkMandatoryAttr("port","name",portName);
        inputIds->push_back(portName);
      } else
        if (child_name == "match") {
          vector<string>* opInputIds = parseIterationStrategy(child_elt, procNode);
          if (opInputIds->size() > 2)
            throw XMLParsingException(XMLParsingException::eBAD_STRUCT,
                                      "MATCH iterator only accepts 2 input ports");
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
            if (child_name == "flatcross") {
              vector<string>* opInputIds = parseIterationStrategy(child_elt, procNode);
              const string& opId = procNode->createInputOperator(FProcNode::OPER_FLATCROSS, *opInputIds);
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
                                WfNode * node) {
  // Diet service
  if (elementName == "diet") {
    FActivityNode* aNode = dynamic_cast<FActivityNode*>(node);
    if (!aNode)
      throw XMLParsingException(XMLParsingException::eBAD_STRUCT,
                                "<diet> element applied to non-activity element");
    // PATH attribute
    string path = getAttributeValue("path", element);
    checkMandatoryAttr("diet","path",path);
    aNode->checkDynamicParam("path", path);
    aNode->setDIETServicePath(path);
    // MAX-INSTANCES attribute
    string maxInstStr = getAttributeValue("max-instances", element);
    if (!maxInstStr.empty()) {
      aNode->setMaxInstancePerDag(atoi(maxInstStr.c_str()));
    }
    // ESTIMATION attribute
    string estimAttr = getAttributeValue("estimation", element);
    if (!estimAttr.empty()) {
      aNode->setDIETEstimationOption(estimAttr);
    }
  } else if ((elementName == "beanshell") || (elementName == "gasw")) {
   ;  // tags used by other workflow engines
    // VALUE of constants (may be replaced by a 'value' attribute within
    //  the <constant> tag)
  } else if (elementName == "value") {
    FConstantNode* cstNode = dynamic_cast<FConstantNode*>(node);
    if (!cstNode)
      throw XMLParsingException(XMLParsingException::eBAD_STRUCT,
                                "<value> element defined outside a <constant>");
    string value;
    getTextContent(element, value);
    cstNode->setValue(value);
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
  } else if (elementName == "if") {
    FIfNode* ifNode = dynamic_cast<FIfNode*>(node);
    if (!ifNode)
      throw XMLParsingException(XMLParsingException::eBAD_STRUCT,
                                "<if> element applied to non-conditional element");
    string condition;
    getTextContent(element, condition);
    try {
      ifNode->setCondition(condition);
    } catch (WfStructException& e) {
      throw XMLParsingException(XMLParsingException::eBAD_STRUCT,
                                "<if> element contains invalid expression");
    }
  } else if (elementName == "outThen") {
    FIfNode* ifNode = dynamic_cast<FIfNode*>(node);
    if (!ifNode)
      throw XMLParsingException(XMLParsingException::eBAD_STRUCT,
                                "<outThen> element applied to non-conditional element");
    parsePortCommon(element, portIndex++, ifNode, "outThen", WfPort::PORT_OUT_THEN);
  } else if (elementName == "outElse") {
    FIfNode* ifNode = dynamic_cast<FIfNode*>(node);
    if (!ifNode)
      throw XMLParsingException(XMLParsingException::eBAD_STRUCT,
                                "<outElse> element applied to non-conditional element");
    parsePortCommon(element, portIndex++, ifNode, "outElse", WfPort::PORT_OUT_ELSE);
  } else if (elementName == "then") {
    FIfNode* ifNode = dynamic_cast<FIfNode*>(node);
    if (!ifNode)
      throw XMLParsingException(XMLParsingException::eBAD_STRUCT,
                                "<then> element applied to non-conditional element");
    string thenMapStr;
    getTextContent(element, thenMapStr);
    map<string,string>  thenMap;
    getPortMap(thenMapStr, thenMap);
    for (map<string,string>::iterator mapIter = thenMap.begin();
         mapIter != thenMap.end();
         ++mapIter) {
      ifNode->setThenMap(mapIter->first,mapIter->second);
    }
  } else if (elementName == "else") {
    FIfNode* ifNode = dynamic_cast<FIfNode*>(node);
    if (!ifNode)
      throw XMLParsingException(XMLParsingException::eBAD_STRUCT,
                                "<else> element applied to non-conditional element");
    string elseMapStr;
    getTextContent(element, elseMapStr);
    map<string,string>  elseMap;
    getPortMap(elseMapStr, elseMap);
    for (map<string,string>::iterator mapIter = elseMap.begin();
         mapIter != elseMap.end();
         ++mapIter) {
      ifNode->setElseMap(mapIter->first,mapIter->second);
    }
  } else if (elementName == "while") {
    FLoopNode* loopNode = dynamic_cast<FLoopNode*>(node);
    if (!loopNode)
      throw XMLParsingException(XMLParsingException::eBAD_STRUCT,
                                "<while> element applied to non-loop element");
    string condition;
    getTextContent(element, condition);
    try {
      loopNode->setWhileCondition(condition);
    } catch (WfStructException& e) {
      throw XMLParsingException(XMLParsingException::eBAD_STRUCT,
                                "<while> element contains invalid expression");
    }
  } else if (elementName == "inLoop") {
    FLoopNode* loopNode = dynamic_cast<FLoopNode*>(node);
    if (!loopNode)
      throw XMLParsingException(XMLParsingException::eBAD_STRUCT,
                                "<inLoop> element applied to non-loop element");
    WfPort *lPort = parsePortCommon(element, portIndex++, loopNode, "inLoop", WfPort::PORT_IN_LOOP);
    // INIT attribute
    string init = getAttributeValue("init", element);
    checkMandatoryAttr("inLoop","init",init);
    lPort->setInterfaceRef(init);
  } else if (elementName == "outLoop") {
    FLoopNode* loopNode = dynamic_cast<FLoopNode*>(node);
    if (!loopNode)
      throw XMLParsingException(XMLParsingException::eBAD_STRUCT,
                                "<outLoop> element applied to non-loop element");
    WfPort *lPort = parsePortCommon(element, portIndex++, loopNode, "outLoop", WfPort::PORT_OUT_LOOP);
    // FINAL attribute
    string final = getAttributeValue("final", element);
    checkMandatoryAttr("inLoop","final",final);
    lPort->setInterfaceRef(final);
  } else if (elementName == "do") {
    FLoopNode* loopNode = dynamic_cast<FLoopNode*>(node);
    if (!loopNode)
      throw XMLParsingException(XMLParsingException::eBAD_STRUCT,
                                "<do> element applied to non-loop element");
    string doMapStr;
    getTextContent(element, doMapStr);
    map<string,string>  doMap;
    getPortMap(doMapStr, doMap);
    for (map<string,string>::iterator mapIter = doMap.begin();
         mapIter != doMap.end();
         ++mapIter) {
      loopNode->setDoMap(mapIter->first,mapIter->second);
    }
  } else {
    throw XMLParsingException(XMLParsingException::eUNKNOWN_TAG,
                              "Invalid element : " + elementName);
  }
}

/*****************************************************************************/
/*                    CLASS DataSourceParser                                 */
/*****************************************************************************/

DataSourceParser::DataSourceParser(FSourceNode* node)
  : myNode(node) {
}

DataSourceParser::~DataSourceParser() {
}

void
DataSourceParser::parseXml(const string& dataFileName) throw(XMLParsingException) {
  //   const XMLCh gLS[] = { chLatin_L, chLatin_S, chNull };

  TRACE_TEXT(TRACE_ALL_STEPS, "PARSING XML START" << endl);

  if (dataFileName.empty())
    throw XMLParsingException(XMLParsingException::eINVALID_DATA,
                              "Empty XML File name");

  struct stat buffer;
  if ( stat( dataFileName.c_str(), &buffer ) )
    throw XMLParsingException(XMLParsingException::eFILENOTFOUND,
                              "XML File '" + dataFileName + "' does not exist");


  SAX2XMLReader* parser = XMLReaderFactory::createXMLReader();
  parser->setFeature(XMLUni::fgSAX2CoreValidation, false);
  parser->setFeature(XMLUni::fgSAX2CoreNameSpaces, false);

  DataSourceHandler* defaultHandler = new DataSourceHandler(myNode);
  parser->setContentHandler(defaultHandler);
  parser->setErrorHandler(defaultHandler);

  try {
    parser->parse(dataFileName.c_str());
  } catch (XMLParsingException& e) {
    throw;
  } catch (...) {
    throw XMLParsingException(XMLParsingException::eBAD_STRUCT,
                              "Invalid XML in data source file");
  }

  delete defaultHandler;
  delete parser;
  TRACE_TEXT(TRACE_ALL_STEPS, "PARSING XML END" << endl);
}

/*****************************************************************************/
/*                    CLASS DataSourceHandler                                */
/*****************************************************************************/

DataSourceHandler::DataSourceHandler(FSourceNode* node)
  : myNode(node), myCurrTag(NULL), myCurrListDH(NULL), myCurrItemDH(NULL),
    isSourceFound(false), isListFound(false), isItemFound(false) {
}

/**
 * Main handlers
 */

void
DataSourceHandler::startElement(const   XMLCh* const    uri,
                                const   XMLCh* const    localname,
                                const   XMLCh* const    qname,
                                const   Attributes&     attrs ) {
  char* eltName = XTOC(qname);

  if (strcmp("source",eltName) == 0) startSource(attrs);
  if (strcmp("array",eltName) == 0)  startList(attrs);
  if (strcmp("list",eltName) == 0)   startList(attrs);  // obsolete
  if (strcmp("item",eltName) == 0)   startItem(attrs);
  if (strcmp("tag",eltName) == 0)    startTag(attrs);

  XREL(eltName);
}

void
DataSourceHandler::endElement(const   XMLCh* const    uri,
                              const   XMLCh* const    localname,
                              const   XMLCh* const    qname) {
  char* eltName = XTOC(qname);

  if (strcmp("source",eltName) == 0) endSource();
  if (strcmp("array",eltName) == 0)  endList();
  if (strcmp("list",eltName) == 0)   endList();  // obsolete
  if (strcmp("item",eltName) == 0)   endItem();
  if (strcmp("tag",eltName) == 0)    endTag();

  XREL(eltName);
}

/**
 * Tag-specific handlers
 */

void
DataSourceHandler::startSource(const   Attributes&     attrs) {
  XMLCh* attName = CTOX("name");
  char* srcName = XTOC(attrs.getValue(attName));

  if (strcmp(myNode->getId().c_str(), srcName) == 0) {
    isSourceFound = true;
  }

  XREL(attName);
  XREL(srcName);
}

void
DataSourceHandler::endSource() {
  isListFound = false;
  isSourceFound = false;
  delete myCurrTag;
  myCurrTag = NULL;
}

void
DataSourceHandler::startList(const   Attributes&     attrs) {
  if (isSourceFound) {

    // only one <list> element (root tag) is possible
    if (isListFound && myCurrTag && myCurrTag->isEmpty()) {
      string errorMsg = "Error in data source XML file (source must contain one array element)";
      throw XMLParsingException(XMLParsingException::eBAD_STRUCT, errorMsg);
    }

    // start processing list
    isListFound = true;

    // store a copy of my own tag
    FDataTag* myTag = myCurrTag;

    // initialize my own tag
    if (myTag == NULL) {
      myTag = new FDataTag();
    }

    // create my data handle & insert in the data tree
    myCurrListDH = myNode->createList( *myTag);
    if (!myTag->isEmpty())
      myNode->insertData( myCurrListDH );

    // create tag for first child
    myCurrTag = new FDataTag(*myTag, 0, false);

    // check data ID attribute
    string currListDataID = "";
    for (XMLSize_t i = 0; i < attrs.getLength(); i++) {
      char * name  = XTOC(attrs.getQName(i));
      char * value = XTOC(attrs.getValue(i));
      if (!strcmp(name,"dataId")) {
        currListDataID = value;
      }
      XREL(name);
      XREL(value);
    }

    // update data ID
    if (!currListDataID.empty()) {
      myNode->setDataID( myCurrListDH, myCurrItemDataID);
    }

    // delete temporary
    delete myTag;
  }
}

void
DataSourceHandler::endList() {
  if (isListFound) {
    myCurrTag->getParent();
    if (!myCurrTag->isEmpty())
      myCurrTag->getSuccessor();
  }
}

void
DataSourceHandler::startItem(const   Attributes&     attrs) {
  if (isListFound) {
    // start new item
    isItemFound = true;

    // create and store new DH
    myCurrItemDH = myNode->createData( *myCurrTag );
    myNode->insertData( myCurrItemDH );

    // check attributes
    myCurrItemValue = "";
    myCurrItemDataID = "";
    for (XMLSize_t i = 0; i < attrs.getLength(); i++) {
      char * name  = XTOC(attrs.getQName(i));
      char * value = XTOC(attrs.getValue(i));
      if (!strcmp(name,"value")) {
        myCurrItemValue = value;
      } else if (!strcmp(name,"dataId")) {
        myCurrItemDataID = value;
      }
      XREL(name);
      XREL(value);
    }
  }
}

void
DataSourceHandler::characters (const  XMLCh* const     chars,
                               const  XMLSize_t     length) {
  // note that this test will work only for small chunks of text (only one call
  // of the handler)
  if (isItemFound) {
    char* value = XTOC(chars);
    myCurrItemValue = value;
    XREL(value);
  }
}

void
DataSourceHandler::endItem() {
  if (isItemFound) {
    // update data ID and value
    if (!myCurrItemDataID.empty())
      myNode->setDataID( myCurrItemDH, myCurrItemDataID);
    if (!myCurrItemValue.empty())
      myNode->setDataValue( myCurrItemDH, myCurrItemValue );

    myCurrTag->getSuccessor();
    isItemFound = false;
  }
}

void
DataSourceHandler::startTag(const   Attributes&     attrs) {
  if (isListFound || isItemFound) {
    // check attributes
    string currTagKey;
    string currTagValue;
    for (XMLSize_t i = 0; i < attrs.getLength(); i++) {
      char * name  = XTOC(attrs.getQName(i));
      char * value = XTOC(attrs.getValue(i));
      if (!strcmp(name,"name")) {
        currTagKey = value;
      } else if (!strcmp(name,"value")) {
        currTagValue = value;
      }
      XREL(name);
      XREL(value);
    }
    // get the right current data handle
    FDataHandle* currDH = NULL;
    if (isItemFound)
      currDH = myCurrItemDH;
    else if (isListFound)
      currDH = myCurrListDH;

    // store tag as DH property
    if (currDH)
      currDH->addProperty( currTagKey, currTagValue );
    else {
      INTERNAL_ERROR("Cannot store data tag: no current data",1);
    }
  }
}

void
DataSourceHandler::endTag() {
}

void
DataSourceHandler::fatalError(const SAXParseException& e) {
  string errorMsg = "Error in data source XML file (line "
    + itoa(e.getLineNumber()) + ")";
  throw XMLParsingException(XMLParsingException::eBAD_STRUCT, errorMsg);
}

void
DataSourceHandler::warning(const SAXParseException& e)
{
  string errorMsg = "Warning in data source XML file (line "
    + itoa(e.getLineNumber()) + ")";
  throw XMLParsingException(XMLParsingException::eBAD_STRUCT, errorMsg);
}


/*****************************************************************************/
/*                   CLASS DagWfParser - Static utility methods              */
/*****************************************************************************/

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
 * Check that an attribute is non-empty
 */
void
DagWfParser::checkMandatoryAttr(const string& tagName,
                                const string& attrName,
                                const string& attrValue) {
  if (attrValue.empty())
    throw XMLParsingException(XMLParsingException::eEMPTY_ATTR,
                              "Attribute " + attrName + " of tag " + tagName + " is empty");
}

/**
 * Get the text content of a DOM element (either parsed or non-parsed)
 */
void
DagWfParser::getTextContent(const DOMElement * element, string& buffer) {
  DOMNode * child = element->getFirstChild();
  if ((child != NULL)
      && ((child->getNodeType() == DOMNode::TEXT_NODE)
          || (child->getNodeType() == DOMNode::CDATA_SECTION_NODE))) {
    DOMText * child_elt = (DOMText*) child;
    char *child_content = XMLString::transcode(child_elt->getData());
    buffer = child_content;
    XMLString::release(&child_content);
  }
}

void
DagWfParser::checkLeafElement(const DOMElement * element, const string& tagName) {
  if (element->getFirstChild() != NULL)
    throw XMLParsingException(XMLParsingException::eBAD_STRUCT,
                              "The element '" + tagName +  "' does not accept a child element. \
          May be a </" + tagName + "> is forgotten");
}

/**
 * Trim string
 */
string&
DagWfParser::stringTrim(string& str) {
  string::size_type pos = str.find_last_not_of(' ');
  if(pos != string::npos) {
    str.erase(pos + 1);
    pos = str.find_first_not_of(' ');
    if(pos != string::npos) str.erase(0, pos);
  }
  else str.erase(str.begin(), str.end());
  return str;
}

/**
 * Parse an assignment string (eg 'portA = portB; portC = VOID;')
 */
void
DagWfParser::getPortMap(const string& thenMapStr,
                        map<string,string>& thenMap) throw(XMLParsingException) {
  string::size_type startPos = 0;
  string mapStr = thenMapStr;
  stringTrim(mapStr);
  while (startPos < mapStr.length()) {
    string::size_type sepPos = mapStr.find(";",startPos);
    if (sepPos == string::npos)
      throw XMLParsingException(XMLParsingException::eINVALID_DATA,
                                "missing semi-column at end of assignment in '"
                                + mapStr +"'");
    string::size_type opPos = mapStr.find("=",startPos);
    if (opPos > sepPos)
      throw XMLParsingException(XMLParsingException::eINVALID_DATA,
                                "missing assignment (=) operator in '"
                                + mapStr + "'");
    string left(mapStr.substr(startPos, opPos-startPos));
    string right(mapStr.substr(opPos+1, sepPos-opPos-1));
    thenMap.insert(pair<string,string>(stringTrim(left),stringTrim(right)));
    startPos = sepPos+1;
  }
}
