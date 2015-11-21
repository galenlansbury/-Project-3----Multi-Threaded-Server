#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "thread_pool.h"
#include "seats.h"
#include "util.h"

/**
 *  @struct threadpool_task
 *  @brief the work struct
 *
 *  Feel free to make any modifications you want to the function prototypes and structs
 *
 *  @var function Pointer to the function that will perform the task.
 *  @var argument Argument to be passed to the function.
 */

#define MAX_THREADS 10
#define STANDBY_SIZE 10

typedef struct {
    void (*function)(void *);
    void *argument;
} pool_task_t;


struct pool_t {
  pthread_mutex_t lock;
  pthread_cond_t not_full;
  pthread_cond_t not_empty;
  pthread_t *threads;
  pool_task_t *queue;
  int thread_count;
  int queue_used; //keep track of size of the queue
  int task_queue_size_limit;
  int queue_front;
  int queue_rear;
  int close;
};

static void *thread_do_work(void *pool);
void * dowork(void* fd);

/*
 * Create a threadpool, initialize variables, etc
 *
 */
pool_t *pool_create(int queue_size, int num_threads)
{
  pool_t *pl = (pool_t*)malloc(sizeof(pool_t));
  pthread_mutex_init(&(pl->lock), NULL);
  pthread_cond_init (&(pl->not_empty), NULL);
  pthread_cond_init (&(pl->not_full), NULL);
  pl->thread_count = 0;
  pl->queue_front = 0;
  pl->queue_rear = 0;
  pl->queue_used = 0;
  pl->task_queue_size_limit = 20;
  pl->threads = (pthread_t *)malloc(sizeof(pthread_t) * MAX_THREADS);
  pl->queue = (pool_task_t *)malloc(sizeof(pool_task_t) * pl->task_queue_size_limit);
  for(int i = 0; i < MAX_THREADS; i++)
  {
    pthread_create(&(pl->threads[i]),NULL, thread_do_work, (void*)pl);
  }
  return pl;
}



/*
 * Add a task to the threadpool
 *
 */
int pool_add_task(pool_t *pool, void (*function)(void *), void *argument)
{
  int err = 0;
  pthread_mutex_lock(&(pool->lock)); //need to protect while altering the thread pool

  while(pool->queue_used == pool->task_queue_size_limit) //in the event that there is no longer any space, program will wait for space to open up and block incoming requests
  {
    pthread_cond_wait(&(pool->not_full),&(pool->lock)); //not full must be signaled upon completion of every thread
  }

  (pool->queue[pool->queue_rear]).function = function; //records the function associated with the job
  (pool->queue[pool->queue_rear]).argument = argument; //and the argument

  pool->queue_rear = (pool->queue_rear+1) % pool->task_queue_size_limit; //this will increase the size of the buffer
  pool->queue_used++; //will make the used more

  pthread_cond_broadcast(&(pool->not_empty)); //we usse broadcast because there are many threads waiting for a ready
  pthread_mutex_unlock(&(pool->lock));     

  return err;
}

/*
 * Destroy the threadpool, free all memory, destroy treads, etc
 *
 */
int pool_destroy(pool_t *pool)
{
    int err = 0;
 
    return err;
}


/*
 * Work loop for threads. Should be passed into the pthread_create() method.
 *
 */
void * dowork(void* fd)
{
        int connfd = *((int*)fd); //will dereference the pointer to a page descriptor
        struct request req;
        // parse_request fills in the req struct object
        parse_request(connfd, &req);
        // process_request reads the req struct and processes the command
        process_request(connfd, &req);
        close(connfd);

}

/*
 * Work loop for threads. Should be passed into the pthread_create() method.
 *
 */
static void *thread_do_work(void *pool)
{ 
	//these threads will constantly be running
    while(1) {

		pthread_mutex_lock(&(pool->lock)); //must lock this because we are working with the worker queue
		if((pool->queue_used) == 0) //if worker queue is empty, put thread to sleep until new tasks are added to the worker queue
		{
			pthread_cond_wait(&(pool->not_empty),&(pool->lock));
		}
		//if worker queue is not empty than complete the first task in the queue
		
		//void * dowork(void* fd)
		 // (pool->queue[pool->queue_rear]).function = function; 
		//(pool->queue[pool->queue_rear]).argument = argument; 

		pool_task_t temp = pool->queue[pool->queue_front]; //I don't know how to do dereference of the void function pointers specifically so I'm going to do this and use a temp

		pool->queue_used--; //total amount of jobs completed is minus one
		pool->queue_front++; //upon completing the task we want to increment the queue by one
		pthread_cond_signal(&(pool->not_full)); ///signal that we are available to add another job
		pthread_mutex_unlock(&(pool->lock));    //release the lock before executing the function

		//execute the actual function simultaneously with any other threads
		(temp.function)(temp.argument);
	}

    pthread_exit(NULL);
    return(NULL);
}

// TODO: Declare your threadpool!


