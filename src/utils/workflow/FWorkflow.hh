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
 * Revision 1.7  2009/02/06 14:55:08  bisnard
 * setup exceptions
 *
 * Revision 1.6  2009/01/16 16:31:54  bisnard
 * added option to specify data source file name
 *
 * Revision 1.5  2009/01/16 13:54:50  bisnard
 * new version of the dag instanciator with iteration strategies for nodes with multiple input ports
 *
 * Revision 1.4  2008/12/09 12:15:59  bisnard
 * pending instanciation handling (uses dag outputs for instanciation
 * of a functional wf)
 *
 * Revision 1.3  2008/12/02 10:09:36  bisnard
 * added instanciation methods
 *
 * Revision 1.2  2008/10/22 09:29:00  bisnard
 * replaced uint by standard type
 *
 * Revision 1.1  2008/10/20 08:09:58  bisnard
 * new classes for functional workflows
 *
 */

#ifndef _FWORKFLOW_HH_
#define _FWORKFLOW_HH_

#include <map>
#include <iostream>

#include "Dag.hh"
#include "FNode.hh"

using namespace std;

class FWorkflow : public NodeSet {

public:

  /***************************************************/
  /* constructors/destructor                         */
  /***************************************************/

  FWorkflow(const string& id);

  /**
   * FWorkflow destructor
   * Note: it does not delete the instantiated dags (use deleteAllDags)
   */
  virtual ~FWorkflow();

  /***************************************************/
  /*            NodeSet methods                      */
  /***************************************************/

  virtual Node *
  getNode(const string& nodeId) throw (WfStructException);

  virtual void
  checkPrec(NodeSet* contextNodeSet) throw (WfStructException);

  /***************************************************/
  /*               public methods                    */
  /***************************************************/

  const string&
  getId();

  /**
   * creation
   * @return pointer to the node (does not return NULL)
   */

  FProcNode*
  createProcessor(const string& id) throw (WfStructException);

  FSourceNode*
  createSource(const string& id, WfCst::WfDataType type) throw (WfStructException);

  FConstantNode*
  createConstant(const string& id, WfCst::WfDataType type) throw (WfStructException);

  FSinkNode*
  createSink(const string& id, WfCst::WfDataType type, unsigned int depth)
      throw (WfStructException);

  /**
   * retrieval
   */

  FProcNode *
  getProcNode(const string& id) throw (WfStructException);

  FNode *
  getInterfaceNode(const string& id) throw (WfStructException);

  /**
   * instanciation
   */

  void
  initialize(const string& dataFileName) throw (WfStructException);

  const string&
  getDataSrcXmlFile();

  Dag *
  instanciateDag();

  void
  stopInstanciation();

  bool
  instanciationReady();

  bool
  instanciationPending(); // for dynamic dependencies

  bool
  instanciationStopped();

  bool
  instanciationCompleted();

  void
  handlerDagNodeDone(DagNode* dagNode) throw (WfDataException);

  /**
   * results
   */
  void
  displayAllResults(ostream& output);

  /**
   * memory free
   */
  void
  deleteAllResults();

  void
  deleteAllDags();


private:

  /**
   * Workflow id
   */
  string id;

  /**
   * Workflow nodes for the interface (sources, sinks, constants)
   */
  map<string, FNode *> myInterface;

  /**
   * Workflow nodes for the processors
   */
  map<string, FProcNode*> myProc;

  /**
   * Workflow nodes to be instanciated (ordered list)
   */
  list<FProcNode*> todoProc;

  /**
   * File to use for instanciation of data sources
   */
  string dataSrcXmlFile;

  /**
   * List of generated dags
   */

  list<Dag*> myDags;

  /**
   * Counter of generated dags
   */
  int dagCounter;

  /**
   * Instanciation status
   */
  typedef enum {
    INSTANC_READY,
    INSTANC_ONHOLD,
    INSTANC_PENDING,
    INSTANC_END,
    INSTANC_STOPPED } instanciationStatus_t;

  instanciationStatus_t myStatus;

  /**
   * Critical section
   */
  omni_mutex myLock;

};


#endif // _FWORKFLOW_HH_




