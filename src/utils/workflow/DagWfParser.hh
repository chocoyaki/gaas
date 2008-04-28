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
 * Revision 1.2  2008/04/28 12:06:28  bisnard
 * changed constructor for Node (new param wfReqId)
 *
 * Revision 1.1  2008/04/21 14:35:50  bisnard
 * added NodeQueue and renamed WfParser as DagWfParser
 *
 ****************************************************************************/

#ifndef _DAGWFPARSER_HH_
#define _DAGWFPARSER_HH_

#include <vector>

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
  /* public attributes                                                    */
  /*********************************************************************/

  /**
   * Profiles list
   */
  std::vector<corba_pb_desc_t> pbs_list;

  /**
   * Nodes list
   */
  std::map<string, corba_pb_desc_t> nodes_list;

  /*********************************************************************/
  /* public methods                                                    */
  /*********************************************************************/

  /** Reader constructor
   *
   * @param wfReqId the workflow request ID
   * @param content the workflow description
   */
  DagWfParser(int wfReqId, const char * content);

  /** Reader constructor
   *
   * @param wfReqId the workflow request ID
   * @param content the workflow description
   * @param alloc   indicates if the profile_desc is allocated  (false, the default value) or not
   */
  DagWfParser(int wfReqId, const char * content, bool alloc);

  /**
   * The destructor
   */
  virtual
  ~DagWfParser();

  /**
   * Initialize the processing
   */
  virtual bool
  setup();

  /**
   * get a reference to Dag structure
   */
  Dag * getDag();

  unsigned int
  getDagSize();


  /**
   * return the index of the provided problem in pbs_list vector
   *
   * @param pb the the requested problem index
   */
  unsigned int
  indexOfPb(corba_pb_desc_t& pb);

protected:
  /*********************************************************************/
  /* protected fields                                                  */
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
   * The dag representation as a map of nodes
   * The key is the node identifier. The data is the node reference
   */
  std::map <std::string, Node*> myNodes;

  /**
   * The DAG size
   */
  unsigned int dagSize;

  /**
   * Dag structure
   */
  Dag * myDag;


  /**
   * allocation indicator (for profile construction)
   */
  bool alloc;

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
  virtual bool
  initXml();

  /**
   * parse a node element
   *
   * @param element      the DOM node
   * @param nodeId   the node identifier
   * @param nodePath the node path (or service)
   * @param var_node indicates if it is a variable node
   */
  virtual bool
  parseNode (const DOMNode * element,
	     string nodeId, string nodePath,
	     long int var_node = -1);

  /**
   * Parse an argument element
   *
   * @param child_elt argument DOM element reference
   */
  bool
  parseArg(DOMElement * child_elt);

  /**
   * Parse an input port element
   *
   * @param child_elt input port DOM element reference
   */
  bool
  parseIn(DOMElement * child_elt);

  /**
   * Parse an inout port element
   *
   * @param child_elt Inout DOM element reference
   */
  bool
  parseInout(DOMElement * child_elt);

  /**
   * Parse an output port element
   *
   * @param child_elt Out port DOM element reference
   */
  bool
  parseOut(DOMElement * child_elt);

  /**
   * parse an argument element
   * @param name     argument id
   * @param value    argument value
   * @param type     argument data type
   * @param profile  current profile reference
   * @param lastArg  the argument index in the profile
   * @param var_node Undocumented
   * @param var_port Undocumented
   * @param dagNode  node reference in the Dag structure
   */
  virtual bool
  checkArg(const string& name,
	   const string& value,
	   const string& type,
	   diet_profile_t * profile,
	   const unsigned int lastArg,
	   long int var_node = -1,
	   long int var_port = -1,
	   Node * dagNode = NULL);

  /**
   * parse an input port element
   * @param name     input port id
   * @param type     input port data type
   * @param source   linked output port id
   * @param profile  current profile reference
   * @param lastArg  the input port index in the profile
   * @param var_node Undocumented
   * @param var_port Undocumented
   * @param dagNode  node reference in the Dag structure
   */
  virtual bool
  checkIn(const string& name,
	  const string& type,
	  const string& source,
	  diet_profile_t * profile,
	  unsigned int lastArg,
	  long int var_node = -1,
	  long int var_port = -1,
	  Node * dagNode = NULL);

  /**
   * parse an input/output port element
   * @param name     inoutput port id
   * @param type     inoutput port data type
   * @param source   linked output port id
   * @param profile  current profile reference
   * @param lastArg  the inoutput port index in the profile
   * @param var_node Undocumented
   * @param var_port Undocumented
   * @param dagNode  node reference in the Dag structure
   */
  virtual bool
  checkInout(const string& name,
	     const string& type,
	     const string& source,
	     diet_profile_t * profile,
	     unsigned int lastArg,
	     long int var_node = -1,
	     long int var_port = -1,
	     Node * dagNode = NULL);

  /**
   * parse an output port element
   * @param name     output port id
   * @param type     output port data type
   * @param sink     linked input port id
   * @param profile  current profile reference
   * @param lastArg  the output port index in the profile
   * @param var_node Undocumented
   * @param var_port Undocumented
   * @param dagNode  node reference in the Dag structure
   */
  virtual bool
  checkOut(const string& name,
	   const string& type,
	   const string& sink,
	   diet_profile_t * profile,
	   unsigned int lastArg,
	   long int var_node = -1,
	   long int var_port = -1,
	   Node * dagNode = NULL);

  /**
   * fill a profile with the appropriate parameter type and create the
   * node ports
   */
  virtual bool
  setParam(const wf_port_t param_type,
	   const string& name,
	   const string& type,
	   diet_profile_t* profile,
	   unsigned int lastArg,
	   long int var_node = -1,
	   long int var_port = -1,
	   Node * dagNode = NULL,
	   const string * value = NULL);

  /**
   * parse a matrix argument
   * @param id         Port complete id (node id + # + port name)
   * @param element    Dom node representing a matrix
   * @param profile  current profile reference
   * @param lastArg  the output port index in the profile
   * @param var_node Undocumented
   * @param var_port Undocumented
   * @param dagNode  Undocumented
   */
  virtual bool
  checkMatrixArg(const string& id, const DOMElement * element,
		 diet_profile_t * profile, unsigned int lastArg,
		 long int var_node = -1,
		 long int var_port = -1,
		 Node * dagNode = NULL);

  /**
   * parse a matrix input port
   * @param id         Port complete id (node id + # + port name)
   * @param element    Dom node representing a matrix
   * @param profile  current profile reference
   * @param lastArg  the output port index in the profile
   * @param var_node Undocumented
   * @param var_port Undocumented
   * @param dagNode  Undocumented
   */
  virtual bool
  checkMatrixIn(const string& id, const string& source,
		const DOMElement * element,
		diet_profile_t * profile, unsigned int lastArg,
		long int var_node = -1,
		long int var_port = -1,
		Node * dagNode = NULL);

  /**
   * parse a matrix inout port
   * @param id         Port complete id (node id + # + port name)
   * @param element    Dom node representing a matrix
   * @param profile  current profile reference
   * @param lastArg  the output port index in the profile
   * @param var_node Undocumented
   * @param var_port Undocumented
   * @param dagNode  Undocumented
   */
  virtual bool
  checkMatrixInout(const string& id, const DOMElement * element,
		   diet_profile_t * profile, unsigned int lastArg,
		   long int var_node = -1,
		   long int var_port = -1,
		   Node * dagNode = NULL);

  /**
   * parse a matrix output port
   * @param id         Port complete id (node id + # + port name)
   * @param element    Dom node representing a matrix
   * @param profile  current profile reference
   * @param lastArg  the output port index in the profile
   * @param var_node Undocumented
   * @param var_port Undocumented
   * @param dagNode  Undocumented
   */
  virtual bool
  checkMatrixOut(const string& id, const DOMElement * element,
		 diet_profile_t * profile, unsigned int lastArg,
		 long int var_node = -1,
		 long int var_port = -1,
		 Node * dagNode = NULL);

  /**
   * parse a matrix argument.
   * Check only the commun attributes of In, Inout and Out ports
   * @param id         Port complete id (node id + # + port name)
   * @param element    Dom node representing a matrix
   * @param base_type  Undocumented
   * @param nb_rows    Undocumented
   * @param nb_cols    Undocumented
   * @param matrix_order Undocumented
   */
  virtual bool
  checkMatrixCommun(const string& id, const DOMElement * element,
		    string& base_type,
		    string& nb_rows,
		    string& nb_cols,
		    string& matrix_order);

  /**
   * fill a profile with matrix parameter type
   * The data are NULL
   */
  virtual bool
  setMatrixParam(const wf_port_t param_type,
		 const string& name,
		 const string& base_type,
		 const string& nb_rows,
		 const string& nb_cols,
		 const string& matrix_order,
		 diet_profile_t * profile,
		 unsigned int lastArg,
		 Node * dagNode = NULL,
		 const string * value = NULL);

  /**
   * Return the expression between {} in the id
   *
   * @param id the identifier to parse
   */
  string
  getExpr(string& id);

  /**
   * Return the name of source/sink after expansion
   *
   * @param real_name the source/sink identifier
   * @param var_node  the linked node number
   * @param var_port  the port node number
   */
  string
  getRealName(string& real_name, long int var_node, long int var_port);

  /**
   * Check if the profile is already in the problems list
   *
   * @param pb_desc the problem to found
   */
  bool
  pbAlreadyRegistred(corba_pb_desc_t& pb_desc);

  /**
   * Fill a profile with the appropriate parameter type
   * The data are NULL
   *
   * @param param_type the parameter type (ARG, IN, INOUT)
   * @param name       the port type
   * @param type       the parameter data type
   * @param profile    the diet profile reference
   * @param lastArg    the parameter index
   * @param var        not used
   * @param dagNode    the node reference
   * @param value      the parameter value as a reference
   */
  virtual bool
  setParamDesc(const wf_port_t param_type,
	       const string& name,
	       const string& type,
	       diet_profile_t * profile,
	       unsigned int lastArg,
	       long int var = -1,
	       Node * dagNode = NULL,
	       const string * value = NULL);

  /**
   * fill a profile with matrix parameter type
   * The data are NULL
   *
   * @param param_type   the parameter type (ARG, IN, INOUT)
   * @param name         the port type
   * @param base_type    the matrix element data type
   * @param nb_rows      the rows count
   * @param nb_cols      the cols count
   * @param matrix_order the matrix order
   * @param profile      the diet profile reference
   * @param lastArg      the parameter index
   * @param dagNode      the node reference
   * @param value        the parameter value as a reference
   */
  virtual bool
  setMatrixParamDesc(const wf_port_t param_type,
		     const string& name,
		     const string& base_type,
		     const string& nb_rows,
		     const string& nb_cols,
		     const string& matrix_order,
		     diet_profile_t * profile,
		     unsigned int lastArg,
		     Node * dagNode = NULL,
		     const string * value = NULL);

  private:

    /**
     * The workflow request ID
     */
    int wfReqId;
};

bool operator == (corba_pb_desc_t& a,   corba_pb_desc_t& b);

class XMLParsingException {
  public:
    enum XMLParsingErrorType { eUNKNOWN_TAG, eUNKNOWN_ATTR, eBAD_STRUCT, eINVALID_REF };
    XMLParsingException(XMLParsingErrorType t) { this->why = t; }
    XMLParsingErrorType Type() { return this->why; }
  private:
    XMLParsingErrorType why;
};

#endif   /* not defined _DAGWFPARSER_HH */

