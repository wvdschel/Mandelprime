#ifndef _MANDELPRIME_LOG_H_
#define _MANDELPRIME_LOG_H_

#ifndef VERBOSE
#define VERBOSE 0
#endif

#include "time.h"

double difftimespec(struct timespec* end, struct timespec* start);
void _vlog(char* fmtstr, ...);

#define dlog(args...) if(VERBOSE) { _vlog(args); }
#define vlog(args...) _vlog(args)

#endif // _MANDELPRIME_LOG_H_
