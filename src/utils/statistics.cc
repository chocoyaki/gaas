/****************************************************************************/
/* $Id$ */
/* DIET statistics tools source code                                        */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Ludovic BERTSCH           - LIP ENS-Lyon (France)                   */
/*                                                                          */
/*  This is part of DIET software.                                          */
/*  Copyright (C) 2002 ReMaP/LIFC/INRIA                                     */
/*                                                                          */
/****************************************************************************/
/*
 * $Log$
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
 *
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "statistics.hh"

#if HAVE_STATISTICS

char* stat_type_string[] = {
  "IN  ",
  "OUT ",
  "INFO"
};

static char* stat_file_name; // We don't need it to be global
FILE* stat_file = NULL;

void do_stat_init() {
  if (stat_file != NULL) {
    fprintf(stderr, "do_stat_init(): Stat module already initialized!\n");
    fprintf(stderr, "                Continuing without reopening it!\n");
  } else {
    stat_file_name = getenv("DIET_STAT_FILE_NAME");

    if (stat_file_name != NULL) {
      stat_file = fopen(stat_file_name, "wc");
      if (stat_file == NULL) {
	fprintf(stderr, "do_stat_init(): Unable to open file \"%s\"\n", stat_file_name);
	fprintf(stderr, "do_stat_init(): (see DIET_STAT_FILE_NAME env variable?)\n");
	perror("do_stat_init");
      }
    }
  }
}

void do_stat_finalize() {
  if (stat_file == NULL) {
    fprintf(stderr, "do_stat_finalize(): Stat module is NOT initialized!\n");
    fprintf(stderr, "                    Continuing...\n");
  } else {
    if (fclose(stat_file) < 0) {
      fprintf(stderr, "Unable to close stat file\n");
      perror("do_stat_finalize");
    }
    stat_file = NULL;
  }
}

#endif // HAVE_STATISTICS
