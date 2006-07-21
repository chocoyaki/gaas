/****************************************************************************/
/* Extended workflow description Reader                                     */
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
 * Revision 1.1  2006/04/14 13:56:17  aamar
 * This class process an XML representation of the Dag and create the equivalent
 * data structure. Used in client side (header).
 *
 ****************************************************************************/

#ifndef _WFEXTREADER_HH_
#define _WFEXTREADER_HH_

#include <vector>

#include "Dag.hh"
#include "WfReader.hh"

class WfExtReader : public WfReader {
public:
  /*********************************************************************/
  /* public methods                                                    */
  /*********************************************************************/

  WfExtReader(const char * content);
  
  virtual 
  ~WfExtReader();

  /**
   * Initialize the processing
   */
  virtual void 
  setup();

  /**
   * get a reference to Dag structure
   */
  Dag * getDag();

protected:
  /*********************************************************************/
  /* protected fields                                                  */
  /*********************************************************************/
  
  /**
   * Dag structure
   */
  Dag * myDag;

  /****************/
  /* Xml methods  */
  /****************/

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
	     long int var_node = -1);

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
  checkMatrixIn(const string& id, const DOMElement * element,
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
   * return the expression between {} in the id
   */
  string
  getExpr(string& id);

  /**
   * return the name of source/sink after expansion
   */
  string
  getRealName(string& real_name, long int var_node, long int var_port);
};


#endif   /* not defined _WFEXTREADER_HH */



