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

#include "Dag.hh"
// Xerces header
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/parsers/AbstractDOMParser.hpp>
#include <xercesc/dom/DOMImplementation.hpp>
#include <xercesc/dom/DOMImplementationLS.hpp>
#include <xercesc/dom/DOMImplementationRegistry.hpp>
#include <xercesc/dom/DOMBuilder.hpp>
#include <xercesc/dom/DOMException.hpp>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMNodeList.hpp>
#include <xercesc/dom/DOMError.hpp>
#include <xercesc/dom/DOMLocator.hpp>
#include <xercesc/dom/DOMNamedNodeMap.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/dom/DOMAttr.hpp>
#include <string.h>
#include <stdlib.h>

// Workflow related headers
#include "WfUtils.hh"
#include "Node.hh"
#include "Dag.hh"
#include "FWorkflow.hh"

XERCES_CPP_NAMESPACE_USE
using namespace std;

/*****************************************************************************/
/*                        CLASS XMLParsingException                          */
/*****************************************************************************/

class XMLParsingException {
  public:
    enum XMLParsingErrorType { eUNKNOWN_TAG,
                               eUNKNOWN_ATTR,
                               eEMPTY_ATTR,
                               eBAD_STRUCT,
                               eINVALID_REF };
    XMLParsingException(XMLParsingErrorType t, const string& info)
      { this->why = t; this->info = info; }
    XMLParsingErrorType Type() { return this->why; }
    const string& Info() { return this->info; }
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

  /**
   * The destructor
   */
  virtual ~DagWfParser();

  /**
   * Parse the XML and check the structure
   */
  virtual bool
  parseAndCheck() throw(XMLParsingException);


protected:

  /**
   * workflow description
   * contains the content of the workflow description file
   */
  string content;

  /**
   * Xml document
   */
  DOMDocument * document;

  /****************/
  /* Xml methods  */
  /****************/

  /**
   * utility method to get the attribute value in a DOM element
   *
   * @param attr_name the attribute name
   * @param elt      the DOM element
   */
  string
  getAttributeValue(const char * attr_name, const DOMElement * elt);

  /**
   * Parse the XML
   */
  bool
  parseXml();

  /**
   * Check the precedence
   */
  virtual bool
  checkPrec() = 0;

  /**
   * Parse the root element
   */
  virtual void
  parseRoot(DOMNode* root) = 0;

  /**
   * Parse a node (element & sub-elements)
   * @param element     the DOM element
   * @param nodeEltName the element tag name (ie 'node' or 'processor' ...)
   */
  void
  parseNode(const DOMElement * element, const string& nodeEltName);

  /**
   * Create a node (using attributes of the element)
   * @param element   the DOM element
   * @param nodeEltName the element tag name (ie 'node' or 'processor' ...)
   * @return a pointer to the newly created node
  */
  virtual Node *
  createNode(const DOMElement * element, const string& nodeEltName) = 0;

  /**
   * Parse an argument element
   * @param child_elt argument DOM element reference
   * @param lastArg   index of the port
   * @param node      ref to the current node
   */
  void
  parseArg(DOMElement * child_elt,
           const unsigned int lastArg,
 	   Node& node);

  /**
   * Parse an input port element
   * @param child_elt input port DOM element reference
   * @param lastArg   index of the port
   * @param node      ref to the current node
   */
  void
  parseIn(DOMElement * child_elt,
          const unsigned int lastArg,
 	  Node& node);

  /**
   * Parse an inout port element
   * @param child_elt Inout DOM element reference
   * @param lastArg   index of the port
   * @param node      ref to the current node
   */
  void
  parseInout(DOMElement * child_elt,
             const unsigned int lastArg,
 	     Node& node);

  /**
   * Parse an output port element
   * @param child_elt Out port DOM element reference
   * @param lastArg   index of the port
   * @param node      ref to the current node
   */
  void
  parseOut(DOMElement * child_elt,
           const unsigned int lastArg,
 	   Node& node);

  /**
   * Parse other sub-elements (not common)
   * @param child_elt Out port DOM element reference
   * @param childName the element name
   * @param node      ref to the current node
   */
  virtual void
  parseOtherNodeSubElt(const DOMElement * childElt,
                       const string& childName,
                       Node& node) = 0;

  /**
   * create a node port
   * @param param_type  the type of port (IN, OUT or INOUT)
   * @param name        the name of the port ('node id'#'port id')
   * @param type        the type of the port (eg 'LIST(LIST(DIET_INT))')
   * @param lastArg     the index of the port (in the profile)
   * @param node     the current node object
   * @param value       the value of the parameter (optional)
   */
  WfPort *
  setParam(const WfPort::WfPortType param_type,
	   const string& name,
	   const string& type,
	   unsigned int lastArg,
	   Node& node,
	   const string * value = NULL);

  /**
   * create a node port of type matrix
   */
  WfPort *
  setMatrixParam(const DOMElement * element,
                 const WfPort::WfPortType param_type,
		 const string& name,
		 unsigned int lastArg,
		 Node& node,
		 const string * value = NULL);

}; // end class DagWfParser

/*****************************************************************************/
/*                         CLASS DagParser                                   */
/*****************************************************************************/

class DagParser : public DagWfParser {

public:
  DagParser(Dag& dag, const char * content);
  ~DagParser();

protected:

  virtual bool
  checkPrec();
  virtual void
  parseRoot(DOMNode* root);
  virtual Node *
  createNode(const DOMElement * element, const string& nodeEltName );
  virtual void
  parseOtherNodeSubElt(const DOMElement * childElt,
                       const string& childName,
                       Node& node);
  /**
   * Parse a prec element
   * @param child_elt Out port DOM element reference
   * @param node      ref to the current node
   */
  void
  parsePrec(const DOMElement * child_elt,
            Node& node);
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
  ~FWfParser();

protected:

  virtual bool
  checkPrec();
  virtual void
  parseRoot(DOMNode* root);
  virtual Node *
  createNode(const DOMElement * element, const string& nodeEltName);
  virtual void
  parseOtherNodeSubElt(const DOMElement * childElt,
                       const string& childName,
                       Node& node);
private:
  FWorkflow&  workflow;
}; // end class FWfParser


#endif   /* not defined _DAGWFPARSER_HH */

