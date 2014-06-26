/**
 * @file Global_macros.hh
 *
 * @brief  DIET macros for use with thte asynchronous versions of dmat_manips
 *
 * @author  Christophe PERA (Philippe.Combes@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


#ifndef _GLOBAL_MACROS_HH_
#define _GLOBAL_MACROS_HH_

#include <ctime>
#include <iostream>
#ifndef __WIN32__
#include <unistd.h>
#include <sys/time.h>
#else
#include <Winsock2.h>
#include <windows.h>
#include <sys/timeb.h>
#endif
#include <omniconfig.h>
#include <omnithread.h>

// Others debug Traces

#define traceTimer 1
#define traceThreadId 1
#define traceFile 1
#define traceLine 1
#define traceFunction 1

#define FILE_OUTPUT()                           \
  cout << "file=" << __FILE__ << "|";
#define LINE_OUTPUT()                           \
  cout << "line=" << __LINE__ << "|";
#define FUNCTION_OUTPUT()                       \
  cout << "function=" << __FUNCTION__ << "|";
#define OMNITHREADID_OUTPUT()                   \
  int id = omni_thread::self()->id();           \
  cout << "ThreaID=" << id << "|";
#define TIMER_OUTPUT()                          \
  timeval tval;                                 \
  gettimeofday(&tval, NULL);                    \
  std::cout << "timer=" << tval.tv_sec << ", " << tval.tv_usec << "|";
#define VARIABLE_OUTPUT(X)                      \
  cout << # X << "=" << X;
#define TEXT_OUTPUT(X)                          \
  printf X;
# if defined (DIET_NDEBUG)
#   define DIET_DEBUG(X)
# else
#   define DIET_DEBUG(X) DIET_TRACE_IMPL(X)
# endif

# define DIET_TRACE_IMPL(X)                             \
  do {                                                  \
    if (traceTimer == 1) {TIMER_OUTPUT()}             \
    if (traceThreadId == 1) {OMNITHREADID_OUTPUT()}    \
    if (traceFile == 1) {FILE_OUTPUT()}               \
    if (traceLine == 1) {LINE_OUTPUT()}               \
    if (traceFunction == 1) {FUNCTION_OUTPUT()}       \
    X                                                   \
    std::cout << "\n";                                \
    fflush(stdout);                                     \
    fflush(stderr);                                     \
  } while (0);

#endif /* _GLOBAL_MACROS_H_ */
