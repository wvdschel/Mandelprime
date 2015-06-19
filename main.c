#include "pthread.h"
#include "inttypes.h"
#include "stdint.h"

#include "workqueue.h"
#include "mandelbrot.h"
#include "primesieve.h"
#include "log.h"

int main(int argc, char** argv)
{
  vlog("Starting prime sieve");
  primesieve_t sieve = create_primesieve(100000000); // Or use UINT64_MAX
  work_queue_t queue = create_work_queue(6,
                                        sieve,
                                        primesieve_do_work,
                                        primesieve_request_work,
                                        primesieve_report_results);

  queue_wait_until_finished(queue);
  vlog("Prime sieve finished");
  primesieve_print(sieve);

  destroy_primesieve(sieve);
  destroy_work_queue(queue);
  return 0;
}
