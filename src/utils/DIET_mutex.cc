/**
 * @file  DIET_mutex.cc
 *
 * @brief  DIET mutex interface for multi-threaded server applications (source code)
 *
 * @author  Philippe COMBES (Philippe.Combes@ens-lyon.fr)
 *          Bert VAN HEUKELOM (Bert.Van-Heukelom@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENCE|
 */


#include "DIET_mutex.h"

#include <cstdio>
#include <cstdlib>
#include <iostream>

#include "debug.hh"
#include "omnithread.h"


static omni_mutex **MUTEX_FIELD = NULL;
static int MUTEXCOUNT = 0;
static int INITIALIZED = 0;


#define MUTEX_ERROR(formatted_text)                     \
  ERROR_DEBUG(__FUNCTION__ << ": " << formatted_text, )

#define MUTEX_CHECK_INIT()                                      \
  if (!INITIALIZED) {                                           \
    MUTEX_ERROR("diet_mutex_initialize has not been called");   \
  }

#define MUTEX_CHECK_VALIDITY(i)                 \
  if ((i) >= MUTEXCOUNT) {                      \
    MUTEX_ERROR("invalid mutex");               \
  }



void
diet_mutex_initialize() {
  int i;
  MUTEX_FIELD = (omni_mutex **) malloc(10 * sizeof(omni_mutex));
  for (i = 0; i < 10; i++) {
    MUTEX_FIELD[i] = NULL;
  }
  MUTEXCOUNT = 10;
  INITIALIZED = 1;
}

void
diet_mutex_create(int *ret) {
  int i;
  omni_mutex **temp;

  MUTEX_CHECK_INIT();

  for (i = 0; i < MUTEXCOUNT; i++) {
    if (MUTEX_FIELD[i] == NULL) {
      MUTEX_FIELD[i] = new omni_mutex();
      *ret = i;
      return;
    }
  }

  temp = (omni_mutex **) malloc((10 + MUTEXCOUNT) * sizeof(omni_mutex));
  for (i = 0; i < MUTEXCOUNT; i++) {
    temp[i] = MUTEX_FIELD[i];
  }
  for (i = 0; i < 10; i++) {
    temp[i + MUTEXCOUNT] = NULL;
  }
  free(MUTEX_FIELD);
  MUTEX_FIELD = temp;

  MUTEX_FIELD[MUTEXCOUNT] = new omni_mutex();
  *ret = MUTEXCOUNT;

  MUTEXCOUNT += 10;
} // diet_mutex_create

void
diet_mutex_free(int *i) {
  MUTEX_CHECK_INIT();
  MUTEX_CHECK_VALIDITY(*i);

  delete (MUTEX_FIELD[*i]);

  MUTEX_FIELD[*i] = NULL;
  *i = 0;
}

void
diet_mutex_lock(int i) {
  MUTEX_CHECK_INIT();
  MUTEX_CHECK_VALIDITY(i);

  MUTEX_FIELD[i]->lock();
}

void
diet_mutex_unlock(int i) {
  MUTEX_CHECK_INIT();
  MUTEX_CHECK_VALIDITY(i);

  MUTEX_FIELD[i]->unlock();
}

void
diet_mutex_finalize() {
  MUTEX_CHECK_INIT();

  free(MUTEX_FIELD);
  INITIALIZED = 0;
}

void
diet_thread_sleep(int m, int n) {
  omni_thread *myThread = NULL;
  myThread = omni_thread::self();
  myThread->sleep(m, n);
}

void
diet_thread_yield() {
  omni_thread *myThread = NULL;
  myThread = omni_thread::self();
  myThread->yield();
}

int
diet_thread_id() {
  omni_thread *myThread = NULL;
  myThread = omni_thread::self();
  return myThread->id();
}
