/****************************************************************************/
/* DIET statistics tools source code                                        */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Ludovic BERTSCH (Ludovic.Bertsch@ens-lyon.fr)                       */
/*    - Eddy CARON (Eddy.Caron@ens-lyon.fr)                                 */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.18  2010/03/31 21:15:40  bdepardo
 * Changed C headers into C++ headers
 *
 * Revision 1.17  2010/03/31 19:37:55  bdepardo
 * Changed "\n" into std::endl
 *
 * Revision 1.16  2008/04/07 12:19:12  ycaniou
 * Except for the class Parsers (someone to re-code it? :)
 *   correct "deprecated conversion from string constant to 'char*'" warnings
 *
 * Revision 1.15  2006/05/22 20:01:49  hdail
 * Introduced uniform output format for SeD configuration option output at launch
 * time.
 *
 * Revision 1.14  2006/05/20 13:57:03  hdail
 * Cleaned up statistics error output messages.  Now you will see an error
 * about missing environment variable only on the first call to log
 * statistics.
 *
 * Revision 1.13  2004/06/09 15:10:38  mcolin
 * add stat_flush in statistics API in order to flush write access to
 * statistic file for agent and sed which never end and can't call
 * stat_finalize
 *
 * Revision 1.12  2003/10/02 23:02:49  ecaron
 * Apply the TRACE_LEVEL API
 *
 * Revision 1.11  2003/09/16 15:01:56  ecaron
 * Add statistics log into MA and LA [getRequest part]
 *
 * Revision 1.10  2003/09/16 13:22:03  pcombes
 * Fix error message.
 *
 * Revision 1.9  2003/04/10 12:50:02  pcombes
 * Change some errors into warnings.
 *
 * Revision 1.8  2003/02/04 10:08:23  pcombes
 * Apply Coding Standards
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

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <unistd.h>
#include "debug.hh"
#include "statistics.hh"

#if HAVE_STATISTICS

const char * const STAT_TYPE_STRING[] = {
  "IN  ",
  "OUT ",
  "INFO"
};

/** The trace level. */
extern unsigned int TRACE_LEVEL;

#define STAT_TRACE_FUNCTION(formatted_text)             \
  TRACE_TEXT(TRACE_ALL_STEPS, "Statistics::");          \
  TRACE_FUNCTION(TRACE_ALL_STEPS,formatted_text)


static char* STAT_FILE_NAME;  // We don't need it to be global
FILE* STAT_FILE = NULL;
static int USING_STATS = 1;

void
do_stat_init()
{
  if (STAT_FILE != NULL) {
    TRACE_TEXT(TRACE_STRUCTURES,
               "do_stat_init() - Stats module already initialized.\n");
  } else {
    STAT_FILE_NAME = getenv("DIET_STAT_FILE_NAME");

    if (STAT_FILE_NAME != NULL) {
      STAT_FILE = fopen(STAT_FILE_NAME, "a");
      if (STAT_FILE == NULL) {
        TRACE_TEXT(TRACE_MAIN_STEPS,
                   "Warning: do_stat_init() - Unable to open file " << STAT_FILE_NAME
                   << ".\n");
        ERROR("do_stat_init() - Check DIET_STAT_FILE_NAME env variable?",);
      } else {
        TRACE_TEXT(TRACE_ALL_STEPS,"* Statistics collection: enabled "
                   << "(file" << STAT_FILE_NAME << ")\n");
        USING_STATS = 1;
      }
    } else if (USING_STATS) {
      TRACE_TEXT(TRACE_ALL_STEPS, "* Statistics collection: disabled\n");
      TRACE_TEXT(TRACE_ALL_STEPS,
                 "\tTo collect statistics, set env variable DIET_STAT_FILE_NAME.\n");
      USING_STATS = 0;
    }
  }
}

void
do_stat_flush()
{
  if (USING_STATS) {
    if (STAT_FILE == NULL) {
      TRACE_TEXT(TRACE_MAIN_STEPS,
                 "Warning (do_stat_finalize): stats module is NOT initialized!\n");
    } else {
      if (fflush(STAT_FILE) != 0) {
        ERROR("do_stat_flush() - Unable to flush stat file.",);
      }
    }
  }
}

void
do_stat_finalize()
{
  if (USING_STATS) {
    if (STAT_FILE == NULL) {
      TRACE_TEXT(TRACE_MAIN_STEPS,
                 "Warning (do_stat_finalize): stats module is NOT initialized!\n");
    } else {
      if (fclose(STAT_FILE) < 0) {
        ERROR("do_stat_finalize() - Unable to close stat file.",);
      }
      STAT_FILE = NULL;
    }
  }
}

#endif  // HAVE_STATISTICS
