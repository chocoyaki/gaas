/****************************************************************************/
/* The class representing the nodes of a functional workflow                */
/*                                                                          */
/* Author(s):                                                               */
/* - Benjamin ISNARD (benjamin.isnard@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.1  2008/10/20 08:09:58  bisnard
 * new classes for functional workflows
 *
 */

#ifndef _FNODE_HH_
#define _FNODE_HH_

#include "WfPort.hh"

using namespace std;

/**
 * Class FNode : represents a node +instance+ in a functional workflow
 *
 * Interface : (inherits from Node class)
 *  - createChildInstance: creates a new instance of the functional node
 *      => returns a new FNode
 *  - getInstancesList: get the list of instances for this node  (NOT RECURSIVE)
 *  - newPort: creates a new port for the instance
 *  - isRealizable: returns true if the instance can be transformed into
 *      a dag node
 *  - realize: make a dag node using same node and ports info
 *
 * Attributes (inherited):
 *  - basic Node information (id, path)
 *  - PORTS (in, out, inout) => contains WfPort(s)
 *
 * Attributes (protected):
 *  - specific Node type info (diet, wsdl, etc...)
 *  - DATA OPERATORS: type, array of ports
 *  - INSTANCE INDEX (integer)
 *  - PARENT INSTANCE (ref to FNode)
 *  - INPUT & OUTPUT counters (int)
 *
 */

class FNode : public Node {


}; // end class FNode

class FConstantNode : public FNode {

}; // end class FConstantNode

class FSourceNode : public FNode {

}; // end class FSourceNode

class FProcNode : public FNode {

}; // end class FProcNode

#endif // _FNODE_HH_
