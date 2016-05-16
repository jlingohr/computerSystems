#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include "uthread.h"
#include "uthread_sem.h"

#ifdef VERBOSE
#define VERBOSE_PRINT(S, ...) printf (S, ##__VA_ARGS__);
#else
#define VERBOSE_PRINT(S, ...) ;
#endif

#define MAX_OCCUPANCY      3
#define NUM_ITERATIONS     100
#define NUM_PEOPLE         20
#define FAIR_WAITING_COUNT 4

#define WAITING_HISTOGRAM_SIZE (NUM_ITERATIONS * NUM_PEOPLE)
int           entryTicker;
int           waitingHistogram         [WAITING_HISTOGRAM_SIZE];
int           waitingHistogramOverflow;
uthread_sem_t waitingHistogramMutex;
int           occupancyHistogram       [2] [MAX_OCCUPANCY + 1];

/**
 * You may find these declarations useful.
 */
enum Sex {MALE = 0, FEMALE = 1};
const static enum Sex otherSex [] = {FEMALE, MALE};

struct Washroom {
  // TODO
  uthread_sem_t empty;
  uthread_sem_t key;
  uthread_sem_t users[2];
  uthread_sem_t door;
  uthread_sem_t lock;
  uthread_sem_t sex_lock;
  int occupants;
  int sex_count[2];


};

struct Washroom* createWashroom() {
  struct Washroom* washroom = malloc (sizeof (struct Washroom));
  // TODO
  washroom->empty = uthread_sem_create(1);
  washroom->key = uthread_sem_create(1);
  washroom->users[0] = uthread_sem_create(MAX_OCCUPANCY);
  washroom->users[1] = uthread_sem_create(MAX_OCCUPANCY);
  washroom->door = uthread_sem_create(1);
  washroom->lock = uthread_sem_create(1);
  washroom->sex_lock = uthread_sem_create(1);
  washroom->occupants = 0;
  washroom->sex_count[0] = 0;
  washroom->sex_count[1] = 0;

  return washroom;
}

void Wait() {
  for (int i = 0; i < NUM_PEOPLE; i++) {
    uthread_yield();
  }
}

void enterWashroom (struct Washroom* w, enum Sex Sex) {
  uthread_sem_wait(w->users[Sex]);
  // In the washroom
  uthread_sem_wait(w->lock);

  w->occupants++;
  occupancyHistogram[Sex][w->occupants]++;
  uthread_sem_signal(w->lock);

  Wait(); // Do your business
  uthread_sem_signal(w->users[Sex]);

}

void leaveWashroom (struct Washroom* w) {
  uthread_sem_wait(w->lock);
  w->occupants--;
  if (w->occupants == 0) {
    uthread_sem_signal(w->empty);
  }
  uthread_sem_signal(w->lock);
}

void recordWaitingTime (int waitingTime) {
  uthread_sem_wait (waitingHistogramMutex);
    if (waitingTime < WAITING_HISTOGRAM_SIZE)
      waitingHistogram [waitingTime] ++;
    else
      waitingHistogramOverflow ++;
  uthread_sem_signal (waitingHistogramMutex);
}

void* user(void* wv) {
  struct Washroom* w = wv;
  int s = random() % 2;
  enum Sex sex = otherSex[s];
  //printf("%d user created...\n", sex);
  for (int i = 0; i < NUM_ITERATIONS; i++) {
    uthread_sem_wait(w->door);
    int start_time = entryTicker;
    if (w->sex_count[otherSex[sex]] != 0) {
      uthread_sem_wait(w->empty);
    }
    uthread_sem_wait(w->sex_lock);
    w->sex_count[sex]++;
    uthread_sem_signal(w->sex_lock);
    uthread_sem_signal(w->door);

    enterWashroom(w, sex);
    recordWaitingTime(entryTicker - start_time);
    Wait();

    leaveWashroom(w);
    uthread_sem_wait(w->sex_lock);
    w->sex_count[sex]--;
    uthread_sem_signal(w->sex_lock);

    Wait();
  }
  return NULL;
}

int main (int argc, char** argv) {
  uthread_init (1);
  struct Washroom* washroom = createWashroom();
  waitingHistogramMutex     = uthread_sem_create (1);

  // TODO
  uthread_t threads[NUM_PEOPLE];
  for (int i = 0; i < NUM_PEOPLE; i++) {
    threads[i] = uthread_create(user, (void*)washroom);
  }
  printf("All threads created...\n");
  for (int i = 0; i < NUM_PEOPLE; i++) {
    uthread_join(threads[i], 0);
  }

  printf ("Times with 1 male    %d\n", occupancyHistogram [MALE]   [1]);
  printf ("Times with 2 males   %d\n", occupancyHistogram [MALE]   [2]);
  printf ("Times with 3 males   %d\n", occupancyHistogram [MALE]   [3]);
  printf ("Times with 1 female  %d\n", occupancyHistogram [FEMALE] [1]);
  printf ("Times with 2 females %d\n", occupancyHistogram [FEMALE] [2]);
  printf ("Times with 3 females %d\n", occupancyHistogram [FEMALE] [3]);
  printf ("Waiting Histogram\n");
  for (int i=0; i<WAITING_HISTOGRAM_SIZE; i++)
    if (waitingHistogram [i])
      printf ("  Number of times people waited for %d %s to enter: %d\n", i, i==1?"person":"people", waitingHistogram [i]);
  if (waitingHistogramOverflow)
    printf ("  Number of times people waited more than %d entries: %d\n", WAITING_HISTOGRAM_SIZE, waitingHistogramOverflow);
}