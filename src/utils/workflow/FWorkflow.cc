/****************************************************************************/
/* The class representing a functional workflow                             */
/* Contains the workflow nodes and manages their instanciation as one or    */
/* several dags.                                                            */
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

#include "debug.hh"
#include "FWorkflow.hh"

FWorkflow::FWorkflow(const string& name) : name(name) {
}

FWorkflow::~FWorkflow() {
}

Node *
FWorkflow::getNode(const string& nodeId) {
}

bool
FWorkflow::checkPrec() {
}

uint
FWorkflow::size() {
}

FProcNode*
FWorkflow::createProcessor(const string& id) {
}

FSourceNode*
FWorkflow::createSource(const string& id) {
}

FConstantNode*
FWorkflow::createConstant(const string& id) {
}
