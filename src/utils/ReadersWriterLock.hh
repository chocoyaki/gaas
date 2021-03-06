/**
 * @file  ReadersWriterLock.hh
 *
 * @brief  ReadersWriter guard algorithm from comp.programming.threads and c.schmidt scope
 *
 * @author  Christophe PERA (christophe.pera@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


#ifndef _ReadersWriterLock_h_
#define _ReadersWriterLock_h_

#define DIET_READERLOCKGUARD(X) ScopeReaderLock(&X);
#define DIET_WRITERLOCKGUARD(X) ScopeWriterLock(&X);

#define DIET_READERLOCKGUARD_BEGIN(X) do {ReaderLockGuard(&X);
#define DIET_READERLOCKGUARD_END() } while (0)

#define DIET_WRITERLOCKGUARD_BEGIN(X) do {WriterLockGuard(&X);
#define DIET_WRITERLOCKGUARD_END() } while (0)

#include <iostream>
#include <omnithread.h>
#include "debug.hh"

// managing multiple reads and one write
// managing recursive lock for reader or sole writer.
class DietReadersWriterLock {
public:
  DietReadersWriterLock()
    : c(&m), writer_count(0), readers_waiting(0),
    reader_count(0), is_write_lock(0) {
  }

  void
  readLock(void) {
    // if there is at least one writer using the lock or waiting for it,
    // we need to wait for access
    m.lock();
    while (writer_count > 0) {
      c.wait();
    }
    reader_count++;
    m.unlock();
  }

  void
  writeLock(void) {
    m.lock();
    while (is_write_lock == 1 || reader_count > 0) {
      c.wait();  // wait until the access lock is available
    }

    is_write_lock = 1;  // lock is a write lock
    m.unlock();
    c.broadcast();  // give readers something to wait for
  }

  void
  unlock(void) {
    m.lock();
    if (is_write_lock) {  // if this is a write lock
      is_write_lock = 0;  // let it go
      c.broadcast();  // now let someone else have a chance
    } else if (--reader_count == 0) {  // if we're the last reader
      c.broadcast();  // release the access lock
    }
    m.unlock();
  }

private:
  omni_mutex m;
  omni_condition c;
  int writer_count, readers_waiting, reader_count, is_write_lock;
};

// simple Scoped Lock
class ReaderLockGuard {
public:
  ReaderLockGuard(DietReadersWriterLock & l): lock(l), state(false) {
    acquire();
  }

  ~ReaderLockGuard(void) {
    release();
  }

  void
  acquire(void) {
    if (state == false) {
      lock.readLock();
      state = true;
    }
  }

  void
  release(void) {
    if (state == true) {
      lock.unlock();
      state = false;
    }
  }

private:
  DietReadersWriterLock &lock;
  bool state;
};

// simple Scoped Lock
class WriterLockGuard {
public:
  WriterLockGuard(DietReadersWriterLock & l): lock(l), state(false) {
    acquire();
  }

  ~WriterLockGuard(void) {
    release();
  }

  void
  acquire(void) {
    if (state == false) {
      lock.writeLock();
      state = true;
    }
  }

  void
  release(void) {
    if (state == true) {
      lock.unlock();
      state = false;
    }
  }

private:
  DietReadersWriterLock &lock;
  bool state;
};

#endif /* ifndef _ReadersWriterLock_h_ */
