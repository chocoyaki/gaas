/****************************************************************************/
/* [Documentation Here!]                                                    */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.1  2008/04/10 08:38:50  bisnard
 * New version of the MaDag where workflow node execution is triggered by the MaDag agent and done by a new CORBA object CltWfMgr located in the client
 *
 ****************************************************************************/

#ifndef _SEDSYNCHRO_HH_
#define _SEDSYNCHRO_HH_

#include <list>
#include <string>

class SeDSynchro {
public:
  /**
   * SeDSynchro constructor
   */
  SeDSynchro();

  /**
   * SeDSynchro destructor
   */
  ~SeDSynchro();

  /**
   * Get access to the SeD
   */
  void
  getAccess();

  /**
   * Release  the SeD
   */
  void
  release();

  /**
   * Get the top node
   */
  std::string
  top();

  /**
   * Get the top node and remove it from the list
   */
  std::string
  pop();

  /**
   * Add a node to SeD list
   *
   * @param node_id Node identifier
   */
  void
  add(std::string node_id);
  
  /**
   * Get the SeD queue length
   */
  unsigned int
  getLength();
  
protected:
    /**
   * Synchronisation semaphore
   */
  std::list<std::string> myNodes;

private:
};


#endif   /* not defined _SEDSYNCHRO._HH */

