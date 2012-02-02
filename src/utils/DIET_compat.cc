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



#include <time.h>
#ifdef __WIN32__
#include <Winsock2.h>
#endif
#include "DIET_compat.hh"


namespace diet {
#ifdef __WIN32__
int
usleep(unsigned int useconds) {
	Sleep(useconds/1000);
	return 0;
}
#else
int
usleep(unsigned int useconds) {
  struct timespec req = {0, 1000 * useconds};
  struct timespec rem = {0, 0};

  return nanosleep(&req, &rem);
}
#endif
}
