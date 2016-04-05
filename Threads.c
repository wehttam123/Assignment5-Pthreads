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
#define MAX_QUEUE_SIZE 200

struct thread_data{
   int  thread_id;
   struct queue *queue_ptr;
};

struct thread_data thread_data_array[NUM_THREADS];

typedef struct queue{
int element[MAX_QUEUE_SIZE];
pthread_mutex_t mutex;
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

void queue_initialize( prod_cons_queue *q ){
  q->head = -1;
  q->tail = -1;
  q->remaining_elements = 0;
};

void queue_add(  prod_cons_queue *q,  int element){

  pthread_mutex_lock (&(q->mutex));
  if (q->remaining_elements > 0){
    q->tail = q->remaining_elements;
    q->element[q->remaining_elements] = element;
    q->remaining_elements++;
  } else {
    q->head = 0;
    q->tail = 0;
    q->element[q->remaining_elements] = element;
    q->remaining_elements++;
  }
  pthread_mutex_unlock (&(q->mutex));
};

int queue_remove(  prod_cons_queue *q ){
   pthread_mutex_lock (&(q->mutex));
  int elem;

  if (q->remaining_elements > 1){
    elem = q->element[q->head];
    q->remaining_elements--;
    q->head = q->head+1;
    //elem = q->element[q->head];
  } else if (q->remaining_elements == 1){
    q->head = 0;
    q->tail = 0;
    q->remaining_elements = 0;
    elem = -1;
  } else {
    q->head = -1;
    q->tail = -1;
    q->remaining_elements = 0;
    elem = -1;
  }

  pthread_mutex_unlock (&(q->mutex));
  return elem;
};

void *Consumer(void *c_data)
{
   struct thread_data *data;
   data = (struct thread_data *) c_data;
   long tid = data->thread_id;
   struct queue *queue_ptr = data->queue_ptr;

   for(int i=0;i<100;i++){
     int id = queue_remove(queue_ptr);
     printf("Thread id:%ld\n", id);
   }

   pthread_exit(NULL);
}

void *Producer(void *p_data)
{
   struct thread_data *data;
   data = (struct thread_data *) p_data;
   long tid = data->thread_id;
   struct queue *queue_ptr = data->queue_ptr;
   for(int i=0;i<10;i++){
     queue_add(queue_ptr, tid);
   }

   pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
   pthread_t threads[NUM_THREADS];
   struct queue thread_queue;
   struct queue *queue_ptr;
   int thread_create;
   long t;

   queue_ptr = &thread_queue;

   queue_initialize(queue_ptr);

   pthread_mutex_init(&(queue_ptr->mutex), NULL);

   for(t=0;t<NUM_THREADS;t++){
     thread_data_array[t].thread_id = t;
     thread_data_array[t].queue_ptr = queue_ptr;
     thread_create = pthread_create(&threads[t], NULL, Producer, (void *) &thread_data_array[t]);
     if (thread_create){
       printf("ERROR; return code from pthread_create() is %d\n", thread_create);
       exit(-1);
       }
     }

     thread_data_array[t].thread_id = t;
     thread_data_array[t].queue_ptr = queue_ptr;
     thread_create = pthread_create(&threads[t], NULL, Consumer, (void *) &thread_data_array[t]);
     if (thread_create){
       printf("ERROR; return code from pthread_create() is %d\n", thread_create);
       exit(-1);
     }

     pthread_mutex_destroy(&(queue_ptr->mutex));
     pthread_exit(NULL);
}
