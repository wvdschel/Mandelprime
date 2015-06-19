#include "pthread.h"
#include "errno.h"
#include "stdlib.h"
#include "string.h"

#include "time.h"

#include "log.h"
#include "workqueue.h"

struct work_queue {
  pthread_mutex_t lock;
  pthread_cond_t  cond;

  pthread_t**     worker_threads;
  size_t          worker_count;

  do_work_fp        process_work;
  request_work_fp   request_work;
  report_results_fp report_work;

  int out_of_work;
  size_t workers_done;

  void* priv_data;
};

typedef struct {
  size_t              worker_id;
  work_queue_t        work_queue;
} worker_t;

static void* worker_thread(void *arg)
{
  worker_t* worker = (worker_t*)arg;
  work_queue_t queue = worker->work_queue;

  pthread_mutex_lock(&queue->lock);
  while(1)
  {
    // Request work:
    // - Just break if worker_count has been lowered to <= worker id
    if(queue->worker_count <= worker->worker_id) break;
    // - Increment workers_done and break if no work left
    void* work = queue->request_work(queue, worker->worker_id);
    if(work == NULL)
    {
      queue->workers_done++;
      if(queue->workers_done == queue->worker_count)
        pthread_cond_signal(&queue->cond); // All workers are done, trigger signal.
      break;
    }
    
#if VERBOSE
    struct timespec start_time, stop_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);
#endif

    // Perform work, lock-free
    pthread_mutex_unlock(&queue->lock);
    void* results = queue->process_work(work);
    pthread_mutex_lock(&queue->lock);
    
#if VERBOSE
    clock_gettime(CLOCK_MONOTONIC, &stop_time);
    dlog("Worker %p:%zu has finished a work unit in %1.5lf", queue, worker->worker_id, difftimespec(&stop_time, &start_time));
#endif

    // Report results
    queue->report_work(queue, worker->worker_id, results);
  }
  dlog("Worker %p:%zu is being destroyed.", queue, worker->worker_id);
  pthread_mutex_unlock(&queue->lock);

  free(worker);
  return NULL;
}

work_queue_t create_work_queue(size_t worker_count, void* priv_data,
                               do_work_fp work_func, request_work_fp request_func, report_results_fp report_func)
{
  work_queue_t queue = calloc(1, sizeof(struct work_queue));

  queue->priv_data = priv_data;
  
  int failure = 0;
  while((failure = pthread_mutex_init(&queue->lock, NULL)) && errno == EAGAIN);
  if(failure)
  {
    dlog("Failed to create mutex: %s", strerror(errno));
    free(queue);
    return NULL;
  }
  
  while((failure = pthread_cond_init(&queue->cond, NULL)) && errno == EAGAIN);
  if(failure)
  {
    dlog("Failed to create condition variable: %s", strerror(errno));
    pthread_mutex_destroy(&queue->lock);
    free(queue);
    return NULL;
  }

  queue->process_work = work_func;
  queue->request_work = request_func;
  queue->report_work  = report_func;

  if(queue_set_worker_count(queue, worker_count))
  {
    dlog("Failed to start %zu workers.", worker_count);
    pthread_cond_destroy(&queue->cond);
    pthread_mutex_destroy(&queue->lock);
    free(queue);
    return NULL;
  }

  return queue;
}

void destroy_work_queue(work_queue_t queue)
{
  queue_set_worker_count(queue, 0);
  pthread_mutex_lock(&queue->lock);
  pthread_cond_destroy(&queue->cond);
  pthread_mutex_destroy(&queue->lock);
  free(queue);
  return;
}

void* queue_get_private_data(work_queue_t queue)
{
  return queue->priv_data;
}

int queue_is_finished(work_queue_t queue)
{
  int retval;
  pthread_mutex_lock(&queue->lock);
  retval = queue->workers_done == queue->worker_count;
  pthread_mutex_unlock(&queue->lock);

  return retval;
}

void queue_wait_until_finished(work_queue_t queue)
{
  size_t worker_count;
  pthread_mutex_lock(&queue->lock);
  worker_count = queue->worker_count;
  if(! queue->out_of_work)
  {
    pthread_cond_wait(&queue->cond, &queue->lock);
  }
  pthread_mutex_unlock(&queue->lock);
  
  // Wait for all threads to stop
  for(size_t i = 0;
      i < worker_count;
      i++)
  {
    pthread_join(*queue->worker_threads[i], NULL);
    free(queue->worker_threads[i]);
  }
  // All threads are dead & gone, so no lock needed.
  queue->worker_count = 0;
}

int queue_set_worker_count(work_queue_t queue, size_t worker_count)
{
  size_t prev_worker_count = queue->worker_count;
  
  // Change worker count, which will make excess threads shut down.
  pthread_mutex_lock(&queue->lock);
  queue->worker_count = worker_count;
  pthread_mutex_unlock(&queue->lock);
  
  // Wait for excess threads to exit - work is never cancelled.
  for(size_t i = worker_count;
      i < prev_worker_count;
      i++)
  {
    pthread_join(*queue->worker_threads[i], NULL);
  }
  
  pthread_mutex_lock(&queue->lock);
  // Destroy stopped threads (if shrinking)
  for(size_t i = worker_count;
      i < prev_worker_count;
      i++)
  {
    free(queue->worker_threads[i]);
  }
  // Re-allocate thread list (growing or shrinking it)
  queue->worker_threads = realloc(queue->worker_threads, worker_count * sizeof(pthread_t));
  // Initialize and start new threads (if growing)
  for(size_t i = prev_worker_count;
      i < worker_count;
      i++)
  {
    worker_t* worker = malloc(sizeof(worker_t));
    queue->worker_threads[i] = malloc(sizeof(pthread_t));
    worker->worker_id = i;
    worker->work_queue = queue;
    pthread_create(queue->worker_threads[i], NULL, worker_thread, worker);
  }
  pthread_mutex_unlock(&queue->lock);

  return 0;
}

size_t queue_get_worker_count(work_queue_t queue)
{
  return queue->worker_count;
}
