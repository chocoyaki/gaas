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
 * Revision 1.9  2011/05/13 08:19:01  bdepardo
 * Use new LogORBmgr
 *
 * Revision 1.8  2011/03/21 08:27:38  bdepardo
 * Correctly register the logcomponent into the ORB, and correclty detroy it.
 *
 * Revision 1.7  2011/03/20 18:48:18  bdepardo
 * Be more robust when logComponent initialization fails
 *
 * Revision 1.6  2011/02/24 16:52:22  bdepardo
 * Use TRACE_TEXT instead of cout
 *
 * Revision 1.5  2011/02/07 18:42:31  bdepardo
 * Removed debug messages
 *
 * Revision 1.4  2011/01/20 17:31:10  bdepardo
 * Mismatching allocation and deallocation
 *
 * Revision 1.3  2011/01/20 17:18:27  bdepardo
 * Reduced variables scopes
 *
 * Revision 1.2  2010/12/28 09:03:14  bdepardo
 * Removed a warning
 *
 * Revision 1.1  2010/12/17 09:48:01  kcoulomb
 * * Set diet to use the new log with forwarders
 * * Fix a CoRI problem
 * * Add library version remove DTM flag from ccmake because deprecated
 *
 * Revision 1.6  2010/11/10 02:41:23  kcoulomb
 * Small modifications to use the log service (LogService divided in 2 separated contexts, one for components and one for tools)
 *
 * Revision 1.5  2010/07/20 09:16:44  bisnard
 * Added const
 *
 * Revision 1.4  2010/07/12 16:14:13  glemahec
 * DIET 2.5 beta 1 - Use the new ORB manager and allow the use of SSH-forwarders for all DIET CORBA objects
 *
 * Revision 1.3  2010/03/31 21:15:41  bdepardo
 * Changed C headers into C++ headers
 *
 * Revision 1.2  2010/03/08 13:19:13  bisnard
 * added new events for workflow monitoring
 *
 * Revision 1.1  2010/03/03 14:26:35  bdepardo
 * BEWARE!!!
 * Huge modifications to take into account CYGWIN.
 * Lots of files' directory have been changed.
 *
 * Revision 1.39  2010/01/14 13:15:08  bdepardo
 * "\n" -> endl
 *
 * Revision 1.38  2009/11/30 17:56:15  bdepardo
 * Added message when killing element.
 *
 * Revision 1.37  2009/10/26 09:12:14  bdepardo
 * Added logs for dynamic hierarchy modifications:
 * - NEW_PARENT
 * - DISCONNECT
 *
 * Revision 1.36  2009/06/23 09:28:46  bisnard
 * use new estimation vector entry (EST_EFT)
 *
 * Revision 1.35  2008/07/09 13:16:34  rbolze
 * correct some memory free problems
 *
 * Revision 1.34  2008/07/08 09:49:26  rbolze
 * add new function maDagSchedulerType in order to log
 * what is the scheduler of the madag
 *
 * Revision 1.33  2008/06/25 10:11:17  bisnard
 * Removed logDagSubmit
 *
 * Revision 1.32  2008/06/01 14:06:55  rbolze
 * replace most ot the cout by adapted function from debug.cc
 * there are some left ...
 *
 * Revision 1.31  2008/06/01 09:19:17  rbolze
 * add method logDag
 *
 * Revision 1.30  2008/05/11 16:19:51  ycaniou
 * Check that pathToTmp and pathToNFS exist
 * Check and eventually correct if pathToTmp or pathToNFS finish or not by '/'
 * Rewrite of the propagation of the request concerning job parallel_flag
 * Implementation of Cori_batch system
 * Numerous information can be dynamically retrieved through batch systems
 *
 * Revision 1.29  2008/04/29 06:27:50  rbolze
 * propagate the new estimation tag : EST_NUMWAITINGJOBS
 *
 * Revision 1.28  2007/08/31 16:52:26  bdepardo
 * Implemented the new test() method in LogCentralComponent.
 * Used to check whether or not a component is reachable.
 *
 * Revision 1.27  2007/06/28 18:23:20  rbolze
 * add dietReqID in the profile.
 * and propagate this change to all functions that  have both reqID and profile parameters.
 * TODO : look at the asynchronous mechanism (client->SED) to propage this change.
 *
 * Revision 1.26  2006/11/16 09:55:55  eboix
 *   DIET_config.h is no longer used. --- Injay2461
 *
 * Revision 1.25  2006/11/06 15:43:44  aamar
 * Checking the memory allocated to log msgs.
 *
 * Revision 1.24  2006/11/02 17:12:26  rbolze
 * change function logDagSubmit
 *
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

#include <cstring>
#include <cstdlib>
#include <unistd.h>   // For gethostname()
#include <ctime>
#include <iostream>
#include <string>
using namespace std;

#include "LogORBMgr.hh"
#include "debug.hh"
#include "DietLogComponent.hh"



/**
 * Error message - exit with exit_value.
 */
#define DLC_ERROR(formatted_msg,exit_value)              \
  cerr << "DIET ERROR: " << formatted_msg << "." << endl	 \
       << "cannot proceed." << endl; \
  exit(exit_value)

/**
 * Warning message.
 */
#define DLC_WARNING(formatted_msg)                       \
  cerr << "DIET WARNING: " << formatted_msg << "." << endl			\
       << "DIET can proceed, but you may encounter unexpected behaviour." << endl

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
				   int outBufferMaxSize, int argc, char** argv) {
  try {
    LogORBMgr::init(argc, (char **)argv);
  } catch (...) {
    fprintf (stderr, "Failed to initialize log orb manager \n");
  }

  this->name = CORBA::string_dup(name);
  this->myName = string(name);
  this->outBufferMaxSize = outBufferMaxSize;
  isConnected = false;

  pingThread=NULL;
  flushBufferThread=NULL; // modif bisnard_logs_1

  // define tags
  tagCount = 29;  // modif bisnard_logs_1
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

//   CORBA::Object_ptr myLCCptr;

//   try {
//     myLCCptr = LogORBMgr::getMgr()->resolveObject(LOGCOMPCTXT, "LCC");
//   } catch(CORBA::SystemException &e) {
//     DLC_ERROR("Could not resolve 'LogService./LogComponent (LCC).' from the NS",1);
//   }
//   if (CORBA::is_nil(myLCCptr)) {
//     DLC_ERROR("Could not resolve 'LogService./LogComponent (LCC).' from the NS",1);
//   }

//   try {
//     myLCC = LogCentralComponent::_narrow(myLCCptr);
//   } catch(CORBA::SystemException &e) {
//     DLC_ERROR("Could not narrow the LogCentralComponent",1);
//   }

//   try{
//     LogORBMgr::getMgr()->bind(LOGCOMPCTXT, name, _this(), true);
//   }
//   catch (...){
//     DLC_ERROR("Bind FAILED  in the LogService context\n", 1);
//   }

//   if (CORBA::is_nil(myLCC)){
//     fprintf (stderr, "NARROW NIL ICI AUSSI \n");
//}

//  try{
//    LogORBMgr::getMgr()->bind("LogForwarder", name, _this(), true);
//  }
//  catch (...){
//  }
//  LogORBMgr::getMgr()->activate(this);
}


int DietLogComponent::run(const char* agentType,
                          const char* parentName,
                          int outBufferTime) {
  // Connexion to the LCC
  try {
    myLCC = LogORBMgr::getMgr()->resolve<LogCentralComponent, LogCentralComponent_ptr>("LogServiceC", "LCC");
    if (CORBA::is_nil(myLCC)){
      fprintf (stderr, "Failed to narrow the LCC ! \n");
    }
  } catch (...) {
    ERROR("Problem while resolving LogServiceC/LCC",-1);
  }

  try{
    LogORBMgr::getMgr()->bind(LOGCOMPCTXT, myName, _this(), false);
    LogORBMgr::getMgr()->fwdsBind(LOGCOMPCTXT, myName,
                                  LogORBMgr::getMgr()->getIOR(_this()));
  }
  catch (...){
    ERROR("Bind failed  in the LogService context",-1);
  }

  // Connect myself to the LogCentral
  short ret=0;
  char* hostName = (char*) malloc(256 * sizeof(char));
  if(gethostname(hostName,255) != 0) {
    free(hostName);
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
                                  name,
                                  time,
                                  currentTagList
                                  );
  } catch (CORBA::SystemException &e) {
    free(msg);
    free(hostName);
    TRACE_TEXT(TRACE_MAIN_STEPS, "Error: could not connect to the LogCentral" << endl);
    ERROR("SystemException",-1);
  }
  free(hostName);  // alloc'ed with new[]
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
      LogORBMgr::getMgr()->unbind(LOGCOMPCTXT, myName);
      LogORBMgr::getMgr()->fwdsUnbind(LOGCOMPCTXT, myName);
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
  //delete LogORBMgr::getMgr(); // FIXME: this does not work
}

/**
 * ComponentConfigurator functions
 */

void
DietLogComponent::setTagFilter(const tag_list_t& tagList) {
  bool* newList = createBoolArrayFalse(tagCount);
  bool* oldList;

  // check for '*'
  if (containsStar(&tagList)) {
    for (int i=0; i < tagCount; i++) {
      newList[i]=true;
    }
  } else {
    // change config configuration
    int idx;
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
  dlcMutex.lock();
  // check for '*'
  if (containsStar(&tagList)) {
    for (int i=0; i < tagCount; i++) {
      tagFlags[i]=true;
    }
  } else {
    // create new configuration
    int idx;

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
  dlcMutex.lock();
  // check for '*'
  if (containsStar(&tagList)) {
    for (int i=0; i < tagCount; i++) {
      tagFlags[i]=false;
    }
  } else {
    int idx;
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
char*
DietLogComponent::getEstimationTags(const int v_tag){
	char* ret;
	switch(v_tag){
		case(EST_INVALID): // -1
		ret = strdup("EST_INVALID");
		break;
		case(EST_TOTALTIME): // 1
		ret = strdup("EST_TOTALTIME");
	        break;
		case(EST_COMMTIME): // 2
		ret = strdup("EST_COMMTIME");
	        break;
		case(EST_TCOMP): // 3
		ret = strdup("EST_TCOMP");
	        break;
	        case(EST_TIMESINCELASTSOLVE): // 4
		ret = strdup("EST_TIMESINCELASTSOLVE");
	        break;
	        case(EST_COMMPROXIMITY): // 5
		ret = strdup("EST_COMMPROXIMITY");
	        break;
	        case(EST_TRANSFEREFFORT): // 6
		ret = strdup("EST_TRANSFEREFFORT");
	        break;
	        case(EST_FREECPU): // 7
		ret = strdup("EST_FREECPU");
	        break;
		case(EST_FREEMEM): // 8
		ret = strdup("EST_FREEMEM");
	        break;
		case(EST_NBCPU): // 9
		ret = strdup("EST_NBCPU");
	        break;
		case(EST_CPUSPEED): // 10
		ret = strdup("EST_CPUSPEED");
	        break;
		case(EST_TOTALMEM): // 11
		ret = strdup("EST_TOTALMEM");
	        break;
		case(EST_AVGFREEMEM): // 12
		ret = strdup("EST_AVGFREEMEM");
	        break;
		case(EST_AVGFREECPU): // 13
		ret = strdup("EST_AVGFREECPU");
	        break;
		case(EST_BOGOMIPS): // 14
		ret = strdup("EST_BOGOMIPS");
	        break;
		case(EST_CACHECPU): // 15
		ret = strdup("EST_CACHECPU");
	        break;
		case(EST_TOTALSIZEDISK): // 16
		ret = strdup("EST_TOTALSIZEDISK");
	        break;
		case(EST_FREESIZEDISK): // 17
		ret = strdup("EST_FREESIZEDISK");
	        break;
		case(EST_DISKACCESREAD): // 18
		ret = strdup("EST_DISKACCESREAD");
	        break;
		case(EST_DISKACCESWRITE): // 19
		ret = strdup("EST_DISKACCESWRITE");
	        break;
		case(EST_ALLINFOS): // 20
		ret = strdup("EST_ALLINFOS");
	        break;
		case(EST_NUMWAITINGJOBS): // 21
		ret = strdup("EST_NUMWAITINGJOBS");
	        break;
                case(EST_EFT): // 29
                ret = strdup("EST_EFT");
                break;
		case(EST_USERDEFINED): // 30
		ret = strdup("EST_USERDEFINED");
	        break;
		/********* HAVE_ALT_BATCH ***********/
	/* Type of the server: must be in accordance with DIET_server.h
	-> Should be replaced by the PARAL_ID (correspondings to serial, paral
	   with DIET specific batch system (numerous IDs depending on the
	   scheduling strategy implemented in the SeD), oar, LL, etc.) */
	case(EST_SERVER_TYPE):
	  ret=strdup("EST_SERVER_TYPE") ; // 22
	  break ;
	/* Parallel resources information. Assumed a default queue */
	case(EST_PARAL_NBTOT_RESOURCES): // 23
	  ret=strdup("EST_PARAL_NB_RESOURCES") ;
	  break ;
	case(EST_PARAL_NBTOT_FREE_RESOURCES): // 24
	  ret=strdup("EST_PARAL_NB_FREE_RESOURCES") ;
	  break ;
	case(EST_PARAL_NB_RESOURCES_IN_DEFAULT_QUEUE): // 25
	  ret=strdup("EST_PARAL_NB_RESOURCES_IN_DEFAULT_QUEUE") ;
	  break ;
	case(EST_PARAL_NB_FREE_RESOURCES_IN_DEFAULT_QUEUE): // 26
	  ret=strdup("EST_PARAL_NB_FREE_RESOURCES_IN_DEFAULT_QUEUE") ;
	  break ;
	case(EST_PARAL_MAX_WALLTIME): // 27
	  ret=strdup("EST_PARAL_MAX_WALLTIME") ;
	  break ;
	case(EST_PARAL_MAX_PROCS): // 28
	  ret=strdup("EST_PARAL_MAX_PROCS") ;
	  break ;
	  /* !HAVE_ALT_BATCH */
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
  DLC_ERROR("Cannot proceed without LogCentral connection",1);
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

#ifdef HAVE_DYNAMICS
/**
 * Change parent
 */
void
DietLogComponent::logNewParent(const char* type, const char* parent) {
  if (tagFlags[20]) {
    char * msg = new char[(strlen(type) + strlen(parent) + 2) * sizeof(char)];
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
    char * str = new char[10];
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
    char * str = new char[2];
    sprintf(str, "R");
    log(tagNames[22], str);
    delete [] str;
  }
}
#endif // HAVE_DYNAMICS

/**
 * Find SeD and solve problem
 */
void
DietLogComponent::logAskForSeD(const corba_request_t* request) {
  if (tagFlags[1]) {
    // FIXME: add request parameters (size of request arguments?)
    char* s;
    s = (char*)malloc( (strlen(request->pb.path) + num_Digits(request->reqID)+2)*sizeof(char));
    sprintf(s,"%s %ld",(const char *)(request->pb.path),(unsigned long)(request->reqID));
    log(tagNames[1], s);
    free(s);
  }
}

void
DietLogComponent::logSedChosen(const corba_request_t* request,
			       const corba_response_t* response) {
  if (tagFlags[2]) {
    char* s;
    if (response->servers.length()>0){
      unsigned int i,j;
      string estim_string = "";
      for ( i=0 ; i < response->servers.length();i++){
        estim_string.append(" ");
        estim_string.append(response->servers[i].loc.hostName);
        for (j=0 ; j < response->servers[i].estim.estValues.length() ; j++){
          int valTagInt = response->servers[i].estim.estValues[j].v_tag;
          estim_string.append(";");
          estim_string.append(getEstimationTags(valTagInt));
          estim_string.append("=");
          //char* v_value= new char[256];
          char v_value[128];
          sprintf(v_value,"%f",response->servers[i].estim.estValues[j].v_value);
          estim_string.append(v_value);
          //delete(v_value);
        }
      }
      s = (char*)malloc((strlen(request->pb.path)
                         +num_Digits(request->reqID)
                         +num_Digits(response->servers.length())
                         +estim_string.length()
                         +5)*sizeof(char));
      sprintf(s,"%s %ld %ld%s"
              ,(const char *)(request->pb.path)
              ,(unsigned long)(request->reqID)
              ,(unsigned long)(response->servers.length())
              ,(const char *)(estim_string.c_str())
              );
    } else {
      s = (char*)malloc((strlen(request->pb.path)
                         +num_Digits(request->reqID)
                         +num_Digits(response->servers.length())
                         +3)*sizeof(char));
      sprintf(s,"%s %ld %ld",
              (const char *)(request->pb.path),
              (unsigned long)(request->reqID),
              (unsigned long)(response->servers.length()) );
    }
    log(tagNames[2], s);
    free(s);
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
				const corba_profile_t* problem) {
  if (tagFlags[3]) {
    // FIXME: print problem (argument size?)
    char* s;
    s = (char*)malloc((strlen(path)+num_Digits(problem->dietReqID)+2)*sizeof(char));
    sprintf(s,"%s %ld",(const char *)(path),(unsigned long)(problem->dietReqID));
    log(tagNames[3], s);
    free(s);
  }
}

// modif bisnard_logs_1
void
DietLogComponent::logEndDownload(const char* path,
				 const corba_profile_t* problem) {
  if (tagFlags[28]) {
    char* s;
    s = (char*)malloc((strlen(path)+num_Digits(problem->dietReqID)+2)*sizeof(char));
    sprintf(s,"%s %ld",(const char *)(path),(unsigned long)(problem->dietReqID));
    log(tagNames[28], s);
    free(s);
  }
}
// end modif bisnard_logs_1

void
DietLogComponent::logEndSolve(const char* path,
			      const corba_profile_t* problem) {
  if (tagFlags[4]) {
    char* s;
    s = (char*)malloc((strlen(path)+num_Digits(problem->dietReqID)+2)*sizeof(char));
    sprintf(s,"%s %ld",(const char *)(path),(unsigned long)(problem->dietReqID));
    log(tagNames[4], s);
    free(s);
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
  //  s = new char[strlen(dataID) + sizeof(size)+ strlen(type) + strlen(base) + 4];
  s = (char*)malloc((strlen(dataID) + num_Digits(size)+ strlen(type) + 10 + 4)*sizeof(char));

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
    } // end switch
    log(tagNames[6], s);

  }
  free(base);
  free(s);
}

void
DietLogComponent::logDataBeginTransfer(const char* dataID,
		     const char* destAgent) {
  if (tagFlags[7]) {
    char* s;
    s = (char*)malloc((strlen(dataID)+strlen(destAgent)+2)*sizeof(char));
    sprintf(s,"%s %s",dataID, destAgent);
    log(tagNames[7], s);
    free(s);
  }
}

void
DietLogComponent::logDataEndTransfer(const char* dataID,
		     const char* destAgent) {
  if (tagFlags[8]) {
    char* s;
    s = (char*)malloc((strlen(dataID)+strlen(destAgent)+2)*sizeof(char));
    sprintf(s,"%s %s",dataID, destAgent);
    log(tagNames[8], s);
    free(s);
  }
}

// modif bisnard_logs_1
void
DietLogComponent::logDataTransferTime(const char* dataID,
		     const char* destAgent, const unsigned long elapsedTime) {
  if (tagFlags[23]) {
    char* s;
    s = (char*)malloc((strlen(dataID)+strlen(destAgent)+num_Digits(elapsedTime)+3)*sizeof(char));
    sprintf(s,"%s %s %ld",dataID, destAgent, elapsedTime);
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

#ifdef HAVE_WORKFLOW

/**
 * Send node ready
 */
void DietLogComponent::logWfNodeReady(const char *dagName,
                                      const char *nodeName)
{
  char* log_msg = (char*)malloc((strlen(dagName)+strlen(nodeName)+1)*sizeof(char)+1);
  sprintf(log_msg,"%s:%s",dagName,nodeName);
  log(tagNames[24], log_msg);
  free(log_msg);
}

/**
 * Send node execution start
 */
void DietLogComponent::logWfNodeStart(const char *dagName,
                                      const char *nodeName,
                                      const char *sedName,
                                      const char *pbName,
                                      const unsigned long reqID)
{
  char* log_msg = (char*)malloc((strlen(dagName)+strlen(nodeName)+strlen(pbName)
                                 +strlen(sedName)+num_Digits(reqID)+4)*sizeof(char)+1);
  sprintf(log_msg,"%s:%s:%s:%s %ld",dagName,nodeName,sedName,pbName,reqID);
  log(tagNames[25], log_msg);
  free(log_msg);
}

void DietLogComponent::logWfNodeStart(const char *dagName,
                                      const char *nodeName)
{
  char* log_msg = (char*)malloc((strlen(dagName)+strlen(nodeName)+1)*sizeof(char)+1);
  sprintf(log_msg,"%s:%s",dagName,nodeName);
  log(tagNames[25], log_msg);
  free(log_msg);
}

/**
 * Send node execution finish
 */
void DietLogComponent::logWfNodeFinish(const char *dagName,
                                       const char *nodeName)
{
  char* log_msg = (char*)malloc((strlen(dagName)+strlen(nodeName)+1)*sizeof(char)+1);
  sprintf(log_msg,"%s:%s",dagName,nodeName);
  log(tagNames[26], log_msg);
  free(log_msg);
}

/**
 * Send node failure
 */
void DietLogComponent::logWfNodeFailed(const char *dagName,
                                       const char *nodeName)
{
  char* log_msg = (char*)malloc((strlen(dagName)+strlen(nodeName)+1)*sizeof(char)+1);
  sprintf(log_msg,"%s:%s",dagName,nodeName);
  log(tagNames[27], log_msg);
  free(log_msg);
}

/**
 * Send madag schedulerType
 */
void
  DietLogComponent::maDagSchedulerType(const char* msg) {
  char* log_msg = (char*)malloc(strlen(msg)*sizeof(char)+1);
  sprintf(log_msg,"%s",msg);
  log(tagNames[18], log_msg);
  free(log_msg);
}

/**
 * Send dag identifier and workflow processing time in the MA
 */
void
  DietLogComponent::logDag(const char* msg) {
  char* log_msg = (char*)malloc(strlen(msg)*sizeof(char)+1);
  sprintf(log_msg,"%s",msg);
  log(tagNames[19], log_msg);
  free(log_msg);
}


#endif // HAVE_WORKFLOW

DietLogComponentFwdr::DietLogComponentFwdr(CorbaLogForwarder_ptr fwdr,
																					 const char* objName)
{
	this->forwarder = CorbaLogForwarder::_duplicate(fwdr);
	this->objName = CORBA::string_dup(objName);
}

void DietLogComponentFwdr::setTagFilter(const tag_list_t& tagList) {
	forwarder->setTagFilter(tagList, objName);
}

void DietLogComponentFwdr::addTagFilter(const tag_list_t& tagList) {
	forwarder->addTagFilter(tagList, objName);
}

void DietLogComponentFwdr::removeTagFilter(const tag_list_t& tagList) {
	forwarder->removeTagFilter(tagList, objName);
}

void DietLogComponentFwdr::test() {
	forwarder->test(objName);
}
