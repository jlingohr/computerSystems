#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include "uthread.h"
#include "uthread_mutex_cond.h"

#define NUM_ITERATIONS 100

#ifdef VERBOSE
#define VERBOSE_PRINT(S, ...) printf (S, ##__VA_ARGS__);
#else
#define VERBOSE_PRINT(S, ...) ;
#endif

struct Agent {
  uthread_mutex_t mutex;
  uthread_cond_t  match;
  uthread_cond_t  paper;
  uthread_cond_t  tobacco;
  uthread_cond_t  smoke;
};

struct Agent* createAgent() {
  struct Agent* agent = malloc (sizeof (struct Agent));
  agent->mutex   = uthread_mutex_create();
  agent->paper   = uthread_cond_create (agent->mutex);
  agent->match   = uthread_cond_create (agent->mutex);
  agent->tobacco = uthread_cond_create (agent->mutex);
  agent->smoke   = uthread_cond_create (agent->mutex);
  return agent;
}

//
// TODO
// You will probably need to add some procedures and struct etc.
//
struct Resources {
  int smoked;
  int onTable;
  int rsrc1;
  int rsrc2;

  struct Agent* agent;
};

struct Resources* createResources(struct Agent* a) {
  struct Resources* res = malloc(sizeof(struct Resources));
  res->smoked = 0;
  res->onTable = 0;
  res->rsrc1 = 0;
  res->rsrc2 = 0;
  res->agent = a;
  return res;
}

struct Smoker {
  int rsrc;
  struct Resources* resource;
};

struct Smoker* createSmokers(int r, struct Resources* res) {
  struct Smoker* smoker = malloc(sizeof(struct Smoker));
  smoker->rsrc = r;
  smoker->resource = res;
  return smoker;
}



void clear(struct Resources* r) {
  r->onTable=0;
  r->rsrc1 = r->rsrc2 = 0;
}

/**
 * You might find these declarations helpful.
 *   Note that Resource enum had values 1, 2 and 4 so you can combine resources;
 *   e.g., having a MATCH and PAPER is the value MATCH | PAPER == 1 | 2 == 3
 *    -- AND only TOBACCO (3) can use, so must use
 */
enum Resource            {    MATCH = 1, PAPER = 2,   TOBACCO = 4};
char* resource_name [] = {"", "match",   "paper", "", "tobacco"};

int signal_count [5];  // # of times resource signalled
int smoke_count  [5];  // # of times smoker with resource smoked

/**
 * This is the agent procedure.  It is complete and you shouldn't change it in
 * any material way.  You can re-write it if you like, but be sure that all it does
 * is choose 2 random reasources, signal their condition variables, and then wait
 * wait for a smoker to smoke.
 */
void* agent (void* av) {
  struct Agent* a = av;
  static const int choices[]         = {MATCH|PAPER, MATCH|TOBACCO, PAPER|TOBACCO};
  static const int matching_smoker[] = {TOBACCO,     PAPER,         MATCH};
  
  // Agent only signals to smokers who cant actually smoke?
  // So signals aken smokers who produce the items, then signal smoker who can smoke
  uthread_mutex_lock (a->mutex);
    for (int i = 0; i < NUM_ITERATIONS; i++) {
      int r = random() % 3;
      signal_count [matching_smoker [r]] ++;
      int c = choices [r];
      if (c & MATCH) {
        VERBOSE_PRINT ("match available\n");
        uthread_cond_signal (a->match);   
      }
      if (c & PAPER) {
        VERBOSE_PRINT ("paper available\n");
        uthread_cond_signal (a->paper);
      }
      if (c & TOBACCO) {
        VERBOSE_PRINT ("tobacco available\n");
        uthread_cond_signal (a->tobacco);
      }
      VERBOSE_PRINT ("agent is waiting for smoker to smoke\n");
      uthread_cond_wait (a->smoke);
    }
  uthread_mutex_unlock (a->mutex);
  return NULL;
}

void* smoker(void* sv) {
  struct Smoker* s = sv;
  struct Resources* resources = s->resource;

  uthread_mutex_lock(resources->agent->mutex);
  
  while (resources->smoked < NUM_ITERATIONS) {
    // sleep initially
    switch(s->rsrc) {
      case MATCH:
        uthread_cond_wait(resources->agent->match);
        break;
      case PAPER:
        uthread_cond_wait(resources->agent->paper);
        break;
      case TOBACCO:
        uthread_cond_wait(resources->agent->tobacco);
        break;
    }
    // if no materials on table, place materials
    if (resources->onTable == 0) {
      resources->rsrc1 = s->rsrc;
      resources->onTable += 1;
    }
    // If 1 material, place another and call intended smoker
    else if (resources->onTable == 1) {
      resources->rsrc2 = s->rsrc;
      resources->onTable += 1;
      switch((resources->rsrc1|resources->rsrc2)) {
        case 6:
          uthread_cond_signal(resources->agent->match);
          break;
        case 5:
          uthread_cond_signal(resources->agent->paper);
          break;
        case 3:
          uthread_cond_signal(resources->agent->tobacco);
          break;
      }
    } else {
      smoke_count[s->rsrc]++;
      resources->smoked += 1;
      clear(resources);
      VERBOSE_PRINT ("%s smoked\n", resource_name[s->rsrc]);
      uthread_cond_signal(resources->agent->smoke);

    }
  }
  uthread_mutex_unlock(resources->agent->mutex);
  return NULL;
}

int main (int argc, char** argv) {
  uthread_init (7);
  struct Agent*  a = createAgent();
  // TODO
  struct Resources* res = createResources(a);

  struct Smoker* match = createSmokers(MATCH, res);
  struct Smoker* paper = createSmokers(PAPER, res);
  struct Smoker* tobac = createSmokers(TOBACCO, res);

  uthread_t threads[3];
  threads[0] = uthread_create(smoker, match);
  threads[1] = uthread_create(smoker, paper);
  threads[2] = uthread_create(smoker, tobac);

  uthread_join (uthread_create (agent, a), 0);

  assert (signal_count [MATCH]   == smoke_count [MATCH]);
  assert (signal_count [PAPER]   == smoke_count [PAPER]);
  assert (signal_count [TOBACCO] == smoke_count [TOBACCO]);
  assert (smoke_count [MATCH] + smoke_count [PAPER] + smoke_count [TOBACCO] == NUM_ITERATIONS);
  printf ("Smoke counts: %d matches, %d paper, %d tobacco\n",
          smoke_count [MATCH], smoke_count [PAPER], smoke_count [TOBACCO]);
}