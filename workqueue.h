#ifndef _MANDELPRIME_WORKQUEUE_H_
#define _MANDELPRIME_WORKQUEUE_H_

/**
 * This header offers an interface for a multithreaded work queue.
 *
 * In order to use this interface, the user should provide the following:
 *  - A structure or type that describes a certain unit of work.
 *  - A structure that describes the result of such a unit of work.
 *  - A worker function, which takes a unit of work as an argument and returns the results.
 *  - A request_work function, which returns a new unit of work for a worker to perform.
 *  - A report_results function, which reports the results of a worker once he has finished a unit of work.
 *
 * The types for the unit of work and its results are an opaque void pointer to the
 *  work queue. The signatures of these functions are described in more detail below.
 **/

/**
 * Pointer type referring to a work queue.
 **/
typedef struct work_queue* work_queue_t;

/**
 * Function pointer to a function to request more work from a worker thread.
 *
 * This function will acquire the global queue lock, and as such is guarantueed to
 * never run at the same time as another request_work or report_results call from the
 * same queue.
 *
 * Note that NULL should not be returned if there is temporarily no work available,
 * as this function will no longer be called for this work queue and all workers will
 * exit once their current work description is finished.
 *
 * As such, NULL may only be returned if work will never become available for this queue
 * and this work queue should terminate as soon as all threads are .
 *
 * @param queue     The queue for which a thread is requesting new work.
 * @param worker_id The ID of a thread [< get_worker_count(queue)].
 * @return An arbitrary pointer to a work description, or NULL if all work is finished and the worker should stop.
 **/
typedef void* (*request_work_fp)(work_queue_t queue, size_t worker_id);

/**
 * Function pointer to a function to report results from a worker thread.
 *
 * This function will acquire the global queue lock, and as such is guarantueed to
 * never run at the same time as another request_work or report_results call from the
 * same queue.
 *
 * @param queue     The queue for which a thread is reporting new results.
 * @param worker_id The ID of a thread [May be higher than get_worker_count(queue) if the worker count was recently lowered].
 * @param results   Pointer to the results of a work unit.
 **/
typedef void  (*report_results_fp)(work_queue_t queue, size_t worker_id, void* results);

/**
 * Function pointer to a function that takes a unit of work and transforms it into a result.
 *
 * No locking happens around this function, so it is up to the user to do any locking if necessary.
 * 
 * @param work_desc A unit of work description, as returned by a call to a request_work_fp function.
 *                  Guarantueed to be non-NULL.
 * @return The result of the work, to be provided as an argument to a report_results_fp function.
 **/
typedef void* (*do_work_fp)(void* work_desc);

/**
 * Creates and starts a new work queue.
 *
 * @param worker_count The number of worker threads to spawn.
 * @param priv_data    Any optional private data that can be used by request_func or report_func.
 * @param work_func    The function that performs work (@see do_work_fp).
 * @param request_func The function that will be used to request new work (@see request_work_fp).
 * @param report_func  The function that will be used to report the results of a call to work_func (@see report_results_fp).
 * @return A work queue if succesful, or NULL in case an error occurred.
 **/
work_queue_t create_work_queue(size_t worker_count,
                               void * priv_data,
                               do_work_fp work_func,
                               request_work_fp request_func,
                               report_results_fp report_func);

/**
 * Returns the private data of a work queue.
 *
 * @param queue The queue to get the private data from.
 * @return The priv_data pointer provided to the queue when it was created.
 **/
void* queue_get_private_data(work_queue_t queue);

/**
 * Destroy a work queue.
 *
 * This release any resources associated with the queue. It is up to the user to
 * release the memory or any other resources assiocated with the private data of the
 * queue.
 *
 * The function will wait for all the worker threads to finish their current unit of work.
 * Work cannot be cancelled and do_work_fp that were already started cannot be preempted.
 *
 * @param queue The queue to destroy.
 **/
void destroy_work_queue(work_queue_t queue);

/**
 * Check if a queue has finished its work.
 *
 * Finished means that the associated request_work_fp has returned NULL,
 * and all workers have reported the results for any work they have received via
 * this function.
 *
 * @return non-zero if the queue has finished, 0 if it has not.
 **/
int queue_is_finished(work_queue_t queue);

/**
 * Wait for a work queue to finish, for the definition of
 * "finish" used above.
 *
 * If there are zero workers for this queue, this function will never return.
 **/
void queue_wait_until_finished(work_queue_t queue);

/**
 * Change the number of worker threads for a queue.
 *
 * If the new worker count is lower than the current worker count,
 * this function will halt until the excess workers have finished their
 * work and release any resources associated with them.
 *
 * If the new worker count is larger than the current worker count,
 * new threads will be started to process work.
 *
 * Setting the number of workers to zero will effectively pause any work
 * for this queue.
 *
 * @param queue Queue to change the number of worker threads for.
 * @param worker_count New worker count.
 * @return 0 on success, non-zero in case of an error.
 **/
int queue_set_worker_count(work_queue_t queue, size_t worker_count);

/**
 * Retrieve the amount of worker threads used by a queue.
 *
 * If the queue is out of work, this will still return the total amount of workers,
 * not just the amount of workers that are still actively processing work descriptions.
 *
 * @param queue The queue to get the worker count for.
 * @param The number of workers that are allocated for this queue.
 **/
size_t queue_get_worker_count(work_queue_t queue);

#endif // __WORKQUEUE_H_
