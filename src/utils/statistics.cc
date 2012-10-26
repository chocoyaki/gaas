/**
 * @file  statistics.cc
 *
 * @brief  DIET statistics tools source code
 *
 * @author  Ludovic BERTSCH (Ludovic.Bertsch@ens-lyon.fr)
 *          Eddy CARON (Eddy.Caron@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include "debug.hh"
#include "statistics.hh"
#include "OSIndependance.hh"

#if HAVE_STATISTICS

const char *const STAT_TYPE_STRING[] = {
  "IN  ",
  "OUT ",
  "INFO"
};

/** The trace level. */
extern unsigned int TRACE_LEVEL;

#define STAT_TRACE_FUNCTION(formatted_text)             \
  TRACE_TEXT(TRACE_ALL_STEPS, "Statistics::");          \
  TRACE_FUNCTION(TRACE_ALL_STEPS, formatted_text)


static char *STAT_FILE_NAME;  // We don't need it to be global
FILE *STAT_FILE = NULL;
static int USING_STATS = 1;

void
gen_stat(int type, const char *myname, const char *message) {
  if (STAT_FILE != NULL) {
    struct timeval tv;
#ifdef __WIN32__
    struct timeval tz;
#else
	struct timezone tz;
#endif

    if (gettimeofday(&tv, &tz) == 0) {
      fprintf(STAT_FILE, "%10ld.%06ld|%s|[%s] %s\n",
              (long int) tv.tv_sec, (long int) tv.tv_usec,
              STAT_TYPE_STRING[type], myname, message);

      /* Examples of generated trace :
       * 123456.340569|IN  |[Name of DIET component] submission.start
       * 123456.340867|INFO|[Name of DIET component] submission.phase1
       * 123455.345986|INFO|[Name of DIET component] submission.phase2
       * 123456.354032|OUT |[Name of DIET component] submission.end
       */
    }
  }
} /* gen_stat */

void
do_stat_init() {
  if (STAT_FILE != NULL) {
    TRACE_TEXT(TRACE_STRUCTURES,
               "do_stat_init() - Stats module already initialized.\n");
  } else {
    STAT_FILE_NAME = getenv("DIET_STAT_FILE_NAME");

    if (STAT_FILE_NAME != NULL) {
      STAT_FILE = fopen(STAT_FILE_NAME, "a");
      if (STAT_FILE == NULL) {
        TRACE_TEXT(
          TRACE_MAIN_STEPS,
          "Warning: do_stat_init() - Unable to open file " <<
          STAT_FILE_NAME
                                                           << ".\n");
        ERROR_DEBUG("do_stat_init() - Check DIET_STAT_FILE_NAME env variable?", );
      } else {
        TRACE_TEXT(TRACE_ALL_STEPS, "* Statistics collection: enabled "
                   << "(file" << STAT_FILE_NAME << ")\n");
        USING_STATS = 1;
      }
    } else if (USING_STATS) {
      TRACE_TEXT(TRACE_ALL_STEPS, "* Statistics collection: disabled\n");
      TRACE_TEXT(
        TRACE_ALL_STEPS,
        "\tTo collect statistics, set env variable DIET_STAT_FILE_NAME.\n");
      USING_STATS = 0;
    }
  }
} // do_stat_init

void
do_stat_flush() {
  if (USING_STATS) {
    if (STAT_FILE == NULL) {
      TRACE_TEXT(
        TRACE_MAIN_STEPS,
        "Warning (do_stat_finalize): stats module is NOT initialized!\n");
    } else {
      if (fflush(STAT_FILE) != 0) {
        ERROR_DEBUG("do_stat_flush() - Unable to flush stat file.", );
      }
    }
  }
} // do_stat_flush

void
do_stat_finalize() {
  if (USING_STATS) {
    if (STAT_FILE == NULL) {
      TRACE_TEXT(
        TRACE_MAIN_STEPS,
        "Warning (do_stat_finalize): stats module is NOT initialized!\n");
    } else {
      if (fclose(STAT_FILE) < 0) {
        ERROR_DEBUG("do_stat_finalize() - Unable to close stat file.", );
      }
      STAT_FILE = NULL;
    }
  }
} // do_stat_finalize

#endif  // HAVE_STATISTICS
