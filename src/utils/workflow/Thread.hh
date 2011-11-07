/**
* @file  Thread.hh
* 
* @brief  C++ Thread class description 
* 
* @author  Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)
* 
* @section Licence
*   |LICENCE|                                                                
*/


#ifndef _THREAD_HH_
#define _THREAD_HH_

#include <iostream>
#include <memory>
#include <string>

class Runnable {
public:
  /*********************************************************************/
  /* public methods                                                    */
  /*********************************************************************/
  virtual ~Runnable() = 0;

  virtual void*
  run() = 0;
};

class Thread {
public:
  /*********************************************************************/
  /* public methods                                                    */
  /*********************************************************************/
  Thread(std::auto_ptr<Runnable> runnable_, bool isDetached = false);
  explicit Thread(bool isDetached = false);

  virtual ~Thread();

  void start();

  void* join();

private:
  /*********************************************************************/
  /* private fields                                                    */
  /*********************************************************************/
  /**
   * thread ID *
   */
  pthread_t PthreadThreadID;

  std::auto_ptr<Runnable> runnable;

  /**
   * true if thread created in detached state;false otherwise *
   */
  bool detached;

  pthread_attr_t threadAttribute;

  Thread(const Thread&);

  const Thread&
  operator=(const Thread&);

  void
  setCompleted() const;

  void* result;                      // stores return value of run()

  virtual void*
  run() {
    return NULL;
  }

  static void*
  startThreadRunnable(void* pVoid);

  static void*
  startThread(void* pVoid);

  void
  PrintError(std::string msg, int status,
             std::string fileName, int lineNumber);
};


#endif   /* not defined _THREAD_HH */




