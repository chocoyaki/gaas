/****************************************************************************/
/* C++ Thread                                                               */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.1  2006/04/14 14:02:41  aamar
 * Provides a C++ thread class (based on pthread, used by Node class).
 *
 ****************************************************************************/

#ifndef _THREAD_HH_
#define _THREAD_HH_

#include <iostream>
#include <memory>

using namespace std;

class Runnable {
public:
  /*********************************************************************/
  /* public methods                                                    */
  /*********************************************************************/
  virtual~Runnable() = 0;

  virtual void* 
  run() = 0;
};

class Thread {
public:
  /*********************************************************************/
  /* public methods                                                    */
  /*********************************************************************/
  Thread(auto_ptr<Runnable> runnable_, bool isDetached = false);
  Thread(bool isDetached = false);

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

  auto_ptr<Runnable> runnable;

  /**
   * true if thread created in detached state;false otherwise *
   */
  bool detached;

  pthread_attr_t threadAttribute;

  Thread(const Thread&);
  const Thread& operator= (const Thread&);

  void setCompleted();

  void* result;                      // stores return value of run()

  virtual
  void* run() { return NULL;}

  static void* 
  startThreadRunnable(void* pVoid);

  static void* 
  startThread(void* pVoid);

  void
  PrintError(char* msg, int status, char* fileName, int lineNumber);
};


#endif   /* not defined _THREAD_HH */



  