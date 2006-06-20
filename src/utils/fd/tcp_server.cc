/****************************************************************************/
/* DIET Fault Detector Reconfigure heartbeat sender handler                 */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Aurélien Bouteiller (aurelien.bouteiller@inria.fr)                  */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$ 
 * $Log$
 * Revision 1.3  2006/06/20 13:29:16  abouteil
 *
 *
 * Chandra&Toueg&Aguilera fault detector implementation.
 *
 * A
 *
 ****************************************************************************/

#include <sys/time.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "threadoverlay.h"


#include "fdd.h"

void* process_tcp_query (void* psock)
{
  int acc_sock = *((int*) psock);
  int nbread;      /* nb of bytes read */
  char buffer[1084];    /* communication buffer */
  char machine_name[1024];
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
  nbread = recv (acc_sock, buffer, sizeof (buffer), 0);

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
      p = (beating_process) malloc (sizeof (beating_process));
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
      q = (beating_process) malloc (sizeof (beating_process));
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
  free (psock);
  
  return NULL;  
}

void* setup_tcp_server (void *nothing) {
  /* server */
  int server_sock;    /* socket for accepting connexions */
  struct sockaddr_in my_addr;  /* address of this computer */

  /* client */
  int* acc_sock;      /* socket returned by accept () */  
  struct sockaddr_in acc_addr;  /* address of remote client */
  socklen_t acc_len;      /* length of remote address */

  /* thread */
  thread_t pth;

  /* socket creation */
  server_sock = socket (PF_INET, SOCK_STREAM, 0);
  if (server_sock == -1) {
    fatal_error ("socket");
  };

  /* set up my address */
  if (memset (&my_addr, 0, sizeof (struct sockaddr_in)) != &my_addr) {
    fatal_error ("memset");
  };
  my_addr.sin_family = PF_INET;
  my_addr.sin_port = htons(fd_TCP_port);
  my_addr.sin_addr.s_addr = INADDR_ANY;
  
  /* bind the socket */
  if (bind (server_sock, (struct sockaddr *) &my_addr,
    sizeof (struct sockaddr)) == -1) {
    fatal_error ("TCP bind");
  };

  /* get ready to accept */
  if (listen (server_sock, 128) == -1) {
    fatal_error ("listen");
  };

  while (1) {
    acc_sock = (int*) malloc (sizeof (int));
    
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

