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
#include <assert.h>
#include "Thread.hh"

Runnable::~Runnable() {}

Thread::Thread(auto_ptr<Runnable> runnable_, bool isDetached) :
  runnable(runnable_),detached(isDetached){
  if (runnable.get() == NULL) {
    std::cout << "Thread::Thread(auto_ptr<Runnable> runnable_,"
	      <<"bool isDetached) failed at " << ' ' << __FILE__ << ":"
	      << __LINE__ << "- " << "runnable is NULL " << std::endl; 
    exit(-1);
  }
}

Thread::Thread(bool isDetached) : runnable(NULL), detached(isDetached){ }

Thread::~Thread() { }

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
Thread::setCompleted() {/* completion was handled by pthread_join() */}

void
Thread::PrintError(char* msg, int status, char* fileName, int lineNumber) {
  std::cout << msg << ' ' << fileName << ":" << lineNumber
	    << "- " << strerror(status) << std::endl;
}
