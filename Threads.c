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
#define MAX_QUEUE_SIZE 100

struct thread_data{
   int  thread_id;
   int  thread_prio;
   struct queue *queue_ptr;
};

struct thread_data thread_data_array[NUM_THREADS];

typedef struct queue{
int element[MAX_QUEUE_SIZE];
pthread_mutex_t mutex;
pthread_cond_t threshold;
int curr_prio;
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
  q->curr_prio = 0;
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
    if (elem > 100){
      printf("uhh...\n");
    }
    q->remaining_elements--;
    q->head = q->head + 1;

  } else if (q->remaining_elements == 1){
    printf("removing from queue size 1\n");
    elem = q->element[q->head];
    q->head = -1;
    q->tail = -1;
    q->remaining_elements = 0;
  } else {
    printf("removing from empty queue\n");
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
     if (queue_ptr->remaining_elements == 0) { // Block the consumer if the queue is empty
       pthread_cond_wait(&(queue_ptr->threshold), &(queue_ptr->mutex));
     }
     //if (i == 99){ printf("we made it fam\n"); }
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
   int tprio = data->thread_prio;
   struct queue *queue_ptr = data->queue_ptr;
   for(int j=0;j<10;j++){
     //printf("%ld", tid);
     if (queue_ptr->remaining_elements == MAX_QUEUE_SIZE){ // Block the producer if the queue if full
       //pthread_cond_wait(&(queue_ptr->threshold), &(queue_ptr->mutex));
      }
     queue_add(queue_ptr, tid);
     pthread_cond_signal(&(queue_ptr->threshold));
     //printf("added sumtin \n");
   }
  // (queue_ptr->curr_prio) = tprio + 1;

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
   pthread_cond_init (&(queue_ptr->threshold), NULL);

   for(t = 0; t < (NUM_THREADS - 1); t++){
     thread_data_array[t].thread_id = t;
     thread_data_array[t].thread_prio = 0;
     thread_data_array[t].queue_ptr = queue_ptr;
     thread_create = pthread_create(&threads[t], NULL, Producer, (void *) &thread_data_array[t]);
     if (thread_create){
       printf("ERROR; return code from pthread_create() is %d\n", thread_create);
       exit(-1);
       }
     }
     t++; //
     printf("got here!");
     thread_data_array[t].thread_id = t;
     thread_data_array[t].queue_ptr = queue_ptr;
     thread_create = pthread_create(&threads[t], NULL, Consumer, (void *) &thread_data_array[t]);
     if (thread_create){
       printf("ERROR; return code from pthread_create() is %d\n", thread_create);
       exit(-1);
     }

     pthread_mutex_destroy(&(queue_ptr->mutex));
     pthread_cond_destroy(&(queue_ptr->threshold));
     pthread_exit(NULL);
}
