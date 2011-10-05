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
 * Revision 1.16  2010/03/31 21:15:40  bdepardo
 * Changed C headers into C++ headers
 *
 * Revision 1.15  2008/11/08 19:12:38  bdepardo
 * A few warnings removal
 *
 * Revision 1.14  2008/04/07 12:19:12  ycaniou
 * Except for the class Parsers (someone to re-code it? :)
 *   correct "deprecated conversion from string constant to 'char*'" warnings
 *
 * Revision 1.13  2006/11/16 09:55:56  eboix
 *   DIET_config.h is no longer used. --- Injay2461
 *
 * Revision 1.12  2004/06/09 15:10:38  mcolin
 * add stat_flush in statistics API in order to flush write access to
 * statistic file for agent and sed which never end and can't call
 * stat_finalize
 *
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

#include <cstdio>
#include <sys/time.h>

#if HAVE_STATISTICS

enum stat_type {
  STAT_IN,
  STAT_OUT,
  STAT_INFO
};

// Please, don't use those variables
extern FILE* STAT_FILE;
extern const char * const STAT_TYPE_STRING[3];

// Don't call this, call stat_in, stat_out & stat_info instead
inline void
gen_stat(int type, const char * myname, const char * message) {
  if (STAT_FILE != NULL) {
    struct timeval tv;
    struct timezone tz;

    if (gettimeofday(&tv, &tz) == 0) {
      fprintf(STAT_FILE, "%10ld.%06ld|%s|[%s] %s\n",
              (long int)tv.tv_sec, (long int)tv.tv_usec,
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
do_stat_flush();
void
do_stat_finalize();

/////////////////////////////////
// Use only the following calls :

#  define stat_init()        do_stat_init()
#  define stat_in(myname,message)   gen_stat(STAT_IN, myname, message)
#  define stat_out(myname,message)  gen_stat(STAT_OUT, myname, message)
#  define stat_info(myname,message) gen_stat(STAT_INFO, myname, message)
#  define stat_finalize()    do_stat_finalize()
#  define stat_flush()    do_stat_flush()

#else  // HAVE_STATISTICS

#  define stat_init()
#  define stat_in(myname,message)
#  define stat_out(myname,message)
#  define stat_info(myname,message)
#  define stat_finalize()
#  define stat_flush()

#endif // HAVE_STATISTICS

#endif // _STATISTICS_HH_
