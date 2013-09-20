/**
 * @file DietLogComponent.cc
 *
 * @brief  DietLogComponent - LogService module for DIET
 *
 * @author   Georg Hoesch (hoesch@in.tum.de)
 *           Cyrille Pontvieux (cyrille.pontvieux@edu.univ-fcomte.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


#include <cstring>
#include <cstdlib>
#ifndef __WIN32__ 
#include <unistd.h>   // For gethostname()
#else
#include "OSIndependance.hh"
#endif
#include <ctime>
#include <iostream>
#include <string>
using namespace std;

#include "LogORBMgr.hh"
#include "debug.hh"
#include "DietLogComponent.hh"

#ifdef DIET_USE_DELTACLOUD
#include "Instance.hh"
#include "Image.hh"
#include "SeD_deltacloud.hh"
#endif

/**
 * Error message - exit with exit_value.
 */
#define DLC_ERROR(formatted_msg, exit_value)                     \
  cerr << "DIET ERROR: " << formatted_msg << "." << endl        \
       << "cannot proceed." << endl;                                 \
  exit(exit_value)

/**
 * Warning message.
 */
#define DLC_WARNING(formatted_msg)                                      \
  cerr << "DIET WARNING: " << formatted_msg << "." << endl              \
       << "DIET can proceed, but you may encounter unexpected behaviour." << \
  endl

/**
 * FlushBufferThread
 */
FlushBufferThread::FlushBufferThread(DietLogComponent *DLC,
                                     int outBufferTime) {
  threadRunning = false;
  myDLC = DLC;

  outBufferTimeNSec = outBufferTime;
}

FlushBufferThread::~FlushBufferThread() {
  // nothing to do
}

void
FlushBufferThread::startThread() {
  if (threadRunning) {
    return;
  } else {
    threadRunning = true;
    start_undetached();
  }
}

void
FlushBufferThread::stopThread() {
  if (!threadRunning) {
    return;
  }
  threadRunning = false;
  join(NULL);
}

void *
FlushBufferThread::run_undetached(void *params) {
  while (threadRunning) {
    myDLC->sendOutBuffer();

    sleep(0, outBufferTimeNSec);
  }
  return NULL;
}

/**
 * PingThread
 */

PingThread::PingThread(DietLogComponent *DLC) {
  threadRunning = false;
  myDLC = DLC;
}

PingThread::~PingThread() {
  // cannot be called directly / has nothing to do
}

void
PingThread::startThread() {
  if (threadRunning) {
    return;
  }
  threadRunning = true;
  start_undetached();
}

void
PingThread::stopThread() {
  if (!threadRunning) {
    return;
  }
  threadRunning = false;
  join(NULL);
}

void *
PingThread::run_undetached(void *params) {
  int counter = 0;

  while (threadRunning) {
    myDLC->ping();

    if (counter == PINGTHREAD_SYNCHRO_FREQUENCY) {
      myDLC->synchronize();
      counter = 0;
    }
    counter++;

    sleep(PINGTHREAD_SLEEP_SEC, PINGTHREAD_SLEEP_NSEC);
  }
  return NULL;
} // run_undetached

/****************************************************************************/

/**
 * DietLogComponent
 */

DietLogComponent::DietLogComponent(const char *name,
                                   int outBufferMaxSize, int argc,
                                   char **argv) {
  try {
    LogORBMgr::init(argc, (char **) argv);
  } catch (...) {
    fprintf(stderr, "Failed to initialize log orb manager \n");
  }

  this->name = CORBA::string_dup(name);
  this->myName = string(name);
  this->outBufferMaxSize = outBufferMaxSize;
  isConnected = false;

  pingThread = NULL;
  flushBufferThread = NULL;  // modif bisnard_logs_1

  // define tags
  tagCount = 35;  // modif bisnard_logs_1
  tagFlags = createBoolArrayFalse(tagCount);
  tagNames = new char *[this->tagCount];
  tagNames[0] = strdup("ADD_SERVICE");
  tagNames[1] = strdup("ASK_FOR_SED");
  tagNames[2] = strdup("SED_CHOSEN");
  tagNames[3] = strdup("BEGIN_SOLVE");
  tagNames[4] = strdup("END_SOLVE");
  tagNames[5] = strdup("DATA_RELEASED");
  tagNames[6] = strdup("DATA_STORED");
  tagNames[7] = strdup("DATA_TRANSFER_BEGIN");
  tagNames[8] = strdup("DATA_TRANSFER_END");
  tagNames[9] = strdup("MEM");
  tagNames[10] = strdup("LOAD");
  tagNames[11] = strdup("LATENCY");
  tagNames[12] = strdup("BANDWIDTH");
  tagNames[13] = strdup("NEIGHBORS");
  tagNames[14] = strdup("JUXMEM_DATA_STORE");
  tagNames[15] = strdup("JUXMEM_DATA_USE");
  tagNames[14] = strdup("NOT_DEFINED1");
  tagNames[15] = strdup("NOT_DEFINED2");
  tagNames[16] = strdup("FAILURE");
  tagNames[17] = strdup("FD_OBSERVE");
  tagNames[18] = strdup("MADAG_SCHEDULER");
  tagNames[19] = strdup("DAG");
  tagNames[20] = strdup("NEW_PARENT");
  tagNames[21] = strdup("DISCONNECT");
  tagNames[22] = strdup("REMOVE");
  tagNames[23] = strdup("DATA_TRANSFER_TIME");  // modif bisnard_logs_1
  tagNames[24] = strdup("DAGNODE_READY");       // modif bisnard_logs_1
  tagNames[25] = strdup("DAGNODE_START");       // modif bisnard_logs_1
  tagNames[26] = strdup("DAGNODE_FINISH");      // modif bisnard_logs_1
  tagNames[27] = strdup("DAGNODE_FAILED");      // modif bisnard_logs_1
  tagNames[28] = strdup("END_DOWNLOAD");        // modif bisnard_logs_1

  tagNames[29] = strdup("VM_DEPLOY_BEGIN");
  tagNames[30] = strdup("VM_RUNNING");
  tagNames[31] = strdup("VM_DEPLOY_END");
  tagNames[32] = strdup("VM_WRAP_SERVICE");
  tagNames[33] = strdup("VM_DESTROY_BEGIN");
  tagNames[34] = strdup("VM_DESTROY_END");



  // CORBA::Object_ptr myLCCptr;

  // try {
  // myLCCptr = LogORBMgr::getMgr()->resolveObject(LOGCOMPCTXT, "LCC");
  // } catch (CORBA::SystemException &e) {
  // DLC_ERROR("Could not resolve 'LogService./LogComponent (LCC).' from the NS", 1);
  // }
  // if (CORBA::is_nil(myLCCptr)) {
  // DLC_ERROR("Could not resolve 'LogService./LogComponent (LCC).' from the NS", 1);
  // }

  // try {
  // myLCC = LogCentralComponent::_narrow(myLCCptr);
  // } catch (CORBA::SystemException &e) {
  // DLC_ERROR("Could not narrow the LogCentralComponent", 1);
  // }

  // try{
  // LogORBMgr::getMgr()->bind(LOGCOMPCTXT, name, _this(), true);
  // }
  // catch (...){
  // DLC_ERROR("Bind FAILED  in the LogService context\n", 1);
  // }

  // if (CORBA::is_nil(myLCC)){
  // fprintf (stderr, "NARROW NIL ICI AUSSI \n");
  // }

  // try{
  // LogORBMgr::getMgr()->bind("LogForwarder", name, _this(), true);
  // }
  // catch (...){
  // }
  // LogORBMgr::getMgr()->activate(this);
}


int
DietLogComponent::run(const char *agentType,
                      const char *parentName,
                      int outBufferTime) {
  // Connexion to the LCC
  try {
    myLCC =
      LogORBMgr::getMgr()->resolve<LogCentralComponent, LogCentralComponent_ptr>(
        "LogServiceC", "LCC");
    if (CORBA::is_nil(myLCC)) {
      fprintf(stderr, "Failed to narrow the LCC ! \n");
    }
  } catch (...) {
    ERROR_DEBUG("Problem while resolving LogServiceC/LCC", -1);
  }

  try {
    LogORBMgr::getMgr()->bind(LOGCOMPCTXT, myName, _this(), false);
    LogORBMgr::getMgr()->fwdsBind(LOGCOMPCTXT, myName,
                                  LogORBMgr::getMgr()->getIOR(_this()));
  } catch (...) {
    ERROR_DEBUG("Bind failed  in the LogService context", -1);
  }

  // Connect myself to the LogCentral
  short ret = 0;
  char *hostName = (char *) malloc(256 * sizeof(char));
  if (gethostname(hostName, 255) != 0) {
    free(hostName);
    hostName = strdup("unknownHost");
  }
  char *msg;
  if (parentName != NULL) {
    msg = (char *) malloc(strlen(agentType) + strlen(parentName) + 2);
    sprintf(msg, "%s %s", agentType, parentName);
  } else {
    msg = strdup(agentType);
  }

  log_time_t time = getLocalTime();
  tag_list_t currentTagList;
  try {
    ret = myLCC->connectComponent(
      name,
      hostName,
      msg,
      name,
      time,
      currentTagList
      );
  } catch (CORBA::SystemException &e) {
    TRACE_TEXT(TRACE_MAIN_STEPS,
               "Error: could not connect to the LogCentral\n");
    ERROR_DEBUG("SystemException", -1);
  }
  free(hostName);  // alloc'ed with new[]
  free(msg);       // alloc'ed with strdup (e.g. malloc)

  if (ret != LS_OK) {
    ERROR_DEBUG("LogCentral refused connection", ret);
  }

  setTagFilter(currentTagList);

  pingThread = new PingThread(this);
  pingThread->startThread();

  if (outBufferMaxSize != 0) {
    flushBufferThread = new FlushBufferThread(this, outBufferTime);
    flushBufferThread->startThread();
  }

  isConnected = true;
  return 0;
} // run

DietLogComponent::~DietLogComponent() {
  // disconnect LogComponent

  if (pingThread != NULL) {
    pingThread->stopThread();
  }
  if (flushBufferThread != NULL) {
    flushBufferThread->stopThread();
  }

  if (isConnected) {
    try {
      myLCC->disconnectComponent(name, "");
      LogORBMgr::getMgr()->unbind(LOGCOMPCTXT, myName);
      LogORBMgr::getMgr()->fwdsUnbind(LOGCOMPCTXT, myName);
    } catch (CORBA::SystemException &e) {
      isConnected = false;
      DLC_WARNING("Could not disconnect from the LogCentral");
      // do nothing - we are already terminating
    }
  }

  for (int i = 0; i < tagCount; i++) {
    free(tagNames[i]);
  }
  delete tagNames;
  delete tagFlags;
  // delete LogORBMgr::getMgr();  // FIXME: this does not work
}

/**
 * ComponentConfigurator functions
 */

void
DietLogComponent::setTagFilter(const tag_list_t &tagList) {
  bool *newList = createBoolArrayFalse(tagCount);
  bool *oldList;

  // check for '*'
  if (containsStar(&tagList)) {
    for (int i = 0; i < tagCount; i++) {
      newList[i] = true;
    }
  } else {
    // change config configuration
    int idx;
    for (int i = 0; i < (int) tagList.length(); i++) {
      idx = getTagIndex(tagList[i]);
      if (idx != -1) {
        newList[idx] = true;
      }
      // else: ignore unknown tags
    }
  }

  // set new config
  dlcMutex.lock();
  oldList = tagFlags;
  tagFlags = newList;
  dlcMutex.unlock();
  delete[] oldList;
} // setTagFilter

void
DietLogComponent::addTagFilter(const tag_list_t &tagList) {
  dlcMutex.lock();
  // check for '*'
  if (containsStar(&tagList)) {
    for (int i = 0; i < tagCount; i++) {
      tagFlags[i] = true;
    }
  } else {
    // create new configuration
    int idx;

    for (int i = 0; i < (int) tagList.length(); i++) {
      idx = getTagIndex(tagList[i]);
      if (idx != -1) {
        tagFlags[idx] = true;
      }
      // else: ignore unknown tags
    }
  }
  dlcMutex.unlock();
} // addTagFilter

void
DietLogComponent::removeTagFilter(const tag_list_t &tagList) {
  dlcMutex.lock();
  // check for '*'
  if (containsStar(&tagList)) {
    for (int i = 0; i < tagCount; i++) {
      tagFlags[i] = false;
    }
  } else {
    int idx;
    // change configuration
    for (int i = 0; i < (int) tagList.length(); i++) {
      idx = getTagIndex(tagList[i]);
      if (idx != -1) {
        tagFlags[idx] = false;
      }
      // else: ignore unknown tags
    }
  }
  dlcMutex.unlock();
} // removeTagFilter


void
DietLogComponent::test() {
}

/**
 * Helper functions
 */

/*
 * This function gives the string corresponding to the v_tag index;
 * This function must be consitent with the #define EST_* in DIET_data.h
 */
char *
DietLogComponent::getEstimationTags(const int v_tag) {
  char *ret;
  switch (v_tag) {
  case (EST_INVALID): // -1
    ret = strdup("EST_INVALID");
    break;
  case (EST_TOTALTIME): // 1
    ret = strdup("EST_TOTALTIME");
    break;
  case (EST_COMMTIME): // 2
    ret = strdup("EST_COMMTIME");
    break;
  case (EST_TCOMP): // 3
    ret = strdup("EST_TCOMP");
    break;
  case (EST_TIMESINCELASTSOLVE): // 4
    ret = strdup("EST_TIMESINCELASTSOLVE");
    break;
  case (EST_COMMPROXIMITY): // 5
    ret = strdup("EST_COMMPROXIMITY");
    break;
  case (EST_TRANSFEREFFORT): // 6
    ret = strdup("EST_TRANSFEREFFORT");
    break;
  case (EST_FREECPU): // 7
    ret = strdup("EST_FREECPU");
    break;
  case (EST_FREEMEM): // 8
    ret = strdup("EST_FREEMEM");
    break;
  case (EST_NBCPU): // 9
    ret = strdup("EST_NBCPU");
    break;
  case (EST_CPUSPEED): // 10
    ret = strdup("EST_CPUSPEED");
    break;
  case (EST_TOTALMEM): // 11
    ret = strdup("EST_TOTALMEM");
    break;
  case (EST_AVGFREEMEM): // 12
    ret = strdup("EST_AVGFREEMEM");
    break;
  case (EST_AVGFREECPU): // 13
    ret = strdup("EST_AVGFREECPU");
    break;
  case (EST_BOGOMIPS): // 14
    ret = strdup("EST_BOGOMIPS");
    break;
  case (EST_CACHECPU): // 15
    ret = strdup("EST_CACHECPU");
    break;
  case (EST_TOTALSIZEDISK): // 16
    ret = strdup("EST_TOTALSIZEDISK");
    break;
  case (EST_FREESIZEDISK): // 17
    ret = strdup("EST_FREESIZEDISK");
    break;
  case (EST_DISKACCESREAD): // 18
    ret = strdup("EST_DISKACCESREAD");
    break;
  case (EST_DISKACCESWRITE): // 19
    ret = strdup("EST_DISKACCESWRITE");
    break;
  case (EST_ALLINFOS): // 20
    ret = strdup("EST_ALLINFOS");
    break;
  case (EST_NUMWAITINGJOBS): // 21
    ret = strdup("EST_NUMWAITINGJOBS");
    break;
  case (EST_EFT): // 29
    ret = strdup("EST_EFT");
    break;
  case (EST_USERDEFINED): // 30
    ret = strdup("EST_USERDEFINED");
    break;
  /********* HAVE_ALT_BATCH ***********/
  /* Type of the server: must be in accordance with DIET_server.h
     -> Should be replaced by the PARAL_ID (correspondings to serial, paral
     with DIET specific batch system (numerous IDs depending on the
     scheduling strategy implemented in the SeD), oar, LL, etc.) */
  case (EST_SERVER_TYPE):
    ret = strdup("EST_SERVER_TYPE");  // 22
    break;
  /* Parallel resources information. Assumed a default queue */
  case (EST_PARAL_NBTOT_RESOURCES): // 23
    ret = strdup("EST_PARAL_NB_RESOURCES");
    break;
  case (EST_PARAL_NBTOT_FREE_RESOURCES): // 24
    ret = strdup("EST_PARAL_NB_FREE_RESOURCES");
    break;
  case (EST_PARAL_NB_RESOURCES_IN_DEFAULT_QUEUE): // 25
    ret = strdup("EST_PARAL_NB_RESOURCES_IN_DEFAULT_QUEUE");
    break;
  case (EST_PARAL_NB_FREE_RESOURCES_IN_DEFAULT_QUEUE): // 26
    ret = strdup("EST_PARAL_NB_FREE_RESOURCES_IN_DEFAULT_QUEUE");
    break;
  case (EST_PARAL_MAX_WALLTIME): // 27
    ret = strdup("EST_PARAL_MAX_WALLTIME");
    break;
  case (EST_PARAL_MAX_PROCS): // 28
    ret = strdup("EST_PARAL_MAX_PROCS");
    break;
  /* !HAVE_ALT_BATCH */
  default:
    ret = strdup("UNKNOWN");
  } // switch
  return ret;
} // getEstimationTags

int
DietLogComponent::getTagIndex(const char *tag) {
  for (int i = 0; i < tagCount; i++) {
    if (strcmp(tag, tagNames[i]) == 0) {
      return i;
    }
  }
  return -1;
}

bool
DietLogComponent::containsStar(const tag_list_t *tagList) {
  for (int i = 0; i < (int) tagList->length(); i++) {
    if (strcmp((*tagList)[i], "*") == 0) {
      return true;
    }
  }
  return false;
}

bool *
DietLogComponent::createBoolArrayFalse(int size) {
  bool *ba;
  ba = new bool[size];
  for (int i = 0; i < size; i++) {
    ba[i] = false;
  }
  return ba;
}

#ifdef DIET_USE_DELTACLOUD

void DietLogComponent::logVMDeployStart(const IaaS::Image& image,
    const char* cloudName, const char* vmId, const char* sedName) {
  if (tagFlags[29]) {
    ostringstream out;
    out << cloudName << " " << vmId << " " << image.id;
    log(tagNames[29], out.str().c_str());
   }
}

void DietLogComponent::logVMRunning(const IaaS::Instance& vmInstance) {
  if (tagFlags[30]) {
    ostringstream out;
    out << vmInstance.id;
    log(tagNames[30], out.str().c_str());
   }
}

void DietLogComponent::logVMOSReady(const IaaS::Instance& vmInstance) {
  if (tagFlags[31]) {
    ostringstream out;
    out << vmInstance.id;
    log(tagNames[31], out.str().c_str());
   }
}

void DietLogComponent::logVMServiceWrapped(const char* sedName,
    const ServiceWrapper& serviceWrapper, const char* vmIP,
    const char* vmUserName) {
  if (tagFlags[32]) {
    ostringstream out;
    out << sedName <<
        " " << serviceWrapper.name_of_service <<
        " " << serviceWrapper.executable_path <<
        " " << vmUserName << " " << vmIP;
    log(tagNames[32], out.str().c_str());
   }


}

void DietLogComponent::logVMDestroyStart(const IaaS::Instance& vmInstance) {
  if (tagFlags[33]) {
    ostringstream out;
    out << vmInstance.id;
    log(tagNames[33], out.str().c_str());
   }
}

void DietLogComponent::logVMDestroyEnd(const IaaS::Instance& vmInstance) {
  if (tagFlags[34]) {
    ostringstream out;
    out << vmInstance.id;
    log(tagNames[34], out.str().c_str());
   }
}


#endif


log_time_t
DietLogComponent::getLocalTime() {
  struct timeval tv;
#ifdef __WIN32__
  struct timeval tz;
#else
  struct timezone tz;
#endif
  gettimeofday(&tv, &tz);
  log_time_t ret;
  ret.sec = tv.tv_sec;
  ret.msec = tv.tv_usec / 1000;
  return ret;
}
/**
 * Count the number of Digits in a unsigned long
 */
static int
num_Digits(unsigned long num) {
  if (num < 10) {
    return 1;
  } else {
    return 1 + num_Digits(num / 10);
  }
}
/**
 * "Synchronised" functions with errorhandling
 */

void
DietLogComponent::ping() {
  if (isConnected) {
    try {
      myLCC->ping(name);
    } catch (CORBA::SystemException &e) {
      // looks like we were disconnected
      handleDisconnect(e);
    }
  }
}

void
DietLogComponent::synchronize() {
  log_time_t time = getLocalTime();
  if (isConnected) {
    try {
      myLCC->synchronize(name, time);
    } catch (CORBA::SystemException &e) {
      // looks like we were disconnected
      handleDisconnect(e);
    }
  }
} // synchronize

void
DietLogComponent::sendOutBuffer() {
  if (isConnected) {
    dlcMutex.lock();
    if (outBuffer.length() > 0) {
      try {
        myLCC->sendBuffer(outBuffer);
      } catch (CORBA::SystemException &e) {
        // looks like we were disconnected
        handleDisconnect(e);
      }
      outBuffer.length(0);
    }
    dlcMutex.unlock();
  }
} // sendOutBuffer

void
DietLogComponent::log(const char *tag, const char *msg) {
  log_msg_t logMsg;
  if (isConnected) {
    logMsg.componentName = CORBA::string_dup(name);
    logMsg.time = getLocalTime();
    logMsg.tag = CORBA::string_dup(tag);
    logMsg.msg = CORBA::string_dup(msg);

    dlcMutex.lock();
    CORBA::ULong bufsize;
    bufsize = outBuffer.length();
    bufsize++;
    outBuffer.length(bufsize);
    outBuffer[bufsize - 1] = logMsg;
    dlcMutex.unlock();

    if ((int) bufsize > outBufferMaxSize) {
      sendOutBuffer();
    }
  }
} // log

void
DietLogComponent::handleDisconnect(CORBA::SystemException &e) {
  DLC_WARNING("Connection to LogCentral failed");
#ifdef DLC_ERROR_BEHAVIOUR_FATAL
  DLC_ERROR("Cannot proceed without LogCentral connection", 1);
#else
  DLC_WARNING(
    "LogComponent module stopped - agent will continue without LogCentral");
  isConnected = false;
#endif
}

/**
 * Diet specific wrappers for the functions above
 */

/**
 * pronounce new service
 */
void
DietLogComponent::logAddService(const
                                corba_profile_desc_t *serviceDescription) {
  if (tagFlags[0]) {
    // FIXME: add service profile ?
    log(tagNames[0], serviceDescription->path);
  }
}

/**
 * Change parent
 */
void
DietLogComponent::logNewParent(const char *type, const char *parent) {
  if (tagFlags[20]) {
    char *msg = new char[(strlen(type) + strlen(parent) + 2) * sizeof(char)];
    sprintf(msg, "%s %s", type, parent);
    log(tagNames[20], msg);
    delete [] msg;
  }
}

/**
 * Disconnect element
 */
void
DietLogComponent::logDisconnect() {
  if (tagFlags[21]) {
    char *str = new char[10];
    sprintf(str, "D");
    log(tagNames[21], str);
    delete [] str;
  }
}

/**
 * Remove element
 */
void
DietLogComponent::logRemoveElement() {
  if (tagFlags[22]) {
    char *str = new char[2];
    sprintf(str, "R");
    log(tagNames[22], str);
    delete [] str;
  }
}

/**
 * Find SeD and solve problem
 */
void
DietLogComponent::logAskForSeD(const corba_request_t *request) {
  if (tagFlags[1]) {
    // FIXME: add request parameters (size of request arguments?)
    char *s;
    s =
      (char *) malloc((strlen(request->pb.path) + num_Digits(
                         request->reqID) + 2) * sizeof(char));
    sprintf(s, "%s %ld", (const char *) (request->pb.path),
            (unsigned long) (request->reqID));
    log(tagNames[1], s);
    free(s);
  }
}

void
DietLogComponent::logSedChosen(const corba_request_t *request,
                               const corba_response_t *response) {
  if (tagFlags[2]) {
    char *s;
    if (response->servers.length() > 0) {
      unsigned int i, j;
      string estim_string = "";
      for (i = 0; i < response->servers.length(); i++) {
        estim_string.append(" ");
        estim_string.append(response->servers[i].loc.hostName);
        for (j = 0; j < response->servers[i].estim.estValues.length(); j++) {
          int valTagInt = response->servers[i].estim.estValues[j].v_tag;
          estim_string.append(";");
          estim_string.append(getEstimationTags(valTagInt));
          estim_string.append("=");
          // char* v_value= new char[256];
          #define BUFSIZE 128
          char v_value[BUFSIZE];
          switch(response->servers[i].estim.estValues[j].v_value._d()) {
          case scalar:
            snprintf(v_value, BUFSIZE, "%f",
                     response->servers[i].estim.estValues[j].v_value.d());
            break;
          case str:
            snprintf(v_value, BUFSIZE,
                     response->servers[i].estim.estValues[j].v_value.s());
            break;
          case bin:
            snprintf(v_value, BUFSIZE,
                     reinterpret_cast<const char*>(response->servers[i].estim.estValues[j].v_value.b().get_buffer()));
            break;
          default:
            sprintf(v_value, "<unknown type>");
          }
          estim_string.append(v_value);
          // delete(v_value);
        }
      }
      s = (char *) malloc((strlen(request->pb.path)
                           + num_Digits(request->reqID)
                           + num_Digits(response->servers.length())
                           + estim_string.length()
                           + 5) * sizeof(char));
      sprintf(s, "%s %ld %ld%s"
              , (const char *) (request->pb.path)
              , (unsigned long) (request->reqID)
              , (unsigned long) (response->servers.length())
              , (const char *) (estim_string.c_str())
              );
    } else {
      s = (char *) malloc((strlen(request->pb.path)
                           + num_Digits(request->reqID)
                           + num_Digits(response->servers.length())
                           + 3) * sizeof(char));
      sprintf(s, "%s %ld %ld",
              (const char *) (request->pb.path),
              (unsigned long) (request->reqID),
              (unsigned long) (response->servers.length()));
    }
    log(tagNames[2], s);
    free(s);
  }
} // logSedChosen


#ifdef HAVE_MULTI_MA
void
DietLogComponent::logNeighbors(const char *list) {
  log(tagNames[13], list);
}
#endif  // HAVE_MULTI_MA

void
DietLogComponent::logBeginSolve(const char *path,
                                const corba_profile_t *problem) {
  if (tagFlags[3]) {
    // FIXME: print problem (argument size?)
    char *s;
    s = (char *) malloc((strlen(path) + num_Digits(
                           problem->dietReqID) + 2) * sizeof(char));
    sprintf(s, "%s %ld", (const char *) (path),
            (unsigned long) (problem->dietReqID));
    log(tagNames[3], s);
    free(s);
  }
}

// modif bisnard_logs_1
void
DietLogComponent::logEndDownload(const char *path,
                                 const corba_profile_t *problem) {
  if (tagFlags[28]) {
    char *s;
    s = (char *) malloc((strlen(path) + num_Digits(
                           problem->dietReqID) + 2) * sizeof(char));
    sprintf(s, "%s %ld", (const char *) (path),
            (unsigned long) (problem->dietReqID));
    log(tagNames[28], s);
    free(s);
  }
}
// end modif bisnard_logs_1

void
DietLogComponent::logEndSolve(const char *path,
                              const corba_profile_t *problem) {
  if (tagFlags[4]) {
    char *s;
    s = (char *) malloc((strlen(path) + num_Digits(
                           problem->dietReqID) + 2) * sizeof(char));
    sprintf(s, "%s %ld", (const char *) (path),
            (unsigned long) (problem->dietReqID));
    log(tagNames[4], s);
    free(s);
  }
}

/**
 * Data transfer functions
 */

void
DietLogComponent::logDataRelease(const char *dataID) {
  if (tagFlags[5]) {
    log(tagNames[5], dataID);
  }
}

void
DietLogComponent::logDataStore(const char *dataID, const long unsigned int size,
                               const long base_type,
                               const char *type) {
  char *base = (char *) malloc(10 * sizeof(char));
  char *s;
  // s = new char[strlen(dataID) + sizeof(size)+ strlen(type) + strlen(base) + 4];
  s =
    (char *) malloc((strlen(dataID) + num_Digits(size) + strlen(
                       type) + 10 + 4) * sizeof(char));

  if (tagFlags[6]) {
    switch (base_type) {
    case DIET_CHAR: {
      strcpy(base, "CHAR");
      sprintf(s, "%s %ld %s %s", (const char *) (dataID),
              (long unsigned int) (size), type, base);
      break;
    }
    case DIET_SHORT: {
      strcpy(base, "SHORT");
      sprintf(s, "%s %ld %s %s", (const char *) (dataID),
              (long unsigned int) (size), type, base);
      break;
    }
    case DIET_INT: {
      strcpy(base, "INTEGER");
      sprintf(s, "%s %ld %s %s", (const char *) (dataID),
              (long unsigned int) (size), type, base);
      break;
    }
    case DIET_LONGINT: {
      strcpy(base, "LONGINT");
      sprintf(s, "%s %ld %s %s", (const char *) (dataID),
              (long unsigned int) (size), type, base);
      break;
    }
    case DIET_FLOAT: {
      strcpy(base, "FLOAT");
      sprintf(s, "%s %ld %s %s", (const char *) (dataID),
              (long unsigned int) (size), type, base);
      break;
    }
    case DIET_DOUBLE: {
      strcpy(base, "DOUBLE");
      sprintf(s, "%s %ld %s %s", (const char *) (dataID),
              (long unsigned int) (size), type, base);
      break;
    }
    } // end switch
    log(tagNames[6], s);
  }
  free(base);
  free(s);
} // logDataStore

void
DietLogComponent::logDataBeginTransfer(const char *dataID,
                                       const char *destAgent) {
  if (tagFlags[7]) {
    char *s;
    s = (char *) malloc((strlen(dataID) + strlen(destAgent) + 2) * sizeof(char));
    sprintf(s, "%s %s", dataID, destAgent);
    log(tagNames[7], s);
    free(s);
  }
}

void
DietLogComponent::logDataEndTransfer(const char *dataID,
                                     const char *destAgent) {
  if (tagFlags[8]) {
    char *s;
    s = (char *) malloc((strlen(dataID) + strlen(destAgent) + 2) * sizeof(char));
    sprintf(s, "%s %s", dataID, destAgent);
    log(tagNames[8], s);
    free(s);
  }
}

// modif bisnard_logs_1
void
DietLogComponent::logDataTransferTime(const char *dataID,
                                      const char *destAgent,
                                      const unsigned long elapsedTime) {
  if (tagFlags[23]) {
    char *s;
    s =
      (char *) malloc((strlen(dataID) + strlen(destAgent) +
                       num_Digits(elapsedTime) + 3) * sizeof(char));
    sprintf(s, "%s %s %ld", dataID, destAgent, elapsedTime);
    log(tagNames[23], s);
  }
}
// end modif bisnard_logs_1

/**
 * NWS data
 */
void
DietLogComponent::logMem(double mem) {
  if (tagFlags[9]) {
    char buf[20];
    sprintf(buf, "%14.4f", mem);
    log(tagNames[9], buf);
  }
}

void
DietLogComponent::logLoad(double load) {
  if (tagFlags[10]) {
    char buf[20];
    sprintf(buf, "%14.4f", load);
    log(tagNames[10], buf);
  }
}

void
DietLogComponent::logLatency(double latency) {
  if (tagFlags[11]) {
    char buf[20];
    sprintf(buf, "%14.4f", latency);
    log(tagNames[11], buf);
  }
}

void
DietLogComponent::logBandwidth(double bandwidth) {
  if (tagFlags[12]) {
    char buf[20];
    sprintf(buf, "%14.4f", bandwidth);
    log(tagNames[12], buf);
  }
}

#ifdef HAVE_WORKFLOW

/**
 * Send node ready
 */
void
DietLogComponent::logWfNodeReady(const char *dagName,
                                 const char *nodeName) {
  char *log_msg = (char *) malloc((strlen(dagName) + strlen(
                                     nodeName) + 1) * sizeof(char) + 1);
  sprintf(log_msg, "%s:%s", dagName, nodeName);
  log(tagNames[24], log_msg);
  free(log_msg);
}

/**
 * Send node execution start
 */
void
DietLogComponent::logWfNodeStart(const char *dagName,
                                 const char *nodeName,
                                 const char *sedName,
                                 const char *pbName,
                                 const unsigned long reqID) {
  char *log_msg =
    (char *) malloc((strlen(dagName) + strlen(nodeName) + strlen(pbName)
                     + strlen(sedName) + num_Digits(
                       reqID) + 4) * sizeof(char) + 1);
  sprintf(log_msg, "%s:%s:%s:%s %ld", dagName, nodeName, sedName, pbName, reqID);
  log(tagNames[25], log_msg);
  free(log_msg);
}

void
DietLogComponent::logWfNodeStart(const char *dagName,
                                 const char *nodeName) {
  char *log_msg = (char *) malloc((strlen(dagName) + strlen(
                                     nodeName) + 1) * sizeof(char) + 1);
  sprintf(log_msg, "%s:%s", dagName, nodeName);
  log(tagNames[25], log_msg);
  free(log_msg);
}

/**
 * Send node execution finish
 */
void
DietLogComponent::logWfNodeFinish(const char *dagName,
                                  const char *nodeName) {
  char *log_msg = (char *) malloc((strlen(dagName) + strlen(
                                     nodeName) + 1) * sizeof(char) + 1);
  sprintf(log_msg, "%s:%s", dagName, nodeName);
  log(tagNames[26], log_msg);
  free(log_msg);
}

/**
 * Send node failure
 */
void
DietLogComponent::logWfNodeFailed(const char *dagName,
                                  const char *nodeName) {
  char *log_msg = (char *) malloc((strlen(dagName) + strlen(
                                     nodeName) + 1) * sizeof(char) + 1);
  sprintf(log_msg, "%s:%s", dagName, nodeName);
  log(tagNames[27], log_msg);
  free(log_msg);
}

/**
 * Send madag schedulerType
 */
void
DietLogComponent::maDagSchedulerType(const char *msg) {
  char *log_msg = (char *) malloc(strlen(msg) * sizeof(char) + 1);
  sprintf(log_msg, "%s", msg);
  log(tagNames[18], log_msg);
  free(log_msg);
}

/**
 * Send dag identifier and workflow processing time in the MA
 */
void
DietLogComponent::logDag(const char *msg) {
  char *log_msg = (char *) malloc(strlen(msg) * sizeof(char) + 1);
  sprintf(log_msg, "%s", msg);
  log(tagNames[19], log_msg);
  free(log_msg);
}


#endif \
  // HAVE_WORKFLOW

DietLogComponentFwdr::DietLogComponentFwdr(CorbaLogForwarder_ptr fwdr,
                                           const char *objName) {
  this->forwarder = CorbaLogForwarder::_duplicate(fwdr);
  this->objName = CORBA::string_dup(objName);
}

void
DietLogComponentFwdr::setTagFilter(const tag_list_t &tagList) {
  forwarder->setTagFilter(tagList, objName);
}

void
DietLogComponentFwdr::addTagFilter(const tag_list_t &tagList) {
  forwarder->addTagFilter(tagList, objName);
}

void
DietLogComponentFwdr::removeTagFilter(const tag_list_t &tagList) {
  forwarder->removeTagFilter(tagList, objName);
}

void
DietLogComponentFwdr::test() {
  forwarder->test(objName);
}
