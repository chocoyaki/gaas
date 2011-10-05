/****************************************************************************/
/* DIET compatibility file                                                  */
/*                                                                          */
/*  Author(s):                                                              */
/*   - Haikel Guemard (haikel.guemard@sysfera.com)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.2  2011/05/10 13:14:58  bdepardo
 * Added missing header
 *
 ****************************************************************************/

#include "DIET_compat.hh"
#include <time.h>

namespace diet {
  int usleep(unsigned int useconds) {
    struct timespec req = {0, 1000 * useconds};
    struct timespec rem = {0, 0};

    return nanosleep(&req, &rem);
  }
}
