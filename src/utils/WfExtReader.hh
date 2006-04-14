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
 * Revision 1.1  2006/04/14 13:56:17  aamar
 * This class process an XML representation of the Dag and create the equivalent
 * data structure. Used in client side (header).
 *
 ****************************************************************************/

#ifndef _WFEXTREADER_HH_
#define _WFEXTREADER_HH_

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
  parseNode (const QDomElement & element, 
	     QString nodeId, QString nodePath);

  /**
   * parse an argument element
   * @param name     argument id
   * @param value    argument value
   * @param type     argument data type 
   * @param profile  current profile reference
   * @param lastArg  the argument index in the profile
   * @param dagNode  node reference in the Dag structure
   */
  virtual bool 
  checkArg(const QString& name, 
	   const QString& value, 
	   const QString& type, 
	   diet_profile_t * profile, 
	   const unsigned int lastArg,
	   Node * dagNode = NULL);

  /**
   * parse an input port element
   * @param name     input port id
   * @param type     input port data type 
   * @param source   linked output port id
   * @param profile  current profile reference
   * @param lastArg  the input port index in the profile
   * @param dagNode  node reference in the Dag structure
   */
  virtual bool 
  checkIn(const QString& name, 
	  const QString& type, 
	  const QString& source,
	  diet_profile_t * profile,
	  unsigned int lastArg,
	  Node * dagNode = NULL);

  /**
   * parse an input/output port element
   * @param name     inoutput port id
   * @param type     inoutput port data type 
   * @param source   linked output port id
   * @param profile  current profile reference
   * @param lastArg  the inoutput port index in the profile
   * @param dagNode  node reference in the Dag structure
   */  
  virtual bool 
  checkInout(const QString& name, 
	     const QString& type, 
	     const QString& source,
	     diet_profile_t * profile,
	     unsigned int lastArg,
	     Node * dagNode = NULL);

  /**
   * parse an output port element
   * @param name     output port id
   * @param type     output port data type 
   * @param sink     linked input port id
   * @param profile  current profile reference
   * @param lastArg  the output port index in the profile
   * @param dagNode  node reference in the Dag structure
   */  
  virtual bool 
  checkOut(const QString& name, 
	   const QString& type,
	   const QString& sink,
	   diet_profile_t * profile,
	   unsigned int lastArg,
	   Node * dagNode = NULL);

  /**
   * fill a profile with the appropriate parameter type and create the 
   * node ports
   */  
  virtual bool 
  setParam(const wf_port_t param_type,
	   const QString& name,
	   const QString& type,
	   diet_profile_t* profile,
	   unsigned int lastArg,
	   Node * dagNode,
	   const QString * value = NULL);
};


#endif   /* not defined _WFEXTREADER_HH */



