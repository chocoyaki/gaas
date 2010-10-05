/****************************************************************************/
/* DIET Fault Detector Heartbeat Management Background Thread               */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Aurélien Bouteiller (aurelien.bouteiller@inria.fr)                  */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$ 
 * $Log$
 * Revision 1.7  2010/10/05 02:39:41  bdepardo
 * Initialize global variables in fd_init() (called in init_library())
 *
 * Revision 1.6  2010/10/04 08:17:23  bdepardo
 * Changed memory management from C to C++ (malloc/free -> new/delete)
 * This corrects a bug at initialization
 *
 * Revision 1.5  2010/03/31 21:15:40  bdepardo
 * Changed C headers into C++ headers
 *
 * Revision 1.4  2006/11/07 00:37:44  ecaron
 * Bug fix for MacOSX support
 *
 * Revision 1.3  2006/06/20 13:29:16  abouteil
 *
 *
 * Chandra&Toueg&Aguilera fault detector implementation.
 *
 * A
 *
 ****************************************************************************/
 
#include <sys/stat.h>
#include <sys/msg.h>

#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <csignal>
#include <cerrno>
#include "threadoverlay.h"

#include "fdd.h"
#include "formulae.h"
#include "udp_server.h"
#include "tcp_server.h"
#include "heartbeat_sender.h"
#include "application_watcher.h"

/* _NGIS under Intel NGIS under PPC */
#if (defined(__ppc__) || defined(__ppc64__))
#define _NSIG  NSIG
#endif

unsigned int next_watched_id = 0;  /* to give unique IDs */
heap watched;    /* watched applications (we accept the heartbeats) */
hashtable watched_hash;
thread_mutex_t watched_mutex = NULL;
thread_cond_t new_watched_cond = NULL;

thread_mutex_t provided_mutex = NULL;
hashtable provided_hash;  /* all the services we provide */

heap beating;    /* beating applications (we send the heartbeats) */
hashtable beating_hash;
thread_mutex_t beating_mutex = NULL;
thread_cond_t new_beating_cond = NULL;


/* compares watches processes */
int compare_watched_processes (void *a, void *b)
{
  struct timespec ta, tb;
  int d;
  ta = ((watched_process) a)->tau_lp1;
  tb = ((watched_process) b)->tau_lp1;

  d = ta.tv_sec - tb.tv_sec;
  if (d == 0) d = ta.tv_nsec - tb.tv_nsec;
  return d;
}

/* hashing function for watched processes */
unsigned int hash_watched_process (void *p)
{
  return ((watched_process) p)->id;
}

/* determines if watched processes match */
unsigned int match_watched_processes (void *a, void *b)
{
  watched_process wa, wb;
  wa = (watched_process) a;
  wb = (watched_process) b;
  return (wa->id == wb->id);
}

/* hashing function for watched processes */
unsigned int hash_provided_service (void *p)
{
  return ((provided_service) p)->service_number;
}

/* determines if provided services match */
unsigned int match_provided_services (void *a, void *b)
{
  provided_service pa, pb;
  pa = (provided_service) a;
  pb = (provided_service) b;
  return (pa->service_number == pb->service_number);
}

/* hashing function for beating processes */
unsigned int hash_beating_process (void *p)
{
  return ((beating_process) p)->id;
}

/* compares beating processes */
int compare_beating_processes (void *a, void *b)
{
  struct timespec ta, tb;
  int d;
  ta = ((beating_process) a)->next;
  tb = ((beating_process) b)->next;

  d = ta.tv_sec - tb.tv_sec;
  if (d == 0) d = ta.tv_nsec - tb.tv_nsec;
  return d;
}

/* determines if beating processes match */
unsigned int match_beating_processes (void *a, void *b)
{
  beating_process ba, bb;
  ba = (beating_process) a;
  bb = (beating_process) b;
  return (ba->id == bb->id);
}

/* checks whether a process is alive
 * on Linux and BSD, this is true if a file named after the PID
 * exists in the /proc directory
 */
int process_alive (pid_t pid)
{
  char str[32];
  struct stat s;

  sprintf (str, "/proc/%d", pid);
  return (stat (str, &s) == 0);  /* stat succeeds <=> file exists */
}

/* auxiliary function to launch a new thread */
int launch_thread (void *(*f)(void *)) {
  thread_t pth;
  if (thread_create (&pth, NULL, f, NULL) != 0) {
    return 0;
  } else {
    return 1;
  }
}

/* initialisation */
void fd_init(void) {
  /* Init all global variables */
  watched_mutex = THREAD_MUTEX_INITIALIZER;
  new_watched_cond = THREAD_COND_INITIALIZER;
  provided_mutex = THREAD_MUTEX_INITIALIZER;
  beating_mutex = THREAD_MUTEX_INITIALIZER;
  new_beating_cond = THREAD_COND_INITIALIZER;


  watched = heap_new (compare_watched_processes);
  watched_hash = hashtable_new (hash_watched_process,
    match_watched_processes);

  beating = heap_new (compare_beating_processes);
  beating_hash = hashtable_new (hash_beating_process,
    match_beating_processes);

  provided_hash = hashtable_new (hash_provided_service,
    match_provided_services);
  
  /* launches the UDP server */
  if (! launch_thread (setup_udp_server))
    fatal_error ("Could not start UDP server");
  /* launches the TCP server */
  if (! launch_thread (setup_tcp_server))
    fatal_error ("Could not start TCP server");             
  /* launches the heartbeat sender */
  if (! launch_thread (setup_heartbeat_sender))
    fatal_error ("Could not start UDP Heartbeat sender");
  /* launches the applications watcher */
  if (! launch_thread (setup_application_watcher))
    fatal_error ("Could not start Application watcher");
}

/* service registration */
void fd_register_service (pid_t client, int service_number)
{
  provided_service s, sp;

  s = new _provided_service;
  s->service_number = service_number;
  s->client = client;
  /* check the service is not already provided */
  thread_mutex_lock (&provided_mutex);
  sp = (provided_service) hashtable_search (provided_hash, s);
  if (sp == NULL) {
    hashtable_insert (provided_hash, s);
  } else {
    delete s;
  }
  thread_mutex_unlock (&provided_mutex);
}

/* service unregistration */
void fd_unregister_service (pid_t client, int service_number)
{
  provided_service s, sp;

  s = new _provided_service;
  s->service_number = service_number;
  thread_mutex_lock (&provided_mutex);
  sp = (provided_service) hashtable_search (provided_hash, s);
  if ((sp != NULL) && (sp->client == client)) {
    hashtable_remove (provided_hash, sp);
    delete sp;
  }
  thread_mutex_unlock (&provided_mutex);
  delete s;
}
