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

// Qt header 
#include <Qt/QDomDocument>

// Workflow related headers
#include "WfUtils.hh"
#include "BasicNode.hh"

class WfReader {
public:
  friend class MaDag_impl;
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
  QDomDocument document;


  /**
   * Nodes list
   */
  std::map<string, corba_pb_desc_t> nodes_list;

  /**
   * Profiles list
   */
  std::vector<corba_pb_desc_t> pbs_list;

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
	   BasicNode * dagNode = NULL);

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
	  BasicNode * dagNode = NULL);

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
	     BasicNode * dagNode = NULL);

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
	   BasicNode * dagNode = NULL);

  /**
   * fill a profile with the appropriate parameter type
   */  
  virtual bool 
  setParam(const wf_port_t param_type,
	   const QString& name,
	   const QString& type,
	   diet_profile_t * profile,
	   unsigned int lastArg,
	   BasicNode * dagNode = NULL,
	   const QString * value = NULL);

  /**
   * check if the profile is already in the problems list
   */
  bool
  pbAlreadyRegistred(corba_pb_desc_t& pb_desc);
};

bool operator == (corba_pb_desc_t& a,   corba_pb_desc_t& b);

#endif   /* not defined _WFREADER_HH */



