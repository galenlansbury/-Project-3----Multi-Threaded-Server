#ifndef _THREADPOOL_H_
#define _THREADPOOL_H_

typedef struct pool_t pool_t;


pool_t *pool_create(int thread_count, int queue_size); //initializes a particular pool_t with predetermined threads and a queue

int pool_add_task(pool_t *pool, void (*routine)(void *), void *arg); //this is basically a feeder function for new aruguments that will be integrated intot he thread

int pool_destroy(pool_t *pool);

#endif


//struct pool_t {
//  pthread_mutex_t lock;
//  pthread_cond_t notify;
//  pthread_t *threads; //each queue will have a specific amount of threads
//  pool_task_t *queue; //option to have many queues with varying priority
//  int thread_count;
//  int task_queue_size_limit;
//};