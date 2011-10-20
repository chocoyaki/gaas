/****************************************************************************/
/* DIET macros for use with thte asynchronous versions of dmat_manips.      */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Christophe PERA (Philippe.Combes@ens-lyon.fr)                       */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.3  2006/02/03 01:45:39  ecaron
 * Take into account the OS for omnithread thus MacOSX compilation is done.
 *
 * Revision 1.2  2003/06/30 11:15:12  cpera
 * Fix bugs in ReaderWriter and new internal debug macros.
 *
 * Revision 1.1  2003/06/16 17:12:49  pcombes
 * Move the examples using the asynchronous API into this directory.
 *
 ****************************************************************************/

#ifndef _GLOBAL_MACROS_HH_
#define _GLOBAL_MACROS_HH_

#include <ctime>
#include <iostream>
#include <sys/time.h>
#include <unistd.h>
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
  cout << #X << "=" << X;
#define TEXT_OUTPUT(X)                          \
  printf X;
# if defined (DIET_NDEBUG)
#   define DIET_DEBUG(X)
# else
#   define DIET_DEBUG(X) DIET_TRACE_IMPL(X)
# endif

# define DIET_TRACE_IMPL(X)                             \
  do {                                                  \
    if (traceTimer == 1) { TIMER_OUTPUT() }             \
    if (traceThreadId == 1){ OMNITHREADID_OUTPUT() }    \
    if (traceFile == 1) { FILE_OUTPUT() }               \
    if (traceLine == 1) { LINE_OUTPUT() }               \
    if (traceFunction == 1) { FUNCTION_OUTPUT() }       \
    X                                                   \
      std::cout << "\n";                                \
    fflush(stdout);                                     \
    fflush(stderr);                                     \
  } while (0);

#endif /* _GLOBAL_MACROS_H_ */
