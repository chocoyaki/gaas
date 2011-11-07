/**
 * @file  DIET_compat.cc
 *
 * @brief  DIET compatibility file
 *
 * @author  Haikel Guemard (haikel.guemard@sysfera.com)
 *
 * @section Licence
 *   |LICENCE|
 */


#include "DIET_compat.hh"
#include <time.h>

namespace diet {
int
usleep(unsigned int useconds) {
  struct timespec req = {0, 1000 * useconds};
  struct timespec rem = {0, 0};

  return nanosleep(&req, &rem);
}
}
