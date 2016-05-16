#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "uthread.h"
#include "spinlock.h"

#define MAX_ITEMS 10

int items = 0;

const int NUM_ITERATIONS = 100;
const int NUM_CONSUMERS  = 2;
const int NUM_PRODUCERS  = 2;

static spinlock_t lock;
static spinlock_t p_lock;
static spinlock_t c_lock;
int producer_wait_count;     // # of times producer had to wait
int consumer_wait_count;     // # of times consumer had to wait
int histogram [MAX_ITEMS+1]; // histogram [i] == # of times list stored i items

int test_and_set(spinlock_t* lock) {
  int initial_val = *lock;
  *lock = 1;
  return initial_val;
}

void produce() {
  // TODO ensure proper synchronization
  int already_held = 1;
  do {
    while (items >= MAX_ITEMS) {
      spinlock_lock(&p_lock);
      producer_wait_count++;
      spinlock_unlock(&p_lock);
    } 
    // acquire lock and recheck condition
    spinlock_lock(&lock);
    // condition failed due to race
    if (items >= MAX_ITEMS) {
      spinlock_unlock(&lock);
      spinlock_lock(&p_lock);
      producer_wait_count++;
      spinlock_unlock(&p_lock);
      
    } else {
      assert(items < MAX_ITEMS);
      items++;
      histogram [items] += 1;
      spinlock_unlock(&lock);
      break;
    }
  } while (already_held);  // what about initial phase when less than?

  
  
}

void consume() {
  // TODO ensure proper synchronization
  int already_held = 1;
  do {
    while (items == 0) {
      spinlock_lock(&c_lock);
      consumer_wait_count++;
      spinlock_unlock(&c_lock);
    } 
    // acquire lock and recheck condition
    spinlock_lock(&lock);
    // condition failed from race
    if (items == 0) {
      spinlock_unlock(&lock);
      spinlock_lock(&c_lock);
      consumer_wait_count++;
      spinlock_unlock(&c_lock);
      
    } else {
      assert(items > 0);
      items--;
      histogram [items] += 1;
      spinlock_unlock(&lock);
      break;    
    }
  } while (already_held);

  
}

void* producer() {
  // TODO - You might have to change this procedure slightly
  for (int i=0; i < NUM_ITERATIONS; i++) {
    //printf("iteration: %d\n", i);
    produce();
  }
  return NULL;
}

void* consumer() {
  // TODO - You might have to change this procedure slightly
  for (int i=0; i< NUM_ITERATIONS; i++) {
    consume();
  }
  return NULL;
}

int main (int argc, char** argv) {
  // TODO create threads to run the producers and consumers
  spinlock_create(&lock); 
  spinlock_create(&p_lock);
  spinlock_create(&c_lock);
  uthread_init(4);
  
  uthread_t p1 = uthread_create(producer, NULL);
  uthread_t p2 = uthread_create(producer, NULL);
  uthread_t c1 = uthread_create(consumer, NULL);
  uthread_t c2 = uthread_create(consumer, NULL);

  uthread_join(p1, NULL);
  uthread_join(p2, NULL);
  uthread_join(c1, NULL);
  uthread_join(c2, NULL);
  
  

  

  printf("Producer wait: %d\nConsumer wait: %d\n",
         producer_wait_count, consumer_wait_count);
  int total;
  for(int i=0;i<MAX_ITEMS+1;i++){
    //total += histogram[i];
    printf("items %d count %d\n", i, histogram[i]);
  }
  //printf("Total: %d\n", total);
}