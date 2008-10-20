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

#ifndef _FWORKFLOW_HH_
#define _FWORKFLOW_HH_

#include "Dag.hh"
#include "FNode.hh"

using namespace std;

class FWorkflow : public NodeSet {

public:

  /***************************************************/
  /* constructors/destructor                         */
  /***************************************************/

  FWorkflow(const string& name);
  virtual ~FWorkflow();

  /***************************************************/
  /*            NodeSet methods                      */
  /***************************************************/

  virtual Node *
  getNode(const string& nodeId);

  virtual bool
  checkPrec();

  virtual uint
  size();

  /***************************************************/
  /*               public methods                    */
  /***************************************************/

  FProcNode*
  createProcessor(const string& id);

  FSourceNode*
  createSource(const string& id);

  FConstantNode*
  createConstant(const string& id);


private:

  /**
   * Workflow name
   */
  string name;

  /**
   * Workflow nodes for the interface (sources, sinks, constants)
   */
  map<string, FNode *> myInterface;

  /**
   * Workflow nodes for the processors
   */
  map<string, FProcNode*> myProc;

};


#endif // _FWORKFLOW_HH_




