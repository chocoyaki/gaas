/****************************************************************************/
/* $Id$ */
/* DIET mutex interface for multi-threaded server applications              */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Philippe COMBES (Philippe.Combes@ens-lyon.fr)                       */
/*    - Bert VAN HEUKELOM (Bert.Van-Heukelom@ens-lyon.fr)                   */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/*
 * $Log$
 * Revision 1.4  2003/02/07 17:03:05  pcombes
 * Apply Coding Standards.
 *
 * Revision 1.1  2002/12/03 19:08:24  pcombes
 * Update configure, update to FAST 0.3.15, clean CVS logs in files.
 * Put main Makefile in root directory.
 ****************************************************************************/


#ifndef _DIET_MUTEX_H_
#define _DIET_MUTEX_H_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
  
typedef int diet_mutex_t;

void
diet_mutex_initialize();
void
diet_mutex_create(int*);
void
diet_mutex_lock(int);
void
diet_mutex_unlock(int);
void
diet_mutex_free(int*);
void
diet_mutex_finalize();

void
diet_thread_sleep(int, int);
void
diet_thread_yield();
int
diet_thread_id();

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _DIET_MUTEX_H_


