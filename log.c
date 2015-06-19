#include "stdio.h"
#include "time.h"
#include "stdarg.h"

double difftimespec(struct timespec* end, struct timespec* start)
{
	struct timespec temp;
	if ((end->tv_nsec - start->tv_nsec) < 0) {
		temp.tv_sec = end->tv_sec - start-> tv_sec - 1;
		temp.tv_nsec = 1000000000 + end->tv_nsec - start->tv_nsec;
	} else {
		temp.tv_sec = end->tv_sec - start->tv_sec;
		temp.tv_nsec = end->tv_nsec - start->tv_nsec;
	}
	return temp.tv_sec + temp.tv_nsec / 1000000000.0;
}

void _vlog(char* fmtstr, ...)
{
  struct timespec time;
  clock_gettime(CLOCK_MONOTONIC, &time);
  printf("[%lld.%09ld] ", (long long) time.tv_sec, time.tv_nsec);

  va_list args;
  va_start(args, fmtstr);
  vprintf(fmtstr, args);
  va_end(args);

  puts("");
}
