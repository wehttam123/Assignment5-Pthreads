#include <stdint.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#define NUM_THREADS	11
#define MAX_QUEUE_SIZE 20


//pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;

struct thread_data{
   int  thread_id;
   int  thread_prio;
   struct queue *queue_ptr;
};

struct thread_data thread_data_array[NUM_THREADS];

typedef struct queue{
int element[MAX_QUEUE_SIZE];
int curr_prio;
uint8_t  head;
uint8_t  remaining_elements; // #of elements in the queue
pthread_mutex_t mutex;
pthread_cond_t cond;
bool blocked[10];
int num_blocked_threads;
// any more variables that you need can be added
}prod_cons_queue;

void queue_initialize( prod_cons_queue *q );
// initialize all queue variables and set element pointers to
// NULL
void queue_add(  prod_cons_queue *q,  int element);
int queue_remove(  prod_cons_queue *q );
//the removed element is returned in adouble pointer “data”

void queue_initialize( prod_cons_queue *q ){
  q->head = 0;
  q->remaining_elements = 0;
  q->curr_prio = 0;
  for (int i=0; i < 10; i++){
    q->blocked[i] = false;
  }
  q->num_blocked_threads = 0;
};


// Add an element to the tail of the queue //
void queue_add(  prod_cons_queue *q,  int element){
  pthread_mutex_lock (&(q->mutex));

   while (q->remaining_elements == MAX_QUEUE_SIZE) { // Block the producer if the queue is full
     pthread_cond_wait(&(q->cond), &(q->mutex));
    }

  // Adding to queue with nonzero elements (adds to the tail) //
  if (q->remaining_elements > 0){
    q->element[q->remaining_elements] = element;
    q->remaining_elements++;

  // Adding to queue with no elements //
  } else {
    q->element[q->remaining_elements] = element;
    q->remaining_elements++;
  }
   if(q->remaining_elements != 0){  // Signal the Consumer after the queue is no longer empty
     pthread_cond_signal(&(q->cond));
   }

  pthread_mutex_unlock (&(q->mutex));
}

// Remove an element from the head of the queue and return it //
int queue_remove(  prod_cons_queue *q ){
  pthread_mutex_lock (&(q->mutex));
  int elem;
  //printf("Removing!\n");

  while(q->remaining_elements == 0) { // Block the consumer if the queue is empty
    pthread_cond_wait(&(q->cond), &(q->mutex));
  }


  // Removing from queue with more than 1 element //
  if (q->remaining_elements > 1){
    elem = q->element[q->head];
    q->remaining_elements--;

    for (int i=0; i< (q->remaining_elements - 1); i++){
      q->element[i] = q->element[i+1];
    }

  // Removing from queue with 1 element //
  } else if (q->remaining_elements == 1){
    q->head = 0;
    elem = q->element[q->head];
    q->element[0] = q->element[1];
    q->remaining_elements = 0;

  // Removing from queue with no elements //
  } else {
    q->head = 0;
    q->remaining_elements = 0;
    elem = -1;
  }

  if (q->remaining_elements < MAX_QUEUE_SIZE) { // Signal the Producer after the queue is no longer full
    pthread_cond_signal(&(q->cond));
  }

  printf("Thread id:%ld\n", elem);
  pthread_mutex_unlock (&(q->mutex));
  return elem;
}

void *Consumer(void *c_data)
{
   struct thread_data *data;
   data = (struct thread_data *) c_data;
   long tid = data->thread_id;
   struct queue *queue_ptr = data->queue_ptr;

   // Consume 100 elements before exiting //
   for(int i=0;i<100;i++){
    //  pthread_mutex_lock (&mutex);


     int id = queue_remove(queue_ptr);


     //printf("Thread id:%ld\n", id); // Print the consumed element
     //pthread_mutex_unlock (&mutex);
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
   // Produce 10 elements before exiting //
   for(int j=0;j<10;j++){
      if (queue_ptr->blocked[tid] == false) {

        while(queue_ptr->num_blocked_threads > 0);
        if (queue_ptr->remaining_elements == MAX_QUEUE_SIZE) {
          pthread_mutex_lock (&(queue_ptr->mutex));
          queue_ptr->blocked[tid] = true;
          queue_ptr->num_blocked_threads++;
          pthread_mutex_unlock (&(queue_ptr->mutex));
        }
       queue_add(queue_ptr, tid);
     }


     else if (queue_ptr->blocked[tid] == true){
       //printf("%d\n" + tid);
        if (queue_ptr->remaining_elements == MAX_QUEUE_SIZE) {
          pthread_mutex_lock (&(queue_ptr->mutex));
          queue_ptr->blocked[tid] = true;
          queue_ptr->num_blocked_threads--;
          pthread_mutex_unlock (&(queue_ptr->mutex));
        }
        else{
          pthread_mutex_lock (&(queue_ptr->mutex));
          queue_ptr->num_blocked_threads--;
          queue_ptr->blocked[tid] = false;
          pthread_mutex_unlock (&(queue_ptr->mutex));
        }
       queue_add(queue_ptr, tid);

   }
 }

   pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
   pthread_t threads[NUM_THREADS];
   pthread_attr_t attr;
   struct queue thread_queue;
   struct queue *queue_ptr;
   int thread_create;
   long t;
   queue_ptr = &thread_queue;

   // Initialize the queue, attr and detach state //
   queue_initialize(queue_ptr);
   pthread_attr_init(&attr);
   pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
   pthread_mutex_init(&(queue_ptr->mutex), NULL);
   pthread_cond_init (&(queue_ptr->cond), NULL);



   // Create 10 producers //
   for(t = 0; t < (NUM_THREADS - 1); t++){
     thread_data_array[t].thread_id = t;
     thread_data_array[t].thread_prio = 0; // Initial priority is 0
     thread_data_array[t].queue_ptr = queue_ptr;
     thread_create = pthread_create(&threads[t], &attr, Producer, (void *) &thread_data_array[t]);
   }


   t++;
   // Create 1 consumer //
   thread_data_array[t].thread_id = t;
   thread_data_array[t].queue_ptr = queue_ptr;
   thread_create = pthread_create(&threads[t], &attr, Consumer, (void *) &thread_data_array[t]);

   //pthread_cond_signal(&cond);

  // // Join the 10 producers //
  // for (int i=0; i< (NUM_THREADS - 1); i++) {
  //   pthread_join(threads[i], NULL);
  // }
  //
  // // Then join the consumer //
  // pthread_join(threads[NUM_THREADS], NULL);

  // Clean up & exit //
  pthread_attr_destroy(&attr);
  pthread_mutex_destroy(&(queue_ptr->mutex));
  pthread_cond_destroy(&(queue_ptr->cond));
  //pthread_cond_destroy(&cond1);
  pthread_exit(NULL);
}
