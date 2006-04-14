/****************************************************************************/
/* Basic Workflow Scheduler                                                 */
/* This scheduler use only the data dependencies to execute the workflow    */
/* The order between ready nodes is random                                  */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.1  2006/04/14 14:04:51  aamar
 * The basis scheduler (random scheduling).
 *
 ****************************************************************************/

#ifndef _SIMPLEWFSCHED_HH_
#define _SIMPLEWFSCHED_HH_

#include "AbstractWfSched.hh"

class SimpleWfSched : public AbstractWfSched {
public:
  /*********************************************************************/
  /* public methods                                                    */
  /*********************************************************************/
  SimpleWfSched();

  virtual
  ~SimpleWfSched();

  /**
   * The scheduler execution method *
   * implicitely called by the runtime *
   */
  virtual 
  void execute();
private:
};


#endif   /* not defined _SIMPLEWFSCHED_HH */



