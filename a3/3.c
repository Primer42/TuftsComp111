/*****************************************
 * William Richard
 * Comp111
 * a3
 * This program just eats CPU cycles.
 *****************************************/
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

int main(int argc, char** argv) {

  unsigned long i ;
  int k;
  FILE* f = fopen("./3data.txt", "w");
  for(i = 0; i < 5000000; i++) {
    k += (i+42)*42 + (k*42) + 42 * i;
    fprintf(f, "%ld\t%u\n", time(NULL), k);
    fflush(f);
  }

  exit(0);

}
