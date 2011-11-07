/**
 * @file  DIET_compat.hh
 *
 * @brief  DIET compatibility header
 *
 * @author  Haikel Guemard (haikel.guemard@sysfera.com)
 *
 * @section Licence
 *   |LICENCE|
 */


#ifndef _DIET_COMPAT_HH_
#define _DIET_COMPAT_HH_



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
