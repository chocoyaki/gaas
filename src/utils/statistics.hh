#ifndef __STATISTICS_HH__

#define __STATISTICS_HH__

// The first part is ALWAYS defined, so we can link any program
// even when the macro HAVE_STATISTICS is not defined

#include <stdio.h>
#include <sys/time.h>

enum stat_type { STAT_IN, STAT_OUT, STAT_INFO };

// Please, don't use those variables
extern FILE* stat_file;
extern char* stat_type_string[3];

// Don't call this, call stat_in, stat_out & stat_info instead
inline void gen_stat(int type, char *message) {
  if (stat_file != NULL) {
    struct timeval tv;
    struct timezone tz;

    if (gettimeofday(&tv, &tz) == 0) {
      fprintf(stat_file, "%10ld.%06ld|%s|%s\n", 
	      tv.tv_sec, tv.tv_usec,
	      stat_type_string[type],
	      message);

      /* Examples of generated trace :
       * 123456.340569|IN  |submission.start
       * 123456.340867|INFO|submission.phase1
       * 123455.345986|INFO|submission.phase2
       * 123456.354032|OUT |submission.end
       */
    }
  }
}

// Don't call this, call init_stat instead!
void do_stat_init();

/////////////////////////////////
// Use only the following calls :

#ifdef HAVE_STATISTICS

#  define stat_init()        do_stat_init()

#  define stat_in(message)   gen_stat(STAT_IN, message)
#  define stat_out(message)  gen_stat(STAT_OUT, message)
#  define stat_info(message) gen_stat(STAT_INFO, message)

#else // HAVE_STATISTICS

#  define stat_init()

#  define stat_in(message)
#  define stat_out(message)
#  define stat_info(message)

#endif // HAVE_STATISTICS

#endif // __STATISTICS_HH__
