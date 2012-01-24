/**
 * @file  debug.hh
 *
 * @brief  DIET debug utils source code
 *
 * @author  Philippe COMBES (Philippe.Combes@ens-lyon.fr)
 *          Frederic LOMBARD (Frederic.Lombard@lifc.univ-fcomte.fr)
 *
 * @section Licence
 *   |LICENCE|
 */



#ifndef _DEBUG_HH_
#define _DEBUG_HH_

#include <cmath> /* include used for the definition of HUGE_VAL*/
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <omniconfig.h>
#include <omnithread.h>
#include "common_types.hh"
#include "response.hh"
#include "DIET_data_internal.hh"
#include "OSIndependance.hh"
#ifdef WIN32
   #define DIET_API_LIB __declspec(dllexport)
#else
   #define DIET_API_LIB
#endif

/** The trace level. */
extern "C" DIET_API_LIB unsigned int TRACE_LEVEL;
/** mutex used by the debug library to share the stderr and stdout */
extern omni_mutex debug_log_mutex;

/**
 * Various values for the trace level
 */
#define NO_TRACE 0
#define TRACE_ERR_AND_WARN 1
#define TRACE_MAIN_STEPS 2
#define TRACE_ALL_STEPS 5
#define TRACE_STRUCTURES 10
#define TRACE_MAX_VALUE TRACE_STRUCTURES
#define TRACE_DEFAULT TRACE_MAIN_STEPS

/**
 * Definition of the EXIT_FUNCTION when not yet defined.
 * You must place the EXIT_FUNCTION's definition before any
 * preprocessing inclusion for this to work.
 */
#ifndef EXIT_FUNCTION
#define EXIT_FUNCTION std::cout << \
  "DEBUG WARNING: EXIT_FUNCTION undeclared !\n"
#endif


/**
 * Always useful
 */
#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif /* MIN */
#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif  /* MAX */

/**
 * Print a the name of the file and the line number where this macro
 * is put to stdout.
 */
#define BIP printf("bip - " __FILE__ " %i\n", __LINE__)

/**
 * Error message - return with return_value.
 */
#define ERROR_DEBUG(formatted_msg, return_value) {                     \
    if ((int) TRACE_LEVEL >= (int) TRACE_ERR_AND_WARN) {          \
      debug_log_mutex.lock();                                   \
      std::cerr << "DIET ERROR: " << formatted_msg << ".\n";    \
      debug_log_mutex.unlock();                                 \
    }                                                           \
    return return_value; }

#define ERROR_EXIT(formatted_msg) {                             \
    if ((int) TRACE_LEVEL >= (int) TRACE_ERR_AND_WARN) {          \
      debug_log_mutex.lock();                                   \
      std::cerr << "DIET ERROR: " << formatted_msg << ".\n";    \
      debug_log_mutex.unlock();                                 \
    }                                                           \
    exit(1); }

#define INTERNAL_ERROR_EXIT(formatted_msg) {                    \
    if ((int) TRACE_LEVEL >= (int) TRACE_ERR_AND_WARN) {          \
      debug_log_mutex.lock();                                   \
      std::cerr << "DIET ERROR: " << formatted_msg << ".\n";    \
      debug_log_mutex.unlock();                                 \
    }                                                           \
    exit(1); }

/**
 * Warning message.
 */
#define WARNING(formatted_msg)                                  \
  if ((int) TRACE_LEVEL >= (int) TRACE_ERR_AND_WARN) {            \
    debug_log_mutex.lock();                                     \
    std::cerr << "DIET WARNING: " << formatted_msg << ".\n";    \
    debug_log_mutex.unlock(); }


/**
 * Internal Error message - exit with exit_value.
 */
#define INTERNAL_ERROR(formatted_msg, exit_value)                        \
  if ((int) TRACE_LEVEL >= (int) TRACE_ERR_AND_WARN) {                    \
    debug_log_mutex.lock();                                             \
    std::cerr << "DIET INTERNAL ERROR: " << formatted_msg << ".\n" <<   \
    "Please send bug report to diet-usr@ens-lyon.fr\n";               \
    debug_log_mutex.unlock(); }                                         \
  EXIT_FUNCTION;                                                        \
  exit(exit_value)

/**
 * Internal Warning message.
 */
#define INTERNAL_WARNING(formatted_msg)                                 \
  if ((int) TRACE_LEVEL >= (int) TRACE_ERR_AND_WARN) {                    \
    debug_log_mutex.lock();                                             \
    std::cerr << "DIET INTERNAL WARNING: " << formatted_msg << ".\n" << \
    "This is not a fatal bug, but please send a report "              \
    "to diet-dev@ens-lyon.fr\n";            \
    debug_log_mutex.unlock(); }


// DEBUG pause: insert a pause of duration <s>+<us>E-6 seconds
#define PAUSE(s, us)                             \
  {                                             \
    struct timeval tv;                          \
    tv.tv_sec = s;                             \
    tv.tv_usec = us;                            \
    select(0, NULL, NULL, NULL, &tv);           \
  }


// DEBUG trace: print "function(formatted_text)\n", following the iostream
// format. First argument is the minimum TRACE_LEVEL for the line to be printed.
#define TRACE_FUNCTION(level, formatted_text)                            \
  if ((int) TRACE_LEVEL >= (int) (level)) {                               \
    debug_log_mutex.lock();                                             \
    std::cout << __FUNCTION__ << '(' << formatted_text << ")\n";        \
    debug_log_mutex.unlock(); }

// DEBUG trace: print formatted_text following the iostream format (no '\n'
// added). First argument is the minimum TRACE_LEVEL for the line to be printed.
#define TRACE_TEXT(level, formatted_text)        \
  if ((int) TRACE_LEVEL >= (int) (level)) {       \
    debug_log_mutex.lock();                     \
    std::cout << formatted_text;                \
    debug_log_mutex.unlock(); }

// DEBUG trace: print "file:line: formatted_text", following the iostream format
// (no '\n' added). First argument is the minimum TRACE_LEVEL for the line to be
// printed.
#define TRACE_TEXT_POS(level, formatted_text)                            \
  if ((int) TRACE_LEVEL >= (int) (level)) {                               \
    debug_log_mutex.lock();                                             \
    std::cout << __FILE__ << ':' << __LINE__ << ": " << formatted_text; \
    debug_log_mutex.unlock(); }

// DEBUG trace: print "time: formatted_text", following the iostream format (no
// '\n' added). First argument is the minimum TRACE_LEVEL for the line to be
// printed.
#define TRACE_TIME(level, formatted_text)                                \
  if ((int) TRACE_LEVEL >= (int) (level)) {                               \
    struct timeval tv;                                                  \
    debug_log_mutex.lock();                                             \
    gettimeofday(&tv, NULL);                                            \
    printf("%10ld.%06ld: ",                                             \
           (unsigned long) tv.tv_sec, (unsigned long) tv.tv_usec);        \
    std::cout << formatted_text;                                        \
    debug_log_mutex.unlock();                                           \
  }

// DEBUG trace: print variable name and value
#define TRACE_VAR(var) {                                        \
    debug_log_mutex.lock();                                     \
    TRACE_TEXT_POS(NO_TRACE, # var << " = " << (var) << "\n");   \
    debug_log_mutex.unlock(); }










// Others debug Traces
#define traceTimer 1
#define traceThreadId 1
#define traceFile 1
#define traceLine 1
#define traceFunction 1
#define debug_format 0

#define FILE_OUTPUT()                           \
  if (debug_format == 1) {std::cout << "file="; }  \
  std::cout << __FILE__ << "|";
#define LINE_OUTPUT()                           \
  if (debug_format == 1) {std::cout << "line="; }  \
  std::cout << __LINE__ << "|";
#define FUNCTION_OUTPUT()                               \
  if (debug_format == 1) {std::cout << "function="; }      \
  std::cout << __FUNCTION__ << "|";
#define OMNITHREADID_OUTPUT()                           \
  int id = omni_thread::self()->id();                   \
  if (debug_format == 1) {std::cout << "ThreaID="; }      \
  std::cout << id << "|";
#define TIMER_OUTPUT()                                          \
  timeval tval;                                                 \
  gettimeofday(&tval, NULL);                                    \
  if (debug_format == 1) {std::cout << "timer="; }                 \
  std::cout << tval.tv_sec << ", " << tval.tv_usec << "|";
#define VARIABLE_OUTPUT(X)                      \
  std::cout << # X << "=" << X;
#define TEXT_OUTPUT(X)                          \
  printf X;
# if defined (NDEBUG)
#   define DIET_DEBUG(X)
# else
#   define DIET_DEBUG(X) DIET_TRACE_IMPL(X, Y)
# endif

# define DIET_TRACE_IMPL(X, Y)                           \
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




/*--------------------------------------------------------------------------*/
/* Tracing structures                                                       */
/*--------------------------------------------------------------------------*/


#define print_matrix(mat, m, n, rm)             \
  {                                             \
    size_t i, j;                                \
    printf("%s (%s-major) = \n", # mat,          \
           (rm) ? "row" : "column");            \
    for (i = 0; i < (m); i++) {                 \
      for (j = 0; j < (n); j++) {               \
        if (rm) {                                 \
          printf("%3f ", (mat)[j + i * (n)]); }     \
        else {                                    \
          printf("%3f ", (mat)[i + j * (m)]); }     \
      }                                         \
      printf("\n");                             \
    }                                           \
    printf("\n");                               \
  }

void
displayArgDesc(FILE *f, int type, int base_type);
void
displayArg(FILE *f, const corba_data_desc_t *arg);
void
displayArg(FILE *f, const diet_data_desc_t *arg);

void
displayConvertor(FILE *f, const diet_convertor_t *cvt);

void
displayProfileDesc(const diet_profile_desc_t *desc, const char *path);
void
displayProfileDesc(const corba_profile_desc_t *desc);
void
displayProfile(const diet_profile_t *profile, const char *path);
void
displayProfile(const corba_profile_t *profile, const char *path);
void
displayPbDesc(const corba_pb_desc_t *pb_desc);

void
displayResponse(FILE *os, const corba_response_t *resp);

void
displayResponseShort(FILE *os, const corba_response_t *resp);

#endif  // _DEBUG_HH_
