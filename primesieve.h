#ifndef _MANDELPRIME_PRIMESIEVE_H_
#define _MANDELPRIME_PRIMESIEVE_H_

#include "workqueue.h"

typedef struct primesieve* primesieve_t;

primesieve_t create_primesieve(uint64_t max_number);
void destroy_primesieve(primesieve_t);

void* primesieve_request_work(work_queue_t queue, size_t worker_id);
void  primesieve_report_results(work_queue_t queue, size_t worker_id, void* results);
void* primesieve_do_work(void* work_desc);

void primesieve_print(primesieve_t sieve);


#endif // _MANDELPRIME_PRIMESIEVE_H_
