/*********************************************************************
 * William Richard
 * Comp 111
 * Assignment 4
 *
 * This code specifies the behavior of 8 aardvarks.
 * They all call the same function, eatAtHill, after the appropriate
 * variables have been initialzed by aardvark A
 * They each start at a certain hill, and then cycle through the hills
 * as they eat, resulting in the most even distribution of ants
 * being eaten from each hill possible.
 *
 * eatAtHill uses a custom ring buffer.
 * When enqueue is called on this ring buffer, the current time is
 * enqueued, if the buffer is not full.
 * An additional function, peek, is added to the buffer. It returns
 * the time storied at the head of the buffer.
 * Dequeue requires that the caller passes the time they intend to 
 * dequeue, as discovered by peek.  If the time requesetd is not
 * at the head of the queue, the dequeue fails.
 *
 * This ensures that only one thread is able to dequeue each value,
 * as this causes a race condition to see who calls dequeue first
 * trying to retrieve each value.
 * 
 * This is used as such:
 * An aardvark is only able to start slurping from a hill when it sucessfully
 * enqueues the current time to the ring buffer corresponding to the hill
 * it wants to eat from.
 * When there are the maximum aardvarks already slurping from a hall,
 * enqueue will fail as the buffer will be full.
 * If another aardvark tries to enqueue to that buffer, they will fail.
 * They will then peek at the head of the buffer, and wait until the 
 * amount of time it takes to slurp an ant after the value they peeked 
 * at has passed.
 * At which point, they will try to dequeue the time they peeked at,
 * and then attempt to enqueue again.
 * Since they just dequeued the time they peeked at, the enqueue should 
 * suceed.
 * This processe continues, which each anteater eating at all hills in
 * turn, until all ants are consumed.
 *
 * HOW TO COMPILE AND RUN
 *
 * There should be 4 files provided:
 * - aardvarks.c
 * - ring_buffer.c
 * - ring_buffer.h
 * - Makefile
 *
 * If you call make on the Makefile, ant anthills.o is in the current 
 * directory, it should make an executable "ants"
 * You then may call ants in the normal fashion:
 *./ants usage: ./ants [debug] [trace] [quiet] [csv]
 * debug: print detailed debugging information
 * trace: print a trace of system state
 * quiet: suppress running narrative
 * csv: write state log to output.csv
 *********************************************************************/

#include "/comp/111/assignments/a4/anthills.h" 

#include "ring_buffer.h"

#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/timeb.h>
#include <time.h>
#include <unistd.h>

#define DEBUG FALSE

int READY = FALSE;

int* antsLeft;
pthread_mutex_t antsLeftLock;
RingBuffer* anthillRingBuffers;

void initialize (){ 
  int i;
  anthillRingBuffers = (RingBuffer*) malloc(sizeof(RingBuffer) * ANTHILLS);
  antsLeft = (int*) malloc(sizeof(int) * ANTHILLS);
  pthread_mutex_init(&antsLeftLock, NULL);
  for(i = 0; i < ANTHILLS; ++i) {
    rbCreate(AARDVARKS_PER_HILL, &(anthillRingBuffers[i]));
    antsLeft[i] = ANTS_PER_HILL;
  } 

  READY = TRUE;
}

int eatAtHill(const int hillNum, char aardvarkName) {
  //try to enqueue a time in the right anthill ring buffer
  while(! ( ( enqueue(&(anthillRingBuffers[hillNum]), hillNum) ).time ) ) {
    if(DEBUG)
      printf("%c failed enqueue on hill %d with %d ants left\n", aardvarkName, hillNum, antsLeft[hillNum]);
    if(antsLeft[hillNum] <= 0) {
      return 0;
    }
    //we could not enqueue - buffer is full
    //so anthill is full
    struct timeb headValue = peek(anthillRingBuffers[hillNum]);
    struct timeb endSlurpTime;
    endSlurpTime.time = headValue.time + SLURP_TIME;
    endSlurpTime.millitm = headValue.millitm;
    //sleep until the current head anteater is done
    struct timeb curTime;
    ftime(&curTime);
    if(curTime.time < endSlurpTime.time || curTime.millitm < endSlurpTime.millitm) {
      struct timespec sleepTime;
      sleepTime.tv_sec = (time_t) endSlurpTime.time - curTime.time;
      sleepTime.tv_nsec = (long) (labs(endSlurpTime.millitm - curTime.millitm + 10) * 1000000);
      if(DEBUG)
	printf("%c sleeping for %lu.%lu seconds\n", aardvarkName, sleepTime.tv_sec, sleepTime.tv_nsec );
      nanosleep(&sleepTime, NULL);
    }
    //try dequeuing the value that was at the head
    //if we succeed, the enqueue in the while condition will suceed, and we'll
    //be in business to slurp
    //otherwise, we'll go through the loop again
    dequeue(&(anthillRingBuffers[hillNum]), headValue, hillNum);
  }

  if(DEBUG)
    printf("Aardvark %c is going to eat with from hill %d with %d ants left\n", aardvarkName, hillNum, antsLeft[hillNum]);

  //double check that there are ants left to eat
  //otherwise, decrease the count of ants that are left, as we are about
  //to eat an ant.
  pthread_mutex_lock(&antsLeftLock);
  if(antsLeft[hillNum] <= 0) {
    pthread_mutex_unlock(&antsLeftLock);
    return 0;
  }
  antsLeft[hillNum] = antsLeft[hillNum] - 1;
  pthread_mutex_unlock(&antsLeftLock);

  if(!slurp(aardvarkName, hillNum)) {
    //did not manage to eat an ant
    //increment the counter of ants left
    //so it is accurate.
    pthread_mutex_lock(&antsLeftLock);
    antsLeft[hillNum] = antsLeft[hillNum] + 1;
    pthread_mutex_unlock(&antsLeftLock);
  }
  
  return 1;
}


void *thread_A(void *input) { 
    char aname = *(char *)input; // name of aardvark, for debugging
    initialize(); // this is the only line that is unique to this thread
    while(!READY) {}
    int curHill = 0;
    while (chow_time()) {
      //cycles through, starting at 0
      if(!eatAtHill(curHill, aname))
	return NULL;
      curHill = (curHill+1) % ANTHILLS;
    }
    return NULL; 
} 
void *thread_B(void *input) { 
    char aname = *(char *)input; // name of aardvark, for debugging
    while(!READY) {}
    int curHill = 1;
    while (chow_time()) {
      //cycles through, starting at 1
      if(!eatAtHill(curHill, aname))
	return NULL;      
      curHill = (curHill + 1) % ANTHILLS;
    }
    return NULL; 
} 
void *thread_C(void *input) { 
    char aname = *(char *)input; // name of aardvark, for debugging
    while(!READY) {}
    int curHill = 2;
    while (chow_time()) {
      //cycles through, starting at 2
      if(!eatAtHill(curHill, aname))
	return NULL;
      curHill = (curHill + 1) % ANTHILLS;
    }
    return NULL; 
} 
void *thread_D(void *input) { 
    char aname = *(char *)input; // name of aardvark, for debugging
    while(!READY) {}
    int curHill = 0;
    while (chow_time()) {
      //cycles through, starting at 0
      if(!eatAtHill(curHill, aname))
	return NULL;
      curHill = (curHill + 1) % ANTHILLS;
    }
    return NULL; 
} 
void *thread_E(void *input) { 
    char aname = *(char *)input; // name of aardvark, for debugging
    while(!READY) {}
    int curHill = 1;
    while (chow_time()) {
      //cycles through, starting at 1
      if(!eatAtHill(curHill, aname))
	return NULL;
      curHill = (curHill + 1) % ANTHILLS;
    }
    return NULL; 
} 
void *thread_F(void *input) { 
  char aname = *(char *)input; // name of aardvark, for debugging
  while(!READY) {}
  int curHill = 2;
  while (chow_time()) {
    //cycles through, starting at 2
    if(!eatAtHill(curHill, aname))
      return NULL;
    curHill = (curHill + 1) % ANTHILLS;
  }
  return NULL; 
} 
void *thread_G(void *input) { 
  char aname = *(char *)input; // name of aardvark, for debugging
  while(!READY) {}
  int curHill = 0;
  while (chow_time()) {
    //cycles through, starting at 0
    if(!eatAtHill(curHill, aname))
      return NULL;
    curHill = (curHill + 1) % ANTHILLS;
  }
  return NULL; 
} 
void *thread_H(void *input) { 
  char aname = *(char *)input; // name of aardvark, for debugging
  while(!READY) {}
  int curHill = 1;
  while (chow_time()) {
    //cycles through, starting at 1
    if(!eatAtHill(curHill, aname))
      return NULL;
    curHill = (curHill + 1) % ANTHILLS;
  }
  return NULL; 
} 
