/**
* @file  FWorkflow.hh
* 
* @brief  The class representing a functional workflow Contains the workflow node
* 
* @author  Benjamin ISNARD (Benjamin.Isnard@ens-lyon.fr)
* 
* @section Licence
*   |LICENSE|                                                                
*/
/****************************************************************************/
/* The class representing a functional workflow                             */
/* Contains the workflow nodes and manages their instanciation as one or    */
/* several dags.                                                            */
/****************************************************************************/

#ifndef _FWORKFLOW_HH_
#define _FWORKFLOW_HH_

#include <iostream>
#include <list>
#include <map>
#include <string>
#include <vector>

#include "NodeSet.hh"
#include "FNode.hh"

/* The different types of nodes used in a functional workflow */

class FActivityNode;
class FIfNode;
class FMergeNode;
class FFilterNode;
class FLoopNode;

class FWorkflow : public NodeSet, public FProcNode {
public:
  /***************************************************/
  /* constructors/destructor                         */
  /***************************************************/

  /**
   * Constructor of toplevel workflow (no parent)
   * @param id    workflow identifier
   * @param name  name of the workflow (only descriptive)
   */
  FWorkflow(std::string id, std::string name);

  /**
   * Constructor of sub-workflow (child of another workflow)
   * @param id        a unique identifier (among all parent wf processors)
   * @param name      name of the sub-workflow
   * @param parentWf  ref to the parent wf
   */
  FWorkflow(std::string id, std::string name, FWorkflow* parentWf);

  /**
   * FWorkflow destructor
   * Note: it does not delete the instantiated dags (use deleteAllDags)
   */
  virtual ~FWorkflow();

  /***************************************************/
  /* event message types                             */
  /***************************************************/

  enum eventMsg_e {
    INST,
    INSTDONE,
    INSTERROR,
    COMPLETED,
    CANCELLED
  };

  /***************************************************/
  /*            NodeSet methods                      */
  /***************************************************/
  virtual WfNode *
  getNode(const std::string& nodeId) throw(WfStructException);

  virtual void
  checkPrec(NodeSet* contextNodeSet) throw(WfStructException);

  /***************************************************/
  /*               FNode methods                     */
  /***************************************************/

  virtual void
  initialize();

  virtual void
  instanciate(Dag* dag);

  virtual void
  freeNodePersistentData(MasterAgent_var& MA);

  /***************************************************/
  /*             FProcNode methods                   */
  /***************************************************/
  virtual void
  createRealInstance(Dag* dag,
                     const FDataTag& currTag,
                     std::vector<FDataHandle*>& currDataLine);

  virtual void
  createVoidInstance(const FDataTag& currTag,
                     std::vector<FDataHandle*>& currDataLine);

  virtual void
  updateInstanciationStatus();

  /***************************************************/
  /*               public methods                    */
  /***************************************************/
  std::string
  getName() const;

  virtual std::string
  toString() const;

  virtual FWorkflow*
  getRootWorkflow() const;

  void
  setDataSrcXmlFile(const std::string& dataFileName);

  const std::string&
  getDataSrcXmlFile();

  /**
   * Factory methods for workflow nodes
   * @return pointer to the node (does not return NULL)
   */

  FActivityNode*
  createActivity(const std::string& id) throw(WfStructException);

  FIfNode*
  createIf(const std::string& id) throw(WfStructException);

  FMergeNode*
  createMerge(const std::string& id) throw(WfStructException);

  FFilterNode*
  createFilter(const std::string& id) throw(WfStructException);

  FLoopNode*
  createLoop(const std::string& id) throw(WfStructException);

  FWorkflow*
  createSubWorkflow(const std::string& id, const std::string& name)
    throw(WfStructException);

  FSourceNode*
  createSource(const std::string& id, WfCst::WfDataType type)
    throw(WfStructException);

  FConstantNode*
  createConstant(const std::string& id, WfCst::WfDataType type)
    throw(WfStructException);

  FSinkNode*
  createSink(const std::string& id, WfCst::WfDataType type, unsigned int depth)
    throw(WfStructException);

  /**
   * Get methods for workflow nodes
   * @return pointer to the node (does not return NULL)
   */

  FProcNode *
  getProcNode(const std::string& id) throw(WfStructException);

  FNode *
  getInterfaceNode(const std::string& id) throw(WfStructException);


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
   * Write a complete status of the workflow on a stream (XML encoded)
   * This status contains the current state of all generated dags
   * @param output  output stream
   */
  void
  writeAllDagsState(std::ostream& output);

  /**
   * Write a status of all the sources and sinks of the workflow
   * including sub-workflows sources. This can be used as input for
   * another execution of the same data to avoid re-uploading all
   * source data to the platform. (uses existing data IDs)
   * @param output  output stream
   */
  void
  writeAllSourcesAndSinksData(std::ostream& output);

  /**
   * Initialize a workflow using the transcript of all generated
   * dags from a previous execution of the workflow. Only
   * tasks that have not been executed during that execution will
   * be instanciated and executed. Data already saved on the platform
   * will be re-used (uses data IDs written in the transcript)
   */
  void
  readDagsState(std::list<Dag*>& dagList);

  /**
   * Check workflow execution transcript for a given dag node id
   * @param dagNodeId   id of the node (must match)
   * @param dagNodePtr  returns a ptr to the dagNode if found
   * @param isDone      returns true if dagNode is done
   */
  void
  findDagNodeTranscript(const std::string& dagNodeId, DagNode* & dagNodePtr,
                        bool& isDone);

  /**
   * results
   */

  /**
   * Download all sinks data description
   * (data IDs for all nodes and leaves + values for leaves only)
   * Must be called before other results methods
   */
  void
  downloadSinkData();

  void
  displayAllResults(std::ostream& output);

  void
  displayDagSummary(std::ostream& output);

  void
  getSinkContainer(const std::string& sinkName, std::string& containerID);

  /**
   * memory free
   */

  /**
   * Remove all data produced by the workflow from the platform
   */
  void
  deleteAllResults();

  /**
   * Remove input data created by the workflow engine on the platform
   * (sources and constants)
   */
  void
  deleteAllInputData(MasterAgent_var& MA);

  /**
   * Remove intermediate data created by the workflow engine on the platform
   */
  void
  deleteAllIntermediateData(MasterAgent_var& MA);

  /**
   * Free memory used for dags generated by the workflow engine
   */
  void
  deleteAllDags();

private:
  /**
   * Workflow Name
   * (descriptive)
   */
  std::string myName;

  /**
   * Workflow nodes for the interface (sources, sinks, constants)
   */
  std::map<std::string, FNode *> myInterface;

  /**
   * Workflow nodes for the processors
   */
  std::map<std::string, FProcNode*> myProc;

  /**
   * Workflow nodes to be instanciated (ordered list)
   */
  std::list<FProcNode*> todoProc;

  /**
   * File to use for instanciation of data sources
   */
  std::string dataSrcXmlFile;

  /**
   * List of generated dags
   */

  std::list<Dag*> myDags;

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
  std::multimap<DagNode*, pendingDagNodeInfo_t> pendingNodes;

  /**
   * The execution transcript of the workflow
   * It contains all dagNodes that have been executed (status isDone)
   */
  std::map<std::string, DagNode*> transcriptNodes;

  /**
   * Critical section
   */
  omni_mutex myLock;

  /***********************/
  /*   PRIVATE METHODS   */
  /***********************/

  void
  writeAllSourcesState(std::ostream& output);

  void
  writeAllSinksState(std::ostream& output);
};


#endif  // _FWORKFLOW_HH_




