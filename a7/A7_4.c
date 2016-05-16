#include <stdio.h>

int foo(int i, int x, int y) {
  int z;

  switch(i) {
    case 10:
      return x+y;
    case 12:
      return x-y;
    case 14:
      return ((x>y) ? 1: 0);
    case 16:
      return ((y>x) ? 1: 0);
    case 18:
      return ((x==y) ? 1: 0);
    default:
      return 0;
  }
}
