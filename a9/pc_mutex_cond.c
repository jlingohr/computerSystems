#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "uthread.h"
#include "uthread_mutex_cond.h"

#define MAX_ITEMS 10
const int NUM_ITERATIONS = 100;
const int NUM_CONSUMERS  = 2;
const int NUM_PRODUCERS  = 2;

int producer_wait_count;     // # of times producer had to wait
int consumer_wait_count;     // # of times consumer had to wait
int histogram [MAX_ITEMS+1]; // histogram [i] == # of times list stored i items


struct Pool {
  uthread_mutex_t mutex;
  uthread_cond_t  notEmpty;
  uthread_cond_t  notFull;
  int             items;
};

struct Pool* createPool() {
  struct Pool* pool = malloc (sizeof (struct Pool));
  pool->mutex    = uthread_mutex_create();
  pool->notEmpty = uthread_cond_create (pool->mutex);
  pool->notFull  = uthread_cond_create (pool->mutex);
  pool->items    = 0;
  return pool;
}

void produce(struct Pool* p) {
  uthread_mutex_lock(p->mutex);
  while (p->items == MAX_ITEMS) {
    // Need to lock this
    //producer_wait_count++;
    uthread_cond_wait(p->notFull);
  }
  // Other producer waiting, so should be able to freely change producer count
  producer_wait_count++;
  p->items++;
  histogram[p->items]++;
  uthread_cond_signal(p->notEmpty);
  uthread_mutex_unlock(p->mutex);
}

void consume(struct Pool* p) {
  uthread_mutex_lock(p->mutex);
  while (p->items == 0) {
    // Need to lock this
    //consumer_wait_count++;
    uthread_cond_wait(p->notEmpty);
  }
  consumer_wait_count++;
  p->items--;
  histogram[p->items]++;
  uthread_cond_signal(p->notFull);
  uthread_mutex_unlock(p->mutex);
}

void* producer (void* pv) {
  struct Pool* p = pv;
  
  for (int i=0; i<NUM_ITERATIONS; i++) {
    // TODO
    produce(p);
    
  }
  return NULL;
}

void* consumer (void* pv) {
  struct Pool* p = pv;
  
  for (int i=0; i<NUM_ITERATIONS; i++) {
    // TODO
    consume(p);
    
  }
  return NULL;
}

int main (int argc, char** argv) {
  uthread_t t[4];

  uthread_init (4);
  
  // TODO: Create Threads and Join
  struct Pool* p = createPool();

  t[0] = uthread_create(producer, p);
  t[1] = uthread_create(producer, p);
  t[2] = uthread_create(consumer, p);
  t[3] = uthread_create(consumer, p);

  //printf("Threads created...\n");
  for (int i = 0; i < 4; i++) {
    uthread_join(t[i], NULL);
  }

  
  printf ("producer_wait_count=%d, consumer_wait_count=%d\n", producer_wait_count, consumer_wait_count);
  printf ("items value histogram:\n");
  int sum=0;
  for (int i = 0; i <= MAX_ITEMS; i++) {
    printf ("  items=%d, %d times\n", i, histogram [i]);
    sum += histogram [i];
  }
  assert (sum == sizeof (t) / sizeof (uthread_t) * NUM_ITERATIONS);

  // Cleanup
  uthread_mutex_destroy(p->mutex);
  uthread_cond_destroy(p->notEmpty);
  uthread_cond_destroy(p->notFull);
}