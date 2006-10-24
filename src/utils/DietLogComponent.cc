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
 * Revision 1.23  2006/10/24 00:04:32  aamar
 * Add the logDagSubmit method.
 *
 * Revision 1.22  2006/07/25 14:37:40  ycaniou
 * dietJobID changed to dietReqID
 *
 * Update batch code
 *
 * Revision 1.21  2006/06/16 10:37:33  abouteil
 * Chandra&Toueg&Aguilera fault detector support added
 *
 * Revision 1.20  2006/01/25 21:08:00  pfrauenk
 * CoRI - plugin scheduler: the type diet_est_tag_t est replace by int
 *        some new fonctions in DIET_server.h to manage the estVector
 *
 * Revision 1.19  2005/12/15 12:25:06  pfrauenk
 * CoRI functionality added by Peter Frauenkron
 *
 * Revision 1.18  2005/12/15 11:06:55  eboix
 *      FIXME note --- Injay2461
 *
 * Revision 1.17  2005/11/10 14:37:51  eboix
 *     Clean-up of Cmake/DIET_config.h.in and related changes. --- Injay2461
 *
 * Revision 1.16  2005/10/07 14:28:30  rbolze
 * remove num_Digits(int num) function.
 * We do not need this function anymore.
 * It fix one bug with some strict compiler.
 *
 * Revision 1.15  2005/10/05 09:18:09  hdail
 * Last check-in was an error.  Here is a correction.
 *
 * Revision 1.13  2005/08/02 09:17:07  hdail
 * Corrected bug in tagNames structure whereby when we don't use JuxMem we have
 * 2 empty entries at the end and a count that is too high.  This bug was causing
 * DIET agents and servers to seg fault in one condition: when launched with
 * GoDIET but only if GoDIET was using LogService for information return.
 *
 * Revision 1.12  2005/07/01 13:00:12  rbolze
 * Agents send their list of SeD to LogCentral with each value of the estimation vector.
 *
 * Revision 1.11  2005/06/03 14:05:18  mjan
 * Fix issue in JuxMem log funtions
 *
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
#include <unistd.h>   // For gethostname()
#include <time.h>
#include <iostream>
#include <string>
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
  tagCount = 19;
  tagFlags = createBoolArrayFalse(tagCount);
  tagNames = new char*[this->tagCount];
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
  tagNames[18] = strdup("DAG_SUBMIT");

  
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

/*
 * This function gives the string corresponding to the v_tag index;
 * This function must be consitent with the #define EST_* in DIET_data.h
 */
char* 
DietLogComponent::getEstimationTags(const int v_tag){
	char* ret;
	switch(v_tag){
		case(EST_INVALID):
		ret = strdup("EST_INVALID");
		break;
		case(EST_TOTALTIME):
		ret = strdup("EST_TOTALTIME");
	        break;
		case(EST_COMMTIME):
		ret = strdup("EST_COMMTIME");
	        break;
		case(EST_TCOMP):
		ret = strdup("EST_TCOMP");
	        break;
	        case(EST_TIMESINCELASTSOLVE):
		ret = strdup("EST_TIMESINCELASTSOLVE");
	        break;		
	        case(EST_COMMPROXIMITY):
		ret = strdup("EST_COMMPROXIMITY");
	        break;
	        case(EST_TRANSFEREFFORT):
		ret = strdup("EST_TRANSFEREFFORT");
	        break;
	        case(EST_FREECPU):
		ret = strdup("EST_FREECPU");
	        break;
		case(EST_FREEMEM):
		ret = strdup("EST_FREEMEM");
	        break;
		case(EST_NBCPU):
		ret = strdup("EST_NBCPU");
	        break;
		case(EST_CPUSPEED):
		ret = strdup("EST_CPUSPEED");
	        break;
		case(EST_TOTALMEM):
		ret = strdup("EST_TOTALMEM");
	        break;
		case(EST_AVGFREEMEM):
		ret = strdup("EST_AVGFREEMEM");
	        break;
		case(EST_AVGFREECPU):
		ret = strdup("EST_AVGFREECPU");
	        break;
		case(EST_BOGOMIPS):
		ret = strdup("EST_BOGOMIPS");
	        break;
		case(EST_CACHECPU):
		ret = strdup("EST_CACHECPU");
	        break;
		case(EST_TOTALSIZEDISK):
		ret = strdup("EST_TOTALSIZEDISK");
	        break;
		case(EST_FREESIZEDISK):
		ret = strdup("EST_FREESIZEDISK");
	        break;
		case(EST_DISKACCESREAD):
		ret = strdup("EST_DISKACCESREAD");
	        break;
		case(EST_DISKACCESWRITE):
		ret = strdup("EST_DISKACCESWRITE");
	        break;
		case(EST_ALLINFOS):
		ret = strdup("EST_ALLINFOS");
	        break;
		case(EST_USERDEFINED):
		ret = strdup("EST_USERDEFINED");
	        break;
		default:
		ret = strdup("UNKNOWN");
	}
	return ret;
}

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
    s = new char[strlen(request->pb.path) + num_Digits(request->reqID)+2]; 
    sprintf(s,"%s %ld",(const char *)(request->pb.path),(unsigned long)(request->reqID));
    log(tagNames[1], s);
    delete(s);
  }
}

void
DietLogComponent::logSedChosen(const corba_request_t* request,
			       const corba_response_t* response) {
  if (tagFlags[2]) {
    char* s;
    unsigned int i,j;
    if (response->servers.length()>0){
	string estim_string = "";
    	for ( i=0 ; i < response->servers.length();i++){
		estim_string.append(" ");
		estim_string.append(response->servers[i].loc.hostName);
		for (j=0 ; j < response->servers[i].estim.estValues.length() ; j++){
			int valTagInt = response->servers[i].estim.estValues[j].v_tag;
			estim_string.append(";");
			estim_string.append(getEstimationTags(valTagInt));
			estim_string.append("=");
			char* v_value= new char[256];
			sprintf(v_value,"%f",response->servers[i].estim.estValues[j].v_value);
			estim_string.append(v_value);
			delete(v_value); 
		}
	}
    s = new char[strlen(request->pb.path)
	    +num_Digits(request->reqID)
	    +num_Digits(response->servers.length())
	    +estim_string.length()
	    +5]; 
    sprintf(s,"%s %ld %ld%s"
	    ,(const char *)(request->pb.path)
	    ,(unsigned long)(request->reqID)
	    ,(unsigned long)(response->servers.length())
	    ,(const char *)(estim_string.c_str()) 
	    );
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
    delete(s);
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
				const corba_profile_t* problem,
				const unsigned long reqID ) {
  if (tagFlags[3]) {
    // FIXME: print problem (argument size?)
    char* s;
    s = new char[strlen(path)+num_Digits(reqID)+2]; 
    sprintf(s,"%s %ld",(const char *)(path),(unsigned long)(reqID));
    log(tagNames[3], s);
    delete(s);
  }
}

void
DietLogComponent::logEndSolve(const char* path,
			      const corba_profile_t* problem,
			      const unsigned long reqID ) {
  if (tagFlags[4]) {
    char* s;
    s = new char[strlen(path)+num_Digits(reqID)+2]; 
    sprintf(s,"%s %ld",(const char *)(path),(unsigned long)(reqID));
    log(tagNames[4], s);
    delete(s);
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
    delete(s);
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

#if HAVE_JUXMEM
void 
DietLogComponent::logJuxMemDataStore(const unsigned long reqID, const char* dataID, const long unsigned int size, const long base_type, const char * type, const float time) {
   char * base = (char *)malloc(10*sizeof(char));
   char* s;
   s = new char[sizeof(reqID) + strlen(dataID) + sizeof(size)+ strlen(type) + (10 * sizeof(char)) + sizeof(time) + 5];
   if (tagFlags[14]) {
    switch (base_type) {
    case DIET_CHAR: {
      strcpy(base,"CHAR");
      break;
    }
    case DIET_SHORT: {
      strcpy(base,"SHORT");
      break;
    }
    case DIET_INT: {
      strcpy(base,"INTEGER");
      break;
    }
    case DIET_LONGINT: {
      strcpy(base,"LONGINT");
      break;
    }
    case DIET_FLOAT: {
      strcpy(base,"FLOAT");
      break;
    }
    case DIET_DOUBLE: {
      strcpy(base,"DOUBLE");
      break;
    }
    }
    sprintf(s,"%ld %s %ld %s %s %f", (unsigned long) reqID, (const char *)(dataID), (long unsigned int)(size), type, base, time);
    log(tagNames[14], s);
  }
  free(base);
  delete(s);
}

void 
DietLogComponent::logJuxMemDataUse(const unsigned long reqID, const char* dataID, const char* access_mode, const long unsigned int size, const long base_type, const char * type, const float time) {
   char * base = (char *)malloc(10*sizeof(char));
   char* s;
   s = new char[sizeof(reqID) + strlen(dataID) + strlen(access_mode) + sizeof(size)+ strlen(type) + (sizeof(char) * 10) + sizeof(time) + 6];
   if (tagFlags[15]) {
    switch (base_type) {
    case DIET_CHAR: {
      strcpy(base,"CHAR");
      break;
    }
    case DIET_SHORT: {
      strcpy(base,"SHORT");
      break;
    }
    case DIET_INT: {
      strcpy(base,"INTEGER");
      break;
    }
    case DIET_LONGINT: {
      strcpy(base,"LONGINT");
      break;
    }
    case DIET_FLOAT: {
      strcpy(base,"FLOAT");
      break;
    }
    case DIET_DOUBLE: {
      strcpy(base,"DOUBLE");
      break;
    }
    }
    sprintf(s,"%ld %s %s %ld %s %s %f", (unsigned long) reqID, (const char *)(dataID), (const char*) access_mode, (long unsigned int)(size), type, base, time);
    log(tagNames[15], s);
  }
  free(base);
  //delete(s);
}

#endif // HAVE_JUXMEM

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

#if HAVE_FD
void DietLogComponent::logFailure(const char *observed) {
  if (tagFlags[16]) {
    log(tagNames[16], observed);
  }
}

void DietLogComponent::logDetectorParams(const char *observed, double Pl, double Vd, double eta, double alpha) {
  if (tagFlags[17]) {
    char *buf;
    
    buf = (char *) malloc(strlen(observed) + 84);
    sprintf(buf, "%s %14.4f %14.4f %14.4f %14.4f", observed, Pl, Vd, eta, alpha);
    log(tagNames[17], buf);
  }
}
#endif

#ifdef HAVE_WORKFLOW
/**
 * Send dag identifier and workflow processing time in the MA
 */
void
DietLogComponent::logDagSubmit(CORBA::Long dag_id,
			       time_t ptime) {
  char msg[256];
  sprintf(msg, "DAG_ID=%ld, TIME=%ld", dag_id, ptime);
  log(tagNames[18], msg);
}
	       
#endif // HAVE_WORKFLOW
