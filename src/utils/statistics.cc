/****************************************************************************/
/* $Id$ */
/* DIET statistics tools source code                                        */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Ludovic BERTSCH (Ludovic.Bertsch@ens-lyon.fr)                       */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/*
 * $Log$
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "statistics.hh"

#if HAVE_STATISTICS

char* STAT_TYPE_STRING[] = {
  "IN  ",
  "OUT ",
  "INFO"
};

static char* STAT_FILE_NAME; // We don't need it to be global
FILE* STAT_FILE = NULL;

void
do_stat_init() 
{
  if (STAT_FILE != NULL) {
    fprintf(stderr, "do_stat_init(): Stat module already initialized!\n");
    fprintf(stderr, "                Continuing without reopening it!\n");
  } else {
    STAT_FILE_NAME = getenv("DIET_STAT_FILE_NAME");

    if (STAT_FILE_NAME != NULL) {
      STAT_FILE = fopen(STAT_FILE_NAME, "wc");
      if (STAT_FILE == NULL) {
	fprintf(stderr, "do_stat_init(): Unable to open file \"%s\"\n",
		STAT_FILE_NAME);
	fprintf(stderr,
		"do_stat_init(): (see DIET_STAT_FILE_NAME env variable?)\n");
	perror("do_stat_init");
      }
    }
  }
}

void
do_stat_finalize() 
{
  if (STAT_FILE == NULL) {
    fprintf(stderr, "do_stat_finalize(): Stat module is NOT initialized!\n");
    fprintf(stderr, "                    Continuing...\n");
  } else {
    if (fclose(STAT_FILE) < 0) {
      fprintf(stderr, "Unable to close stat file\n");
      perror("do_stat_finalize");
    }
    STAT_FILE = NULL;
  }
}

#endif // HAVE_STATISTICS
