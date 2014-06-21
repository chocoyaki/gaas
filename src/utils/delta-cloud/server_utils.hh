#ifndef _SERVER_UTILS_HH_
#define _SERVER_UTILS_HH_

#include <stdint.h>
#include <time.h>
#include <string>

uint64_t utime();
time_t utime_seconds(uint64_t t);
std::string format_time(time_t t);

std::string pexec(std::string cmdline);

void stress(double duration, int num_processes);

#endif
