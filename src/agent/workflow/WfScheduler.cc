/****************************************************************************/
/* The MA DAG scheduler interface                                           */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.1  2008/04/10 09:13:29  bisnard
 * New version of the MaDag where workflow node execution is triggered by the MaDag agent and done by a new CORBA object CltWfMgr located in the client
 *
 ****************************************************************************/
#include "WfScheduler.hh"

using namespace madag;

// Initialize static member
map<std::string, double> WfScheduler::avail;

/**
 * WfScheduler constructor
 */
WfScheduler::WfScheduler() {
  
} // end WfScheduler constructor

/**
 * WfScheduler destructor
 */
WfScheduler::~WfScheduler() {

} // end WfScheduler desctructor
