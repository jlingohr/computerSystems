#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "list.h"
#include <string.h>
// Allowed to use?
//#include <ctype.h>

/**
 * Print the value of *ev
 */
void print (element_t ev) {
  char* e = (char*) ev;
  printf ("%s\n", e);
}

/*
Print numbers
**/
void print_num (element_t ev) {
  intptr_t e = (intptr_t) ev;
  printf ("%ld\n", e);
}


void isNum(element_t* outv, element_t inv) {
  char* in = (char*) inv;
  intptr_t* out = (intptr_t*)outv;
  char* tmp;

  intptr_t converted = (intptr_t)strtol(in, &tmp, 10);
  if (*tmp) {
    *out = -1;
  } else {
    *out = converted;
  }

}
/*
Set values equal to numbers to NULL
**/
void set_NULL(element_t* rv, element_t av, element_t bv) {
  char* a = (char*) av;
  intptr_t b = (intptr_t) bv;
  char** r = (char**)rv;
  if (b != -1) {
    *r = (char*)NULL;
  } else {
    *r = a;
  }
}

/*
  Returns 1 if av is positive
**/
int is_pos(element_t av) {
  intptr_t a = (intptr_t) av;
  return (a >= 0);
}

/*
  Returns 1 if av is not NULL
**/
int not_NULL(element_t av) {
  char* a = (char*)av;
  return (a != NULL);
}

/*
  Truncate bv by av and place in rv
**/
void trun_str(element_t* rv, element_t av, element_t bv) {
  intptr_t a = (intptr_t)av;
  char* b = (char*)bv;
  char** r = (char**)rv;
  char* buf = malloc(a*sizeof(char)+1);
  strncpy(buf, b, a);
  buf[a] = '\0';
  *r = buf;

}

/*
  set *rv = max(*av, *bv)
**/
void max(element_t* rv, element_t av, element_t bv) {
  intptr_t a = (intptr_t) av;
  intptr_t b = (intptr_t)bv;
  intptr_t* r = (intptr_t*)rv;

  if (a > b) {
    *r = a;
  } else {
    *r = b;
  }
}

int main(int argc, char** argv) {
  struct list* l0;
  struct list* l1;
  struct list* l2;
  struct list* l3;
  struct list* l4;
  struct list* l5;
  intptr_t max_val;
  // string of command line arguments
  l0 = list_create();
  for (int i = 1; i < argc; i++) {
    list_append(l0, (element_t) argv[i]); // 1
  }
  //list_foreach(print, l0);

  // numbers: -1 if corresponding elem in l0 is a string
  // else the number itself
  l1 = list_create();
  list_map1(isNum, l1, l0);               // 2
  //list_foreach(print_num, l1);

  // list of string containing NULL for values equal to -1
  // in l1
  l2  = list_create();
  list_map2(set_NULL, l2, l0, l1);      // 3
  //list_foreach(print, l2);

  // filter list of numbers from l1 removing all negatives
  l3 = list_create();               // 4
  list_filter(is_pos, l3, l1);
  //list_foreach(print_num, l3);

  // List of strings (l2) with NULL removed
  l4 = list_create();                           // 5
  list_filter(not_NULL, l4, l2);
  //list_foreach(print, l4);

  // List os truncated strings
  l5 = list_create();               // 6
  list_map2(trun_str, l5, l3, l4);
  list_foreach(print, l5);          // 7

  // find max value in l3
  max_val = 0;
  list_foldl(max, (element_t*) &max_val, l3);
  printf("%ld\n", max_val);

  list_destroy(l0);
  list_destroy(l1);
  list_destroy(l2);
  list_destroy(l3);
  list_destroy(l4);
  list_destroy(l5);

}