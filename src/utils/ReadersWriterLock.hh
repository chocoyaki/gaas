/****************************************************************************/
/* ReadersWriter guard		 					    */
/* 	algorithm from comp.programming.threads and c.schmidt scope  	    */
/* 	guard paper							    */
/*                                  					    */
/*  Author(s):                                                              */
/*    - Christophe PERA (christophe.pera@ens-lyon.fr)                       */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $log$
 ****************************************************************************/

#ifndef _ReadersWriterLock_h_
#define _ReadersWriterLock_h_

#define DIET_READERLOCKGUARD(X) ScopeReaderLock(&X);
#define DIET_WRITERLOCKGUARD(X) ScopeWriterLock(&X);

#define DIET_READERLOCKGUARD_BEGIN(X) do {ReaderLockGuard(&X);
#define DIET_READERLOCKGUARD_END() } while (0)

#define DIET_WRITERLOCKGUARD_BEGIN(X) do {WriterLockGuard(&X);
#define DIET_WRITERLOCKGUARD_END() }while (0)

#include <iostream>
#include <omnithread.h>




// managing multiple reads and one write
// managing recursive lock for reader or sole writer.
class DietReadersWriterLock {
  omni_mutex m;
  omni_condition c;
  int writer_count, readers_waiting, reader_count, is_write_lock;
  
public:

DietReadersWriterLock() : c(&m), writer_count(0), readers_waiting(0), reader_count(0), is_write_lock(0){}

  void readLock(void) { 
    m.lock(); // if there is at least one writer using the lock or waiting for it, 
		    // we need to wait for access 
    if (writer_count > 0) { 
      if (readers_waiting++ == 0){ // if we're the first reader in line 
	m.unlock(); 
	c.wait(); // get the access lock 
	m.lock();
	if (readers_waiting > 1) // then if there are other readers 
	  c.broadcast(); // let them go 
      } else { 
	m.unlock(); 
	c.wait(); // otherwise wait until someone lets us go 
	m.lock(); 
      } 
      readers_waiting--; 
    } 
    reader_count++; 
    m.unlock(); 
  }
      
  void writeLock(void) { 
    m.lock(); 
    writer_count++; // one more writer 
    m.unlock(); 
    c.wait(); // wait until the access lock is available 
    m.lock(); 
    is_write_lock = 1; // lock is a write lock 
    c.broadcast(); // give readers something to wait for 
    m.unlock(); 
  } 
    
  void unlock(void) { 
    m.lock(); 
    if (is_write_lock) { // if this is a write lock 
      is_write_lock = 0; // let it go 
      writer_count--; // one less writer 
      c.broadcast(); // now let someone else have a chance 
    } else if (--reader_count == 0) // if we're the last reader 
      c.broadcast(); // release the access lock 
    m.unlock(); 
  }
};

//simple Scoped Lock
class ReaderLockGuard {
  DietReadersWriterLock& lock;
  bool state;
public:
  ReaderLockGuard(DietReadersWriterLock& l) : lock(l), state(false) {
    acquire();
    state = true;
    }
  ~ReaderLockGuard(void) {
    release();
    }
  void acquire(void){
    if (state == false){
      lock.readLock();
      state = true;
    }
    }
  void release(void){
    if (state == true){
      lock.unlock();
      state = false;
    }
  }
};

//simple Scoped Lock
class WriterLockGuard {
  DietReadersWriterLock& lock;
  bool state;
public:
  WriterLockGuard(DietReadersWriterLock& l) : lock(l), state(false) {
    acquire();
    state = true;
    }
  ~WriterLockGuard(void) {
    release();
    }
  void acquire(void){
    if (state == false){
      lock.writeLock();
      state = true;
    }
    }
  void release(void){
    if (state == true){
      lock.unlock();
      state = false;
    }
    }
};

#endif
