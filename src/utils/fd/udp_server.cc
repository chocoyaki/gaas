/****************************************************************************/
/* DIET Fault Detector UDP Heartbeat Watcher                                */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Aurélien Bouteiller (aurelien.bouteiller@inria.fr)                  */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$ 
 * $Log$
 * Revision 1.7  2010/10/05 03:16:40  bdepardo
 * Use hostname and buffer sizes constants.
 * Loop until an available port is found.
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
#include <sys/msg.h>

#include <cstdio>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <cmath>
#include "threadoverlay.h"


#include "fdd.h"
#include "formulae.h"


void* setup_udp_server (void *nothing) {
  int server_sock;    /* socket for accepting connexions */
  int optval = 1;     /* for REUSE_ADDR */
  struct sockaddr_in my_addr;  /* address of this computer */
  char buffer[1+HEARTBEAT_LENGTH]; /* buffer for network communication */
  struct sockaddr_in client_addr;  /* client's address */
  socklen_t client_address_length; /* client's address length */
  int received;      /* number of bytes received */

  struct timeval tv;
  double ea_lp1;
  struct _watched_process ps;  /* to perform the hashtable search */
  watched_process p;    /* process corresponding to heartbeat */
  int seq_num;      /* heartbeat sequence number */
  unsigned int sender_sec, sender_usec;  /* heartbeat sender time */
  double pl, vd, alpha, eta;
  int i;

  /* for HTTP reconfigure request */
  char h_buffer[64];
  unsigned int h_id;
  int h_request_len;
  int h_sock;
  struct sockaddr_in http_addr;

  int ppp = 1;

  /* socket creation */
  server_sock = socket (PF_INET, SOCK_DGRAM, 0);
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
  my_addr.sin_port = htons(fd_UDP_port);
  my_addr.sin_addr.s_addr = htonl (INADDR_ANY);
  
  /* bind the socket */
  while (bind (server_sock, (struct sockaddr *) &my_addr,
    sizeof (struct sockaddr)) == -1) {
    my_addr.sin_port = htons(fd_UDP_port + ppp);
    ++ ppp;
  }
  // if (bind (server_sock, (struct sockaddr *) &my_addr,
  //   sizeof (struct sockaddr)) == -1) {
  //   fatal_error ("UDP bind");
  // };

  while (1) {
    client_address_length = sizeof (struct sockaddr);
    received = recvfrom (server_sock, &buffer, sizeof (buffer), 0,
        (struct sockaddr *) &client_addr,
        &client_address_length);

    /* we have just received a heartbeat */

    gettimeofday (&tv, 0);

    buffer[32] = '\0';
    sscanf (buffer+24, "%X", &sender_usec);
    buffer[24] = '\0';
    sscanf (buffer+16, "%X", &sender_sec);
    buffer[16] = '\0';
    sscanf (buffer+8, "%X", &seq_num);
    buffer[8] = '\0';
    sscanf (buffer, "%X", &ps.id);
    
    thread_mutex_lock (&watched_mutex);

    p = (watched_process) hashtable_search (watched_hash,
      (void *) &ps);
    if (p != NULL) {
      /* !! if client is dead, we should stop observing for it */
      
      if ((p->l >= 0) && (seq_num % SAMPLE_SIZE < p->l % SAMPLE_SIZE)) {
        /* a sample is complete, we may need to reconfigure */
        pl = estimate_pl (p->pv);
        vd = estimate_vd (p->pv);
        if (configure_nfde (pl, vd, p->tdu, p->tmrl, p->tmu,
          &alpha, &eta)) {
          /* criterion to reconfigure : eta has varied
           * by at least 10%
           */
          if (fabs (eta - p->eta) > (p->eta / 10)) {
            /* reset EA statistics */
            for (i = 0; i < SAMPLE_SIZE; ++i) {
              p->st[i] = 0.0;
            }
            
            /* new ID => needs reinsertion in hash */
            hashtable_remove (watched_hash, p);
            
            /* new values */
            p->alpha = alpha;
            p->eta = eta;
            p->l = -1;
            seconds_to_timespec (timeval_to_seconds (&tv)
              + eta + alpha, &p->next_tau_lp1);
            h_id = p->id;
            p->id = next_watched_id;
            ++next_watched_id;

            hashtable_insert (watched_hash, p);
            /* no need to reinsert in heap since it
             * is insensitive to the ID
             */

            p->handle->log->logDetectorParams(p->handle->machine_name, 
                                              pl, vd, eta, alpha);


            /* send HTTP message to peer */
            h_request_len = sprintf (h_buffer,
              "RECONFIGURE %X %X %f",
              h_id, p->id, eta);
            
            
            h_sock = socket (PF_INET, SOCK_STREAM, 0);
            if (h_sock == -1) {
              fatal_error ("socket");
            };
  
            /* set up address */
            if (memset (&http_addr, 0, sizeof
              (struct sockaddr_in)) != &http_addr) {
              fatal_error ("memset");
            };
            http_addr.sin_family = PF_INET;
            http_addr.sin_port = htons(fd_TCP_port);
            http_addr.sin_addr.s_addr = p->addr;
            if (connect (h_sock, (struct sockaddr *)&http_addr,
              sizeof (http_addr)) != -1) {
              /* send request */
	      printf("## Sending a new request \n");
              write (h_sock, h_buffer, h_request_len+1);
            }
            close (h_sock);
          } else {
          }
        }
        /* reset p_{L} and V(D) statistics */
        for (i = 0; i < SAMPLE_SIZE; ++i) {
          p->pv[i] = 0.0;
        }
      } else {
        p->st[seq_num % SAMPLE_SIZE] = timeval_to_seconds (&tv)
          - (p->eta * (double) seq_num);
        p->pv[seq_num % SAMPLE_SIZE] = timeval_to_seconds (&tv)
          - sender_sec - ((double) sender_usec / (double) 1000000);

      }
      /* if we got a newer heartbeat than we had AND we didn't reconfigure */
      if ((ps.id == p->id) && (seq_num > p->l)) {
        p->l = seq_num;
        
        /* calculate \tau_{l+1} = EA (l+1) + \alpha */
        ea_lp1 = avg63 (p->st) +
               (((double) (seq_num+1)) * p->eta);
        seconds_to_timespec (ea_lp1 + p->alpha, &p->next_tau_lp1);

        if ((p->suspect) &&
          (ea_lp1 + p->alpha > timeval_to_seconds (&tv))) {
          p->suspect = 0;
          p->tau_lp1.tv_sec = p->next_tau_lp1.tv_sec;
          p->tau_lp1.tv_nsec = p->next_tau_lp1.tv_nsec;
          heap_insert (watched, p);
          if (heap_peek (watched) == p) {
            thread_cond_signal (&new_watched_cond);
          }
          /* a process is not suspected
           * anymore => signal it to the library
           */
          fd_notify(p->handle);
        }
      }
    }
    thread_mutex_unlock (&watched_mutex);
  };
  return NULL;
}


