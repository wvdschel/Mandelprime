#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "inttypes.h"

#include "primesieve.h"
#include "log.h"
#include "refcount.h"

// These macro's have double evaluation, so be weary.
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

static uint64_t firstprimes[500] = {
  2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71,
  73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131, 137, 139, 149, 151, 157, 163, 167, 173,
  179, 181, 191, 193, 197, 199, 211, 223, 227, 229, 233, 239, 241, 251, 257, 263, 269, 271, 277, 281,
  283, 293, 307, 311, 313, 317, 331, 337, 347, 349, 353, 359, 367, 373, 379, 383, 389, 397, 401, 409,
  419, 421, 431, 433, 439, 443, 449, 457, 461, 463, 467, 479, 487, 491, 499, 503, 509, 521, 523, 541,
  547, 557, 563, 569, 571, 577, 587, 593, 599, 601, 607, 613, 617, 619, 631, 641, 643, 647, 653, 659,
  661, 673, 677, 683, 691, 701, 709, 719, 727, 733, 739, 743, 751, 757, 761, 769, 773, 787, 797, 809,
  811, 821, 823, 827, 829, 839, 853, 857, 859, 863, 877, 881, 883, 887, 907, 911, 919, 929, 937, 941,
  947, 953, 967, 971, 977, 983, 991, 997, 1009, 1013, 1019, 1021, 1031, 1033, 1039, 1049, 1051, 1061, 1063, 1069,
  1087, 1091, 1093, 1097, 1103, 1109, 1117, 1123, 1129, 1151, 1153, 1163, 1171, 1181, 1187, 1193, 1201, 1213, 1217, 1223,
  1229, 1231, 1237, 1249, 1259, 1277, 1279, 1283, 1289, 1291, 1297, 1301, 1303, 1307, 1319, 1321, 1327, 1361, 1367, 1373,
  1381, 1399, 1409, 1423, 1427, 1429, 1433, 1439, 1447, 1451, 1453, 1459, 1471, 1481, 1483, 1487, 1489, 1493, 1499, 1511,
  1523, 1531, 1543, 1549, 1553, 1559, 1567, 1571, 1579, 1583, 1597, 1601, 1607, 1609, 1613, 1619, 1621, 1627, 1637, 1657,
  1663, 1667, 1669, 1693, 1697, 1699, 1709, 1721, 1723, 1733, 1741, 1747, 1753, 1759, 1777, 1783, 1787, 1789, 1801, 1811,
  1823, 1831, 1847, 1861, 1867, 1871, 1873, 1877, 1879, 1889, 1901, 1907, 1913, 1931, 1933, 1949, 1951, 1973, 1979, 1987,
  1993, 1997, 1999, 2003, 2011, 2017, 2027, 2029, 2039, 2053, 2063, 2069, 2081, 2083, 2087, 2089, 2099, 2111, 2113, 2129,
  2131, 2137, 2141, 2143, 2153, 2161, 2179, 2203, 2207, 2213, 2221, 2237, 2239, 2243, 2251, 2267, 2269, 2273, 2281, 2287,
  2293, 2297, 2309, 2311, 2333, 2339, 2341, 2347, 2351, 2357, 2371, 2377, 2381, 2383, 2389, 2393, 2399, 2411, 2417, 2423,
  2437, 2441, 2447, 2459, 2467, 2473, 2477, 2503, 2521, 2531, 2539, 2543, 2549, 2551, 2557, 2579, 2591, 2593, 2609, 2617,
  2621, 2633, 2647, 2657, 2659, 2663, 2671, 2677, 2683, 2687, 2689, 2693, 2699, 2707, 2711, 2713, 2719, 2729, 2731, 2741,
  2749, 2753, 2767, 2777, 2789, 2791, 2797, 2801, 2803, 2819, 2833, 2837, 2843, 2851, 2857, 2861, 2879, 2887, 2897, 2903,
  2909, 2917, 2927, 2939, 2953, 2957, 2963, 2969, 2971, 2999, 3001, 3011, 3019, 3023, 3037, 3041, 3049, 3061, 3067, 3079,
  3083, 3089, 3109, 3119, 3121, 3137, 3163, 3167, 3169, 3181, 3187, 3191, 3203, 3209, 3217, 3221, 3229, 3251, 3253, 3257,
  3259, 3271, 3299, 3301, 3307, 3313, 3319, 3323, 3329, 3331, 3343, 3347, 3359, 3361, 3371, 3373, 3389, 3391, 3407, 3413,
  3433, 3449, 3457, 3461, 3463, 3467, 3469, 3491, 3499, 3511, 3517, 3527, 3529, 3533, 3539, 3541, 3547, 3557, 3559, 3571
};
#define firstprimes_count (sizeof(firstprimes)/sizeof(firstprimes[1]))
#define INIT_SIZE 10000
#define WORK_SIZE 5000
  
typedef struct work {
  uint64_t  start, stop;
  uint64_t* primes;
  size_t    count;
  uint64_t* sieve;
  uint64_t* sieve_end;
  struct work* next;
} work_t;

struct primesieve
{
  uint64_t* primes;
  size_t    count;
  size_t    capacity;

  uint64_t  max_checked;   ///< Largest number checked for primality.
  uint64_t  max_dispensed; ///< Largest number that has been sent to a worker.
  uint64_t  max_number;    ///< Bound to stop at (no number above this will be checked).

  work_t*   results_list;  ///< Sorted list of results that have predecessors which are not finished yet.
};

static void grow_sieve(primesieve_t sieve)
{
  sieve->capacity *= 2;
  sieve->primes = refcount_resize(sieve->primes, sieve->capacity * sizeof(uint64_t));
}

primesieve_t create_primesieve(uint64_t max_number)
{
  primesieve_t sieve = calloc(1, sizeof(struct primesieve));

  sieve->primes = refcount_allocate(sizeof(uint64_t) * INIT_SIZE);
  memcpy(sieve->primes, firstprimes, sizeof(firstprimes));

  sieve->count = firstprimes_count;
  sieve->capacity = INIT_SIZE;
  sieve->max_checked = firstprimes[firstprimes_count - 1];
  sieve->max_dispensed = sieve->max_checked;
  sieve->max_number = max_number;

  return sieve;
}

void destroy_primesieve(primesieve_t sieve)
{
  work_t* work = sieve->results_list;
  while(work)
  {
    work_t* next = work->next;

    free(work->primes);
    refcount_free(work->sieve);
    free(work);
    
    work = next;
  }

  refcount_free(sieve->primes);
  free(sieve);
}

void* primesieve_request_work(work_queue_t queue, size_t worker_id)
{
  work_t* new_work = calloc(1, sizeof(work_t));
  primesieve_t sieve = queue_get_private_data(queue);

  // Check if all work is done.
  if(sieve->max_dispensed >= sieve->max_number) return NULL;

  new_work->sieve = sieve->primes;
  refcount_increment(new_work->sieve);
  new_work->sieve_end = sieve->primes + sieve->count;

  new_work->start = sieve->max_dispensed + 1;
  new_work->stop  = MIN(sieve->max_checked * sieve->max_checked,
                        new_work->start + WORK_SIZE - 1);
  new_work->stop  = MIN(sieve->max_number, new_work->stop);
  new_work->primes = malloc(sizeof(uint64_t) * WORK_SIZE);

  if(new_work->start <= new_work->stop)
  {
    dlog("Handing out [%" PRIu64 ", %" PRIu64 "] to worker %zu.",
         new_work->start, new_work->stop, worker_id);
  } else {
    vlog("No work available - worker %zu will be idle for a while.",
         worker_id);
  }

  sieve->max_dispensed = MAX(new_work->stop, sieve->max_dispensed);

  return new_work;
}

// Helper function that updates the sieve to include any primes found in work
// and then frees all the memory used for work.
// Returns work->next
static work_t* append_work(primesieve_t sieve, work_t* work)
{
  work_t* res = work->next;

  if(work->start != sieve->max_checked + 1)
  {
    vlog("!!! WARNING! Appending primes in range [%" PRIu64 ", %" PRIu64 "], but the current sieve has only checked values up to %" PRIu64 "!!",
         work->start, work->stop, sieve->max_checked);
  } else {
    dlog("Appending primes in range [%" PRIu64 ", %" PRIu64 "]", work->start, work->stop); 
  }

  while(sieve->capacity < sieve->count + work->count)
  { // Out of space, resize array
    grow_sieve(sieve);
  }

  memcpy(sieve->primes + sieve->count,
         work->primes, work->count * sizeof(uint64_t));
  sieve->count += work->count;
  sieve->max_checked = MAX(work->stop, sieve->max_checked);
  
  refcount_decrement(work->sieve);
  free(work->primes);
  free(work);

  return res;
}

static void insert_in_list(primesieve_t sieve, work_t* work_res)
{
  dlog("Queueing [%" PRIu64 ", %" PRIu64 "] because of missing work starting at %" PRIu64,
       work_res->start, work_res->stop, sieve->max_checked+1);
  
  if(sieve->results_list == NULL
     || sieve->results_list->start >= work_res->start)
  { // Current work should be at the head of the list
    work_res->next = sieve->results_list;
    sieve->results_list = work_res;
    dlog("Inserting  [%" PRIu64 ", %" PRIu64 "] at the front of the list.", work_res->start, work_res->stop);
  } else {
    // Current work should be inserted somewhere in the list.
    work_t *prev_work = NULL,
      *next_work = sieve->results_list;
    
    while(next_work && next_work->start < work_res->start)
    {
      prev_work = next_work;
      next_work = next_work->next;
      
    }
    
    if(next_work) // work_res should be inserted between prev and next
    {
      dlog("Inserting  [%" PRIu64 ", %" PRIu64 "] between [%" PRIu64 ", %" PRIu64 "] and [%" PRIu64 ", %" PRIu64 "]",
           work_res->start, work_res->stop, prev_work->start, prev_work->stop, next_work->start, next_work->stop);
      work_res->next  = prev_work->next;
      prev_work->next = work_res;
    } else { // work_res should be inserted at the end of the list.
      dlog("Inserting  [%" PRIu64 ", %" PRIu64 "] after [%" PRIu64 ", %" PRIu64 "]", work_res->start, work_res->stop, prev_work->start, prev_work->stop);
      prev_work->next = work_res;
    }
  }
}

void  primesieve_report_results(work_queue_t queue, size_t worker_id, void* results)
{
  work_t* work_res = (work_t*)results;
  primesieve_t sieve = queue_get_private_data(queue);

  dlog("Recieved [%" PRIu64 ", %" PRIu64 "] from worker %zu, found %zu new primes.",
       work_res->start, work_res->stop, worker_id, work_res->count);

  if(sieve->max_checked + 1 >= work_res->start)
  {
    // No gap between the last accepted work and these results
    // Insert this result
    append_work(sieve, work_res);

    // Insert any results that were queued but are now ready to be added
    while(sieve->results_list &&
          sieve->results_list->start <= sieve->max_checked + 1)
    {
      sieve->results_list = append_work(sieve, sieve->results_list);
    }
  } else {
    // Gap between the last accepted work and these results,
    // so just insert them into the queue for later copying.

    insert_in_list(sieve, work_res);
  }

#if VERBOSE
  dlog("Next value needed is %" PRIu64 ", results left in queue:", sieve->max_checked+1);

  work_res = sieve->results_list;
  while(work_res)
  {
    dlog(" - [%" PRIu64 ", %" PRIu64 "]", work_res->start, work_res->stop);
    work_res = work_res->next;
  }
#endif
}

void* primesieve_do_work(void* work_desc)
{
  work_t* work = (work_t*)work_desc;

  if(work->start <= work->stop)
  {
    uint64_t curr_num = work->start;
    while(curr_num <= work->stop)
    {
      uint64_t* divisor = work->sieve;
      
      while(curr_num % *divisor != 0
            && (*divisor) * (*divisor) < curr_num)
        divisor++;
      
      if(curr_num % *divisor != 0)
      {
        // curr_num is prime.
        work->primes[work->count] = curr_num;
        work->count++;
      }
      
      curr_num++;
    }
  } else {
    // No work, sleep for a while.
    struct timespec sleep;
    sleep.tv_sec  = 3;
    sleep.tv_nsec = 0;
    clock_nanosleep(CLOCK_MONOTONIC, 0, &sleep, NULL);
  }

  return work;
}

void primesieve_print(primesieve_t sieve)
{
  vlog("Sieve %p has checked all primes up to %" PRIu64, sieve, sieve->max_checked);
  vlog(" => %zu primes found", sieve->count);
  vlog(" => Largest prime: %" PRIu64, sieve->primes[sieve->count-1]);
}
