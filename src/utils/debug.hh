/****************************************************************************/
/* DIET debug utils header                                                  */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Philippe COMBES (Philippe.Combes@ens-lyon.fr)                       */
/*    - Frederic LOMBARD (Frederic.Lombard@lifc.univ-fcomte.fr)             */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.21  2006/02/06 23:06:05  ecaron
 * define under condition of MAX and MIN to avoid duplicate declaration
 * with /usr/include/sys/param.h
 *
 * Revision 1.20  2004/12/15 13:53:22  sdahan
 * - the trace function are now thread safe.
 * - add "extern unsigned int TRACE_LEVEL" in debug.hh
 *
 * Revision 1.19  2004/10/14 15:03:47  hdail
 * Added shorter, cleaner response debug message.
 *
 * Revision 1.18  2004/09/29 13:35:32  sdahan
 * Add the Multi-MAs feature.
 *
 * Revision 1.17  2003/12/01 14:49:31  pcombes
 * Rename dietTypes.hh to DIET_data_internal.hh, for more coherency.
 *
 * Revision 1.16  2003/10/06 10:07:37  cpontvie
 * Add the EXITFUNCTION call to the INTERNAL_ERROR macro
 *
 * Revision 1.15  2003/08/01 19:38:02  pcombes
 * Include stdlib for exit.
 *
 * Revision 1.14  2003/07/04 09:48:09  pcombes
 * Add new macros for traces, errors and warning - internal or not.
 *
 * Revision 1.12  2003/04/10 12:45:44  pcombes
 * Set TRACE_LEVEL as a static variable, used by all other modules.
 * Update displayResponse to the new corba_response_t structure.
 *
 * Revision 1.10  2002/12/03 19:08:24  pcombes
 * Update configure, update to FAST 0.3.15, clean CVS logs in files.
 * Put main Makefile in root directory.
 ****************************************************************************/


#ifndef _DEBUG_HH_
#define _DEBUG_HH_

#include <iostream>
using namespace std;
#include <omniconfig.h>
#include <omnithread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "common_types.hh"
#include "response.hh"
#include "DIET_data_internal.hh"


/** The trace level. */
extern unsigned int TRACE_LEVEL;
/** mutex used by the debug library to share the stderr and stdout */
extern omni_mutex debug_log_mutex ;

/**
 * Various values for the trace level
 */
#define NO_TRACE            0
#define TRACE_MAIN_STEPS    1
#define TRACE_ALL_STEPS     5
#define TRACE_STRUCTURES   10
#define TRACE_MAX_VALUE    TRACE_STRUCTURES
#define TRACE_DEFAULT      TRACE_MAIN_STEPS


/**
 * Definition of the EXIT_FUNCTION if it not exists
 * You must put the EXIT_FUNCTION's definition before any
 * #include "xxx" in order to work.
 */
#ifndef EXIT_FUNCTION
#define EXIT_FUNCTION cout << "DEBUG WARNING: EXIT_FUNCTION undeclared !\n"
#endif


/**
 * Always useful
 */
#ifndef MIN
#define	MIN(a,b) (((a)<(b))?(a):(b))
#endif /* MIN */
#ifndef MAX
#define	MAX(a,b) (((a)>(b))?(a):(b))
#endif	/* MAX */

/**
 * Error message - return with return_value.
 */
#define ERROR(formatted_msg,return_value)           \
  debug_log_mutex.lock() ;                          \
  cerr << "DIET ERROR: " << formatted_msg << ".\n"; \
  debug_log_mutex.unlock() ;                        \
  return return_value

/**
 * Warning message.
 */
#define WARNING(formatted_msg) {                     \
  debug_log_mutex.lock() ;                           \
  cerr << "DIET WARNING: " << formatted_msg << ".\n";\
  debug_log_mutex.unlock() ; }


/**
 * Internal Error message - exit with exit_value.
 */
#define INTERNAL_ERROR(formatted_msg,exit_value) {            \
  debug_log_mutex.lock() ;                                    \
  cerr << "DIET INTERNAL ERROR: " << formatted_msg << ".\n"   \
       "Please send bug report to diet-usr@ens-lyon.fr\n" ;   \
  debug_log_mutex.unlock() ; }                                \
  EXIT_FUNCTION; \
  exit(exit_value)

/**
 * Internal Warning message.
 */
#define INTERNAL_WARNING(formatted_msg) {                     \
  debug_log_mutex.lock() ;                                    \
  cerr << "DIET INTERNAL WARNING: " << formatted_msg << ".\n" \
       "This is not a fatal bug, but please send a report "   \
      "to diet-dev@ens-lyon.fr\n"                           ; \
  debug_log_mutex.unlock() ; }


// DEBUG pause: insert a pause of duration <s>+<us>E-6 seconds
#define PAUSE(s,us)                 \
{                                   \
  struct timeval tv;                \
  tv.tv_sec  = s;                   \
  tv.tv_usec = us;                  \
  select(0, NULL, NULL, NULL, &tv); \
}


// DEBUG trace: print "function(formatted_text)\n", following the iostream
// format. First argument is the minimum TRACE_LEVEL for the line to be printed.
#define TRACE_FUNCTION(level,formatted_text)               \
  if (TRACE_LEVEL >= (level)) {                            \
    debug_log_mutex.lock() ;                               \
    cout << __FUNCTION__ << '(' << formatted_text << ")\n";\
    debug_log_mutex.unlock() ; }

// DEBUG trace: print formatted_text following the iostream format (no '\n'
// added). First argument is the minimum TRACE_LEVEL for the line to be printed.
#define TRACE_TEXT(level,formatted_text)             \
  if (TRACE_LEVEL >= (level)) {                      \
    debug_log_mutex.lock() ;                         \
    cout << formatted_text ;                         \
    debug_log_mutex.unlock() ; }

// DEBUG trace: print "file:line: formatted_text", following the iostream format
// (no '\n' added). First argument is the minimum TRACE_LEVEL for the line to be
// printed.
#define TRACE_TEXT_POS(level,formatted_text)                      \
  if (TRACE_LEVEL >= (level)) {                                   \
    debug_log_mutex.lock() ;                                      \
    cout << __FILE__ << ':' << __LINE__ << ": " << formatted_text;\
    debug_log_mutex.unlock() ; }

// DEBUG trace: print "time: formatted_text", following the iostream format (no
// '\n' added). First argument is the minimum TRACE_LEVEL for the line to be
// printed.
#define TRACE_TIME(level,formatted_text)            \
  if (TRACE_LEVEL >= (level)) {                     \
    struct timeval tv;                              \
    debug_log_mutex.lock() ;                        \
    gettimeofday(&tv, nuLL);                        \
    printf("%10ld.%06ld: ", tv.tv_sec, tv.tv_usec); \
    cout << formatted_text;                         \
    debug_log_mutex.unlock() ;                      \
  }

// DEBUG trace: print variable name and value
#define TRACE_VAR(var) { \
  debug_log_mutex.lock() ;                          \
  TRACE_TEXT_POS(NO_TRACE, #var << " = " << (var) << endl) ; \
  debug_log_mutex.unlock() ; }










// Others debug Traces
#define traceTimer 1
#define traceThreadId 1
#define traceFile 1
#define traceLine 1
#define traceFunction 1
#define debug_format 0

#define FILE_OUTPUT() \
	if (debug_format == 1) cout << "file="; \
	cout << __FILE__ << "|";
#define LINE_OUTPUT() \
	if (debug_format == 1) cout << "line="; \
	cout << __LINE__ << "|";
#define FUNCTION_OUTPUT() \
	if (debug_format == 1) cout << "function="; \
	cout << __FUNCTION__ << "|";
#define OMNITHREADID_OUTPUT()\
	int id = omni_thread::self()->id(); \
	if (debug_format == 1)	cout << "ThreaID="; \
	cout << id << "|";
#define TIMER_OUTPUT() \
	timeval tval; \
	gettimeofday(&tval, NULL); \
	if (debug_format == 1) cout << "timer="; \
	cout << tval.tv_sec << "," << tval.tv_usec << "|";
#define VARIABLE_OUTPUT(X) \
	cout << #X << "=" << X;
#define TEXT_OUTPUT(X) \
	printf X;
# if defined (NDEBUG)
#   define DIET_DEBUG(X)
# else
#   define DIET_DEBUG(X) DIET_TRACE_IMPL(X,Y)
# endif

# define DIET_TRACE_IMPL(X,Y) \
	do { \
		if (traceTimer == 1) { TIMER_OUTPUT() } \
		if (traceThreadId == 1){ OMNITHREADID_OUTPUT() } \
		if (traceFile == 1) { FILE_OUTPUT() } \
		if (traceLine == 1) { LINE_OUTPUT() } \
		if (traceFunction == 1) { FUNCTION_OUTPUT() } \
		X \
		cout << endl; \
		fflush(stdout); \
		fflush(stderr); \
  } while(0);




/*--------------------------------------------------------------------------*/
/* Tracing structures                                                       */
/*--------------------------------------------------------------------------*/


#define print_matrix(mat, m, n, rm)        \
  {                                        \
    size_t i, j;                           \
    printf("%s (%s-major) = \n", #mat,     \
           (rm) ? "row" : "column");       \
    for (i = 0; i < (m); i++) {            \
      for (j = 0; j < (n); j++) {          \
        if (rm)                            \
	  printf("%3f ", (mat)[j + i*(n)]);\
        else                               \
	  printf("%3f ", (mat)[i + j*(m)]);\
      }                                    \
      printf("\n");                        \
    }                                      \
    printf("\n");                          \
  }

void
displayArgDesc(FILE* f, int type, int base_type);
void
displayArg(FILE* f, const corba_data_desc_t* arg);
void
displayArg(FILE* f, const diet_data_desc_t* arg);

void
displayConvertor(FILE* f, const diet_convertor_t* cvt);

void
displayProfileDesc(const diet_profile_desc_t* desc, const char* path);
void
displayProfileDesc(const corba_profile_desc_t* desc);
void
displayProfile(const diet_profile_t* profile, const char* path);
void
displayProfile(const corba_profile_t* profile, const char* path);
void
displayPbDesc(const corba_pb_desc_t* pb_desc);

void
displayResponse(FILE* os, const corba_response_t* resp);

void
displayResponseShort(FILE* os, const corba_response_t* resp);

#endif // _DEBUG_HH_
