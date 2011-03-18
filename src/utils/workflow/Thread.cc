/****************************************************************************/
/* C++ Thread class implementation                                          */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.11  2011/03/18 16:38:03  bdepardo
 * Fixed compilation
 *
 * Revision 1.10  2011/03/18 16:28:33  hguemar
 * fix initialization and const-correctness issues in src/utils/workflow/Thread.{hh,cc} raised by cppchecks
 *
 * Revision 1.9  2011/02/24 16:50:06  bdepardo
 * Code cleanup.
 * Use TRACE_TEXT instead of cout
 *
 * Revision 1.8  2010/03/31 21:15:41  bdepardo
 * Changed C headers into C++ headers
 *
 * Revision 1.7  2010/03/15 14:00:09  bdepardo
 * C++ includes instead of C includes
 *
 * Revision 1.6  2009/09/29 11:42:45  bisnard
 * commented out getId() due to pthread_t being actually an opaque type
 *
 * Revision 1.5  2009/09/25 12:49:11  bisnard
 * avoid deadlocks due to new thread mgmt in DagNodeLauncher
 *
 * Revision 1.4  2008/12/12 16:35:39  bdepardo
 * Added #include <cstdlib> for exit
 *
 * Revision 1.3  2008/10/22 09:51:17  bisnard
 * missing include
 *
 * Revision 1.2  2008/06/01 15:50:30  rbolze
 * correct warning
 *
 * Revision 1.1  2008/04/10 08:38:50  bisnard
 * New version of the MaDag where workflow node execution is triggered by the MaDag agent and done by a new CORBA object CltWfMgr located in the client
 *
 * Revision 1.3  2006/11/27 10:15:12  aamar
 * Correct headers of files used in workflow support.
 *
 * Revision 1.2  2006/07/10 11:08:43  aamar
 * Added a missing assert.h header
 *
 * Revision 1.1  2006/04/14 14:02:41  aamar
 * Provides a C++ thread class (based on pthread, used by Node class).
 *
 ****************************************************************************/

#include <iostream>
#include <cassert>
#include <cstring>
#include <cstdlib>

#include "Thread.hh"
#include "debug.hh"


Runnable::~Runnable() {}

Thread::Thread(auto_ptr<Runnable> runnable_, bool isDetached) :
  runnable(runnable_), detached(isDetached), result(NULL) {
  if (runnable.get() == NULL) {
    ERROR_EXIT("Thread::Thread(auto_ptr<Runnable> runnable_,"
               <<"bool isDetached) failed at " << ' ' << __FILE__ << ":"
               << __LINE__ << "- " << "runnable is NULL " << std::endl);
  }
}

Thread::Thread(bool isDetached)
  : runnable(NULL), detached(isDetached), result(NULL) { }

Thread::~Thread() { }

// long unsigned int
// Thread::getId() {	return PthreadThreadID; }

void*
Thread::startThreadRunnable(void* pVoid){
  // thread start function when a Runnable is involved
  Thread* runnableThread = static_cast<Thread*> (pVoid);
  assert(runnableThread);
  runnableThread->result = runnableThread->runnable->run();
  runnableThread->setCompleted();
  return runnableThread->result;
}

void*
Thread::startThread(void* pVoid) {
  // thread start function when no Runnable is involved
  Thread* aThread = static_cast<Thread*> (pVoid);
  assert(aThread);
  aThread->result = aThread->run();
  aThread->setCompleted();
  return aThread->result;
}


void Thread::start() {
  int status = pthread_attr_init(&threadAttribute); // initialize attribute object
  if (status != 0) {
    PrintError("pthread_attr_init failed at", status, __FILE__,
	       __LINE__); exit(status);
  }
  status = pthread_attr_setscope(&threadAttribute,
				 PTHREAD_SCOPE_SYSTEM);
  if (status != 0) {
    PrintError("pthread_attr_setscope failed at",
	       status, __FILE__, __LINE__); exit(status);
  }
  if (!detached) {
    if (runnable.get() == NULL) {
      int status = pthread_create(&PthreadThreadID,&threadAttribute,
				  Thread::startThread,(void*) this);
      if (status != 0) { PrintError("pthread_create failed at",
				    status, __FILE__, __LINE__); exit(status);}
    }
    else {
      int status = pthread_create(&PthreadThreadID,&threadAttribute,
				  Thread::startThreadRunnable, (void*)this);
      if (status != 0) {PrintError("pthread_create failed at",
				   status, __FILE__, __LINE__); exit(status);}
    }
  }
  else {
    // set the detachstate attribute to detached
    status = pthread_attr_setdetachstate(&threadAttribute,
					 PTHREAD_CREATE_DETACHED);
    if (status != 0){
      PrintError("pthread_attr_setdetachstate failed at",
                 status,__FILE__,__LINE__);exit(status);
    }
    if (runnable.get() == NULL) {
      status = pthread_create(&PthreadThreadID,&threadAttribute,
			      Thread::startThread, (void*) this);
      if (status != 0) {PrintError("pthread_create failed at",
				   status, __FILE__, __LINE__);exit(status);}
    }
    else {
      status = pthread_create(&PthreadThreadID,&threadAttribute,
			      Thread::startThreadRunnable, (void*) this);
      if (status != 0) {PrintError("pthread_create failed at",
				   status, __FILE__, __LINE__); exit(status);}
    }
  }
  status = pthread_attr_destroy(&threadAttribute);
  if (status != 0) { PrintError("pthread_attr_destroy failed at",
				status, __FILE__, __LINE__); exit(status);}
}

void*
Thread::join() {
  int status = pthread_join(PthreadThreadID,NULL);
  // result was already saved by thread start functions
  if (status != 0) { PrintError("pthread_join failed at",
				status, __FILE__, __LINE__); exit(status);}
  return result;
}

void
Thread::setCompleted() const { /* completion was handled by pthread_join() */ }

void
Thread::PrintError(std::string msg, int status, std::string fileName, int lineNumber) {
  TRACE_TEXT(TRACE_MAIN_STEPS, msg << ' ' << fileName << ":" << lineNumber
             << "- " << strerror(status) << std::endl);
}
