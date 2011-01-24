/****************************************************************************/
/* DIET Fault Detector Reconfigure heartbeat sender handler                 */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Aurélien Bouteiller (aurelien.bouteiller@inria.fr)                  */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$ 
 * $Log$
 * Revision 1.9  2011/01/24 23:27:38  bdepardo
 * Remove unused variables
 *
 * Revision 1.8  2010/10/05 03:16:40  bdepardo
 * Use hostname and buffer sizes constants.
 * Loop until an available port is found.
 *
 * Revision 1.7  2010/10/04 08:17:23  bdepardo
 * Changed memory management from C to C++ (malloc/free -> new/delete)
 * This corrects a bug at initialization
 *
 * Revision 1.6  2010/03/31 21:15:40  bdepardo
 * Changed C headers into C++ headers
 *
 * Revision 1.5  2007/03/30 15:48:34  dart
 * - Add <sys/socket.h> to compile under AIX with XLC v8.0
 * - Change h_addr to http_addr in udp_server.cc to avoid the error message
 *   "The array bound cannot be zero" (visibly, h_addr already exists in
 *   /usr/include/isode/internet.h", is it the real reason ?)
 *
 * Revision 1.4  2006/11/09 21:11:13  abouteil
 * fixed "addr already in use" when starting on the same server more than once
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
#include <sys/socket.h>

#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include "threadoverlay.h"


#include "fdd.h"

void* process_tcp_query (void* psock)
{
  int acc_sock = *((int*) psock);
  char buffer[MAX_BUFFER_SIZE];    /* communication buffer */
  char machine_name[MAX_HOSTNAME_SIZE];
  struct hostent *ent;

  struct _provided_service serv;  
  provided_service s;
  beating_process p, q;
  struct timeval tv;
  int cn_id, cn_service_number;
  double cn_eta;

  int reconf_old_id, reconf_new_id;
  double reconf_eta;
  struct _beating_process sp;

  /* read the incoming data */
  recv (acc_sock, buffer, sizeof (buffer), 0);

  /* process the query */
  if (sscanf (buffer, "OBSERVE %s %X %X %lf", machine_name, &cn_id, &cn_service_number, &cn_eta) == 4) {
    ent = gethostbyname (machine_name);
    /* someone wants to receive heartbeats */

    /* check if service is provided */
    serv.service_number = cn_service_number;

    thread_mutex_lock (&provided_mutex);
    s = (provided_service) hashtable_search (provided_hash, &serv);
    thread_mutex_unlock (&provided_mutex);

    if (s != NULL) {
      /* create a new beating process */
      p = new _beating_process;
      p->id = cn_id;
      p->service_number = cn_service_number;
      p->eta = cn_eta;
      memcpy (&(p->address), ent->h_addr_list[0], ent->h_length);

      p->sequence_number = 0;
      p->client = s->client;
      
      gettimeofday (&tv, NULL);
      p->next.tv_sec = tv.tv_sec;
      p->next.tv_nsec = tv.tv_usec * 1000;

      p->valid = 1;
    
      /* register it */
      thread_mutex_lock (&beating_mutex);
      heap_insert (beating, p);
      hashtable_insert (beating_hash, p);
      thread_mutex_unlock (&beating_mutex);
      
      /* wake up the heartbeat sender */
      thread_cond_signal (&new_beating_cond);

      /* write back answer */
      write (acc_sock, "OK", 3);
    } else {
      write (acc_sock, "NO", 3);
    }
  } else if (sscanf (buffer, "RECONFIGURE %X %X %lf", &reconf_old_id, &reconf_new_id, &reconf_eta) == 3) {
    thread_mutex_lock (&beating_mutex);
    sp.id = reconf_old_id;
    p = (beating_process) hashtable_search (beating_hash, &sp);
    if (p != NULL) {
      hashtable_remove (beating_hash, p);
      p->valid = 0;
      q = new _beating_process;
      q->id = reconf_new_id;
      q->service_number = p->service_number;
      q->valid = 1;
      q->eta = reconf_eta;
      memcpy (&(q->address), &(p->address),sizeof (p->address));
      gettimeofday (&tv, NULL);
      q->next.tv_sec = tv.tv_sec;
      q->next.tv_nsec = tv.tv_usec * 1000;
      q->sequence_number = 0;
      q->client = p->client;
      hashtable_insert (beating_hash, q);
      heap_insert (beating, q);
    }
    thread_mutex_unlock (&beating_mutex);
    thread_cond_signal (&new_beating_cond);
  } else if (strcmp (buffer, "QUIT") == 0) {
    exit (EXIT_SUCCESS);
  } 
  /* close connexion */
  close (acc_sock);
  delete (int*) psock;
  
  return NULL;  
}

void* setup_tcp_server (void *nothing) {
  /* server */
  int server_sock;    /* socket for accepting connexions */
  int optval = 1;     /* for REUSE_ADDR */
  struct sockaddr_in my_addr;  /* address of this computer */

  /* client */
  int* acc_sock;      /* socket returned by accept () */  
  struct sockaddr_in acc_addr;  /* address of remote client */
  socklen_t acc_len;      /* length of remote address */

  /* thread */
  thread_t pth;

  int ppp = 1;

  /* socket creation */
  server_sock = socket (PF_INET, SOCK_STREAM, 0);
  if (server_sock == -1) {
    fatal_error ("socket");
  };

  if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int)) == -1) {
    fatal_error ("setsockopt(SO_REUSEADDR)");
  };
  
  /* set up my address */
  if (memset (&my_addr, 0, sizeof (struct sockaddr_in)) != &my_addr) {
    fatal_error ("memset");
  };
  my_addr.sin_family = PF_INET;
  my_addr.sin_port = htons(fd_TCP_port);
  my_addr.sin_addr.s_addr = INADDR_ANY;
  
  /* bind the socket */
  while (bind (server_sock, (struct sockaddr *) &my_addr,
    sizeof (struct sockaddr)) == -1) {
    my_addr.sin_port = htons(fd_TCP_port + ppp);
    ++ ppp;
  };
  // if (bind (server_sock, (struct sockaddr *) &my_addr,
  //   sizeof (struct sockaddr)) == -1) {
  //   fatal_error ("TCP bind");
  // };

  /* get ready to accept */
  if (listen (server_sock, 128) == -1) {
    fatal_error ("listen");
  };

  while (true) {
    acc_sock = new int;
    
    /* accept a new connexion */
    acc_len = sizeof (struct sockaddr_in);
    (*acc_sock) = accept (server_sock, (struct sockaddr *)
             &acc_addr,  &acc_len);

    if (thread_create (&pth, NULL, process_tcp_query,
      (void*) acc_sock) != 0) {
      return 0;
    } else {
      /* we don't want to join it */
      thread_detach (pth);
    }
    /* don't free or close acc_sock, done by process_tcp_query */
  };
  return NULL;
}

