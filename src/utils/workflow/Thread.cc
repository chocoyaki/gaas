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


class Callable {
public:
  explicit Callable(Thread* thread, void *result) : t(thread), res(result) {}

  void operator() () {
      res = t->run();
  }

private:
  Thread* t;
  void* res;
};


Thread::Thread(bool isDetached)
  : detached(isDetached), result(NULL) {
}

Thread::~Thread() {
}

void Thread::start() {
  Callable c(this, result);
  boost::thread tmp(c);
  t.swap(tmp);
  if (detached) {
    t.detach();
  }
}

void*
Thread::join() {
  t.join();

  return result;
}

void
Thread::PrintError(std::string msg, int status,
                   std::string fileName, int lineNumber) {
  TRACE_TEXT(TRACE_MAIN_STEPS, msg << ' ' << fileName << ":" << lineNumber
             << "- " << strerror(status) << std::endl);
}
