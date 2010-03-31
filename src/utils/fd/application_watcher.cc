/****************************************************************************/
/* DIET Fault Detector Application watcher                                  */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Aurélien Bouteiller (aurelien.bouteiller@inria.fr)                  */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$ 
 * $Log$
 * Revision 1.4  2010/03/31 21:15:40  bdepardo
 * Changed C headers into C++ headers
 *
 * Revision 1.3  2006/06/20 13:29:16  abouteil
 *
 *
 * Chandra&Toueg&Aguilera fault detector implementation.
 *
 * A
 *
 ****************************************************************************/	
#include <sys/time.h>
#include <sys/msg.h>

#include <cerrno>
#include "threadoverlay.h"

#include "fdd.h"
#include "formulae.h"


void* setup_application_watcher (void *nothing)
{
  watched_process p;
  struct timeval t;

  thread_mutex_lock (&watched_mutex);
  while (1) {
    if (heap_is_empty (watched)) {
      /* wait for a watched process to be added */
      thread_cond_wait (&new_watched_cond, &watched_mutex);
    } else {
      /* wait for either a new watched process to be added,
       * or the current next process to need attention
       */
      p = (watched_process) heap_peek (watched);
      thread_cond_timedwait (&new_watched_cond,
        &watched_mutex, &p->tau_lp1);
    }
    /* an application may have become suspect */
    p = (watched_process) heap_extract (watched);

    p->tau_lp1.tv_sec = p->next_tau_lp1.tv_sec;
    p->tau_lp1.tv_nsec = p->next_tau_lp1.tv_nsec;
    gettimeofday (&t, NULL);

    if (timespec_to_seconds (&p->tau_lp1) < timeval_to_seconds (&t)) {
      /* suspect !*/       
      p->suspect = 1;
      fd_notify(p->handle);
    } else {
      /* not suspect */
      heap_insert (watched, p);
    }
  }
}

