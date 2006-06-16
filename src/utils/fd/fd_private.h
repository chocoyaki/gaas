
	/*******************************************\
	 * Failure Detector : private include file *
	 * Aurélien Bouteiller                     *
	\*******************************************/


#ifndef __FD_PRIVATE_H_
#define __FD_PRIVATE_H_

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include "fd.h"

#define fd_TCP_port 4000	/* for TCP communications */
#define fd_UDP_port 4001	/* for UDP communications */

/* prints out a human-readable message and exits with failure status */
#define fatal_error(s) (perror(s),exit(-errno))

/* init fd lib */
void fd_init(void);
/* notify client that h has changed state (is suspect, is not any more suspect) */
void fd_notify(fd_handle h);


#endif /* __FD_PRIVATE_H_ */
