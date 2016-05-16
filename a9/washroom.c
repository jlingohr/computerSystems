#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include "uthread.h"
#include "uthread_mutex_cond.h"

#ifdef VERBOSE
#define VERBOSE_PRINT(S, ...) printf (S, ##__VA_ARGS__);
#else
#define VERBOSE_PRINT(S, ...) ;
#endif

#define MAX_OCCUPANCY      3
#define NUM_ITERATIONS     100
#define NUM_PEOPLE         20
#define FAIR_WAITING_COUNT 4

/**
 * You might find these declarations useful.
 */
enum Sex {MALE = 0, FEMALE = 1};
const static enum Sex otherSex [] = {FEMALE, MALE};

struct Washroom {
  // TODO
  uthread_mutex_t mx;
  uthread_cond_t waiting[2];
  int emptying;
  int sex_i;
  int occupants;
  int users_waiting[2];
  int users_processed;
  enum Sex sexes[2];
  enum Sex sex;
  


};

struct Washroom* createWashroom() {
  struct Washroom* washroom = malloc (sizeof (struct Washroom));
  washroom->mx = uthread_mutex_create();
  washroom->waiting[0] = uthread_cond_create(washroom->mx);
  washroom->waiting[1] = uthread_cond_create(washroom->mx);
  washroom->emptying = 0;
  washroom->sex_i = random()%2;
  washroom->sexes[0] = MALE;
  washroom->sexes[1] = FEMALE;
  washroom->sex = washroom->sexes[washroom->sex_i];
  washroom->occupants = 0;
  washroom->users_waiting[0] = 0;
  washroom->users_waiting[1] = 0;
  washroom->users_processed = 0;
  return washroom;
}

#define WAITING_HISTOGRAM_SIZE (NUM_ITERATIONS * NUM_PEOPLE)
int             entryTicker;  // incremented with each entry
int             waitingHistogram         [WAITING_HISTOGRAM_SIZE];
int             waitingHistogramOverflow;
uthread_mutex_t waitingHistogrammutex;
int             occupancyHistogram       [2] [MAX_OCCUPANCY + 1];

void recordWaitingTime (int waitingTime) {
  uthread_mutex_lock (waitingHistogrammutex);
  if (waitingTime < WAITING_HISTOGRAM_SIZE)
    waitingHistogram [waitingTime] ++;
  else
    waitingHistogramOverflow ++;
  uthread_mutex_unlock (waitingHistogrammutex);
}


void enterWashroom (struct Washroom* washroom, enum Sex Sex) {
  // TODO
  static const char* sex_string[2] = {"Male", "Female"};
  uthread_mutex_lock(washroom->mx);

  int waiting_time = entryTicker;
  if (entryTicker == 0) {
    // washroom empty
    VERBOSE_PRINT("Washroom is empty...\n");
    washroom->sex = Sex;
  }
  while (washroom->sex != Sex || washroom->occupants == MAX_OCCUPANCY || washroom->emptying == 1) {
    VERBOSE_PRINT("%s user waiting for washroom. Washroom is %s with %d occupants\n", sex_string[Sex], sex_string[washroom->sex], washroom->occupants);
    washroom->users_waiting[Sex]++;
    //waiting_time++;
    uthread_cond_wait(washroom->waiting[Sex]);
    washroom->users_waiting[Sex]--;
    if (washroom->sex != Sex && washroom->users_waiting[otherSex[Sex]] == 0) {
      washroom->sex = Sex;
    }
  } 

  VERBOSE_PRINT("%s in the washroom. Washroom is: %s with %d users\n", sex_string[Sex], sex_string[washroom->sex], washroom->occupants);
  entryTicker++;
  recordWaitingTime(entryTicker - waiting_time);
  washroom->occupants++;
  washroom->users_processed++;
  occupancyHistogram[Sex][washroom->occupants]++;
  uthread_mutex_unlock(washroom->mx);
}

void leaveWashroom (struct Washroom* washroom) {
  // TODO
  uthread_mutex_lock(washroom->mx);

  if (washroom->emptying) {
    VERBOSE_PRINT("Emptying washroom...\n");
    uthread_cond_signal(washroom->waiting[otherSex[washroom->sex]]);
  } else {
    if (washroom->users_processed < FAIR_WAITING_COUNT) {
      if (washroom->users_waiting[washroom->sex] > 0) {
        uthread_cond_signal(washroom->waiting[washroom->sex]);
      } else {
        // signal other sex
        washroom->emptying = 1;
        uthread_cond_signal(washroom->waiting[otherSex[washroom->sex]]);
      } 
    } else {
      // already processed sex, now make fair
      washroom->emptying = 1;
      uthread_cond_signal(washroom->waiting[otherSex[washroom->sex]]);
    }
  }

  washroom->occupants--;
  if (washroom->occupants == 0 && washroom->emptying == 1) {
    washroom->emptying = 0;
    washroom->sex = otherSex[washroom->sex];
    washroom->users_processed = 0;
  }
  uthread_mutex_unlock(washroom->mx);
  
}


//
// TODO
// You will probably need to create some additional produres etc.
//

void Wait() {
  for (int i = 0; i < NUM_PEOPLE; i++)
    uthread_yield();
}


void* user(void* wv) {
  struct Washroom* washroom = (struct Washroom*)wv;
  int s = random() % 2;
  enum Sex sex = otherSex[s];

  for (int i = 0; i < NUM_ITERATIONS; i++) {
    enterWashroom(washroom, sex);
    Wait();
    leaveWashroom(washroom);
    Wait();
  }
  return NULL;
}

void deleteWashroom(struct Washroom* w) {
  uthread_cond_destroy(w->waiting[0]);
  uthread_cond_destroy(w->waiting[1]);
  uthread_mutex_destroy(w->mx);
  free(w);
}

int main (int argc, char** argv) {
  uthread_init (1);
  struct Washroom* washroom = createWashroom();
  uthread_t        pt [NUM_PEOPLE];
  waitingHistogrammutex = uthread_mutex_create ();

  // TODO
  for (int i=0; i< NUM_PEOPLE; i++) {
    pt[i] = uthread_create(user, washroom);
  }
  
  for (int i = 0; i < NUM_PEOPLE; i++) {
    uthread_join(pt[i],0);
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
  deleteWashroom(washroom);
  uthread_mutex_destroy(waitingHistogrammutex);
}