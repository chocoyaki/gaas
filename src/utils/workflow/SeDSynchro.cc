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

#include <algorithm>

#include "SeDSynchro.hh"

using namespace std;

/**
 * SeDSynchro constructor
 */
SeDSynchro::SeDSynchro() {
}

/**
 * SeDSynchro destructor
 */
SeDSynchro::~SeDSynchro() {
}

/**
 * Get access to the SeD
 */
void
SeDSynchro::getAccess() {
} // end getAccess

/**
 * Release  the SeD
 */
void
SeDSynchro::release() {
} // end release

/**
 * Get the top node
 */
std::string
SeDSynchro::top() {
  return this->myNodes.front();
} // end top

/**
 * Get the top node and remove it from the list
 */
std::string
SeDSynchro::pop() {
  string s = this->myNodes.front();
  this->myNodes.pop_front();
  return s;
} // end pop

/**
 * Add a node to SeD list
 */
void
SeDSynchro::add(std::string node_id) {
  if (find(this->myNodes.begin(), this->myNodes.end(), node_id) ==
      this->myNodes.end())
    this->myNodes.push_back(node_id);
} // end add

/**
 * Get the SeD queue length
 */
unsigned int
SeDSynchro::getLength() {
	return this->myNodes.size();
} // end getLength
