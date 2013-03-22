/**
 * @file  Thread.hh
 *
 * @brief  C++ Thread class description
 *
 * @author  Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
*/


#ifndef _THREAD_HH_
#define _THREAD_HH_

#include <iostream>
#include <string>
#include <memory>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/thread.hpp>
#ifdef WIN32
#define DIET_API_LIB __declspec(dllexport)
#else
#define DIET_API_LIB
#endif
class Callable;

class DIET_API_LIB Thread : boost::noncopyable {
public:
  explicit Thread(bool isDetached = false);

  virtual ~Thread();

  void start();

  void* join();

private:
  friend class Callable;

  virtual void*
  run() {
    return NULL;
  }

  /**
   * true if thread created in detached state;false otherwise *
   */
  bool detached;

  boost::thread t;

  void* result;                      // stores return value of run()

  void
  PrintError(std::string msg, int status,
             std::string fileName, int lineNumber);
};


#endif   /* not defined _THREAD_HH */




