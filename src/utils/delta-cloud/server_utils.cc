#include "server_utils.hh"
#include <cmath>
#include <cstdlib>
#include <sys/time.h>
#include <sstream>
#include <cstdio>
#include <unistd.h>
#include <sys/wait.h>

uint64_t utime() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (((uint64_t)tv.tv_sec) << 32) | (uint32_t)(tv.tv_usec * (4294967296.0 / 1e6));
}

time_t utime_seconds(uint64_t t) {
  // with rounding
  return (time_t)((t + 0x80000000) >> 32);
}

std::string format_time(time_t t) {
  const int bufsize = 256;
  char buf[bufsize];
  struct tm tms;
  gmtime_r(&t, &tms);
  int ret = strftime(buf, bufsize, "%Y-%m-%d %H:%M:%S", &tms);
  return std::string(buf);
}

std::string pexec(std::string cmdline) {
  std::stringstream result;
  if (FILE *f = popen(cmdline.c_str(), "r")) {
    const int bufsize = 1024;
    char buf[bufsize];
    size_t num_read;
    do {
      num_read = fread(buf, 1, bufsize, f);
      result.write(buf, num_read);
    } while (num_read == bufsize);
    pclose(f);
  }
  return result.str();
}

void stress(double duration, int num_processes) {
  uint64_t start_t = utime();
  pid_t processes[num_processes];
  for (int pnum = 0; pnum < num_processes; pnum++) {
    if ((processes[pnum] = fork()) == 0) {
      volatile double d;
      while(utime() < start_t + (duration * 4294967296)) {
        d = sqrt(random());
      }
      exit(0);
    }
  }
  for (int pnum = 0; pnum < num_processes; pnum++) {
    waitpid(processes[pnum], NULL,0);
  }
}
