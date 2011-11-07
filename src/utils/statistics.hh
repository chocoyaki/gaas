/**
 * @file  statistics.hh
 *
 * @brief  DIET statistics tools source code
 *
 * @author  Ludovic BERTSCH (Ludovic.Bertsch@ens-lyon.fr)
 *          Eddy CARON (Eddy.Caron@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENCE|
 */



#ifndef _STATISTICS_HH_
#define _STATISTICS_HH_

// The first part is ALWAYS defined, so we can link any program
// even when the macro HAVE_STATISTICS is not defined

#include <cstdio>
#include <sys/time.h>

#if HAVE_STATISTICS

enum stat_type {
  STAT_IN,
  STAT_OUT,
  STAT_INFO
};

// Please, don't use those variables
extern FILE *STAT_FILE;
extern const char *const STAT_TYPE_STRING[3];

// Don't call this, call stat_in, stat_out & stat_info instead
inline void
gen_stat(int type, const char *myname, const char *message) {
  if (STAT_FILE != NULL) {
    struct timeval tv;
    struct timezone tz;

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

// Don't call this, call init_stat instead!
void
do_stat_init();
void
do_stat_flush();
void
do_stat_finalize();

// ///////////////////////////////
// Use only the following calls :

#  define stat_init() do_stat_init()
#  define stat_in(myname, message) gen_stat(STAT_IN, myname, message)
#  define stat_out(myname, message) gen_stat(STAT_OUT, myname, message)
#  define stat_info(myname, message) gen_stat(STAT_INFO, myname, message)
#  define stat_finalize() do_stat_finalize()
#  define stat_flush() do_stat_flush()

#else  // HAVE_STATISTICS

#  define stat_init()
#  define stat_in(myname, message)
#  define stat_out(myname, message)
#  define stat_info(myname, message)
#  define stat_finalize()
#  define stat_flush()

#endif  // HAVE_STATISTICS

#endif  // _STATISTICS_HH_
