#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "statistics.hh"

char* stat_type_string[] = {
  "IN  ",
  "OUT ",
  "INFO"
};

static char* stat_file_name; // We don't need it to be global
FILE* stat_file = NULL;

void do_stat_init() {
  char *stat_file_name_base;
  char tmp[100000];

  if (stat_file != NULL) {
    fprintf(stderr, "do_stat_init(): Stat module already initialized!\n"
	    "             Continuing without reopening it!\n");
  } else {
    stat_file_name_base = getenv("DIET_STAT_FILE_NAME");
    sprintf(tmp, "%s.%d", stat_file_name_base, getpid());
    stat_file_name_base = strdup(tmp);

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
