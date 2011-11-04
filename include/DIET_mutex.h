/**
* @file DIET_mutex.h
* 
* @brief  DIET mutex interface for multi-threaded server applications 
* 
* @author Philippe COMBES (Philippe.Combes@ens-lyon.fr)                  
*         Bert VAN HEUKELOM (Bert.Van-Heukelom@ens-lyon.fr) 
* 
* @section Licence
*   |LICENSE|                                                                
*/



#ifndef _DIET_MUTEX_H_
#define _DIET_MUTEX_H_

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus
  typedef int diet_mutex_t;

  /**
   * @brief initialize DIET mutexes
   *
   */
  void
  diet_mutex_initialize();

  /**
   * @brief create mutex
   *
   * @param ret
   */
  void
  diet_mutex_create(int* ret);

  /**
   * @brief lock mutex
   *
   * @param i index of the mutex to be locked
   */
  void
  diet_mutex_lock(int i);

  /**
   * @brief unlock mutex
   *
   * @param i index of the mutex to be unlocked
   */
  void
  diet_mutex_unlock(int i);

  /**
   * @brief free mutex
   *
   * @param i
   */
  void
  diet_mutex_free(int* i);

  /**
   * @brief finalize DIET mutexes
   *
   */
  void
  diet_mutex_finalize();

  /**
   * @brief put current thread to sleep
   *
   * @param m seconds
   * @param n nanoseconds
   */
  void
  diet_thread_sleep(int m, int n);

  /**
   * @brief yield current thread
   *
   */
  void
  diet_thread_yield();

  /**
   * @brief get current thread id
   *
   * @return thread id
   */
  int
  diet_thread_id();
#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // _DIET_MUTEX_H_


