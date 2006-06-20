/****************************************************************************/
/* DIET Fault Detector Client API headers                                   */
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
#ifndef __FD_H__
#define __FD_H__

#include <unistd.h>
#include <netdb.h>

#include "DietLogComponent.hh"

/* type of handles - should be condidered opaque */
typedef struct _fd_handle {
	int suspect;	/* is the watched application suspect ? */
	double tdu;	/* QoS : upper bound on detection time */
	double tmrl;	/* QoS : lower bound on avg mistake recurrence time */
	double tmu;	/* QoS : upper bound on avg mistake duration */
	char* machine_name;	/* observed machine name */
	in_addr_t address;	/* observed machine IP address */
	int service_number;	/* observed service */
        DietLogComponent *log;  /* Diet centralized log service to use */
} _fd_handle;
typedef struct _fd_handle* fd_handle;

      /**************************\
       * the CLIENT library API *
      \**************************/

/* makes a new handle with all data set to default values */
fd_handle fd_make_handle (void);

/* gets the QoS characteristics of a handle */
void fd_get_qos (fd_handle handle, double *, double *, double *);

/* sets the QoS characteristics of a handle */
void fd_set_qos (fd_handle handle, double, double, double);

/* gets the machine name and service number of a handle */
void fd_get_service (fd_handle handle, char*, int, int*);

/* sets the machine name and service number of a handle */
int fd_set_service (fd_handle handle, const char*, int);

/* sets the diet logger of a handle */
void fd_set_logger(fd_handle handle, DietLogComponent *l);

/* gets the current diet logger used by a handle */
void fd_get_logger(fd_handle handle, DietLogComponent **l);

/* frees a handle */
void fd_free_handle (fd_handle handle);

/* observation */
int fd_observe (fd_handle handle);

/* liveliness checking */
int fd_alive (fd_handle handle);

/* warning specification */
void fd_warn (int signal);

/* transition handler setting */
void fd_set_transition_handler (void (*f) (fd_handle));

      /***************************\
       * the SERVICE library API *
      \***************************/
      
/* service registration */
void fd_register_service (pid_t client, int service_number);

/* service unregistration */
void fd_unregister_service (pid_t client, int service_number);

#endif /* __FD_H_ */
