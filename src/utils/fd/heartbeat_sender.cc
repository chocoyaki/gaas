/****************************************************************************/
/* DIET Fault Detector Heartbeat Sender                                     */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Aurélien Bouteiller (aurelien.bouteiller@inria.fr)                  */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$ 
 * $Log$
 * Revision 1.6  2010/10/04 08:17:23  bdepardo
 * Changed memory management from C to C++ (malloc/free -> new/delete)
 * This corrects a bug at initialization
 *
 * Revision 1.5  2010/03/31 21:15:40  bdepardo
 * Changed C headers into C++ headers
 *
 * Revision 1.4  2007/03/30 15:48:34  dart
 * - Add <sys/socket.h> to compile under AIX with XLC v8.0
 * - Change h_addr to http_addr in udp_server.cc to avoid the error message
 *   "The array bound cannot be zero" (visibly, h_addr already exists in
 *   /usr/include/isode/internet.h", is it the real reason ?)
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
#include <sys/socket.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "threadoverlay.h"


#include "fdd.h"
#include "formulae.h"


/* sends the heartbeats */
void* setup_heartbeat_sender (void *nothing)
{
  beating_process p;
  struct _provided_service service;
  void* search_result;
  char b[HEARTBEAT_LENGTH];
  int s;
  struct sockaddr_in addr;
  struct timeval tv;
  double next_sec;

  /* set up address */
  if (memset (&addr, 0, sizeof (struct sockaddr_in)) != &addr) {
    fatal_error ("memset");
  };
  addr.sin_family = PF_INET;
  addr.sin_port = htons(fd_UDP_port);
  
  /* socket creation */
  s = socket (PF_INET, SOCK_DGRAM, 0);
  if (s == -1) {
    fatal_error ("socket");
  };
  thread_mutex_lock (&beating_mutex);
  while (1) {
    if (heap_is_empty (beating)) {
      /* wait for a beating process to be added */
      thread_cond_wait (&new_beating_cond,
        &beating_mutex);
    } else {
      /* wait for either a new beating process to be
       * added, or the current next process to need
       * attention
       */
      p = (beating_process) heap_peek (beating);
      thread_cond_timedwait (&new_beating_cond,
        &beating_mutex, &(p->next));
    }
    /* a new heartbeat has to be sent */
    p = (beating_process) heap_extract (beating);
    if (p->valid) {
      /* check if it is still in provided hash */
      service.service_number = p->service_number;

      thread_mutex_lock (&provided_mutex);
      search_result = hashtable_search (provided_hash,
        (void *) &service);
      thread_mutex_unlock (&provided_mutex);
      if (search_result != NULL) {
        if (process_alive (p->client)) {
          /* setup heartbeat */
          sprintf (b, "%08X", p->id);
          sprintf (b+8, "%08X", p->sequence_number);
          ++p->sequence_number;
          gettimeofday (&tv, NULL);
          sprintf (b+16, "%08X",(unsigned int)tv.tv_sec);
          sprintf (b+24, "%08X",(unsigned int)tv.tv_usec);
          /* send it */
          addr.sin_addr.s_addr = p->address;
          sendto (s, b, sizeof (b), 0, (struct sockaddr*)
            &addr, sizeof (struct sockaddr));
          next_sec = timespec_to_seconds (&p->next) + p->eta;
          seconds_to_timespec (next_sec, &p->next);
          heap_insert (beating, (void *) p);
        } else {
          thread_mutex_lock (&provided_mutex);
          hashtable_remove (provided_hash,
            (void *) &service);
          thread_mutex_unlock (&provided_mutex);
          hashtable_remove (beating_hash, (void *) p);
          delete p;
        }
      }
    } else {
      delete p;
    }
  }
}


