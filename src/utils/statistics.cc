#include <stdio.h>
#include <stdlib.h>

#include "statistics.hh"

char* stat_type_string[] = {
  "IN  ",
  "OUT ",
  "INFO"
};

static char* stat_file_name; // We don't need it to be global
FILE* stat_file = NULL;

void do_stat_init() {
  if (stat_file != NULL) {
    fprintf(stderr, "init_stat(): Stat module already initialized!\n"
	    "             Continuing without reopening it!\n");
  } else {
    stat_file_name = getenv("DIET_STAT_FILE_NAME");
    if (stat_file_name != NULL) {
      stat_file = fopen(stat_file_name, "wc");
      if (stat_file == NULL) {
	fprintf(stderr, "init_stat(): Unable to open file \"%s\"\n", stat_file_name);
	perror("init_stat");
      }
    }
  }
}
