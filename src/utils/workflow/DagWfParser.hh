/****************************************************************************/
/* DAG Workflow description Reader class description                        */
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
 * Revision 1.23  2009/07/24 15:06:46  bisnard
 * XML validation using DTD for functional workflows
 *
 * Revision 1.22  2009/07/10 12:55:59  bisnard
 * implemented while loop workflow node
 *
 * Revision 1.21  2009/07/07 11:25:54  bisnard
 * modified data file parser
 *
 * Revision 1.20  2009/07/07 09:03:22  bisnard
 * changes for sub-workflows (FWorkflow class now inherits from FProcNode)
 *
 * Revision 1.19  2009/06/19 07:47:19  bisnard
 * removed deprecated header file
 *
 * Revision 1.18  2009/06/15 12:11:12  bisnard
 * use new XML Parser (SAX) for data source file
 * use new class WfValueAdapter to avoid data duplication
 * use new method FNodeOutPort::storeData
 * changed method to compute total nb of data items
 *
 * Revision 1.17  2009/05/27 08:49:43  bisnard
 * - modified condition output: new IF_THEN and IF_ELSE port types
 * - implemented MERGE and FILTER workflow nodes
 *
 * Revision 1.16  2009/05/15 11:10:20  bisnard
 * release for workflow conditional structure (if)
 *
 * Revision 1.15  2009/04/17 08:54:43  bisnard
 * renamed Node class as WfNode
 *
 * Revision 1.14  2009/02/24 14:01:05  bisnard
 * added dynamic parameter mgmt for wf processors
 *
 * Revision 1.13  2009/02/06 14:55:08  bisnard
 * setup exceptions
 *
 * Revision 1.12  2009/01/16 16:31:54  bisnard
 * added option to specify data source file name
 *
 * Revision 1.11  2009/01/16 13:54:50  bisnard
 * new version of the dag instanciator with iteration strategies for nodes with multiple input ports
 *
 * Revision 1.10  2008/12/02 10:14:51  bisnard
 * modified nodes links mgmt to handle inter-dags links
 *
 * Revision 1.9  2008/10/20 08:02:57  bisnard
 * new classes XML parser (Dagparser,FWfParser)
 *
 * Revision 1.8  2008/10/14 13:31:01  bisnard
 * new class structure for dags (DagNode,DagNodePort)
 *
 * Revision 1.7  2008/10/02 07:35:10  bisnard
 * new constants definitions (matrix order and port type)
 *
 * Revision 1.6  2008/09/30 15:29:22  bisnard
 * code refactoring after profile mgmt change
 *
 * Revision 1.5  2008/09/30 09:23:29  bisnard
 * removed diet profile initialization from DagWfParser and replaced by node methods initProfileSubmit and initProfileExec
 *
 * Revision 1.4  2008/09/19 13:11:07  bisnard
 * - added support for containers split/merge in workflows
 * - added support for multiple port references
 * - profile for node execution initialized by port (instead of node)
 * - ports linking managed by ports (instead of dag)
 *
 * Revision 1.3  2008/09/08 09:12:58  bisnard
 * removed obsolete attribute nodes_list, pbs_list, alloc
 *
 * Revision 1.2  2008/04/28 12:06:28  bisnard
 * changed constructor for Node (new param wfReqId)
 *
 * Revision 1.1  2008/04/21 14:35:50  bisnard
 * added NodeQueue and renamed WfParser as DagWfParser
 *
 ****************************************************************************/

#ifndef _DAGWFPARSER_HH_
#define _DAGWFPARSER_HH_

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
#include <string.h>
#include <stdlib.h>

// Workflow related headers
#include "WfUtils.hh"
#include "WfNode.hh"
#include "Dag.hh"

// Functional workflow classes
class FWorkflow;
class FNode;
class FSourceNode;
class FDataTag;


XERCES_CPP_NAMESPACE_USE
using namespace std;

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
                               eINVALID_DATA };
    XMLParsingException(XMLParsingErrorType t, const string& info)
      { this->why = t; this->info = info; }
    XMLParsingErrorType Type() { return this->why; }
    const string& Info() { return this->info; }
    string ErrorMsg();
  private:
    XMLParsingErrorType why;
    string info;
};

/*****************************************************************************/
/*                     ##ABSTRACT## CLASS DagWfParser                        */
/*****************************************************************************/

class DagWfParser {
public:

  /** Reader constructor
   * @param content the workflow description
   */
  DagWfParser(const char * content);
  DagWfParser(const string& fileName);

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

  /**
   * utility method to get the attribute value in a DOM element
   *
   * @param attr_name the attribute name
   * @param elt      the DOM element
   */
  static string
      getAttributeValue(const char * attr_name, const DOMElement * elt);

  /**
   * utility method to check that the attribute is non-empty
   */
  static void
      checkMandatoryAttr(const string& tagName,
                         const string& attrName,
                         const string& attrValue);

  /**
   * utility method to check that an element does not contain any child
   */
  static void
      checkLeafElement(const DOMElement * element, const string& tagName);

  /**
   * Utility method to get the text content of a DOM element
   * @param element the DOM element
   * @param buffer  the string that will contain the value
   */
  static void
      getTextContent(const DOMElement * element, string& buffer);

  /**
   * Utility method to trim spaces from strings
   */
  static string&
      stringTrim(string& str);

protected:

  /**
   * Utility method to parse multiple assignmnent data
   */
  static void
      getPortMap(const string& thenMapStr,
                 map<string,string>& thenMap) throw (XMLParsingException);

  /**
   * workflow description
   * contains the content of the workflow description file
   */
  string content;
  string myXmlFileName;

  /**
   * Xml document
   */
  DOMDocument * document;

  /****************/
  /* Xml methods  */
  /****************/


  /**
   * Parse the root element
   */
  virtual void
  parseRoot(DOMNode* root) = 0;

  /**
   * Parse a node (element & sub-elements)
   * @param element     the DOM element
   * @param elementName the element tag name (ie 'node' or 'processor' ...)
   */
  void
  parseNode(const DOMElement * element, const string& elementName);

  /**
   * Create a node (using attributes of the element)
   * @param element     the DOM element
   * @param elementName the element tag name (ie 'node' or 'processor' ...)
   * @return a pointer to the newly created node
  */
  virtual WfNode *
  createNode(const DOMElement * element, const string& elementName) = 0;

  /**
   * Parse an argument element
   * @param element   port DOM element reference
   * @param lastArg   index of the port
   * @param node      ref to the current node
   * @return ref to the port created
   */
  virtual WfPort *
  parseArg(const DOMElement * element,
           const unsigned int lastArg,
 	   WfNode * node);

  /**
   * Parse an input port element
   * @param element   port DOM element reference
   * @param lastArg   index of the port
   * @param node      ref to the current node
   * @return ref to the port created
   */
  virtual WfPort *
  parseIn(const DOMElement * element,
          const unsigned int lastArg,
 	  WfNode * node);

  /**
   * Parse an inout port element
   * @param element   port DOM element reference
   * @param lastArg   index of the port
   * @param node      ref to the current node
   * @return ref to the port created
   */
  virtual WfPort *
  parseInOut(const DOMElement * element,
             const unsigned int lastArg,
 	     WfNode * node);

  /**
   * Parse an output port element
   * @param element   port DOM element reference
   * @param lastArg   index of the port
   * @param node      ref to the current node
   * @return ref to the port created
   */
  virtual WfPort *
  parseOut(const DOMElement * element,
           const unsigned int lastArg,
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
                       const string& elementName,
                       unsigned int& portIndex,
                       WfNode * node) = 0;

  /**
   * create a node port
   * Note: two different syntax are possible for container types:
   *  * either type="LIST(LIST(<base_type>))" : oldest syntax used in MaDag
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
  setParam(const WfPort::WfPortType param_type,
	   const string& name,
	   const string& type,
           const string& depth,
	   unsigned int lastArg,
	   WfNode * node);

  /**
   * create a node port of type matrix
   */
  WfPort *
  setMatrixParam(const DOMElement * element,
                 const WfPort::WfPortType param_type,
		 const string& name,
		 unsigned int lastArg,
		 WfNode * node);

}; // end class DagWfParser

/*****************************************************************************/
/*                         CLASS DagParser                                   */
/*****************************************************************************/

class DagParser : public DagWfParser {

public:
  DagParser(Dag& dag, const char * content);
  ~DagParser();

protected:

  virtual void
  parseRoot(DOMNode* root);
  virtual WfNode *
  createNode(const DOMElement * element, const string& elementName );
  virtual void
  parseOtherNodeSubElt(const DOMElement * element,
                       const string& elementName,
                       unsigned int& portIndex,
                       WfNode * node);
  /**
   * Parse a prec element
   * @param element   port DOM element reference
   * @param node      ref to the current node
   */
  void
  parsePrec(const DOMElement * element,
            WfNode * node);
private:

  /**
   * Dag structure
   */
  Dag&  dag;

}; // end class DagParser

/*****************************************************************************/
/*                         CLASS FWfParser                                   */
/*****************************************************************************/

class FWfParser : public DagWfParser {

public:
  FWfParser(FWorkflow& workflow, const char * content);
  FWfParser(FWorkflow& workflow, const string& fileName);
  ~FWfParser();

protected:

  /**
   * METHODS already declared in parent class
   */

  virtual void
  parseRoot(DOMNode* root);

  virtual WfNode *
  createNode(const DOMElement * element, const string& elementName);

  virtual WfPort *
  parseIn(const DOMElement * element,
          const unsigned int lastArg,
          WfNode * node);

  virtual WfPort *
  parseOut(const DOMElement * element,
           const unsigned int lastArg,
           WfNode * node);

  virtual WfPort *
  parseInOut(const DOMElement * element,
             const unsigned int lastArg,
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
  parseParamPort(const DOMElement * element,
                 const unsigned int lastArg,
                 WfNode* node);

  virtual void
  parseOtherNodeSubElt(const DOMElement * element,
                       const string& elementName,
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
   * Parse a <link> element
   * @param element the DOM element corresponding to the <link> tag
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
  parseLinkRef(const string& strRef, string& nodeName, string& portName);

  /**
   * Parse the iteration strategy tree
   * @param element the DOM element corresponding to the <iterationstragegy>
   *                or to an operator tag
   * @param procNode  the current processor node
   * @return a vector containing the ids of the operator(s) created
   */
  vector<string>*
  parseIterationStrategy(const DOMElement * element,
                         FProcNode* procNode);

private:

  /**
   * Parse a workflow file name and get the corresponding class name
   * @param fileName  file containing the wf XML
   * @return  class of the wf
   */
  string getWfClassName(const string& fileName);

  WfPort *
  parsePortCommon(const DOMElement * element,
                  const unsigned int portIndex,
                  WfNode * node,
                  const string& portName,
                  const WfPort::WfPortType portType);

  /**
   * Map containing the included workflow classes with the corresponding file
   */
  map<string,string>  myClassFiles;

  /**
   * Ref to the workflow
   */
  FWorkflow&  workflow;

}; // end class FWfParser

/*****************************************************************************/
/*                    CLASS DataSourceParser                                 */
/*****************************************************************************/

class DataSourceParser {

  public:

    DataSourceParser(FSourceNode* node);
    ~DataSourceParser();

    /**
     * Initialize the parser
     * Note: after this call the next available value can be retrieved
     * with getValue() if end() is not true
     * @param dataFileName  path of the file containing data description
     */
    void parseXml(const string& dataFileName) throw (XMLParsingException);


  private:

    FSourceNode* myNode;
};


/*****************************************************************************/
/*                    CLASS DataSourceHandler                                */
/*****************************************************************************/

class DataSourceHandler : public DefaultHandler {

  public:

    DataSourceHandler(FSourceNode* node);

    void startElement(
        const   XMLCh* const    uri,
        const   XMLCh* const    localname,
        const   XMLCh* const    qname,
        const   Attributes&     attrs
    );

    void endElement(const   XMLCh* const    uri,
                    const   XMLCh* const    localname,
                    const   XMLCh* const    qname);

    void characters (const  XMLCh* const     chars,
                     const  unsigned int     length);

    void fatalError(const SAXParseException& e);
    void warning(const SAXParseException& e);

  private:
    void startSource(const   Attributes&     attrs);
    void startList(const   Attributes&     attrs);
    void startItem(const   Attributes&     attrs);

    void endSource();
    void endList();
    void endItem();

    FSourceNode*  myNode;
    FDataTag*     myCurrTag;
    string        myCurrItemValue;

    bool          isSourceFound;
    bool          isItemFound;


};

/*****************************************************************************/
/*                    CLASS MyDOMErrorHandler                                */
/*****************************************************************************/

class MyDOMErrorHandler : public DOMErrorHandler {

  virtual bool
      handleError (const DOMError &domError);

};


#endif   /* not defined _DAGWFPARSER_HH */

