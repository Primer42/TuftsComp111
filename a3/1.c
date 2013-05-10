/*****************************************
 * William Richard
 * Comp111
 * a3
 * This program just sleeps for 2 seconds and exits
 *****************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char** argv) {
  //waste some time
  int i;
  for(i = 0; i < 2; i++) {
    sleep(1);
  }

  exit(0);
}
