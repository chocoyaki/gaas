/**
* @file DagWfParser.hh
* 
* @brief  DAG Workflow description Reader class implementation 
* 
* @author  Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)
*          Benjamin ISNARD (Benjamin.Isnard@ens-lyon.fr)
* 
* @section Licence
*   |LICENCE|                                                                
*/
/****************************************************************************/
/* DAG Workflow description Reader class description                        */
/* This class read a textual representation of DAG workflow and return the  */
/* corresponding DAG object                                                 */
/****************************************************************************/

#ifndef _DAGWFPARSER_HH_
#define _DAGWFPARSER_HH_

#include <cstring>
#include <cstdlib>
#include <list>
#include <map>
#include <string>
#include <vector>
// Xerces header
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/parsers/AbstractDOMParser.hpp>
#include <xercesc/sax2/DefaultHandler.hpp>
#include <xercesc/dom/DOMImplementation.hpp>
#include <xercesc/dom/DOMImplementationLS.hpp>
#include <xercesc/dom/DOMImplementationRegistry.hpp>
#include <xercesc/dom/DOMException.hpp>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMNodeList.hpp>
#include <xercesc/dom/DOMError.hpp>
#include <xercesc/dom/DOMErrorHandler.hpp>
#include <xercesc/dom/DOMLocator.hpp>
#include <xercesc/dom/DOMNamedNodeMap.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/dom/DOMAttr.hpp>
// Workflow related headers
#include "WfUtils.hh"
#include "WfNode.hh"
class Dag;

// Functional workflow classes
class FWorkflow;
class FNode;
class FSourceNode;
class FProcNode;
class FDataTag;
class FDataHandle;


XERCES_CPP_NAMESPACE_USE
/*****************************************************************************/
/*                        CLASS XMLParsingException                          */
/*****************************************************************************/
class XMLParsingException {
public:
  enum XMLParsingErrorType { eUNKNOWN,
                             eUNKNOWN_TAG,
                             eUNKNOWN_ATTR,
                             eEMPTY_ATTR,
                             eBAD_STRUCT,
                             eINVALID_REF,
                             eINVALID_DATA,
                             eFILENOTFOUND };

  XMLParsingException(XMLParsingErrorType t, const std::string& info) {
    this->why = t;
    this->info = info;
  }

  XMLParsingErrorType
  Type() {
    return this->why;
  }

  const std::string&
  Info() {
    return this->info;
  }

  std::string
  ErrorMsg();

private:
  XMLParsingErrorType why;
  std::string info;
};

/*****************************************************************************/
/*                     ##ABSTRACT## CLASS DagWfParser                        */
/*****************************************************************************/

class DagWfParser {
public:
  /**
   * @brief Reader constructor
   */
  DagWfParser();

  explicit DagWfParser(const char * content);

  explicit DagWfParser(const std::string& fileName);

  /**
   * The destructor
   */
  virtual ~DagWfParser();

  /**
   * Parse the XML
   * @param checkValid  if true a warning is displayed if no DTD provided
   */
  void
  parseXml(bool checkValid = false);


  /********************/
  /* Utility methods  */
  /********************/

  /**
   * utility method to get the attribute value in a DOM element
   * @param attr_name the attribute name
   * @param elt      the DOM element
   */
  static std::string
  getAttributeValue(const char * attr_name, const DOMElement * elt);

  /**
   * utility method to check that the attribute is non-empty
   */
  static void
  checkMandatoryAttr(const std::string& tagName, const std::string& attrName,
                     const std::string& attrValue);

  /**
   * utility method to check that an element does not contain any child
   */
  static void
  checkLeafElement(const DOMElement * element, const std::string& tagName);

  /**
   * Utility method to get the text content of a DOM element
   * @param element the DOM element
   * @param buffer  the string that will contain the value
   */
  static void
  getTextContent(const DOMElement * element, std::string& buffer);

  /**
   * Utility method to trim spaces from strings
   */
  static std::string&
  stringTrim(std::string& str);

  /**
   * Utility method to parse multiple assignmnent data
   */
  static void
  getPortMap(const std::string& thenMapStr,
             std::map<std::string, std::string>& thenMap)
    throw(XMLParsingException);

protected:
  /**
   * workflow description
   * Either XML is stored in the 'content' attribute or it is provided
   * in a file
   */
  std::string content;
  std::string myXmlFileName;

  /**
   * Xml document
   */
  xercesc::DOMDocument * document;


  /****************/
  /* Xml methods  */
  /****************/

  /**
   * Parse the root element
   * The implementation depends on the document structure so this is defined in
   * child classes.
   */
  virtual void
  parseRoot(DOMNode* root) = 0;

  /**
   * Parse a node (element & sub-elements)
   * This common implementation parses the node and its ports, and calls
   * parseOtherNodeSubElt(...) for specific node sub-elements
   * @param element     the DOM element
   * @param elementName the element tag name (ie 'node' or 'processor' ...)
   */
  void
  parseNode(const DOMElement * element, const std::string& elementName);

  /**
   * Create a node (using attributes of the element)
   * @param element     the DOM element
   * @param elementName the element tag name (ie 'node' or 'processor' ...)
   * @return a pointer to the newly created node
   */
  virtual WfNode *
  createNode(const DOMElement * element, const std::string& elementName) = 0;

  /**
   * Parse an argument element
   * @param element   port DOM element reference
   * @param lastArg   index of the port
   * @param node      ref to the current node
   * @return ref to the port created
   */
  virtual WfPort *
  parseArg(const DOMElement * element, const unsigned int lastArg,
           WfNode * node);

  /**
   * Parse an input port element
   * @param element   port DOM element reference
   * @param lastArg   index of the port
   * @param node      ref to the current node
   * @return ref to the port created
   */
  virtual WfPort *
  parseIn(const DOMElement * element, const unsigned int lastArg,
          WfNode * node);

  /**
   * Parse an inout port element
   * @param element   port DOM element reference
   * @param lastArg   index of the port
   * @param node      ref to the current node
   * @return ref to the port created
   */
  virtual WfPort *
  parseInOut(const DOMElement * element, const unsigned int lastArg,
             WfNode * node);

  /**
   * Parse an output port element
   * @param element   port DOM element reference
   * @param lastArg   index of the port
   * @param node      ref to the current node
   * @return ref to the port created
   */
  virtual WfPort *
  parseOut(const DOMElement * element, const unsigned int lastArg,
           WfNode * node);

  /**
   * Parse other sub-elements (not common) - called by parseNode
   * @param element     port DOM element reference
   * @param elementName the element name
   * @param portIndex   the current port index (to be updated if a new port is created)
   * @param node        ref to the current node
   */
  virtual void
  parseOtherNodeSubElt(const DOMElement * element,
                       const std::string& elementName,
                       unsigned int& portIndex,
                       WfNode * node) = 0;

  /**
   * create a node port
   * Note: two different syntax are possible for container types:
   *  * either type="LIST(LIST(\<base_type\>))" : oldest syntax used in MaDag
   *  * either type="<base_type>" depth="2" : new syntax used for funct parser
   *
   * @param param_type  the type of port (IN, OUT or INOUT)
   * @param name        the name of the port ('node id'#'port id')
   * @param type        the type of the port (eg 'LIST(LIST(DIET_INT))')
   * @param depth       the depth of the port (replaces LIST)
   * @param lastArg     the index of the port (in the profile)
   * @param node        the current node object
   */
  WfPort *
  createPort(const WfPort::WfPortType param_type, const std::string& name,
             const std::string& type, const std::string& depth,
             unsigned int lastArg, WfNode * node);

  /**
   * create a node port of type matrix
   */
  WfPort *
  createMatrixPort(const DOMElement * element,
                   const WfPort::WfPortType param_type,
                   const std::string& name,
                   unsigned int lastArg,
                   WfNode * node);
};

/*****************************************************************************/
/*                         CLASS DagParser  *** ABSTRACT ***                 */
/*****************************************************************************/
class DagParser : public DagWfParser {
public:
  DagParser();

  explicit DagParser(const char * content);

  explicit DagParser(const std::string& xmlFileName);

  virtual ~DagParser();

protected:
  Dag* myCurrDag;

  /**
   * Set the current dag
   */
  void
  setCurrentDag(Dag& dag);

  /**
   * @brief Parse XML for one dag (beginning with <dag>)
   * @param root root node of the subtree
   */
  void
  parseOneDag(DOMNode* root);

  /**
   * Creation of a Dag node
   */
  virtual WfNode *
  createNode(const DOMElement * element, const std::string& elementName);

  /**
   * Parse dag node specific sub-tags (
   */
  virtual void
  parseOtherNodeSubElt(const DOMElement * element,
                       const std::string& elementName,
                        unsigned int& portIndex,
                        WfNode * node);
  /**
   * Parse a prec element
   * @param element   port DOM element reference
   * @param node      ref to the current node
   */
  void
  parsePrec(const DOMElement * element, WfNode * node);
};

/*****************************************************************************/
/*                         CLASS SingleDagParser                             */
/*****************************************************************************/

class SingleDagParser : public DagParser {
public:
  SingleDagParser(Dag& dag, const char * content);

  virtual ~SingleDagParser();

protected:
  virtual void
  parseRoot(DOMNode* root);
};


/*****************************************************************************/
/*                         CLASS MultiDagParser                              */
/*****************************************************************************/

class MultiDagParser : public DagParser {
public:
  MultiDagParser();

  explicit MultiDagParser(const std::string& xmlFileName);

  virtual ~MultiDagParser();

  /**
   * Set the default workflow that contains the dags
   */
  void
  setWorkflow(FWorkflow* wf);

  /**
   * Parse a \<dags\> DOM element
   * @param root      the DOM node corresponding to \<dags\>
   */
  virtual void
  parseRoot(DOMNode* root);

  /**
   * Get the list of dags created during parsing
   * IMPORTANT: - created dags have no execution agent defined
   *            - parser does not deallocate dags at destruction
   */
  std::list<Dag*>& getDags();

protected:
  std::list<Dag*>  myDags;
  FWorkflow*  myWorkflow;
};

/*****************************************************************************/
/*                         CLASS FWfParser                                   */
/*****************************************************************************/
class FWfParser : public DagWfParser {
public:
  FWfParser(FWorkflow& workflow, const char * content);

  FWfParser(FWorkflow& workflow, const std::string& fileName);

  ~FWfParser();

protected:
  /**
   * METHODS already declared in parent class
   */

  virtual void
  parseRoot(DOMNode* root);

  virtual WfNode *
  createNode(const DOMElement * element, const std::string& elementName);

  virtual WfPort *
  parseIn(const DOMElement * element, const unsigned int lastArg,
          WfNode * node);

  virtual WfPort *
  parseOut(const DOMElement * element, const unsigned int lastArg,
           WfNode * node);

  virtual WfPort *
  parseInOut(const DOMElement * element, const unsigned int lastArg,
             WfNode * node);

  //   virtual void
  //   parseOutThen(const DOMElement * element,
  //                const unsigned int portIndex,
  //                WfNode * node);
  //
  //   virtual void
  //   parseOutElse(const DOMElement * element,
  //                const unsigned int portIndex,
  //                WfNode * node);

  virtual WfPort *
  parseParamPort(const DOMElement * element, const unsigned int lastArg,
                 WfNode* node);

  virtual void
  parseOtherNodeSubElt(const DOMElement * element,
                       const std::string& elementName,
                       unsigned int& portIndex,
                       WfNode * node);

  /**
   * Parse the cardinal attribute of a processor
   * @param element the DOM element corresponding to the <processor> tag
   * @param port    the current port object
   */
  void
  parseCardAttr(const DOMElement * element, WfPort* port);

  /**
   * Parse a \<link\> element
   * @param element the DOM element corresponding to the \<link\> tag
   */
  void
  parseLink(const DOMElement * element);

  /**
   * Parse the reference to another node or port inside a link attribute
   * @param strRef either an interface node name or nodeName:portName
   * @param nodeName returns the node name
   * @param portName returns the port name
   * @return the ref to the node object
   */
  FNode *
  parseLinkRef(const std::string& strRef, std::string& nodeName,
               std::string& portName);

  /**
   * Parse the iteration strategy tree
   * @param element the DOM element corresponding to the \<iterationstragegy\>
   *                or to an operator tag
   * @param procNode  the current processor node
   * @return a vector containing the ids of the operator(s) created
   */
  std::vector<std::string>*
  parseIterationStrategy(const DOMElement * element, FProcNode* procNode);

private:
  /**
   * Parse a workflow file name and get the corresponding class name
   * @param fileName  file containing the wf XML
   * @return  class of the wf
   */
  std::string
  getWfClassName(const std::string& fileName);

  WfPort *
  parsePortCommon(const DOMElement * element, const unsigned int portIndex,
                  WfNode * node, const std::string& portName,
                  const WfPort::WfPortType portType);

  /**
   * Map containing the included workflow classes with the corresponding file
   */
  std::map<std::string, std::string>  myClassFiles;

  /**
   * Ref to the workflow
   */
  FWorkflow&  workflow;
};

/*****************************************************************************/
/*                    CLASS DataSourceParser                                 */
/*****************************************************************************/

class DataSourceParser {
public:
  explicit DataSourceParser(FSourceNode* node);

  ~DataSourceParser();

  /**
   * Initialize the parser
   * Note: after this call the next available value can be retrieved
   * with getValue() if end() is not true
   * @param dataFileName  path of the file containing data description
   */
  void
  parseXml(const std::string& dataFileName) throw(XMLParsingException);

private:
  FSourceNode* myNode;
};


/*****************************************************************************/
/*                    CLASS DataSourceHandler                                */
/*****************************************************************************/

class DataSourceHandler : public DefaultHandler {
public:
  explicit DataSourceHandler(FSourceNode* node);

  void
  startElement(const XMLCh* const uri, const XMLCh* const localname,
    const XMLCh* const qname, const Attributes& attrsx);

  void
  endElement(const XMLCh* const uri, const XMLCh* const localname,
             const XMLCh* const qname);

  void
  characters(const XMLCh* const chars, const XMLSize_t length);

  void
  fatalError(const SAXParseException& e);

  void
  warning(const SAXParseException& e);

private:
  void
  startSource(const Attributes& attrs);

  void
  startList(const Attributes& attrs);

  void
  startItem(const Attributes& attrs);

  void
  startTag(const Attributes& attrs);

  void
  endSource();

  void
  endList();

  void
  endItem();

  void
  endTag();

  FSourceNode* myNode;
  FDataTag* myCurrTag;
  FDataHandle* myCurrListDH;
  std::string myCurrItemValue;
  std::string myCurrItemDataID;
  FDataHandle* myCurrItemDH;

  bool isSourceFound;
  bool isListFound;
  bool isItemFound;
};

/*****************************************************************************/
/*                    CLASS MyDOMErrorHandler                                */
/*****************************************************************************/

class MyDOMErrorHandler : public DOMErrorHandler {
  virtual bool
  handleError(const DOMError &domError);
};


#endif   /* not defined _DAGWFPARSER_HH */

