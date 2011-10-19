/****************************************************************************/
/* DIET compatibility header                                                */
/*                                                                          */
/*  Author(s):                                                              */
/*   - Haikel Guemar <haikel.guemar@sysfera.com>                            */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.2  2011/05/10 13:14:58  bdepardo
 * Added missing header
 *
 ****************************************************************************/

#ifndef _DIET_COMPAT_HH_
#define _DIET_COMPAT_HH_

/**
 * @file   DIET_compat.hh
 * @author hguemar <hguemar@sysfera.com>
 * @date   Thu Mar 17 09:08:56 2011
 *
 * @brief  unix compatibility header (this is private API!)
 *
 */

namespace diet {
/**
 * @brief nanonsleep-based implementation of usleep
 * (usleep is deprecated by Posix)
 * @param useconds sleep time (µs)
 * @return 0 (success) or -1 (errno)
 */
int
usleep(unsigned int useconds);
}

#endif /* _DIET_COMPAT_HH_ */
