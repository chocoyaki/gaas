/****************************************************************************/
/* DIET statistics tools specification                                      */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Ludovic BERTSCH (Ludovic.Bertsch@ens-lyon.fr)                       */
/*    - Eddy Caron (Eddy.Caron@ens-lyon.fr)                                 */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.11  2003/09/28 22:10:00  ecaron
 * New API for statistics module
 *   stat_in(myname,message)
 *   stat_out(myname,message)
 * where myname is the identity of DIET component
 *       message is the name of the current function
 *
 * Revision 1.10  2003/09/16 13:22:03  pcombes
 * Fix error message.
 *
 * Revision 1.9  2003/04/10 12:45:10  pcombes
 * Apply Coding Standards.
 *
 * Revision 1.7  2002/12/24 10:28:04  lbertsch
 * Added a stat_finalize function, so that successive initializations of
 * diet don't issue any warning...
 *
 * Revision 1.6  2002/12/18 19:02:46  pcombes
 * Bug fix in statistics integration.
 *
 * Revision 1.5  2002/12/03 19:08:24  pcombes
 * Update configure, update to FAST 0.3.15, clean CVS logs in files.
 * Put main Makefile in root directory.
 ****************************************************************************/


#ifndef _STATISTICS_HH_
#define _STATISTICS_HH_

// The first part is ALWAYS defined, so we can link any program
// even when the macro HAVE_STATISTICS is not defined

#include <stdio.h>
#include <sys/time.h>

#include "DIET_config.h"


#if HAVE_STATISTICS

enum stat_type {
  STAT_IN,
  STAT_OUT,
  STAT_INFO
};

// Please, don't use those variables
extern FILE* STAT_FILE;
extern char* STAT_TYPE_STRING[3];

// Don't call this, call stat_in, stat_out & stat_info instead
inline void
gen_stat(int type, char* myname, char* message) {
  if (STAT_FILE != NULL) {
    struct timeval tv;
    struct timezone tz;

    if (gettimeofday(&tv, &tz) == 0) {
      fprintf(STAT_FILE, "%10ld.%06ld|%s|[%s] %s\n", tv.tv_sec, tv.tv_usec,
	      STAT_TYPE_STRING[type], myname, message);

      /* Examples of generated trace :
       * 123456.340569|IN  |[Name of DIET component] submission.start
       * 123456.340867|INFO|[Name of DIET component] submission.phase1
       * 123455.345986|INFO|[Name of DIET component] submission.phase2
       * 123456.354032|OUT |[Name of DIET component] submission.end
       */
    }
  }
}

// Don't call this, call init_stat instead!
void
do_stat_init();
void
do_stat_finalize();

/////////////////////////////////
// Use only the following calls :

#  define stat_init()        do_stat_init()
#  define stat_in(myname,message)   gen_stat(STAT_IN, myname, message)
#  define stat_out(myname,message)  gen_stat(STAT_OUT, myname, message)
#  define stat_info(myname,message) gen_stat(STAT_INFO, myname, message)
#  define stat_finalize()    do_stat_finalize()

#else  // HAVE_STATISTICS

#  define stat_init()
#  define stat_in(myname,message)
#  define stat_out(myname,message)
#  define stat_info(myname,message)
#  define stat_finalize()

#endif // HAVE_STATISTICS

#endif // _STATISTICS_HH_
