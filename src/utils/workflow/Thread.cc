/**
* @file  Thread.cc
* 
* @brief  C++ Thread class implementation 
* 
* @author  Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)
* 
* @section Licence
*   |LICENCE|                                                                
*/

#include <iostream>
#include <cassert>
#include <cstring>
#include <cstdlib>

#include "Thread.hh"
#include "debug.hh"


Runnable::~Runnable() {
}

Thread::Thread(std::auto_ptr<Runnable> runnable_, bool isDetached)
  : runnable(runnable_), detached(isDetached), result(NULL) {
  if (runnable.get() == NULL) {
    ERROR_EXIT("Thread::Thread(auto_ptr<Runnable> runnable_, "
               <<"bool isDetached) failed at " << ' ' << __FILE__ << ":"
               << __LINE__ << "- " << "runnable is NULL " << std::endl);
  }
}

Thread::Thread(bool isDetached)
  : runnable(NULL), detached(isDetached), result(NULL) {
}

Thread::~Thread() {
}

void*
Thread::startThreadRunnable(void* pVoid) {
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
  int status = pthread_attr_init(&threadAttribute);
  if (status != 0) {
    PrintError("pthread_attr_init failed at", status, __FILE__, __LINE__);
    exit(status);
  }
  status = pthread_attr_setscope(&threadAttribute, PTHREAD_SCOPE_SYSTEM);
  if (status != 0) {
    PrintError("pthread_attr_setscope failed at", status, __FILE__, __LINE__);
    exit(status);
  }
  if (!detached) {
    if (runnable.get() == NULL) {
      int status = pthread_create(&PthreadThreadID, &threadAttribute,
                                  Thread::startThread, (void*) this);
      if (status != 0) {
        PrintError("pthread_create failed at", status, __FILE__, __LINE__);
        exit(status);
      }
    } else {
      int status = pthread_create(&PthreadThreadID, &threadAttribute,
                                  Thread::startThreadRunnable, (void*)this);
      if (status != 0) {
        PrintError("pthread_create failed at", status, __FILE__, __LINE__);
        exit(status);
      }
    }
  } else {
    // set the detachstate attribute to detached
    status =
      pthread_attr_setdetachstate(&threadAttribute, PTHREAD_CREATE_DETACHED);
    if (status != 0) {
      PrintError("pthread_attr_setdetachstate failed at",
                 status, __FILE__, __LINE__);
      exit(status);
    }
    if (runnable.get() == NULL) {
      status = pthread_create(&PthreadThreadID, &threadAttribute,
                              Thread::startThread, (void*) this);
      if (status != 0) {
        PrintError("pthread_create failed at", status, __FILE__, __LINE__);
        exit(status);
      }
    } else {
      status = pthread_create(&PthreadThreadID, &threadAttribute,
                              Thread::startThreadRunnable, (void*) this);
      if (status != 0) {
        PrintError("pthread_create failed at", status, __FILE__, __LINE__);
        exit(status);
      }
    }
  }
  status = pthread_attr_destroy(&threadAttribute);
  if (status != 0) {
    PrintError("pthread_attr_destroy failed at", status, __FILE__, __LINE__);
    exit(status);
  }
}

void*
Thread::join() {
  int status = pthread_join(PthreadThreadID, NULL);
  // result was already saved by thread start functions
  if (status != 0) {
    PrintError("pthread_join failed at", status, __FILE__, __LINE__);
    exit(status);
  }
  return result;
}

void
Thread::setCompleted() const {
  /* completion was handled by pthread_join() */
}

void
Thread::PrintError(std::string msg, int status,
                   std::string fileName, int lineNumber) {
  TRACE_TEXT(TRACE_MAIN_STEPS, msg << ' ' << fileName << ":" << lineNumber
             << "- " << strerror(status) << std::endl);
}
