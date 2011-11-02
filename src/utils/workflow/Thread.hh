/**
* @file  Thread.hh
* 
* @brief  C++ Thread class description 
* 
* @author  - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)
* 
* @section Licence
*   |LICENSE|                                                                
*/
/* $Id$
 * $Log$
 * Revision 1.6  2011/03/18 16:28:33  hguemar
 * fix initialization and const-correctness issues in src/utils/workflow/Thread.{hh, cc} raised by cppchecks
 *
 * Revision 1.5  2011/01/17 16:38:51  bdepardo
 * Missing space in virtual~Runnable() = 0;
 *
 * Revision 1.4  2009/09/29 11:42:45  bisnard
 * commented out getId() due to pthread_t being actually an opaque type
 *
 * Revision 1.3  2009/09/25 12:49:11  bisnard
 * avoid deadlocks due to new thread mgmt in DagNodeLauncher
 *
 * Revision 1.2  2008/06/01 15:50:39  rbolze
 * correct warning
 *
 * Revision 1.1  2008/04/10 08:38:50  bisnard
 * New version of the MaDag where workflow node execution is triggered by the MaDag agent and done by a new CORBA object CltWfMgr located in the client
 *
 * Revision 1.2  2006/11/27 10:15:12  aamar
 * Correct headers of files used in workflow support.
 *
 * Revision 1.1  2006/04/14 14:02:41  aamar
 * Provides a C++ thread class (based on pthread, used by Node class).
 *
 ****************************************************************************/

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




