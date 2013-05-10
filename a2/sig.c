/****************************************************
 * Comp 111
 * Fall '11
 * William Richard
 * Assignment 2
 *
 * This program does the following (taken from the assignment objectives):
 * It waits for input from the user and echoes it back line-by-line. While echoing inputs, it also listens for several signals.
 *
 * First, it schedules a SIGALRM (alarm) every ten seconds, and react to this signal by counting up seconds, by printing the following:
 *
 * tick 0...  
 * tick 10...  
 * tick 20...  
 *
 * Second, the following signals evoke behaviors as indicated:
 *
 *  SIGINT prints a summary of all time spent by the program (system and user) and then continue running the program.
 *  SIGTERM prints a summary of all time spent in the program, and additionally exit the program
 *  SIGTSTP prints the last 10 lines of user input. 
 *
 *
 * Note: The system and user time sums are often 0, as most of the time this program is blocking, waiting for user input.
 ************************************************************/



#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/times.h>

int secCount; //keeps track of what the next tick count to print is
struct tms timebuf; // a struct to get user and system time
char** userInputBuf; // an array of the previous inputs of the user
const int userInputBufSize = 10; // the size of the user input buffer array
const int maxLineLength = 1000; // the maximum length we will allow for a user input line.

void printUsageSummary() {
  //get system and user time stats
  //often will be 0 because, with the fgets call, we are mostly blocking, taking up neither system nor user time.
  times(&timebuf);
  //print them
  printf("user time = %d\tsystem time = %d\n", timebuf.tms_utime, timebuf.tms_stime);
}

void addLineToUserInputBuffer(char* newLine) {
  //free the last line in the buffer
  free(userInputBuf[userInputBufSize - 1]);
  //shift all of the other lines up one
  int i;
  for(i = userInputBufSize - 1; i > 0; --i) {
    userInputBuf[i] = userInputBuf[i-1];
  }

  //put the newest line at slot 0
  userInputBuf[0] = newLine;
}

void printUserInputBuffer() {
  printf("Printing the previous %d lines of user input, in order from newest to oldest\n", userInputBufSize);
  int i;
  for(i = 0; i < userInputBufSize; ++i) {
    printf("%d:\t'%s'\n", i, userInputBuf[i]);
  }
  
}

void cleanup() {
  //free what we have malloc'd
  int i;
  for(i = 0; i < userInputBufSize; ++i) {
    free(userInputBuf[i]);
  }
  free(userInputBuf);
}


void signalHandler(int sig) {
  switch(sig) {
  case SIGALRM:
    printf("tick %d...\n", secCount);
    secCount += 10;
    alarm(10);
    break;
  case SIGINT:
    printUsageSummary();
    break;
  case SIGTERM:
    printUsageSummary();
    cleanup();
    exit(0);
    break;
  case SIGTSTP:
    printUserInputBuffer();
    break;
  }
}

int main(int argc, char** argv) {
  //initialize variables
  secCount = 0;
  userInputBuf = (char**) malloc(sizeof(char*) * userInputBufSize);
  //make sure the buf is empty
  int i;
  for(i = 0; i < userInputBufSize; ++i) {
    userInputBuf[i] = NULL;
  }

  //set up the signal handler
  signal(SIGALRM, signalHandler);
  signal(SIGINT, signalHandler);
  signal(SIGTERM, signalHandler);
  signal(SIGTSTP, signalHandler);

  //start the SIGALRM loop
  raise(SIGALRM);

  //wait until SIGTERM is sent
  while(1) {
    //make a string to store the next line of user input
    char* nextLine = (char*) malloc(sizeof(char) * maxLineLength);
    fgets(nextLine, maxLineLength, stdin);

    //remove the \n because it makes things look ugly
    for(i = 0; i < userInputBufSize; ++i) {
      if(nextLine[i] == '\n') {
	nextLine[i] = '\0';
      /* } */
      if(nextLine[i] == '\0') {
	break;
      }
    }

    printf("echo\t'%s'\n", nextLine);
    addLineToUserInputBuffer(nextLine);
  }
}
