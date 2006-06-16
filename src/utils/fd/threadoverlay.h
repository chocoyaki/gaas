/** Fast remap of pthreads to omni_threads
  * Aurélien Bouteiller <aurelien.bouteiller@ens-lyon.fr> 
  */
#ifndef __INCLUDE_THREADOVERLAY__
#define __INCLUDE_THREADOVERLAY__

#include <omnithread.h>

typedef omni_thread *thread_t;
#define thread_create(id, attr, fn, arg) (!((*(id)) = omni_thread::create(fn, arg)))
#define thread_join(id, ret) ((id)->join(ret))
#define thread_detach(id) 0

typedef omni_mutex *thread_mutex_t;
#define THREAD_MUTEX_INITIALIZER (new omni_mutex())
#define thread_mutex_lock(id) ((*(id))->lock())
#define thread_mutex_unlock(id) ((*(id))->unlock())

typedef omni_condition *thread_cond_t;
#define THREAD_COND_INITIALIZER NULL
#define thread_cond_wait(id, m) ((*(id)) ? (*(id))->wait() : ((*(id)) = new omni_condition(*m))->wait())
#define thread_cond_timedwait(id, m, ts) ((*(id)) ? (*(id))->timedwait((ts)->tv_sec, (ts)->tv_nsec) : ((*(id)) = new omni_condition(*(m)))->timedwait((ts)->tv_sec, (ts)->tv_nsec))
#define thread_cond_signal(id) ((*(id))->signal())
#define thread_cond_boadcast(id) ((*(id))->broadcast())

#endif
