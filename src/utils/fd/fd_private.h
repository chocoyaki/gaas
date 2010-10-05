/****************************************************************************/
/* DIET Fault Detector private header                                       */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Aurélien Bouteiller (aurelien.bouteiller@inria.fr)                  */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$ 
 * $Log$
 * Revision 1.5  2010/10/05 03:32:29  bdepardo
 * C++ headers
 *
 * Revision 1.4  2010/10/05 03:14:55  bdepardo
 * Define constants for hostname and buffer sizes
 *
 * Revision 1.3  2006/06/20 13:29:16  abouteil
 *
 *
 * Chandra&Toueg&Aguilera fault detector implementation.
 *
 * A
 *
 ****************************************************************************/

#ifndef __FD_PRIVATE_H_
#define __FD_PRIVATE_H_

#include <cstdlib>
#include <unistd.h>
#include <cerrno>

#include "fd.h"

#define fd_TCP_port 4000	/* for TCP communications */
#define fd_UDP_port 4001	/* for UDP communications */

const unsigned int MAX_HOSTNAME_SIZE = 1024;
const unsigned int MAX_BUFFER_SIZE = 1084;

/* prints out a human-readable message and exits with failure status */
#define fatal_error(s) (perror(s),exit(-errno))

/* init fd lib */
void fd_init(void);
/* notify client that h has changed state (is suspect, is not any more suspect) */
void fd_notify(fd_handle h);


#endif /* __FD_PRIVATE_H_ */
