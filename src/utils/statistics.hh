/**
 * @file  statistics.hh
 *
 * @brief  DIET statistics tools source code
 *
 * @author  Ludovic BERTSCH (Ludovic.Bertsch@ens-lyon.fr)
 *          Eddy CARON (Eddy.Caron@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */

#include "stats_config.h"

#ifndef _STATISTICS_HH_
#define _STATISTICS_HH_

// The first part is ALWAYS defined, so we can link any program
// even when the macro HAVE_STATISTICS is not defined

#include <cstdio>
#ifdef WIN32
#include <time.h>
#else
#include <sys/time.h>
#endif
#ifdef __WIN32__
#define DIET_API_LIB __declspec(dllexport)
#else
#define DIET_API_LIB
#endif

#if HAVE_STATISTICS

enum stat_type {
  STAT_IN,
  STAT_OUT,
  STAT_INFO
};


// Please, don't use those variables
extern FILE  *  STAT_FILE;
extern const char *const STAT_TYPE_STRING[3];

// Don't call this, call stat_in, stat_out & stat_info instead
DIET_API_LIB void
gen_stat(int type, const char *myname, const char *message);

// Don't call this, call init_stat instead!
DIET_API_LIB void
do_stat_init();
DIET_API_LIB void
do_stat_flush();
DIET_API_LIB void
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
