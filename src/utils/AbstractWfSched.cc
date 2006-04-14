/****************************************************************************/
/* Workflow scheduler (Abstract class)                                      */
/* This abstract class must be implemented to write other workflow          */ 
/* schedulers                                                               */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.1  2006/04/14 14:03:17  aamar
 * Abstract class that defines the workflow scheduler interface.
 *
 ****************************************************************************/

#include "AbstractWfSched.hh"

AbstractWfSched::~AbstractWfSched() {
}

/**
 * Assign a dag to the scheduler *
 * implicitely called by the runtime *
 */
void
AbstractWfSched::setDag(Dag * dag) {
  myDag = dag;
}


/**
 * Set the workflow submission response *
 * implicitely called by the runtime *
 */
void 
AbstractWfSched::setResponse(wf_response_t *response) {
  this->response = response;
}
