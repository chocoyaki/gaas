/****************************************************************************/
/* DietLogComponent - LogService module for DIET                            */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Georg Hoesch (hoesch@in.tum.de)                                     */
/*    - Cyrille Pontvieux (cyrille.pontvieux@edu.univ-fcomte.fr)            */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.8  2005/04/29 15:17:08  ecaron
 * Bug fix for gcc 3.4.x
 *
 * Revision 1.7  2004/12/16 11:16:31  sdahan
 * adds multi-mas informations into the logService
 *
 * Revision 1.6  2004/12/02 15:01:11  bdelfabr
 * good order for base_type and type
 *
 * Revision 1.4  2004/10/04 11:32:04  hdail
 * Modified memory free / delete to agree with variable allocations.
 *
 * Revision 1.3  2004/07/29 18:53:06  rbolze
 * make some change to send more info to the logService.
 *
 * Revision 1.2  2004/03/02 17:38:07  rbolze
 * fix a little inversion between DATA_RELEASED and DATA_STORED in tagNames tab
 *
 * Revision 1.1  2004/03/01 18:55:56  rbolze
 * make Diet objects as LogComponent for LogCentral
 * DietLogComponent objects can contact LogCentral and transmit log message
 *
 ****************************************************************************/

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <iostream>
using namespace std;

#include "ORBMgr.hh"
#include "debug.hh"
#include "DietLogComponent.hh"



/**
 * Error message - exit with exit_value.
 */
#define DLC_ERROR(formatted_msg,exit_value)              \
  cerr << "DIET ERROR: " << formatted_msg << ".\n"   \
       << "cannot proceed.\n"; \
  exit(exit_value)

/**
 * Warning message.
 */
#define DLC_WARNING(formatted_msg)                       \
  cerr << "DIET WARNING: " << formatted_msg << ".\n" \
       << "DIET can proceed, but you may encounter unexpected behaviour.\n"

/**
 * FlushBufferThread
 */
FlushBufferThread::FlushBufferThread(DietLogComponent* DLC,
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
FlushBufferThread::stopThread()
{
  if (!threadRunning) {
    return;
  }
  threadRunning = false;
  join(NULL);
}

void*
FlushBufferThread::run_undetached(void* params)
{
  while (threadRunning) {
    myDLC->sendOutBuffer();

    sleep(0,outBufferTimeNSec);
  }
  return NULL;
}

/**
 * PingThread
 */

PingThread::PingThread(DietLogComponent* DLC)
{
  threadRunning = false;
  myDLC = DLC;
}

PingThread::~PingThread()
{
  // cannot be called directly / has nothing to do
}

void
PingThread::startThread()
{
  if (threadRunning) {
    return;
  }
  threadRunning = true;
  start_undetached();
}

void
PingThread::stopThread()
{
  if (!threadRunning) {
    return;
  }
  threadRunning = false;
  join(NULL);
}

void*
PingThread::run_undetached(void* params)
{
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
}

/****************************************************************************/

/**
 * DietLogComponent
 */

DietLogComponent::DietLogComponent(const char* name,
        int outBufferMaxSize) {
  this->name = CORBA::string_dup(name);
  this->outBufferMaxSize = outBufferMaxSize;
  isConnected = false;

  pingThread=NULL;

  // define tags
  tagCount = 14;
  tagFlags = createBoolArrayFalse(tagCount);
  tagNames = new char*[14];
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
  CORBA::Object_ptr myLCCptr;

  try {
    myLCCptr = ORBMgr::getObjReference(ORBMgr::LOGSERVICE, "LogComponent");
  } catch(CORBA::SystemException &e) {
    DLC_ERROR("Could not resolve 'LogService./LogComponent.' from the NS",1);
  }
  if (CORBA::is_nil(myLCCptr)) {
    DLC_ERROR("Could not resolve 'LogService./LogComponent.' from the NS",1);
  }

  try {
    myLCC = LogCentralComponent::_narrow(myLCCptr);
  } catch(CORBA::SystemException &e) {
    DLC_ERROR("Could not narrow the LogCentralComponent",1);
  }
}

int DietLogComponent::run(const char* agentType,
                          const char* parentName,
                          int outBufferTime) {
  // Connect myself to the LogCentral
  short ret=0;

  char* hostName = new char[256];
  if(gethostname(hostName,255) != 0) {
    delete hostName;
    hostName = strdup("unknownHost");
  }
  char* msg;
  if (parentName != NULL) {
    msg = (char*) malloc(strlen(agentType) + strlen(parentName) + 2);
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
      this->_this(),
      time,
      currentTagList
    );
  } catch (CORBA::SystemException &e) {
    delete(msg);
    delete(hostName);
    cout << "Error: could not connect to the LogCentral" << endl;
    DLC_ERROR("SystemException",-1);
  }
  delete[] hostName;  // alloc'ed with new[]
  free(msg);          // alloc'ed with strdup (e.g. malloc)

  if (ret != LS_OK) {
    ERROR("LogCentral refused connection",ret);
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
}

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
    } catch (CORBA::SystemException &e) {
      isConnected = false;
      DLC_WARNING("Could not disconnect from the LogCentral");
      // do nothing - we are already terminating
    }
  }

  for (int i=0; i<tagCount; i++) {
    free(tagNames[i]);
  }
  delete tagNames;
  delete tagFlags;
}

/**
 * ComponentConfigurator functions
 */

void 
DietLogComponent::setTagFilter(const tag_list_t& tagList) {
  bool* newList = createBoolArrayFalse(tagCount);
  bool* oldList;
  int idx;

  // check for '*'
  if (containsStar(&tagList)) {
    for (int i=0; i < tagCount; i++) {
      newList[i]=true;
    }
  } else {
    // change config configuration
    for (int i=0; i < (int)tagList.length(); i++) {
      idx = getTagIndex(tagList[i]);
      if (idx != -1) {
        newList[idx]=true;
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
}

void
DietLogComponent::addTagFilter(const tag_list_t& tagList) {
  int idx;

  dlcMutex.lock();
  // check for '*'
  if (containsStar(&tagList)) {
    for (int i=0; i < tagCount; i++) {
      tagFlags[i]=true;
    }
  } else {
    // create new configuration
    for (int i=0; i < (int)tagList.length(); i++) {
      idx = getTagIndex(tagList[i]);
      if (idx != -1) {
        tagFlags[idx]=true;
      }
      // else: ignore unknown tags
    }
  }
  dlcMutex.unlock();
}

void
DietLogComponent::removeTagFilter(const tag_list_t& tagList) {
  int idx;

  dlcMutex.lock();
  // check for '*'
  if (containsStar(&tagList)) {
    for (int i=0; i < tagCount; i++) {
      tagFlags[i]=false;
    }
  } else {
    // change configuration
    for (int i=0; i < (int)tagList.length(); i++) {
      idx = getTagIndex(tagList[i]);
      if (idx != -1) {
        tagFlags[idx]=false;
      }
      // else: ignore unknown tags
    }
  }
  dlcMutex.unlock();
}

/**
 * Helper functions
 */

int DietLogComponent::getTagIndex(const char* tag) {
  for (int i=0; i < tagCount; i++) {
    if (strcmp(tag, tagNames[i]) == 0) {
      return i;
    }
  }
  return -1;
}

bool DietLogComponent::containsStar(const tag_list_t* tagList) {
  for (int i=0; i < (int)tagList->length(); i++) {
    if (strcmp((*tagList)[i],"*") == 0) {
      return true;
    }
  }
  return false;
}

bool*
DietLogComponent::createBoolArrayFalse(int size) {
  bool* ba;
  ba = new bool[size];
  for (int i=0; i<size; i++) {
    ba[i] = false;
  }
  return ba;
}

log_time_t
DietLogComponent::getLocalTime() {
  struct timeval tv;
  struct timezone tz;
  gettimeofday(&tv, &tz);
  log_time_t ret;
  ret.sec = tv.tv_sec;
  ret.msec = tv.tv_usec / 1000;
  return ret;
}
/**
 * Count the number of Digits in a unsigned long
 */
 
static int num_Digits(unsigned long num){
	if ( num < 10 )
		return 1;
	else
		return 1+num_Digits(num/10);
}

/**
 * "Synchronised" functions with errorhandling
 */

void
DietLogComponent::ping() {
  if(isConnected) {
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
  if(isConnected) {
    try {
      myLCC->synchronize(name, time);
    } catch (CORBA::SystemException &e) {
      // looks like we were disconnected
      handleDisconnect(e);
    }
  }
}

void DietLogComponent::sendOutBuffer() {
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
}

void
DietLogComponent::log(const char* tag, const char* msg) {
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
    outBuffer[bufsize-1] = logMsg;
    dlcMutex.unlock();

    if ((int)bufsize > outBufferMaxSize) {
      sendOutBuffer();
    }
  }
}

void
DietLogComponent::handleDisconnect(CORBA::SystemException &e) {
  DLC_WARNING("Connection to LogCentral failed");
#ifdef DLC_ERROR_BEHAVIOUR_FATAL
  DLC_ERROR("Cannot proceed without LogCentral connection\n",1);
#else
  DLC_WARNING("LogComponent module stopped - agent will continue without LogCentral");
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
				corba_profile_desc_t* serviceDescription) {
  if (tagFlags[0]) {
    // FIXME: add service profile ?
    log(tagNames[0], serviceDescription->path);
  }
}

/**
 * Find SeD and solve problem
 */

void
DietLogComponent::logAskForSeD(const corba_request_t* request) {
  if (tagFlags[1]) {
    // FIXME: add request parameters (size of request arguments?)
    char* s;
    s = new char[strlen(request->pb.path)+num_Digits(request->reqID)+2]; 
    sprintf(s,"%s %ld",(const char *)(request->pb.path),(unsigned long)(request->reqID));
    log(tagNames[1], s);
    delete s;
  }
}

void
DietLogComponent::logSedChosen(const corba_request_t* request,
			       const corba_response_t* response) {
  if (tagFlags[2]) {
    // FIXME: add the complet list/outline of chosen servers ?
    char* s;
    //cout << "Number of Server = "<< response->servers.length() << endl;
    //cout << "Name of Server : "<< response->servers[0].loc.hostName <<endl;
    if (response->servers.length()>0){
    s = new char[strlen(request->pb.path)
	    +num_Digits(request->reqID)
	    +num_Digits(response->servers.length())
	    +strlen(response->servers[0].loc.hostName)	    
	    +4]; 
    sprintf(s,"%s %ld %ld %s",
	    (const char *)(request->pb.path),
	    (unsigned long)(request->reqID),
	    (unsigned long)(response->servers.length()),
	    (const char *)(response->servers[0].loc.hostName) );
    }else{
     s = new char[strlen(request->pb.path)
	    +num_Digits(request->reqID)
	    +num_Digits(response->servers.length())
	    +3];	  
     sprintf(s,"%s %ld %ld",
	    (const char *)(request->pb.path),
	    (unsigned long)(request->reqID),
	    (unsigned long)(response->servers.length()) );
     }
    log(tagNames[2], s);
  }
}


#ifdef HAVE_MULTI_MA
void
DietLogComponent::logNeighbors(const char* list) {
  log(tagNames[13], list);
}
#endif // HAVE_MULTI_MA

void
DietLogComponent::logBeginSolve(const char* path,
				const corba_profile_t* problem, const unsigned long reqID ) {
  if (tagFlags[3]) {
    // FIXME: print problem (argument size?)
    char* s;
    s = new char[strlen(path)+num_Digits(reqID)+2]; 
    sprintf(s,"%s %ld",(const char *)(path),(unsigned long)(reqID));
    log(tagNames[3], s);
  }
}

void
DietLogComponent::logEndSolve(const char* path,
				const corba_profile_t* problem, const unsigned long reqID ) {
  if (tagFlags[4]) {
    char* s;
    s = new char[strlen(path)+num_Digits(reqID)+2]; 
    sprintf(s,"%s %ld",(const char *)(path),(unsigned long)(reqID));
    log(tagNames[4], s);
  }
}

/**
 * Data transfer functions
 */

void
DietLogComponent::logDataRelease(const char* dataID) {
  if (tagFlags[5]) {
    log(tagNames[5], dataID);
  }
}

void
DietLogComponent::logDataStore(const char* dataID, const long unsigned int size, const long base_type, const char * type) {
  char * base = (char *)malloc(10*sizeof(char));
   char* s;
   s = new char[strlen(dataID) + sizeof(size)+ strlen(type) + strlen(base) + 3];
 if (tagFlags[6]) {
    switch (base_type) {
    case DIET_CHAR: {
      strcpy(base,"CHAR");
      sprintf(s,"%s %ld %s %s",(const char *)(dataID),(long unsigned int)(size),type, base);
      break;
    }
    case DIET_SHORT: {
      strcpy(base,"SHORT");
      sprintf(s,"%s %ld %s %s",(const char *)(dataID),(long unsigned int)(size), type, base);
      break;
    }
    case DIET_INT: {
      strcpy(base,"INTEGER");
      sprintf(s,"%s %ld %s %s",(const char *)(dataID),(long unsigned int)(size), type, base);
      break;
    }
    case DIET_LONGINT: {
      strcpy(base,"LONGINT");
      sprintf(s,"%s %ld %s %s",(const char *)(dataID),(long unsigned int)(size), type, base);
      break;
    }
    case DIET_FLOAT: {
      strcpy(base,"FLOAT");
      sprintf(s,"%s %ld %s %s",(const char *)(dataID),(long unsigned int)(size), type, base);
      break;
    }
    case DIET_DOUBLE: {
      strcpy(base,"DOUBLE");
      sprintf(s,"%s %ld %s %s",(const char *)(dataID),(long unsigned int)(size), type, base);
      break;
    }
    }
    log(tagNames[6], s);
  }
  free(base);
}

void
DietLogComponent::logDataBeginTransfer(const char* dataID,
		     const char* destAgent) {
  if (tagFlags[7]) {
    char* s;
    s = new char[strlen(dataID)+strlen(destAgent)+2];
    sprintf(s,"%s %s",dataID, destAgent);
    log(tagNames[7], s);
    delete(s);
  }
}

void
DietLogComponent::logDataEndTransfer(const char* dataID,
		     const char* destAgent) {
  if (tagFlags[8]) {
    char* s;
    s = new char[strlen(dataID)+strlen(destAgent)+2];
    sprintf(s,"%s %s",dataID, destAgent);
    log(tagNames[8], s);
    delete(s);
  }
}

/**
 * NWS data
 */
void
DietLogComponent::logMem(double mem) {
  if (tagFlags[9]) {
    char buf[20];
    sprintf(buf,"%14.4f",mem);
    log(tagNames[9], buf);
  }
}

void
DietLogComponent::logLoad(double load) {
  if (tagFlags[10]) {
    char buf[20];
    sprintf(buf,"%14.4f",load);
    log(tagNames[10], buf);
  }
}

void
DietLogComponent::logLatency(double latency) {
  if (tagFlags[11]) {
    char buf[20];
    sprintf(buf,"%14.4f",latency);
    log(tagNames[11], buf);
  }
}

void
DietLogComponent::logBandwidth(double bandwidth) {
  if (tagFlags[12]) {
    char buf[20];
    sprintf(buf,"%14.4f",bandwidth);
    log(tagNames[12], buf);
  }

}

