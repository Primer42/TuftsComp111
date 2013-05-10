/*****************************************
 * William Richard
 * Comp111
 * a3
 * This program allocates a lot of memory.
 *****************************************/

#include <stdlib.h>
#include <stdio.h>

int main(int argc, char** argv) {
  //try to allocate lots of memory
  int array[4000000];
  int i;
  for(i = 0; i < 4000000; ++i) {
    array[i] = 0;
  }
  printf("Malloc'd everything\n");

  exit(0);
}
