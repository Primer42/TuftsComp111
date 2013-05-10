/*****************************************
 * William Richard
 * Comp111
 * a3
 * This program just prints. A lot.
 *****************************************/
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

int main(int argc, char** argv) {

  int BEHAVE = 1;

  if(!BEHAVE) {
    FILE* out = fopen("./out.txt", "w");
    close(1);
    dup(fileno(out));
  }

  int i;
  for(i = 0; i < 2000; ++i) {
    printf("%ld\t%d\n", time(NULL), i);
    if(BEHAVE) {
      fflush(stdout);
    }
  }
  printf("All done - you didn't stop me - HA\n");
  exit(0);

}
