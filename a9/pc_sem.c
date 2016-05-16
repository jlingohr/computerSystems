#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "uthread.h"
#include "uthread_sem.h"

#define MAX_ITEMS 10
const int NUM_ITERATIONS = 200;
const int NUM_CONSUMERS  = 2;
const int NUM_PRODUCERS  = 2;

int producer_wait_count;     // # of times producer had to wait
int consumer_wait_count;     // # of times consumer had to wait
int histogram [MAX_ITEMS+1]; // histogram [i] == # of times list stored i items

struct Pool {
  // TODO
  int           items;
  uthread_sem_t item_sem;
  uthread_sem_t full;
  uthread_sem_t empty;
};

struct Pool* createPool() {
  struct Pool* pool = malloc (sizeof (struct Pool));
  // TODO
  pool->items     = 0;
  pool->item_sem = uthread_sem_create(1);
  pool->full = uthread_sem_create(MAX_ITEMS);
  pool->empty = uthread_sem_create(0);
  return pool;
}


void* producer (void* pv) {
  struct Pool* p = pv;

  // TODO
  for (int i = 0; i < NUM_ITERATIONS; i++) {
    //printf("P waiting for space...\n");
    uthread_sem_wait(p->full);
    uthread_sem_wait(p->item_sem);
    p->items++;
    histogram[p->items]++;
    //printf("P placed item in pool...\n");
    uthread_sem_signal(p->item_sem);
    uthread_sem_signal(p->empty);
  }
  return NULL;
}

void* consumer (void* pv) {
  struct Pool* p = pv;
  
  // TODO
  for (int i = 0; i < NUM_ITERATIONS; i ++) {
    //printf("C waiting for items...\n");
    uthread_sem_wait(p->empty);
    uthread_sem_wait(p->item_sem);
    p->items--;
    histogram[p->items]++;
    //printf("Space created...\n");
    uthread_sem_signal(p->item_sem);
    uthread_sem_signal(p->full);
  }
  return NULL;
}

int main (int argc, char** argv) {
  uthread_t t[4];

  uthread_init (4);
  struct Pool* p = createPool();
  
  // TODO
  t[0] = uthread_create(producer, p);
  t[1] = uthread_create(producer, p);
  t[2] = uthread_create(consumer, p);
  t[3] = uthread_create(consumer, p);

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
}