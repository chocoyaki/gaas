/**
 * @file DietLogComponent.hh
 *
 * @brief  DietLogComponent header - LogService module for DIET
 *
 * @author  Georg Hoesch (hoesch@in.tum.de)
 *          Cyrille Pontvieux (cyrille.pontvieux@edu.univ-fcomte.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


#ifndef _DIETLOGCOMPONENT_HH_
#define _DIETLOGCOMPONENT_HH_

#include "deltacloud_config.h"

#include <string>
#include "LogComponent.hh"
#include "DIET_data.h"

#include "omnithread.h"
#include "LogORBMgr.hh"
#include "commonLogTypes.hh"
#include "response.hh"
#include "common_types.hh"

#ifdef DIET_USE_DELTACLOUD

namespace IaaS {
  class Instance;
  class Image;
}

class ServiceWrapper;
#endif


#define PINGTHREAD_SYNCHRO_FREQUENCY 60
#define PINGTHREAD_SLEEP_SEC 1
#define PINGTHREAD_SLEEP_NSEC 0
#ifdef WIN32
#define DIET_API_LIB __declspec(dllexport)
#else
#define DIET_API_LIB
#endif
/**
 * This flag configures the behaviour when communication
 * errors occure during operation. If it is defined, the
 * component will stop the agent/SeD. If it is undefined,
 * the agent will print a warning and continue without
 * sending logmessages.
 */


// forward declaraction
class DietLogComponent;

/**
 * The FlushBufferThread is an internal thread of the
 * DietLogComponent(DLC). It flushes the DLC in regular
 * intervals by calling its sendOutBuffer() function.
 */
class FlushBufferThread : public omni_thread {
public:
  /**
   * Constructor.
   * @param DLC a pointer to the DietLogComponent to flush.
   * @param outBufferTime this value that indicates the maximum
   * for the messages in the outBuffer. The thread sleeps outBufferTime
   * nanoseconds between two flush operations.
   */
  FlushBufferThread(DietLogComponent * DLC, int outBufferTime);

  /**
   * Start the thread.
   */
  void
  startThread();

  /**
   * Stop the thread.
   * The object will be deleted by the orb.
   */
  void
  stopThread();

protected:
  /**
   * The main loop of the thread.
   * It just sleeps and calls dlc->sendOutBuffer().
   */
  void *
  run_undetached(void *params);

  /**
   * Destructor. Is called by the orb when the thread is
   * stopped. Do not call it yourself.
   */
  ~FlushBufferThread();

private:
  /**
   * Reference to the DLC to flush.
   */
  DietLogComponent *myDLC;

  /**
   * controls the main loop
   */
  bool threadRunning;

  /**
   * time the thread should sleep between two flushes in nanoseconds.
   */
  int outBufferTimeNSec;
};

/**
 * The PingThread regularly calls functions in the
 * DietLogComponent(DLC) to synchronize with the
 * LogCentral. If this thread is not active, the
 * LogCentral will disconnect the component after
 * some time.
 */
class PingThread : public omni_thread {
public:
  /**
   * Constructor.
   * @param DLC reference to the DietLogComponent of this thread.
   */
  explicit
  PingThread(DietLogComponent *DLC);

  /**
   * Start the thread.
   */
  void
  startThread();

  /**
   * Stop the thread.
   * The object will be deleted by the orb.
   */
  void
  stopThread();

protected:
  /**
   * The main loop of the thread.
   * It just sleeps and calls ping and/or synchronize of the DLC
   */
  void *
  run_undetached(void *params);

  /**
   * Destructor. Is called by the orb when the thread is
   * stopped. Do not call it yourself.
   */
  ~PingThread();

private:
  /**
   * Reference to this threads DLC.
   */
  DietLogComponent *myDLC;

  /**
   * controls the main loop
   */
  bool threadRunning;
};

/**
 * The DietLogComponent encapsulates all LogCentral communication
 * in a DIET specific class. It acts as servant and as local proxy
 * for the LogCentral at the same time. It offers a number of
 * functions for the creation of all possible logmessages. It also
 * handles errors with the DIET internal mechanisms.
 *
 * The implementation of the ComponentConfigurator is done internally
 * with the dlcMutex, the tagNames[] and the tagFlags[]. The arrays
 * contain the names of the tags this component offers and their
 * actual state. The dlcMutex synchronises the access to these arrays
 * and the outBuffer. To query their status, logXXX functions just
 * check for the flag with the index that corresponds to their tag.
 */
class DIET_API_LIB DietLogComponent :  public POA_ComponentConfigurator,
public PortableServer::RefCountServantBase {
public:
  /**
   * Initialise the DietLogComponent.
   * @param name The name of this component. Set to "" if this is a SeD.
   * @param outBufferMaxSize the size of the outBuffer. If the size is 0,
   * messages will not be cached but sent immediately.
   * @param argc number of command line arguments
   * @param argv command line arguments array
   */
  DietLogComponent(const char *name, int outBufferMaxSize,
                   int argc, char **argv);

  /**
   * Release the memory allocated by this class;
   */
  ~DietLogComponent();

  /**
   * Connect to LogCentral to enable logging. Must be called
   * after activation.
   * @param agentType  Undocumented
   * @param parentName The name of this agent's parent. Pass
   * NULL or "" if this is a Master agent.
   * @param outBufferTime The maximum time messages stay in the
   * outBuffer. After this time, the FlushBufferThread sends them.
   * The value specified nanoseconds.
   */
  int
  run(const char *agentType, const char *parentName, int outBufferTime);

  /**
   * Client functions (for DIET)
   * All these functions
   * - check if the msg must be sent
   * - deal with any errors internally
   * So just envoke these functions, the DLC does the rest.
   */

  /**
   * Notifies the change of services on an agent or SeD.
   * This can only declare one service at a time, so each service
   * has to be declared in a single message.
   */
  void
  logAddService(const corba_profile_desc_t *serviceProfile);

  void
  logNewParent(const char *type, const char *parent);

  void
  logDisconnect();

  void
  logRemoveElement();

  /**
   * Request best SeD for problem
   */
  void
  logAskForSeD(const corba_request_t *request);

  void
  logSedChosen(const corba_request_t *request,
               const corba_response_t *response);

#ifdef HAVE_MULTI_MA
  /**
   * Notifies the list of neighbors MA in multi-MA environment.
   */
  void
  logNeighbors(const char *list);
#endif  // HAVE_MULTI_MA

  /**
   * Solve a problem
   * (No ID here, ID exists only in async and is client-specific)
   */
  void
  logBeginSolve(const char *path, const corba_profile_t *problem);

  // modif bisnard_logs_1
  void
  logEndDownload(const char *path, const corba_profile_t *problem);

  void
  logEndSolve(const char *path, const corba_profile_t *problem);

  /**
   * Track data movements
   */
  void
  logDataStore(const char *dataID, const long unsigned int size,
               const long base_type, const char *type);     // data profile ?

  void
  logDataRelease(const char *dataID);

  // invoked by Sender:
  void
  logDataBeginTransfer(const char *dataID, const char *destAgent);

  void
  logDataEndTransfer(const char *dataID, const char *destAgent);

  void
  logDataTransferTime(const char *dataID, const char *destAgent,
                      const unsigned long elapsedTime);

  /**
   * NWS values
   */
  void
  logMem(double mem);

  void
  logLoad(double load);

  void
  logLatency(double latency);

  void
  logBandwidth(double bandwidth);


  /**
   * No-User functions
   */

  /**
   * Synchronise with the LogCentral.
   * Not to be called by the user.
   */
  void
  synchronize();

  /**
   * Ping the LogCentral. Not to be called by the user.
   */
  void
  ping();

  /**
   * Flushes the outBuffer. All stored messages are sent.
   * Should not be called by the user.
   */
  void
  sendOutBuffer();

  /**
   * create a logmessage and store it in the outBuffer.
   * Send the buffer if it is full. Not to be called by
   * the user.
   */
  void
  log(const char *tag, const char *msg);

  /**
   * Implements the ComponentConfigurator class in LogComponent.idl
   */
  void
  setTagFilter(const tag_list_t &tagList);
  /**
   * Implements the ComponentConfigurator class in LogComponent.idl
   */
  void
  addTagFilter(const tag_list_t &tagList);
  /**
   * Implements the ComponentConfigurator class in LogComponent.idl
   */
  void
  removeTagFilter(const tag_list_t &tagList);
  /**
   * Implements the ComponentConfigurator class in LogComponent.idl
   */
  void
  test();


  /**
   * get a string representation of the v_tag value
   */
  char *
  getEstimationTags(const int v_tag);

#ifdef HAVE_WORKFLOW

  /**
   * Send msg : workflow node queued (ie ready for execution)
   */
  void
  logWfNodeReady(const char *dagName, const char *nodeName);

  /**
   * Send msg : workflow node execution start
   */
  void
  logWfNodeStart(const char *dagName, const char *nodeName,
                 const char *sedName, const char *pbName,
                 const unsigned long reqID);

  void
  logWfNodeStart(const char *dagName, const char *nodeName);

  /**
   * Send msg : workflow node execution finish
   */
  void
  logWfNodeFinish(const char *dagName, const char *nodeName);

  /**
   * Send msg : workflow node failure
   */
  void
  logWfNodeFailed(const char *dagName, const char *nodeName);
  /**
   * Send msg and workflow processing time
   * @param msg the message which identify dag and request_id of the dag
   * @param ptime the time elapsed by the MA_DAG in ms to process
   * the workflow submission
   */
  void
  logDag(const char *msg);
  /**
   * Send msg : madag schedulerType
   * @param msg the message which contains the scheduler type of the MA_DAG
   */
  void
  maDagSchedulerType(const char *msg);
#endif  // HAVE_WORKFLOW

#ifdef DIET_USE_DELTACLOUD

  void
  logVMDeployStart(const IaaS::Image & image, const char * cloudName, const char * vmId, const char * sedName);

  void
  logVMRunning(const IaaS::Instance & vmInstance);

  void
  logVMOSReady(const IaaS::Instance & vmInstance);

  void
  logVMDestroyStart(const IaaS::Instance & vmInstance);

  void
  logVMDestroyEnd(const IaaS::Instance & vmInstance);

  void
  logVMServiceWrapped(const char * sedName, const ServiceWrapper & serviceWrapper, const char * vmIP,const char * vmUserName);

#endif


private:
  /**
   * Helper function - allocates an array of bools which are
   * set to false.
   */
  bool *
  createBoolArrayFalse(int size);

  /**
   * Checks if a given tag_list_t contains a '*'
   */
  bool
  containsStar(const tag_list_t *tagList);

  /**
   * Checks if a given tag is stored in the list of available tags
   * on this component and returns its index in the internal array.
   * Returns -1 if the tag is not known to this component.
   */
  int
  getTagIndex(const char *tag);

  /**
   * Takes all necessary actions if a remove procedure call fails
   * due to a lost connection. It either shuts the whole agent down
   * or deactivates all communication with the LogCentral.
   */
  void
  handleDisconnect(CORBA::SystemException &e);

  /**
   * return the local time (including milliseconds)
   */
  log_time_t
  getLocalTime();


  /**
   * connectionstatus to LogCentral
   */
  bool isConnected;

  /**
   * Remote LogCentralComponent object
   */
  LogCentralComponent_var myLCC;
  // Name of the component
  std::string myName;


  /**
   * this components name
   */
  CORBA::String_var name;

  /**
   * this DLCs pingThread
   */
  PingThread *pingThread;

  /**
   * this DLCs flushBufferThread
   */
  FlushBufferThread *flushBufferThread;

  /**
   * Synchronises the whole class
   */
  mutable omni_mutex dlcMutex;

  /**
   * The size of the outBuffer. If this value is reached, the
   * buffer will be send.
   */
  int outBufferMaxSize;

  /**
   * The log_msg_t buffer storing all the messages that have to
   * be sent.
   */
  log_msg_buf_t outBuffer;

  /**
   * The number of available tags.
   */
  int tagCount;

  /**
   * The names of the available tags
   */
  char **tagNames;

  /**
   * The status of the available tags. If this flag is true,
   * the tag must be sent, otherwise it must not be sent.
   */
  bool *tagFlags;
};

class DietLogComponentFwdr :  public POA_ComponentConfigurator,
public PortableServer::RefCountServantBase {
public:
  DietLogComponentFwdr(CorbaLogForwarder_ptr fwdr, const char *objName);

  void
  setTagFilter(const tag_list_t &tagList);

  void
  addTagFilter(const tag_list_t &tagList);

  void
  removeTagFilter(const tag_list_t &tagList);

  void
  test();

protected:
  CorbaLogForwarder_ptr forwarder;
  char *objName;
};

#endif /* ifndef _DIETLOGCOMPONENT_HH_ */
