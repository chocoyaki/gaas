/****************************************************************************/
/* Basic workflow description Reader                                        */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.3  2006/07/21 09:29:22  eboix
 *  - Added the first brick for test suite [disabled right now].
 *  - Doxygen related cosmetic changes.  --- Injay2461
 *
 * Revision 1.2  2006/07/10 11:09:41  aamar
 * - Adding matrix data type
 * - Adding generic nodes and generic ports parsing
 * - Now, the parser uses Xerces instead of QT
 *
 * Revision 1.1  2006/04/14 13:51:55  aamar
 * This class process a Dag textual representation. It is
 * used by the MA DAG (header).
 *
 ****************************************************************************/

#ifndef _WFREADER_HH_
#define _WFREADER_HH_

#include <string>
#include <vector>

// general DIET headers
#include "DIET_data.h"
#include "common_types.hh"
#include "DIET_data.h"
#include "DIET_server.h"

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
#include "BasicNode.hh"

XERCES_CPP_NAMESPACE_USE
using namespace std;

class WfReader {
public:
  friend class MaDag_impl;

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

  WfReader(const char * content);

  virtual 
  ~WfReader();

  /**
   * Initialize the processing
   */
  virtual void
  setup();

  /**
   * read the workflow description file
   */
  bool 
  getContent(const char * fileName);
  
  /****************************************/
  /* Nodes list manipulation methods      */
  /* not really used (just for debugging) */
  /****************************************/

  /**
   * test if there is node after the current one
   */

  bool
  hasNext();

  /**
   * return the next node description
   */
  map<string, corba_pb_desc_t>::iterator 
  next();

  /**
   * set the iterator to the first element of nodes_list *
   */
  void
  reset();
#if 0
  /**
   * return the first node of the nodes list
   */
  corba_pb_desc_t * 
  begin();

  /**
   * return the last node of the nodes list
   */
  corba_pb_desc_t * 
  end();
#endif
  

  /****************************************/
  /* Problems list manipulation methods   */
  /* not really used (just for debugging) */
  /****************************************/

  /**
   * test if there is node after the current one
   */
  bool
  hasPbNext();

  /**
   * return the first node of the problems list
   */
  corba_pb_desc_t * 
  pbReset();

  /**
   * return the last node of the problems list
   */
  corba_pb_desc_t * 
  pbEnd();
  
  /**
   * return the next problem description
   */
  corba_pb_desc_t * 
  pbNext();

  /**
   * return the index of the provided problem in pbs_list vector
   */
  unsigned int 
  indexOfPb(corba_pb_desc_t& pb);


  string
  getAttributeValue(const char * attr_name, const DOMElement * elt);
  
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
   * The current profile (not used)
   * changed by the next, begin and end methods
   */
  map<string, corba_pb_desc_t>::iterator current_profile;

  /**
   * The current pb
   * changed by the pbNext, pbReset and pbEnd methods
   */
  int current_pb;
  /*********************************************************************/
  /* protected methods                                                 */
  /*********************************************************************/

  /** 
   * Init the XML processing
   */
  virtual bool 
  initXml();

  /**
   * parse a node element
   */
  virtual bool 
  parseNode (const DOMNode * element, 
	     string nodeId, string nodePath,
	     long var = -1);
  
  /**
   * parse an argument element
   * @param name     argument id
   * @param value    argument value
   * @param type     argument data type 
   * @param profile  current profile reference
   * @param lastArg  the argument index in the profile
   * @param var      Undocumented
   * @param dagNode  node reference in the Dag structure
   */
  virtual bool 
  checkArg(const string& name, 
	   const string& value, 
	   const string& type, 
	   diet_profile_t * profile, 
	   const unsigned int lastArg,
	   long var = -1,
	   BasicNode * dagNode = NULL);

  /**
   * parse an input port element
   * @param name     input port id
   * @param type     input port data type 
   * @param source   linked output port id
   * @param profile  current profile reference
   * @param lastArg  the input port index in the profile
   * @param var      Undocumented
   * @param dagNode  node reference in the Dag structure
   */
  virtual bool 
  checkIn(const string& name, 
	  const string& type, 
	  const string& source,
	  diet_profile_t * profile,
	  unsigned int lastArg,
	  long var = -1,
	  BasicNode * dagNode = NULL);

  /**
   * parse an input/output port element
   * @param name     inoutput port id
   * @param type     inoutput port data type 
   * @param source   linked output port id
   * @param profile  current profile reference
   * @param lastArg  the inoutput port index in the profile
   * @param var      Undocumented
   * @param dagNode  node reference in the Dag structure
   */  
  virtual bool
  checkInout(const string& name, 
	     const string& type, 
	     const string& source,
	     diet_profile_t * profile,
	     unsigned int lastArg,
	     long var = -1,
	     BasicNode * dagNode = NULL);

  /**
   * parse an output port element
   * @param name     output port id
   * @param type     output port data type 
   * @param sink     linked input port id
   * @param profile  current profile reference
   * @param lastArg  the output port index in the profile
   * @param var      Undocumented
   * @param dagNode  node reference in the Dag structure
   */  
   virtual bool
  checkOut(const string& name, 
	   const string& type,
	   const string& sink,
	   diet_profile_t * profile,
	   unsigned int lastArg,
	   long var = -1,
	   BasicNode * dagNode = NULL);

  /**
   * fill a profile with the appropriate parameter type
   * The data are NULL
   */  
  virtual bool 
  setParam(const wf_port_t param_type,
	   const string& name,
	   const string& type,
	   diet_profile_t * profile,
	   unsigned int lastArg,
	   long int var = -1,
	   BasicNode * dagNode = NULL,
	   const string * value = NULL);

  /**
   * parse a matrix argument
   * @param id       Port complete id (node id + # + port name)
   * @param element  Dom node representing a matrix
   * @param profile  current profile reference
   * @param lastArg  the output port index in the profile
   * @param var      Undocumented
   * @param dagNode  Undocumented
   */
  virtual bool
  checkMatrixArg(const string& id, const DOMElement * element,
		 diet_profile_t * profile, unsigned int lastArg,
		 long int var = -1,
		 BasicNode * dagNode = NULL);

  /**
   * parse a matrix input port
   * @param id       Port complete id (node id + # + port name)
   * @param element  Dom node representing a matrix
   * @param profile  current profile reference
   * @param lastArg  the output port index in the profile
   * @param var      Undocumented
   * @param dagNode  Undocumented
   */
  virtual bool
  checkMatrixIn(const string& id, const DOMElement * element,
		diet_profile_t * profile, unsigned int lastArg,
		long int var = -1,
		BasicNode * dagNode = NULL);

  /**
   * parse a matrix inout port
   * @param id       Port complete id (node id + # + port name)
   * @param element  Dom node representing a matrix
   * @param profile  current profile reference
   * @param lastArg  the output port index in the profile
   * @param var      Undocumented
   * @param dagNode  Undocumented
   */
  virtual bool
  checkMatrixInout(const string& id, const DOMElement * element,
		   diet_profile_t * profile, unsigned int lastArg,
		   long int var = -1,
		   BasicNode * dagNode = NULL);

  /**
   * parse a matrix output port
   * @param id       Port complete id (node id + # + port name)
   * @param element  Dom node representing a matrix
   * @param profile  current profile reference
   * @param lastArg  the output port index in the profile
   * @param var      Undocumented
   * @param dagNode  Undocumented
   */
  virtual bool
  checkMatrixOut(const string& id, const DOMElement * element,
		 diet_profile_t * profile, unsigned int lastArg,
		 long int var = -1,
		 BasicNode * dagNode = NULL);

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
		 BasicNode * dagNode = NULL,
		 const string * value = NULL);

  /**
   * check if the profile is already in the problems list
   */
  bool
  pbAlreadyRegistred(corba_pb_desc_t& pb_desc);
};

bool operator == (corba_pb_desc_t& a,   corba_pb_desc_t& b);

#endif   /* not defined _WFREADER_HH */



