/****************************************************************************/
/* DIET Fault Detector Client API                                           */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Aurélien Bouteiller (aurelien.bouteiller@inria.fr)                  */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$ 
 * $Log$
 * Revision 1.13  2011/01/23 19:20:02  bdepardo
 * Fixed memory and resources leaks, variables scopes, unread variables
 *
 * Revision 1.12  2010/12/17 09:48:01  kcoulomb
 * * Set diet to use the new log with forwarders
 * * Fix a CoRI problem
 * * Add library version remove DTM flag from ccmake because deprecated
 *
 * Revision 1.11  2010/10/05 03:24:14  bdepardo
 * Constant for buffer size
 *
 * Revision 1.10  2010/10/05 03:17:13  bdepardo
 * Send correct adress to observed process.
 * Use constants for hostname size.
 *
 * Revision 1.9  2010/10/04 09:56:03  bdepardo
 * Machine name was not initialized in fd_observe: directly use
 * handle->machine_name
 *
 * Revision 1.8  2010/10/04 08:17:23  bdepardo
 * Changed memory management from C to C++ (malloc/free -> new/delete)
 * This corrects a bug at initialization
 *
 * Revision 1.7  2010/03/31 21:15:40  bdepardo
 * Changed C headers into C++ headers
 *
 * Revision 1.6  2010/03/03 17:49:18  glemahec
 * Little correction for cygwin compatibility
 *
 * Revision 1.5  2010/01/24 22:21:06  bdepardo
 * Definition of _NSIG under Darwin.
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
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <csignal>
#include <sys/socket.h>
#include <sys/time.h>
#include "threadoverlay.h"

#include "fd_private.h"
#include "fdd.h"
#include "formulae.h"

/* _NGIS under Intel NGIS under PPC */
#if (defined(__ppc__) || defined(__ppc64__))
#define _NSIG  NSIG
#endif

/* NSIG under Mac OS X */
#if (not defined(_NSIG) && defined(__darwin__))
#define _NSIG  NSIG
#endif
 
/* NSIG under Cygwin */
#if (not defined(_NSIG) && defined(__cygwin__))
#define _NSIG  NSIG
#endif

      /****************\
       * private data *
      \****************/

/* having a global variable implies that the library is not thread safe, but
 * what would it mean for several concurrent user threads to call fd_warn?
 */

/* the signal specified via fd_warn, or _NSIG if none */
int warn_signal = _NSIG;

/* the transition handler */
void (*transition_handler) (fd_handle) = NULL;

/* the failure handler */
void (*failure_handler) (fd_handle) = NULL;

      /**********************\
       * Internal functions *
      \**********************/

/* notify client that h has changed state (is suspect, is not any more suspect) */
void fd_notify(fd_handle h)
{
#ifdef USE_LOG_SERVICE
  if(h->log) h->log->logFailure(h->machine_name);
#endif  
  if (transition_handler != NULL) {
    transition_handler (h);
  }
  if (warn_signal != _NSIG) {
    kill (getpid(), warn_signal);
  }
}

void __attribute__ ((constructor)) init_library (void)
{
  fd_init();
}

void __attribute__ ((destructor)) fini_library (void)
{

}

      /*******************\
       * the library API *
      \*******************/

/* makes a new handle with all data set to default values */
fd_handle fd_make_handle (void)
{
  fd_handle result = new _fd_handle;
  result->suspect = 1;  /* watched processes are initially suspected */
  result->tdu = 0;
  result->tmrl = 0;
  result->tmu = 0;
#ifdef USE_LOG_SERVICE
  result->log = NULL;
#endif
  return result;
}

/* gets the QoS characteristics of a handle */
void fd_get_qos (fd_handle handle, double *tdu, double *tmrl, double *tmu)
{
  *tdu = handle->tdu;
  *tmrl = handle->tmrl;
  *tmu = handle->tmu;
}

/* sets the QoS characteristics of a handle */
void fd_set_qos (fd_handle handle, double tdu, double tmrl, double tmu)
{
  handle->tdu = tdu;
  handle->tmrl = tmrl;
  handle->tmu = tmu;
}

/* gets the machine name and service number of a handle */
void fd_get_service (fd_handle handle, char* machine, int machine_len,
  int* service_number)
{
  strncpy (machine, handle->machine_name, machine_len);
  *service_number = handle->service_number;
}

int fd_set_service (fd_handle handle, const char* machine, int service_number)
{
  struct hostent *ent;

  ent = gethostbyname (machine);
  
  if (ent != NULL) {
    memcpy (&(handle->address), ent->h_addr_list[0], ent->h_length);
    handle->machine_name = strdup (machine);
    handle->service_number = service_number;
    return 1;
  } else {
    return 0;
  }
}
#ifdef USE_LOG_SERVICE

void fd_set_logger(fd_handle handle, DietLogComponent *l)
{
  handle->log = l;
}

void fd_get_logger(fd_handle handle, DietLogComponent **l)
{
  *l = handle->log;
}
#endif
/* frees a handle */
void fd_free_handle (fd_handle handle)
{
  delete handle;
}

/* observation */
int fd_observe (fd_handle handle)
{
  watched_process p;
  struct timeval tv;
  char buffer[MAX_BUFFER_SIZE];
  int sock;        /* socket for HTTP connection */
  struct sockaddr_in addr;    /* address of watched machine */
  double alpha, eta;
  int i;
  double tau_sec;

  if (!configure_nfde (0.01, 0.02, handle->tdu, handle->tmrl, handle->tmu, &alpha, &eta)) {
    /* !! QoS impossible */
    return -1;
  } 

  /* send a TCP request to the server
   * if reply is OK then set up a new
   * watched process
   */
  sock = socket (PF_INET, SOCK_STREAM, 0);
  if (sock == -1) {
    fatal_error ("socket");
  }
  /* set up address */
  if (memset (&addr, 0, sizeof(struct sockaddr_in)) != &addr) {
    fatal_error ("memset");
  }
  addr.sin_family = PF_INET;
  addr.sin_port = htons(fd_TCP_port);
  addr.sin_addr.s_addr = handle->address;
  
  p = new _watched_process;
  p->id = next_watched_id;
  ++next_watched_id;
  p->service_number = handle->service_number;
  p->l = -1;
  gettimeofday (&tv, NULL);
  tau_sec = timeval_to_seconds (&tv) + eta + alpha;
  seconds_to_timespec (tau_sec, &p->tau_lp1);
  p->tmu = handle->tmu;
  p->tmrl = handle->tmrl;
  p->tdu = handle->tdu;
  p->alpha = alpha;
  p->eta = eta;
  p->suspect = 1;
  p->client = getpid();
  p->handle = handle;
  p->addr = handle->address;
  for (i = 0; i < SAMPLE_SIZE; ++i) {
    p->st[i] = 0.0;
    p->pv[i] = 0.0;
  }
  thread_mutex_lock (&watched_mutex);
  hashtable_insert (watched_hash, p);
  thread_mutex_unlock (&watched_mutex);
  /* connection */
  if (connect (sock, (struct sockaddr *) &addr, sizeof (addr)) != -1) {
    char machine_name[MAX_HOSTNAME_SIZE];
    gethostname(machine_name, MAX_HOSTNAME_SIZE);
    /* send request */
    int request_len = snprintf (buffer, sizeof (buffer), "OBSERVE %s %X %X %.9f", machine_name, p->id, p->service_number, eta);
    write (sock, buffer, request_len+1);
    /* force system to send data now */
    shutdown (sock, SHUT_WR);
    /* read in the answer */
    int nb_read = read (sock, buffer, sizeof (buffer));
    if ((nb_read == -1) || (strcmp (buffer,"OK") != 0)) {
      /* something went wrong */
      thread_mutex_lock (&watched_mutex);
      hashtable_remove (watched_hash, p);
      thread_mutex_unlock (&watched_mutex);
      delete p;
      close(sock);
      return -1;
    }
    /* disconnection */
    close (sock);
    return 0;
  } 
  /* connection failed */
  delete p;
  return -1;
}

/* liveliness checking */
int fd_alive (fd_handle handle)
{
  return !handle->suspect;
}

/* warning specification */
void fd_warn (int signal)
{
  warn_signal = signal;
}

/* transition handler setting */
void fd_set_transition_handler (void (*f) (fd_handle))
{
  transition_handler = f;
}

/* failure handler setting */
void fd_set_failure_handler (void (*f) (fd_handle))
{
  failure_handler = f;
}
