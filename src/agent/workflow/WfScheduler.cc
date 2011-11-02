/**
* @file WfScheduler.cc
* 
* @brief  The MA DAG scheduler interface
* 
* @author - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)
* 
* @section Licence
*   |LICENSE|                                                                
*/
/* $Id$
 * $Log$
 * Revision 1.2  2008/04/28 11:54:52  bisnard
 * new methods setNodePriorities & setNodesEFT replacing schedule
 * nodes sort done in separate method in Dag class
 *
 * Revision 1.1  2008/04/10 09:13:29  bisnard
 * New version of the MaDag where workflow node execution is triggered by the MaDag agent and done by a new CORBA object CltWfMgr located in the client
 *
 ****************************************************************************/
#include "WfScheduler.hh"

using namespace madag;

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
