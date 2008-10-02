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

XERCES_CPP_NAMESPACE_USE
using namespace std;

class DagWfParser {
public:

  /*********************************************************************/
  /* public methods                                                    */
  /*********************************************************************/

  /** Reader constructor
   *
   * @param wfReqId the workflow request ID
   * @param content the workflow description
   */
  DagWfParser(int wfReqId, const char * content);

  /**
   * The destructor
   */
  ~DagWfParser();

  /**
   * Initialize the processing
   */
  bool
  setup();

  /**
   * get a reference to Dag structure
   */
  Dag * getDag();


protected:
  /*********************************************************************/
  /* protected                                                         */
  /*********************************************************************/

  /**
   * workflow description
   * contains the content of the workflow description file
   */
  std::string content;

  /**
   * Xml document
   */
  DOMDocument * document;

  /**
   * Dag structure
   */
  Dag * myDag;

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
   * Init the XML processing
   */
  bool
  initXml();

  /**
   * parse a node element
   *
   * @param element      the DOM node
   * @param nodeId   the node identifier
   * @param nodePath the node path (or service)
   */
  bool
  parseNode (const DOMNode * element,
	     string nodeId, string nodePath);

  /**
   * Parse an argument element
   *
   * @param child_elt argument DOM element reference
   */
  bool
  parseArg(DOMElement * child_elt,
           const unsigned int lastArg,
 	   Node& dagNode);

  /**
   * Parse an input port element
   *
   * @param child_elt input port DOM element reference
   */
  bool
  parseIn(DOMElement * child_elt,
          const unsigned int lastArg,
 	  Node& dagNode);

  /**
   * Parse an inout port element
   *
   * @param child_elt Inout DOM element reference
   */
  bool
  parseInout(DOMElement * child_elt,
             const unsigned int lastArg,
 	     Node& dagNode);

  /**
   * Parse an output port element
   *
   * @param child_elt Out port DOM element reference
   */
  bool
  parseOut(DOMElement * child_elt,
           const unsigned int lastArg,
 	   Node& dagNode);

  /**
   * create a node port
   * @param param_type  the type of port (IN, OUT or INOUT)
   * @param name        the name of the port ('node id'#'port id')
   * @param type        the type of the port (eg 'LIST(LIST(DIET_INT))')
   * @param lastArg     the index of the port (in the profile)
   * @param dagNode     the current node object
   * @param value       the value of the parameter (optional)
   */
  void
  setParam(const WfPort::WfPortType param_type,
	   const string& name,
	   const string& type,
	   unsigned int lastArg,
	   Node& dagNode,
	   const string * value = NULL);

  /**
   * create a node port of type matrix
   */
  bool
  setMatrixParam(const DOMElement * element,
                 const WfPort::WfPortType param_type,
		 const string& name,
		 unsigned int lastArg,
		 Node& dagNode,
		 const string * value = NULL);

  private:

    /**
     * The workflow request ID
     */
    int wfReqId;

};

class XMLParsingException {
  public:
    enum XMLParsingErrorType { eUNKNOWN_TAG, eUNKNOWN_ATTR, eBAD_STRUCT, eINVALID_REF };
    XMLParsingException(XMLParsingErrorType t) { this->why = t; }
    XMLParsingErrorType Type() { return this->why; }
  private:
    XMLParsingErrorType why;
};

#endif   /* not defined _DAGWFPARSER_HH */

