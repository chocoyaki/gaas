/**
* @file  WfPortAdapter.hh
* 
* @brief  Port adapter classes used to split or merge container content 
* 
* @author  Benjamin ISNARD (benjamin.isnard@ens-lyon.fr)
* 
* @section Licence
*   |LICENSE|                                                                
*/


#ifndef _WFPORTADAPTER_HH_
#define _WFPORTADAPTER_HH_

#include <list>
#include <map>
#include <string>
#include <vector>
#include "MasterAgent.hh"
#include "WfPort.hh"
#include "WfDataWriter.hh"

class DagNodeOutPort;

/*****************************************************************************/
/*                        WfPortAdapter (ABSTRACT)                           */
/*****************************************************************************/

class WfPortAdapter {
public:
  virtual ~WfPortAdapter();

  /**
   * Static parsing & factory method for adapters
   * Note: if reference contains values then the valueAdapters will be defined
   * without a data type.
   * @param strRef  the complete port reference (ie content of source
   *                attribute of the port, for ex: node1#port0[2][3])
   */
  static WfPortAdapter*
  createAdapter(const std::string& strRef);

  /**
   * Node precedence analysis
   * @param node      the node containing the port linked to this adapter
   * @param nodeSet   the node set that contains the linked nodes
   */
  virtual void
  setNodePrecedence(WfNode* node, NodeSet* nodeSet)
    throw(WfStructException) = 0;

  /**
   * Node linking (used for node execution - step 1)
   * Checks the compatibility between both ports (data type and depth)
   * Requires setNodePrecedence to be called before
   * @param port         the port that contains the current adapter
   * @param adapterLevel the level of the current adapter within the port
   */
  virtual void
  connectPorts(WfPort* port, unsigned int adapterLevel)
    throw(WfStructException) = 0;

  /**
   * Returns the string reference (used for generating xml)
   */
  virtual std::string
  getSourceRef() const = 0;

  /**
   * Data ID retrieval (used for node execution - step 2)
   * REQUIRED: Node linking required before calling this method
   * REQUIRED: to be used only with adapters of DagNode ports
   * @return a string containing the data ID of the toplevel data item
   */
  virtual const std::string&
  getSourceDataID() = 0;

  /**
   * Returns the type of the data referenced by the adapter
   * @return the data type (integer)
   */
  virtual WfCst::WfDataType
  getSourceDataType() = 0;

  /**
   * Data value retrieval and display
   * @param dataWriter  the data output formatter
   */
  virtual void
  writeDataValue(WfDataWriter* dataWriter) = 0;

  /**
   * Returns true if the adapter created the dataID
   */
  virtual bool
  isDataIDCreator() = 0;

  /**
   * Destroy containers or data created by the adapter
   * @param MA  the master agent that manages data
   */
  virtual void
  freeAdapterPersistentData(MasterAgent_var& MA) = 0;
};


/*****************************************************************************/
/*                        WfSimplePortAdapter                                */
/*****************************************************************************/

class WfSimplePortAdapter : public WfPortAdapter {
public:
  /**
   * Destructor
   */
  ~WfSimplePortAdapter();

  /**
   * Constructor for a simple port
   * @param strRef the complete port reference
   */
  explicit WfSimplePortAdapter(const std::string& strRef);

  /**
   * Constructor for a simple port
   * @param port        the port to which the adapter points
   * @param portDagName (optional) the name of dag that contains that port
   *                    (for external links => adds a prefix to the ref)
   */
  WfSimplePortAdapter(WfPort* port, const std::string& portDagName = "");

  /**
   * Constructor for a simple port
   * @param port        the port to which the adapter points
   * @param indexes     the list of indexes of the element of the port
   * @param portDagName (optional) the name of dag that contains that port
   *                    (for external links => adds a prefix to the ref)
   */
  WfSimplePortAdapter(WfPort* port,
                      const std::list<unsigned int>& indexes,
                      const std::string& portDagName = "");


  // virtual base methods

  void
  setNodePrecedence(WfNode* node, NodeSet* nodeSet)
    throw(WfStructException);

  void
  connectPorts(WfPort* port, unsigned int adapterLevel)
    throw(WfStructException);

  std::string
  getSourceRef() const;

  const std::string&
  getSourceDataID();

  WfCst::WfDataType
  getSourceDataType();

  bool
  isDataIDCreator();

  void
  writeDataValue(WfDataWriter* dataWriter);

  void
  freeAdapterPersistentData(MasterAgent_var& MA);

protected:
  /**
   * Returns the port ref casted to a dag port
   */
  DagNodeOutPort*
  getSourcePort() const;

  const std::string&
  getPortName() const;

  const std::string&
  getNodeName() const;

  const std::string&
  getDagName() const;

  unsigned int
  getDepth() const;

  const std::list<unsigned int>&
  getElementIndexes();

private:
  std::string portName;
  std::string nodeName;
  std::string dagName;
  std::string dataID;
  std::list<unsigned int> eltIdxList;
  WfNode *nodePtr;
  WfPort *portPtr;

};  // end class WfSimplePortAdapter


/*****************************************************************************/
/*                       WfMultiplePortAdapter                               */
/*****************************************************************************/

class WfMultiplePortAdapter : public WfPortAdapter {
public:
  /**
   * Destructor
   */
  ~WfMultiplePortAdapter();

  /**
   * Separator and Parenthesis characters definition
   */
  static std::string parLeftChar;
  static std::string parRightChar;
  static std::string separatorChar;

  /**
   * Constructor for a multiple port
   * @param strRef the complete port reference
   */
  explicit WfMultiplePortAdapter(const std::string& strRef);

  WfMultiplePortAdapter();

  void
  addSubAdapter(WfPortAdapter* subAdapter);

  void
  setNodePrecedence(WfNode* node, NodeSet* nodeSet)
    throw(WfStructException);

  void
  connectPorts(WfPort* port, unsigned int adapterLevel)
    throw(WfStructException);

  std::string
  getSourceRef() const;

  const std::string&
  getSourceDataID();

  WfCst::WfDataType
  getSourceDataType();

  bool
  isDataIDCreator();

  void
  writeDataValue(WfDataWriter* dataWriter);
  /**
   * Destroy the data items created by the adapter or its childs
   * (calls itself recursively on all its childs)
   */
  void
  freeAdapterPersistentData(MasterAgent_var& MA);

protected:
  WfMultiplePortAdapter(const WfMultiplePortAdapter& mpa);

  void
  parse(const std::string& strRef, std::string::size_type& startPos);

private:
  std::string strRef;
  std::string containerID;
  std::list<WfPortAdapter*>  adapters;

  static std::string errorID;
};  // end class WfMultiplePortAdapter


/*****************************************************************************/
/*                            WfVoidAdapter                                  */
/*****************************************************************************/

class WfVoidAdapter : public WfPortAdapter {
public:
  static std::string voidRef;  // appears in dag

  WfVoidAdapter();
  ~WfVoidAdapter();

  // virtual base methods

  void
  setNodePrecedence(WfNode* node, NodeSet* nodeSet)
    throw(WfStructException);

  void
  connectPorts(WfPort* port, unsigned int adapterLevel)
    throw(WfStructException);

  std::string
  getSourceRef() const;

  const std::string&
  getSourceDataID();  // throws exception WfDataException::eVOID_DATA

  WfCst::WfDataType
  getSourceDataType();  // throws exception WfDataException::eVOID_DATA

  bool
  isDataIDCreator();

  void
  writeDataValue(WfDataWriter* dataWriter);

  void
  freeAdapterPersistentData(MasterAgent_var& MA);
};


/*****************************************************************************/
/*                          WfValueAdapter                                   */
/*****************************************************************************/

class WfValueAdapter : public WfPortAdapter {
  friend WfPortAdapter* WfPortAdapter::createAdapter(const std::string& strRef);
public:

  /**
   * The constructor with type will create an adapter that generates the dataID
   * (i.e. stores the data in the dataMgr) using the provided value.
   * (used by WF INSTANCIATOR for constant and source nodes)
   * @param valueType the type of the data (necessary to initialize data)
   * @param value the stringified value of the data (e.g. path for a file)
   */
  WfValueAdapter(WfCst::WfDataType valueType, const std::string& value);

  ~WfValueAdapter();

  /**
   * Tags used to delimitate the reference within the 'source' attribute
   * inside a DAG node. As a value adapter may be an element of a multiple
   * adapter, these tags are used when parsing the complex reference that
   * may contain different types of adapter references (simple, void,
   * multiple or value)
   */
  static std::string valStartTag;
  static std::string valFinishTag;

  // virtual base methods

  void
  setNodePrecedence(WfNode* node, NodeSet* nodeSet)
    throw(WfStructException);

  void
  connectPorts(WfPort* port, unsigned int adapterLevel)
    throw(WfStructException);

  std::string
  getSourceRef() const;

  const std::string&
  getSourceDataID();

  WfCst::WfDataType
  getSourceDataType();

  bool
  isDataIDCreator();

  void
  writeDataValue(WfDataWriter* dataWriter);

  void
  freeAdapterPersistentData(MasterAgent_var& MA);  //FIXME move to parent class

protected:
  /**
   * The basic constructor stores only the value but cannot be used to
   * generate the dataID due to missing data type
   * (used by MADAG ONLY during DAG PARSING)
   * @param value the (string) value of the data (e.g. path for a file)
   */
  WfValueAdapter(const std::string& value);

  char* newChar();
  short* newShort();
  int*  newInt();
  long* newLong();
  char* newString();
  char* newFile();
  float* newFloat();
  double* newDouble();

  std::string myDataID;  //FIXME move this attribute to parent class
  std::string myValue;
  WfCst::WfDataType myDataType;

  char *cx;
  short *sx;
  int *ix;
  long *lx;
  float *fx;
  double *dx;
  char *str;
};

/*****************************************************************************/
/*                         WfDataIDAdapter                                   */
/*****************************************************************************/

class WfDataIDAdapter : public WfPortAdapter {
  friend WfPortAdapter* WfPortAdapter::createAdapter(const std::string& strRef);
public:

  /**
   * The dataID constructor will create an adapter that re-uses the provided
   * dataID when requested. It will NOT generate a new dataID.
   * @param dataType the type of the data (type of elements if depth>0)
   * @param dataDepth the depth of the data (>0 if container)
   * @param dataID  the dataID provided by the dataMgr
   */
  WfDataIDAdapter(WfCst::WfDataType dataType, unsigned int dataDepth,
                  const std::string& dataID);

  /**
   * Basic constructor with dataID only
   * Note: cannot be used for writing data value (requires depth)
   * TODO modify writeDataValue to avoid using depth
   * @param dataID the data ID
   */
  WfDataIDAdapter(const std::string& dataID);

  ~WfDataIDAdapter();

  /**
   * Event Message types
   */
  enum eventMsg_e {
    ELTIDLIST
  };

  /**
   * Tags used to delimitate the reference within the 'source' attribute
   * inside a DAG node. As a value adapter may be an element of a multiple
   * adapter, these tags are used when parsing the complex reference that
   * may contain different types of adapter references (simple, void,
   * multiple or value)
   */
  static std::string IDStartTag;
  static std::string IDFinishTag;

  // virtual base methods

  void
  setNodePrecedence(WfNode* node, NodeSet* nodeSet)
    throw(WfStructException);

  void
  connectPorts(WfPort* port, unsigned int adapterLevel)
    throw(WfStructException);

  std::string
  getSourceRef() const;

  const std::string&
  getSourceDataID();

  WfCst::WfDataType
  getSourceDataType();

  bool
  isDataIDCreator();

  void
  writeDataValue(WfDataWriter* dataWriter);

  void
  freeAdapterPersistentData(MasterAgent_var& MA);

  // specific dataID adapter methods

  void
  getElements(std::vector<std::string>& vectID);

  std::string
  getDataID() const;

  std::string
  toString() const;

protected:
  static void
  getAndWriteData(WfDataWriter* dataWriter, const std::string& dataID,
                  WfCst::WfDataType  dataType, unsigned int dataDepth);

  std::string myDataID;  //FIXME move to parent class
  WfCst::WfDataType myDataType;
  unsigned int myDepth;

  /**
   * Cache containing the elements of container data
   * Used to avoid requesting the dataMgr for the same vectors of elts
   */
  static std::map<std::string, std::vector<std::string> > myCache;
};


#endif
