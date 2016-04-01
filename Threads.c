/******************************************************************************
* FILE: hello.c
* DESCRIPTION:
*   A "hello world" Pthreads program.  Demonstrates thread creation and
*   termination.
* AUTHOR: Blaise Barney
* LAST REVISED: 08/09/11
******************************************************************************/
#include <stdint.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#define NUM_THREADS	11
#define MAX_QUEUE_SIZE 20

typedef struct queue{
int element[MAX_QUEUE_SIZE];
uint8_t  head;
uint8_t  tail;
uint8_t  remaining_elements; // #of elements in the queue
// any more variables that you need can be added
}prod_cons_queue;

void queue_initialize( prod_cons_queue *q );
// initialize all queue variables and set element pointers to
// NULL
void queue_add(  prod_cons_queue *q,  int element);
int queue_remove(  prod_cons_queue *q );
//the removed element is returned in adouble pointer “data”

void *Consumer(void *threadid)
{
   long tid;
   tid = (long)threadid;
   //queue_remove(tid);
   printf("Thread id:%ld\n", tid);
   pthread_exit(NULL);
}

void *Producer(void *threadid)
{
   long tid;
   tid = (long)threadid;
   //queue_add(tid);
   //printf("Hello World! It's me, thread #%ld!\n", tid);
   printf("Thread id:%ld\n", tid);
   pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
   pthread_t threads[NUM_THREADS];
   int thread_create;
   long t;

   for(t=0;t<NUM_THREADS-1;t++){
     //printf("In main: creating thread %ld\n", t);
     printf("Producer thread %ld\n", t);
     thread_create = pthread_create(&threads[t], NULL, Producer, (void *)t);
     if (thread_create){
       printf("ERROR; return code from pthread_create() is %d\n", thread_create);
       exit(-1);
       }
     }

     printf("Consumer thread %ld\n", t);
     thread_create = pthread_create(&threads[t], NULL, Consumer, (void *)t);
     if (thread_create){
       printf("ERROR; return code from pthread_create() is %d\n", thread_create);
       exit(-1);
     }

   /* Last thing that main() should do */
   pthread_exit(NULL);
}
