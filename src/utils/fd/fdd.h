/****************************************************************************/
/* DIET Fault Detector internal shared headers                              */
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
#ifndef _FDD_H_
#define _FDD_H_


#include <sys/types.h>
#include <sys/msg.h>

#include <netdb.h>
#include "threadoverlay.h"


#include "heap.h"
#include "hashtable.h"


#include "fd_private.h"	/* failure detector header */



/* global variables */
extern pid_t daemon_pid;
extern key_t queue_key;	/* queue key (for library <-> daemon communication) */
extern int queue_id;		/* queue identifier */


extern unsigned int next_watched_id;	/* to give unique IDs */
extern heap watched;			/* watched applications (we accept the heartbeats) */
extern hashtable watched_hash;
extern thread_mutex_t watched_mutex;
extern thread_cond_t new_watched_cond;

extern thread_mutex_t provided_mutex;
extern hashtable provided_hash;		/* all the services we provide */

extern heap beating;		/* beating applications (we send the heartbeats) */
extern hashtable beating_hash;
extern thread_mutex_t beating_mutex;
extern thread_cond_t new_beating_cond;

/* length of a heartbeat */
#define HEARTBEAT_LENGTH 32	/* value = (text, hex)
				 * ID, sequence #,
				 * sender local time (seconds, microseconds)
				 */

/* datatype for a watched process */
#define SAMPLE_SIZE 32
typedef struct _watched_process {
	unsigned int id;	/* unique, used instead of remote address */
	in_addr_t addr;		/* ...which we need anyway */
	int service_number;	/* what service is watched ? */
	int l;			/* greatest sequence # received (see paper) */
	struct timespec tau_lp1;/* time when the process will be suspect */
	struct timespec next_tau_lp1;	/* next tau_lp1 */
	double tmu;
	double tmrl;
	double tdu;
	double alpha;		/* alpha as in the paper (seconds) */
	double eta;		/* eta as in the paper (seconds) */
	int suspect;		/* is it suspect ? */
	pid_t client;		/* who is watching ? */
	fd_handle handle;	/* its handle, we pass it back when needed */
	double st[SAMPLE_SIZE];	/* to estimate EA */
				/* (st field : A-\eta.si in seconds) */
	double pv[SAMPLE_SIZE]; /* to estimate p_{L} and V(D) */
				/* (pv field : A-S in seconds) */
} _watched_process;
typedef struct _watched_process* watched_process;

/* compares watches processes */
int compare_watched_processes (void *, void *);

/* hashing function for watched processes */
unsigned int hash_watched_process (void *);

/* determines if watched processes match */
unsigned int match_watched_processes (void *, void *);

/* datatype for a provided service */
typedef struct _provided_service {
	int service_number;	/* what service is provided ? */
	pid_t client;		/* who must stay alive ? */
} _provided_service;
typedef struct _provided_service* provided_service;

/* hashing function for watched processes */
unsigned int hash_provided_service (void *);

/* determines if provided services match */
unsigned int match_provided_services (void *, void *);

/* datatype for a beating process (we send the heartbeats) */
typedef struct _beating_process {
	int id;			/* passed by sender */
	int service_number;	/* what service is provided ? */
	struct timespec next;	/* time when we will send the next heartbeat */
	double eta;		/* interval between heartbeats (seconds) */
	in_addr_t address;	/* remote machine IP address */
	int sequence_number;	/* last sequence number we sent */
	pid_t client;		/* who must stay alive ? */
	int valid;
} _beating_process;
typedef struct _beating_process* beating_process;

/* hashing function for watched processes */
unsigned int hash_beating_process (void *);

/* compares beating processes */
int compare_beating_processes (void *, void *);

/* determines if beating processes match */
unsigned int match_beating_processes (void *, void *);

/* logs an error message */
void log_err (const char *);

/* logs an informational message */
void log_info (const char *);

/* checks whether a process is alive */
int process_alive (pid_t);

#endif /* _FDD_H_ */

