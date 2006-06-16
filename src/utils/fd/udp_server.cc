  /**************************************************\
   * Failure Detector - UDP Server                  *
   * Recieve UDP heartbeats and store arrival date  *
   * Aurélien Bouteiller                            *
  \**************************************************/

#include <sys/time.h>
#include <sys/msg.h>

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include "threadoverlay.h"


#include "fdd.h"
#include "formulae.h"


void* setup_udp_server (void *nothing) {
  int server_sock;    /* socket for accepting connexions */
  struct sockaddr_in my_addr;  /* address of this computer */
  char buffer[1+HEARTBEAT_LENGTH];/* buffer for network communication */
  struct sockaddr_in client_addr;  /* client's address */
  socklen_t client_address_length;/* client's address length */
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
  struct sockaddr_in h_addr;

  /* socket creation */
  server_sock = socket (PF_INET, SOCK_DGRAM, 0);
  if (server_sock == -1) {
    fatal_error ("socket");
  };

  /* set up my address */
  if (memset (&my_addr, 0, sizeof (struct sockaddr_in)) != &my_addr) {
    fatal_error ("memset");
  };
  my_addr.sin_family = PF_INET;
  my_addr.sin_port = htons(fd_UDP_port);
  my_addr.sin_addr.s_addr = htonl (INADDR_ANY);
  
  /* bind the socket */
  if (bind (server_sock, (struct sockaddr *) &my_addr,
    sizeof (struct sockaddr)) == -1) {
    fatal_error ("UDP bind");
  };

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
            if (memset (&h_addr, 0, sizeof
              (struct sockaddr_in)) != &h_addr) {
              fatal_error ("memset");
            };
            h_addr.sin_family = PF_INET;
            h_addr.sin_port = htons(fd_TCP_port);
            h_addr.sin_addr.s_addr = p->addr;
            if (connect (h_sock, (struct sockaddr *)&h_addr,
              sizeof (h_addr)) != -1) {
              /* send request */
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


