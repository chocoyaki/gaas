/**
* @file DagWfParser.cc
*
* @brief  DAG Workflow description Reader class implementation
*
* @author  Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)
*          Benjamin ISNARD (Benjamin.Isnard@ens-lyon.fr)
*
* @section Licence
*   |LICENSE|
*/
/****************************************************************************/
/* DAG Workflow description Reader class implementation                     */
/* This class read a textual representation of DAG workflow and return the  */
/* corresponding DAG object                                                 */
/****************************************************************************/


#include <iostream>
#include <string>
#include <fstream>

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
#define XTOC(x) XMLString::transcode(x)  // Use iff x is a XMLCh *
#define CTOX(x) XMLString::transcode(x)  // Use iff x is a char *
#define XREL(x) XMLString::release(&x)
#endif

using namespace std;
using namespace events;
#undef interface

/**
 * XML Parsing errors description
 */
std::string
XMLParsingException::ErrorMsg() {
  std::string errorMsg;
  switch (Type()) {
  case eUNKNOWN:
    errorMsg = "UNDEFINED ERROR (" + Info() + ")";
    break;
  case eBAD_STRUCT :
    errorMsg = "BAD XML STRUCTURE (" + Info() + ")";
    break;
  case eEMPTY_ATTR :
    errorMsg = "MISSING DATA (" + Info() + ")";
    break;
  case eUNKNOWN_TAG :
    errorMsg = "UNKNOWN XML TAG ("+ Info() + ")";
    break;
  case eUNKNOWN_ATTR :
    errorMsg = "UNKNOWN XML ATTRIBUTE (" + Info() + ")";
    break;
  case eINVALID_REF :
    errorMsg = "INVALID REFERENCE (" + Info() + ")";
    break;
  case eINVALID_DATA :
    errorMsg = "INVALID DATA (" + Info() + ")";
    break;
  case eFILENOTFOUND :
    errorMsg = "FILE NOT FOUND (" + Info() + ")";
    break;
  }
  return errorMsg;
}

/**
 * Error handler for workflow parser
 */
bool
MyDOMErrorHandler::handleError(const DOMError &domError) {
  char* errorMsg = XTOC(domError.getMessage());
  WARNING("XML Error: " << errorMsg);
  XREL(errorMsg);
  if (domError.getSeverity() == DOMError::DOM_SEVERITY_WARNING) {
    return true;
  } else {
    // exceptions cannot be thrown here - parser *should* stop if false
    return false;
  }
}

/**
 * Constructor
 */
DagWfParser::DagWfParser() : content() {
}
DagWfParser::DagWfParser(const char * wf_desc) : content(wf_desc) {
}
DagWfParser::DagWfParser(const std::string& fileName)
  : content(), myXmlFileName(fileName) {
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
  DOMImplementation *impl =
    DOMImplementationRegistry::getDOMImplementation(gLS);
  DOMLSParser *parser =
    impl->createLSParser(DOMImplementationLS::MODE_SYNCHRONOUS, 0);
  MyDOMErrorHandler* errHandler = new MyDOMErrorHandler();

  // Validation
  if (parser->getDomConfig()->canSetParameter(XMLUni::fgDOMValidateIfSchema,
                                              true)) {
    TRACE_TEXT(TRACE_ALL_STEPS, "Activating XML Validation" << endl);
    parser->getDomConfig()->setParameter(XMLUni::fgDOMValidateIfSchema, true);
    parser->getDomConfig()->setParameter(XMLUni::fgXercesValidationErrorAsFatal,
                                         true);
  } else {
    TRACE_TEXT(TRACE_ALL_STEPS, "XML Validation cannot be activated" << endl);
  }

  // Error handler
  if (parser->getDomConfig()->canSetParameter(XMLUni::fgDOMErrorHandler,
                                              errHandler)) {
    TRACE_TEXT(TRACE_ALL_STEPS, "Activating XML Error handler" << endl);
    parser->getDomConfig()->setParameter(XMLUni::fgDOMErrorHandler, errHandler);
  } else {
    TRACE_TEXT(TRACE_ALL_STEPS, "XML Error handler cannot be activated" << endl);
  }

  // Wrapper
  Wrapper4InputSource * wrapper;
  std::string errorMsgPfx;
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
    throw XMLParsingException(XMLParsingException::eUNKNOWN, "Empty XML filename");
  }

  // PARSE
  TRACE_TEXT(TRACE_ALL_STEPS, "PARSING XML START" << endl);
  try {
    this->document = parser->parse((DOMLSInput*) wrapper);
  } catch (...) {
    WARNING(errorMsgPfx << "Unexpected exception during XML Parsing");
    throw XMLParsingException(XMLParsingException::eUNKNOWN, "");
  }

  if (document == NULL) {
    throw XMLParsingException(XMLParsingException::eFILENOTFOUND,
                              myXmlFileName);
  }

  // Check if DTD was provided
  if (checkValid && !document->getDoctype()) {
    WARNING(errorMsgPfx << "XML is not validated (no DTD provided)\n"
            << "Use <!DOCTYPE workflow SYSTEM "
            << "\"[DIET_INSTALL_DIR]/share/diet/FWorkflow.dtd\">"
            << " instruction to provide it");
  }

  DOMNode * root = (DOMNode*)(document->getDocumentElement());
  if (root == NULL) {
    throw XMLParsingException(XMLParsingException::eUNKNOWN,
                              "No details available");
  }

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
DagWfParser::parseNode(const DOMElement * element,
                       const std::string& elementName) {
  // parse the node start element and its attributes
  WfNode * newNode = this->createNode(element, elementName);
  // parse the node sub-elements
  DOMNode * child = element->getFirstChild();
  unsigned int lastArg = 0;
  while (child != NULL) {
    if (child->getNodeType() == DOMNode::ELEMENT_NODE) {
      DOMElement * child_elt = (DOMElement*) child;
      char *child_name_str = XMLString::transcode(child_elt->getNodeName());
      std::string child_name(child_name_str);
      XMLString::release(&child_name_str);

      if (child_name == "arg") {
        parseArg(child_elt, lastArg++, newNode);
      } else if (child_name == "in") {
        parseIn(child_elt, lastArg++, newNode);
      } else if (child_name == "inOut") {
        parseInOut(child_elt, lastArg++, newNode);
      } else if (child_name == "out") {
        parseOut(child_elt, lastArg++, newNode);
      } else {
        parseOtherNodeSubElt(child_elt, child_name, lastArg, newNode);
      }
    }
    child = child->getNextSibling();
  }  // end while
}

/**
 * Parse an argument element
 */
WfPort *
DagWfParser::parseArg(const DOMElement * element, unsigned int lastArg,
                      WfNode* node) {
  std::string name  = getAttributeValue("name", element);
  std::string value = getAttributeValue("value", element);
  std::string type  = getAttributeValue("type", element);
  std::string depth   = getAttributeValue("depth", element);
  checkMandatoryAttr("node", "name", name);
  checkMandatoryAttr("node", "value", value);
  checkLeafElement(element, "arg");
  WfPort *port;
  if (!WfCst::isMatrixType(type)) {
    port = createPort(WfPort::PORT_ARG, name, type, depth, lastArg, node);
  } else {
    port = createMatrixPort(element, WfPort::PORT_ARG, name, lastArg, node);
  }
  // set the value
  DagNodeArgPort * dagPort = dynamic_cast<DagNodeArgPort*>(port);
  if (dagPort) {
    dagPort->setValue(value);
  } else {
    throw XMLParsingException(XMLParsingException::eBAD_STRUCT,
                              "value attribute not valid for other ports than <arg> port");
  }

  return port;
}

/**
 * Parse an input port element
 */
WfPort *
DagWfParser::parseIn(const DOMElement * element, unsigned int lastArg,
                     WfNode* node) {
  std::string name    = getAttributeValue("name", element);
  std::string type    = getAttributeValue("type", element);
  std::string source  = getAttributeValue("source", element);
  std::string interface  = getAttributeValue("interface", element);
  std::string depth   = getAttributeValue("depth", element);
  checkMandatoryAttr("in", "name", name);
  checkMandatoryAttr("in", "type", name);
  checkLeafElement(element, "in");
  WfPort *port;
  if (!WfCst::isMatrixType(type)) {
    port = createPort(WfPort::PORT_IN, name, type, depth, lastArg, node);
  } else {
    port = createMatrixPort(element, WfPort::PORT_IN, name, lastArg, node);
  }
  if (!source.empty()) {
    port->setConnectionRef(source);
  }
  if (!interface.empty()) {
    port->setInterfaceRef(interface);
  }

  return port;
}

/**
 * parse InOut port element
 */
WfPort *
DagWfParser::parseInOut(const DOMElement * element, unsigned int lastArg,
                        WfNode* node) {
  std::string name    = getAttributeValue("name", element);
  std::string type    = getAttributeValue("type", element);
  std::string source  = getAttributeValue("source", element);
  std::string depth   = getAttributeValue("depth", element);
  checkMandatoryAttr("inOut", "name", name);
  checkMandatoryAttr("inOut", "type", name);
  checkLeafElement(element, "inOut");
  WfPort *port;
  if (!WfCst::isMatrixType(type)) {
    port = createPort(WfPort::PORT_INOUT, name, type, depth, lastArg, node);
  } else {
    port = createMatrixPort(element, WfPort::PORT_INOUT, name, lastArg, node);
  }
  if (!source.empty()) {
    port->setConnectionRef(source);
  }

  return port;
}

/**
 * Parse Out port element
 */
WfPort *
DagWfParser::parseOut(const DOMElement * element, unsigned int lastArg,
                      WfNode* node) {
  std::string name  = getAttributeValue("name", element);
  std::string type  = getAttributeValue("type", element);
  std::string sink  = getAttributeValue("sink", element);
  std::string interface  = getAttributeValue("interface", element);
  std::string depth = getAttributeValue("depth", element);
  std::string dataId = getAttributeValue("dataId", element);
  checkMandatoryAttr("out", "name", name);
  checkMandatoryAttr("out", "type", name);
  checkLeafElement(element, "out");
  WfPort *port;
  if (!WfCst::isMatrixType(type)) {
    port = createPort(WfPort::PORT_OUT, name, type, depth, lastArg, node);
  } else {
    port = createMatrixPort(element, WfPort::PORT_OUT, name, lastArg, node);
  }
  if (!sink.empty()) {
    port->setConnectionRef(sink);
  }
  if (!interface.empty()) {
    port->setInterfaceRef(interface);
  }
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
DagWfParser::createPort(const WfPort::WfPortType param_type,
                        const std::string& name,
                        const std::string& type,
                        const std::string& depth,
                        unsigned int lastArg,
                        WfNode * node) {
  // Get the base type and the depth of the list structure (syntax 'LIST(LIST(<basetype>))')
  unsigned int typeDepth = 0;
  std::string curType = type;
  std::string::size_type listPos = curType.find("LIST");
  while (listPos != std::string::npos) {
    curType = curType.substr(listPos+5, curType.length() - 6);
    listPos = curType.find("LIST");
    ++typeDepth;
  }
  short baseType = WfCst::cvtStrToWfType(curType);
  if (baseType == WfCst::TYPE_UNKNOWN) {
    throw XMLParsingException(XMLParsingException::eINVALID_DATA,
                              "Unknown data type : "+type);
  }

  // Use depth attribute (second syntax)
  if (!depth.empty()) {
    typeDepth = atoi(depth.c_str());
  }

  TRACE_TEXT(TRACE_ALL_STEPS, "Creating new port '" << name
             << "' (base type=" << curType
             << " , depth=" << typeDepth << " , idx=" << lastArg
             << ")" << endl);

  // Initialize the profile with the appropriate parameter type
  // (and optionnally value: used only for Arg ports)

  WfPort *port;
  try {
    port = node->newPort(name, lastArg, param_type,
                         (WfCst::WfDataType) baseType, typeDepth);
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
DagWfParser::createMatrixPort(const DOMElement * element,
                               const WfPort::WfPortType param_type,
                               const std::string& name,
                               unsigned int lastArg,
                               WfNode * node) {
  std::string elt_type_str = getAttributeValue("base_type", element);
  std::string nb_rows_str = getAttributeValue("nb_rows", element);
  std::string nb_cols_str = getAttributeValue("nb_cols", element);
  std::string matrix_order_str = getAttributeValue("matrix_order", element);
  checkMandatoryAttr("(matrix port)", "base_type", elt_type_str);
  checkMandatoryAttr("(matrix port)", "nb_rows", nb_rows_str);
  checkMandatoryAttr("(matrix port)", "nb_cols", nb_cols_str);
  checkMandatoryAttr("(matrix port)", "matrix_order", matrix_order_str);

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
DagParser::DagParser(const std::string& xmlFileName)
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
  if (strcmp(rootNodeName, "dag")) {
    throw XMLParsingException(XMLParsingException::eUNKNOWN_TAG,
                              "XML for DAG should begin with <dag>");
  }

  XREL(rootNodeName);
  DOMNode * child = root->getFirstChild();
  while ((child != NULL)) {
    // Parse all the <node> elements
    if (child->getNodeType() == DOMNode::ELEMENT_NODE) {
      DOMElement * child_elt = (DOMElement*)child;
      char * _nodeName = XTOC(child_elt->getNodeName());
      std::string nodeName(_nodeName);
      XREL(_nodeName);
      TRACE_TEXT(TRACE_ALL_STEPS,
                  "Parsing the element " << nodeName << endl);
      if (nodeName != "node") {
        throw XMLParsingException(XMLParsingException::eUNKNOWN_TAG,
                                  "A dag should only contain <node> elements");
      }
      this->parseNode(child_elt, nodeName);
    }
    child = child->getNextSibling();
  }  // end while
}

WfNode *
DagParser::createNode(const DOMElement* element,
                      const std::string& elementName) {
  if (!myCurrDag) {
    INTERNAL_ERROR(__FUNCTION__ << "Null dag pointer" << endl, 1);
  }
  std::string nodeId(getAttributeValue("id", element));
  std::string pbName = getAttributeValue("path", element);
  std::string estClass = getAttributeValue("est-class", element);
  std::string status = getAttributeValue("status", element);
  checkMandatoryAttr("node", "id", nodeId);
  checkMandatoryAttr("node", "path", pbName);

  DagNode* node = NULL;
  try {
    node = myCurrDag->createDagNode(nodeId);
  } catch (WfStructException& e) {
    throw XMLParsingException(XMLParsingException::eBAD_STRUCT,
                              "Cannot create node : "+e.ErrorMsg());
  }
  node->setPbName(pbName);
  if (!estClass.empty()) {
    node->setEstimationClass(estClass);
  }
  if (!status.empty()) {
    node->setStatus(status);
  }

  return node;
}

void
DagParser::parseOtherNodeSubElt(const DOMElement * element,
                                const std::string& elementName,
                                unsigned int& portIndex,
                                WfNode * node) {
  if (elementName == "prec") {
    parsePrec(element, node);
  } else {
    throw XMLParsingException(XMLParsingException::eUNKNOWN_TAG,
                              "Invalid tag within node (" + elementName + ")");
  }
}

void
DagParser::parsePrec(const DOMElement * element, WfNode* node) {
  std::string precNodeId = getAttributeValue("id", element);
  checkMandatoryAttr("prec", "id", precNodeId);
  checkLeafElement(element, "prec");

  //bug found by Lamiel Toch
  WfNode* precNode = myCurrDag->getNode(precNodeId);

  node->addNodePredecessor(precNode, precNodeId);
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

MultiDagParser::MultiDagParser(const std::string& xmlFileName)
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
  if (strcmp(_rootNodeName, "dags")) {
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
      std::string childName(_childName);
      TRACE_TEXT(TRACE_ALL_STEPS,
                  "Parsing the element " << childName << endl);
      if (strcmp(_childName, "dag")) {
        throw XMLParsingException(XMLParsingException::eUNKNOWN_TAG,
                                  "A MultiDag should only contain <dag> elements");
      }
      XREL(_childName);
      std::string suffix = "." + itoa(dagCounter);
      Dag * currDag = new Dag(myXmlFileName + suffix);
      if (myWorkflow) {
        currDag->setWorkflow(myWorkflow);
        EventManager::getEventMgr()->sendEvent(
          new EventCreateObject<Dag, FWorkflow>(currDag, myWorkflow));
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
FWfParser::FWfParser(FWorkflow& workflow, const std::string& fileName)
  : DagWfParser(fileName), workflow(workflow) {
}

FWfParser::~FWfParser() {
}

std::string
FWfParser::getWfClassName(const std::string& fileName) {
  std::string::size_type lastSlash = fileName.rfind("/");
  std::string::size_type start =
    (lastSlash == std::string::npos) ? 0 : lastSlash+1;
  std::string::size_type suffix = fileName.rfind(".xml");
  std::string::size_type end = (suffix == std::string::npos) ? 0 : suffix-1;
  return fileName.substr(start, end-start+1);
}

/**
 * parse the root node (workflow) and its elements (interface and processor)
 */
void
FWfParser::parseRoot(DOMNode* root) {
  // Check the root node
  char * _rootNodeName = XMLString::transcode(root->getNodeName());
  if (strcmp(_rootNodeName, "workflow")) {
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
      std::string nodeName(_nodeName);
      XMLString::release(&_nodeName);
      TRACE_TEXT(TRACE_ALL_STEPS,
                  "Parsing the element " << nodeName << endl);
      if (nodeName == "include") {
        std::string fileName = getAttributeValue("file", child_elt);
        // store class=>filename mapping
        std::string className = getWfClassName(fileName);
        if (!className.empty()) {
          TRACE_TEXT(TRACE_ALL_STEPS,
                      "Include workflow class : " << className << endl);
          myClassFiles[className] = fileName;
        }
      } else if (nodeName == "interface") {
        // Parse the interface
        // sub-element of <interface>
        DOMNode * child2 = child->getFirstChild();
        while ((child2 != NULL)) {
          if (child2->getNodeType() == DOMNode::ELEMENT_NODE) {
            DOMElement * child_elt2 = (DOMElement*)child2;
            char * _nodeName2 = XMLString::transcode(child_elt2->getNodeName());
            std::string nodeName2(_nodeName2);
            XMLString::release(&_nodeName2);
            TRACE_TEXT(TRACE_ALL_STEPS,
                        "Parsing the element " << nodeName2 << endl);
            parseNode(child_elt2, nodeName2);
          }
          child2 = child2->getNextSibling();
        }  // end while
      } else if (nodeName == "processors") {
        // Parse the processors
        // sub-element of <processors>
        DOMNode * child2 = child->getFirstChild();
        while ((child2 != NULL)) {
          if (child2->getNodeType() == DOMNode::ELEMENT_NODE) {
            DOMElement * child_elt2 = (DOMElement*)child2;
            char * _nodeName2 = XMLString::transcode(child_elt2->getNodeName());
            std::string nodeName2(_nodeName2);
            XMLString::release(&_nodeName2);
            TRACE_TEXT(TRACE_ALL_STEPS,
                        "Parsing the element " << nodeName2 << endl);
            parseNode(child_elt2, nodeName2);
          }
          child2 = child2->getNextSibling();
        }  // end while
      } else if (nodeName == "links") {
        // Parse the links
        DOMNode * child2 = child->getFirstChild();  // sub-element of <links>
        while ((child2 != NULL)) {
          if (child2->getNodeType() == DOMNode::ELEMENT_NODE) {
            DOMElement * child_elt2 = (DOMElement*)child2;
            char * _nodeName2 = XMLString::transcode(child_elt2->getNodeName());
            std::string nodeName2(_nodeName2);
            XMLString::release(&_nodeName2);
            TRACE_TEXT(TRACE_ALL_STEPS,
                        "Parsing the element " << nodeName2 << endl);
            parseLink(child_elt2);
          }
          child2 = child2->getNextSibling();
        }  // end while
      } else {
        throw XMLParsingException(XMLParsingException::eUNKNOWN_TAG,
                                  "Invalid element within <workflow> element ("
                                  + nodeName + ")");
      }
    }
    child = child->getNextSibling();
  }  // end while
}

/**
 * Workflow node creation for functional wf
 */
WfNode *
FWfParser::createNode(const DOMElement* element,
                      const std::string& elementName) {
  std::string name = getAttributeValue("name", element);
  std::string type = getAttributeValue("type", element);
  std::string depth = getAttributeValue("depth", element);
  std::string value = getAttributeValue("value", element);
  std::string nclass = getAttributeValue("class", element);
  checkMandatoryAttr(elementName, "name", name);

  // Convert type
  short dataType = 0;
  if (!type.empty()) {
    dataType = WfCst::cvtStrToWfType(type);
    if (dataType == WfCst::TYPE_UNKNOWN) {
      throw XMLParsingException(XMLParsingException::eINVALID_DATA,
                                "Unknown data type : "+type);
    }
  }
  // Convert depth
  unsigned int typeDepth = 0;
  if (!depth.empty()) {
    typeDepth = atoi(depth.c_str());
  }
  // Create node depending on element name
  WfNode * node;
  if (elementName == "source") {
    checkMandatoryAttr(elementName, "type", type);
    node = workflow.createSource(name, (WfCst::WfDataType) dataType);

  } else if (elementName == "constant") {
    string dataID = getAttributeValue("dataId", element);
    checkMandatoryAttr(elementName, "type", type);
    if (!depth.empty()) {
      throw XMLParsingException(XMLParsingException::eUNKNOWN_ATTR,
                                "Attribute depth is not applicable to constant tag");
    }
    FConstantNode* cstNode =
      workflow.createConstant(name, (WfCst::WfDataType) dataType);
    if (!value.empty()) {
      cstNode->setValue(value);
    }
    if (!dataID.empty()) {
      cstNode->setDataID(dataID);
    }
    node = (WfNode*) cstNode;

  } else if (elementName == "sink") {
    checkMandatoryAttr(elementName, "type", type);
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
    checkMandatoryAttr(elementName, "class", nclass);
    std::map<std::string, std::string>::iterator classIter =
      myClassFiles.find(nclass);
    if (classIter == myClassFiles.end()) {
      throw XMLParsingException(XMLParsingException::eINVALID_REF,
                                "Unknown workflow class (missing include) : "
                                + nclass);
    }

    // Create sub-workflow and parse the workflow XML
    std::string xmlWfFileName = (std::string) classIter->second;
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
  if (node == NULL) {
    throw XMLParsingException(XMLParsingException::eINVALID_REF,
                              "Duplicate node id : " + name);
  }

  return node;
}

/**
 * Override methods for port parsing
 */

WfPort *
FWfParser::parsePortCommon(const DOMElement * element,
                           const unsigned int portIndex,
                           WfNode * node,
                           const std::string& portName,
                           const WfPort::WfPortType portType) {
  std::string name  = getAttributeValue("name", element);
  std::string type  = getAttributeValue("type", element);
  std::string depth = getAttributeValue("depth", element);
  checkMandatoryAttr(portName, "name", name);
  checkMandatoryAttr(portName, "type", name);
  checkLeafElement(element, portName);

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
  WfPort *port = DagWfParser::parseIn(element, lastArg, node);
  parseCardAttr(element, port);
  return port;
}

WfPort*
FWfParser::parseOut(const DOMElement * element,
                    const unsigned int lastArg,
                    WfNode * node) {
  WfPort *port = DagWfParser::parseOut(element, lastArg, node);
  parseCardAttr(element, port);
  return port;
}

WfPort*
FWfParser::parseInOut(const DOMElement * element,
                      const unsigned int lastArg,
                      WfNode * node) {
  WfPort *port = DagWfParser::parseInOut(element, lastArg, node);
  parseCardAttr(element, port);
  return port;
}

/**
 * Parse Param port element (only for functional wf)
 */
WfPort *
FWfParser::parseParamPort(const DOMElement * element,
                          const unsigned int lastArg,
                          WfNode* node) {
  std::string name  = getAttributeValue("name", element);
  std::string type  = getAttributeValue("type", element);
  checkMandatoryAttr("param", "name", name);
  checkMandatoryAttr("param", "type", name);
  checkLeafElement(element, "param");

  if (WfCst::isMatrixType(type)) {
    throw XMLParsingException(XMLParsingException::eBAD_STRUCT,
                              "param port cannot have a matrix type");
  }

  return createPort(WfPort::PORT_PARAM, name, type, "0", lastArg, node);
}

/**
 * Parse the 'card' attribute (the cardinal of containers)
 */
void
FWfParser::parseCardAttr(const DOMElement * element, WfPort* port) {
  const XMLCh * strCard  = element->getAttribute(XMLString::transcode("card"));
  // parse the semi-column separated list
  XMLStringTokenizer *tkizer =
    new XMLStringTokenizer(strCard, XMLString::transcode(";"));
  list<std::string> * tklist = new list<std::string>();
  while (tkizer->hasMoreTokens()) {
    const char *item = XMLString::transcode(tkizer->nextToken());
    tklist->push_back(item);
    TRACE_TEXT(TRACE_ALL_STEPS, "Parsed cardinal item=" << item << endl);
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
  std::string from = getAttributeValue("from", element);
  std::string to = getAttributeValue("to", element);
  std::string fromNodeName, toNodeName, fromPortName, toPortName;
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
FWfParser::parseLinkRef(const std::string& strRef,
                        std::string& nodeName,
                        std::string& portName) {
  FNode *node;
  std::string::size_type nodeSep = strRef.find(":");
  if (nodeSep != std::string::npos) {  // for processor nodes
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
std::vector<std::string>*
FWfParser::parseIterationStrategy(const DOMElement * element,
                                  FProcNode* procNode) {
  const DOMNode * child = element->getFirstChild();
  std::vector<std::string>* inputIds = new std::vector<std::string>();
  while (child != NULL) {
    if (child->getNodeType() == DOMNode::ELEMENT_NODE) {
      const DOMElement * child_elt = (DOMElement*) child;
      char *child_name_str = XMLString::transcode(child_elt->getNodeName());
      std::string child_name(child_name_str);
      XMLString::release(&child_name_str);

      if (child_name == "port") {
        std::string portName = getAttributeValue("name", child_elt);
        checkMandatoryAttr("port", "name", portName);
        inputIds->push_back(portName);
      } else {
        if (child_name == "match") {
          std::vector<string>* opInputIds = parseIterationStrategy(child_elt, procNode);
          if (opInputIds->size() > 2) {
            throw XMLParsingException(XMLParsingException::eBAD_STRUCT,
                                      "MATCH iterator only accepts 2 input ports");
          }

          const string& opId =
            procNode->createInputOperator(FProcNode::OPER_MATCH, *opInputIds);
          inputIds->push_back(opId);
          delete opInputIds;
        } else {
          if (child_name == "cross") {
            std::vector<std::string>* opInputIds =
              parseIterationStrategy(child_elt, procNode);
            const std::string& opId =
              procNode->createInputOperator(FProcNode::OPER_CROSS, *opInputIds);
            inputIds->push_back(opId);
            delete opInputIds;
          } else {
            if (child_name == "flatcross") {
              std::vector<std::string>* opInputIds =
                parseIterationStrategy(child_elt, procNode);
              const std::string& opId =
                procNode->createInputOperator(FProcNode::OPER_FLATCROSS,
                                              *opInputIds);
              inputIds->push_back(opId);
              delete opInputIds;
            } else {
              if (child_name == "dot") {
                std::vector<std::string>* opInputIds =
                  parseIterationStrategy(child_elt, procNode);
                const std::string& opId =
                  procNode->createInputOperator(FProcNode::OPER_DOT,
                                                *opInputIds);
                inputIds->push_back(opId);
                delete opInputIds;
              }
            }
          }
        }
      }
    }  // end if (child->getNodeType() == DOMNode::ELEMENT_NODE)
    child = child->getNextSibling();
  }  // end while
  return inputIds;
}

/**
 * Parse the derived class specific node elements
 */
void
FWfParser::parseOtherNodeSubElt(const DOMElement * element,
                                const std::string& elementName,
                                unsigned int& portIndex,
                                WfNode * node) {
  // Diet service
  if (elementName == "diet") {
    FActivityNode* aNode = dynamic_cast<FActivityNode*>(node);
    if (!aNode) {
      throw XMLParsingException(XMLParsingException::eBAD_STRUCT,
                                "<diet> element applied to non-activity element");
    }

    // PATH attribute
    std::string path = getAttributeValue("path", element);
    checkMandatoryAttr("diet", "path", path);
    aNode->checkDynamicParam("path", path);
    aNode->setDIETServicePath(path);
    // MAX-INSTANCES attribute
    std::string maxInstStr = getAttributeValue("max-instances", element);
    if (!maxInstStr.empty()) {
      aNode->setMaxInstancePerDag(atoi(maxInstStr.c_str()));
    }
    // ESTIMATION attribute
    std::string estimAttr = getAttributeValue("estimation", element);
    if (!estimAttr.empty()) {
      aNode->setDIETEstimationOption(estimAttr);
    }
  } else if ((elementName == "beanshell") || (elementName == "gasw")) {
    // tags used by other workflow engines
    // VALUE of constants (may be replaced by a 'value' attribute within
    //  the <constant> tag)
  } else if (elementName == "value") {
    FConstantNode* cstNode = dynamic_cast<FConstantNode*>(node);
    if (!cstNode) {
      throw XMLParsingException(XMLParsingException::eBAD_STRUCT,
                                "<value> element defined outside a <constant>");
    }

    std::string value;
    getTextContent(element, value);
    cstNode->setValue(value);
  } else if (elementName == "iterationstrategy") {
    FProcNode* procNode = dynamic_cast<FProcNode*>(node);
    if (!procNode) {
      throw XMLParsingException(XMLParsingException::eBAD_STRUCT,
                                "<iterationstrategy> element applied to non-processor element");
    }

    std::vector<std::string>* opInputId = parseIterationStrategy(element, procNode);
    if (opInputId->size() != 1) {
      throw XMLParsingException(XMLParsingException::eBAD_STRUCT,
                                "<iterationstrategy> contains more than one root operator");
    }

    procNode->setRootInputOperator((*opInputId)[0]);
    delete opInputId;
  } else if (elementName == "param") {
    parseParamPort(element, portIndex++, node);
  } else if (elementName == "if") {
    FIfNode* ifNode = dynamic_cast<FIfNode*>(node);
    if (!ifNode) {
      throw XMLParsingException(XMLParsingException::eBAD_STRUCT,
                                "<if> element applied to non-conditional element");
    }

    std::string condition;
    getTextContent(element, condition);
    try {
      ifNode->setCondition(condition);
    } catch (WfStructException& e) {
      throw XMLParsingException(XMLParsingException::eBAD_STRUCT,
                                "<if> element contains invalid expression");
    }
  } else if (elementName == "outThen") {
    FIfNode* ifNode = dynamic_cast<FIfNode*>(node);
    if (!ifNode) {
      throw XMLParsingException(XMLParsingException::eBAD_STRUCT,
                                "<outThen> element applied to non-conditional element");
    }

    parsePortCommon(element, portIndex++, ifNode, "outThen", WfPort::PORT_OUT_THEN);
  } else if (elementName == "outElse") {
    FIfNode* ifNode = dynamic_cast<FIfNode*>(node);
    if (!ifNode) {
      throw XMLParsingException(XMLParsingException::eBAD_STRUCT,
                                "<outElse> element applied to non-conditional element");
    }

    parsePortCommon(element, portIndex++, ifNode, "outElse", WfPort::PORT_OUT_ELSE);
  } else if (elementName == "then") {
    FIfNode* ifNode = dynamic_cast<FIfNode*>(node);
    if (!ifNode) {
      throw XMLParsingException(XMLParsingException::eBAD_STRUCT,
                                "<then> element applied to non-conditional element");
    }

    std::string thenMapStr;
    getTextContent(element, thenMapStr);
    std::map<std::string, std::string>  thenMap;
    getPortMap(thenMapStr, thenMap);
    for (std::map<std::string, std::string>::iterator mapIter = thenMap.begin();
         mapIter != thenMap.end();
         ++mapIter) {
      ifNode->setThenMap(mapIter->first, mapIter->second);
    }
  } else if (elementName == "else") {
    FIfNode* ifNode = dynamic_cast<FIfNode*>(node);
    if (!ifNode) {
      throw XMLParsingException(XMLParsingException::eBAD_STRUCT,
                                "<else> element applied to non-conditional element");
    }

    std::string elseMapStr;
    getTextContent(element, elseMapStr);
    std::map<std::string, std::string>  elseMap;
    getPortMap(elseMapStr, elseMap);
    for (std::map<std::string, std::string>::iterator mapIter = elseMap.begin();
         mapIter != elseMap.end();
         ++mapIter) {
      ifNode->setElseMap(mapIter->first, mapIter->second);
    }
  } else if (elementName == "while") {
    FLoopNode* loopNode = dynamic_cast<FLoopNode*>(node);
    if (!loopNode) {
      throw XMLParsingException(XMLParsingException::eBAD_STRUCT,
                                "<while> element applied to non-loop element");
    }

    std::string condition;
    getTextContent(element, condition);
    try {
      loopNode->setWhileCondition(condition);
    } catch (WfStructException& e) {
      throw XMLParsingException(XMLParsingException::eBAD_STRUCT,
                                "<while> element contains invalid expression");
    }
  } else if (elementName == "inLoop") {
    FLoopNode* loopNode = dynamic_cast<FLoopNode*>(node);
    if (!loopNode) {
      throw XMLParsingException(XMLParsingException::eBAD_STRUCT,
                                "<inLoop> element applied to non-loop element");
    }

    WfPort *lPort = parsePortCommon(element, portIndex++, loopNode,
                                    "inLoop", WfPort::PORT_IN_LOOP);
    // INIT attribute
    std::string init = getAttributeValue("init", element);
    checkMandatoryAttr("inLoop", "init", init);
    lPort->setInterfaceRef(init);
  } else if (elementName == "outLoop") {
    FLoopNode* loopNode = dynamic_cast<FLoopNode*>(node);
    if (!loopNode) {
      throw XMLParsingException(XMLParsingException::eBAD_STRUCT,
                                "<outLoop> element applied to non-loop element");
    }

    WfPort *lPort = parsePortCommon(element, portIndex++, loopNode,
                                    "outLoop", WfPort::PORT_OUT_LOOP);
    // FINAL attribute
    std::string final = getAttributeValue("final", element);
    checkMandatoryAttr("inLoop", "final", final);
    lPort->setInterfaceRef(final);
  } else if (elementName == "do") {
    FLoopNode* loopNode = dynamic_cast<FLoopNode*>(node);
    if (!loopNode) {
      throw XMLParsingException(XMLParsingException::eBAD_STRUCT,
                                "<do> element applied to non-loop element");
    }

    std::string doMapStr;
    getTextContent(element, doMapStr);
    std::map<std::string, std::string>  doMap;
    getPortMap(doMapStr, doMap);
    for (std::map<std::string, std::string>::iterator mapIter = doMap.begin();
         mapIter != doMap.end();
         ++mapIter) {
      loopNode->setDoMap(mapIter->first, mapIter->second);
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
DataSourceParser::parseXml(const std::string& dataFileName)
  throw(XMLParsingException) {
  //   const XMLCh gLS[] = { chLatin_L, chLatin_S, chNull };

  TRACE_TEXT(TRACE_ALL_STEPS, "PARSING XML START" << endl);

  if (dataFileName.empty()) {
    throw XMLParsingException(XMLParsingException::eINVALID_DATA,
                              "Empty XML File name");
  }

  struct stat buffer;
  if (stat(dataFileName.c_str(), &buffer)) {
    throw XMLParsingException(XMLParsingException::eFILENOTFOUND,
                              "XML File '" + dataFileName + "' does not exist");
  }


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
                                const   Attributes&     attrs) {
  char* eltName = XTOC(qname);

  if (strcmp("source", eltName) == 0) {
    startSource(attrs);
  }
  if (strcmp("array", eltName) == 0) {
    startList(attrs);
  }
  if (strcmp("list", eltName) == 0) {
    startList(attrs);  // obsolete
  }
  if (strcmp("item", eltName) == 0) {
    startItem(attrs);
  }
  if (strcmp("tag", eltName) == 0) {
    startTag(attrs);
  }

  XREL(eltName);
}

void
DataSourceHandler::endElement(const   XMLCh* const    uri,
                              const   XMLCh* const    localname,
                              const   XMLCh* const    qname) {
  char* eltName = XTOC(qname);

  if (strcmp("source", eltName) == 0) {
    endSource();
  }
  if (strcmp("array", eltName) == 0) {
    endList();
  }
  if (strcmp("list", eltName) == 0) {
    endList();  // obsolete
  }
  if (strcmp("item", eltName) == 0) {
    endItem();
  }
  if (strcmp("tag", eltName) == 0) {
    endTag();
  }

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
      std::string errorMsg =
        "Error in data source XML file (source must contain one array element)";
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
    myCurrListDH = myNode->createList(*myTag);
    if (!myTag->isEmpty()) {
      myNode->insertData(myCurrListDH);
    }

    // create tag for first child
    myCurrTag = new FDataTag(*myTag, 0, false);

    // check data ID attribute
    std::string currListDataID = "";
    for (XMLSize_t i = 0; i < attrs.getLength(); i++) {
      char * name  = XTOC(attrs.getQName(i));
      char * value = XTOC(attrs.getValue(i));
      if (!strcmp(name, "dataId")) {
        currListDataID = value;
      }
      XREL(name);
      XREL(value);
    }

    // update data ID
    if (!currListDataID.empty()) {
      myNode->setDataID(myCurrListDH, myCurrItemDataID);
    }

    // delete temporary
    delete myTag;
  }
}

void
DataSourceHandler::endList() {
  if (isListFound) {
    myCurrTag->getParent();
    if (!myCurrTag->isEmpty()) {
      myCurrTag->getSuccessor();
    }
  }
}

void
DataSourceHandler::startItem(const   Attributes&     attrs) {
  if (isListFound) {
    // start new item
    isItemFound = true;

    // create and store new DH
    myCurrItemDH = myNode->createData(*myCurrTag);
    myNode->insertData(myCurrItemDH);

    // check attributes
    myCurrItemValue = "";
    myCurrItemDataID = "";
    for (XMLSize_t i = 0; i < attrs.getLength(); i++) {
      char * name  = XTOC(attrs.getQName(i));
      char * value = XTOC(attrs.getValue(i));
      if (!strcmp(name, "value")) {
        myCurrItemValue = value;
      } else if (!strcmp(name, "dataId")) {
        myCurrItemDataID = value;
      }
      XREL(name);
      XREL(value);
    }
  }
}

void
DataSourceHandler::characters(const  XMLCh* const     chars,
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
    if (!myCurrItemDataID.empty()) {
      myNode->setDataID(myCurrItemDH, myCurrItemDataID);
    }
    if (!myCurrItemValue.empty()) {
      myNode->setDataValue(myCurrItemDH, myCurrItemValue);
    }

    myCurrTag->getSuccessor();
    isItemFound = false;
  }
}

void
DataSourceHandler::startTag(const   Attributes&     attrs) {
  if (isListFound || isItemFound) {
    // check attributes
    std::string currTagKey;
    std::string currTagValue;
    for (XMLSize_t i = 0; i < attrs.getLength(); i++) {
      char * name  = XTOC(attrs.getQName(i));
      char * value = XTOC(attrs.getValue(i));
      if (!strcmp(name, "name")) {
        currTagKey = value;
      } else if (!strcmp(name, "value")) {
        currTagValue = value;
      }
      XREL(name);
      XREL(value);
    }
    // get the right current data handle
    FDataHandle* currDH = NULL;
    if (isItemFound) {
      currDH = myCurrItemDH;
    } else if (isListFound) {
      currDH = myCurrListDH;
    }

    // store tag as DH property
    if (currDH) {
      currDH->addProperty(currTagKey, currTagValue);
    } else {
      INTERNAL_ERROR("Cannot store data tag: no current data", 1);
    }
  }
}

void
DataSourceHandler::endTag() {
}

void
DataSourceHandler::fatalError(const SAXParseException& e) {
  std::string errorMsg = "Error in data source XML file (line "
    + itoa(e.getLineNumber()) + ")";
  throw XMLParsingException(XMLParsingException::eBAD_STRUCT, errorMsg);
}

void
DataSourceHandler::warning(const SAXParseException& e) {
  std::string errorMsg = "Warning in data source XML file (line "
    + itoa(e.getLineNumber()) + ")";
  throw XMLParsingException(XMLParsingException::eBAD_STRUCT, errorMsg);
}


/*****************************************************************************/
/*                   CLASS DagWfParser - Static utility methods              */
/*****************************************************************************/

/**
 * Get the attribute value of a DOM element
 */
std::string
DagWfParser::getAttributeValue(const char * attr_name,
                               const DOMElement * elt) {
  XMLCh * attr = XMLString::transcode(attr_name);
  const XMLCh * value   = elt->getAttribute(attr);
  char  * value_str = XMLString::transcode(value);

  std::string result(value_str);

  XMLString::release(&value_str);
  XMLString::release(&attr);

  return result;
}

/**
 * Check that an attribute is non-empty
 */
void
DagWfParser::checkMandatoryAttr(const std::string& tagName,
                                const std::string& attrName,
                                const std::string& attrValue) {
  if (attrValue.empty()) {
    throw XMLParsingException(XMLParsingException::eEMPTY_ATTR,
                              "Attribute " + attrName + " of tag "
                              + tagName + " is empty");
  }
}

/**
 * Get the text content of a DOM element (either parsed or non-parsed)
 */
void
DagWfParser::getTextContent(const DOMElement * element, std::string& buffer) {
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
DagWfParser::checkLeafElement(const DOMElement * element,
                              const std::string& tagName) {
  if (element->getFirstChild() != NULL) {
    throw XMLParsingException(XMLParsingException::eBAD_STRUCT,
                              "The element '" + tagName
                              +  "' does not accept a child element. \
          May be a </" + tagName + "> is forgotten");
  }
}

/**
 * Trim string
 */
std::string&
DagWfParser::stringTrim(std::string& str) {
  std::string::size_type pos = str.find_last_not_of(' ');
  if (pos != std::string::npos) {
    str.erase(pos + 1);
    pos = str.find_first_not_of(' ');
    if (pos != std::string::npos) {
      str.erase(0, pos);
    }
  } else {
    str.erase(str.begin(), str.end());
  }

  return str;
}

/**
 * Parse an assignment string (eg 'portA = portB; portC = VOID;')
 */
void
DagWfParser::getPortMap(const std::string& thenMapStr,
                        std::map<std::string, std::string>& thenMap)
  throw(XMLParsingException) {
  std::string::size_type startPos = 0;
  std::string mapStr = thenMapStr;
  stringTrim(mapStr);
  while (startPos < mapStr.length()) {
    std::string::size_type sepPos = mapStr.find(";", startPos);
    if (sepPos == std::string::npos) {
      throw XMLParsingException(XMLParsingException::eINVALID_DATA,
                                "missing semi-column at end of assignment in '"
                                + mapStr +"'");
    }

    std::string::size_type opPos = mapStr.find("=", startPos);
    if (opPos > sepPos) {
      throw XMLParsingException(XMLParsingException::eINVALID_DATA,
                                "missing assignment (=) operator in '"
                                + mapStr + "'");
    }

    std::string left(mapStr.substr(startPos, opPos-startPos));
    std::string right(mapStr.substr(opPos+1, sepPos-opPos-1));
    thenMap.insert(std::pair<std::string, std::string>(stringTrim(left),
                                                       stringTrim(right)));
    startPos = sepPos+1;
  }
}
