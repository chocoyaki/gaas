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
 * Revision 1.12  2009/05/27 08:49:43  bisnard
 * - modified condition output: new IF_THEN and IF_ELSE port types
 * - implemented MERGE and FILTER workflow nodes
 *
 * Revision 1.11  2009/05/15 11:10:20  bisnard
 * release for workflow conditional structure (if)
 *
 * Revision 1.10  2009/04/17 08:54:43  bisnard
 * renamed Node class as WfNode
 *
 * Revision 1.9  2009/04/09 09:56:20  bisnard
 * refactoring due to new class FActivityNode
 *
 * Revision 1.8  2009/04/08 09:34:56  bisnard
 * pending nodes mgmt moved to FWorkflow class
 * FWorkflow and FNode state graph revisited
 * FNodePort instanciation refactoring
 *
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
#include "FActivityNode.hh"
#include "FIfNode.hh"

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

  virtual WfNode *
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

  FActivityNode*
  createActivity(const string& id) throw (WfStructException);

  FIfNode*
  createIf(const string& id) throw (WfStructException);

  FMergeNode*
  createMerge(const string& id) throw (WfStructException);

  FFilterNode*
  createFilter(const string& id) throw (WfStructException);

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
  instanciationOnHold();

  bool
  instanciationStopped();

  bool
  instanciationCompleted();

  /**
   * Setup the information used when node instance (dagnode) is done
   * and some output ports instances have to be re-submitted to next
   * nodes (when cardinal was not known before node execution)
   * @param dagNode the dag node
   * @param dataHdl the data handle that could not be sent to the in port
   * @param outPort the FNode out port producing the data
   * @param inPort  the FNode in port receiving the data
   */
  void
  setPendingInstanceInfo(DagNode * dagNode,
                         FDataHandle * dataHdl,
                         FNodeOutPort * outPort,
                         FNodeInPort * inPort);

  /**
   * Handles the dagNode end of execution event
   * Uses the info set with setPendingInstanceInfo to re-send the
   * data to connected nodes
   * @param dagNode the dag node
   */
  void
  handlerDagNodeDone(DagNode* dagNode);

  /**
   * results
   */
  void
  displayAllResults(ostream& output);
  void
  displayDagSummary(ostream& output);

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
    W_INSTANC_READY,
    W_INSTANC_ONHOLD,
    W_INSTANC_PENDING,
    W_INSTANC_END,
    W_INSTANC_STOPPED } wfInstStatus_t;

  wfInstStatus_t myStatus;

  /**
   * Pending instance record
   */
  struct pendingDagNodeInfo_t {
    FDataHandle * dataHdl;
    FNodeOutPort * outPort;
    FNodeInPort * inPort;
  };

  /**
   * The list of instances that will trigger new instanciation (due to
   * dependency of other functional nodes on the cardinal of the outputs of
   * this node)
   */
  multimap<DagNode*, pendingDagNodeInfo_t> pendingNodes;

  /**
   * Critical section
   */
  omni_mutex myLock;

};


#endif // _FWORKFLOW_HH_




