#include "/comp/111/assignments/a4/anthills.h" 

#include "ring_buffer.h"

#include <pthread.h>
#include <semaphore.h>
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

  pthread_mutex_lock(&antsLeftLock);
  if(antsLeft[hillNum] <= 0) {
    pthread_mutex_unlock(&antsLeftLock);
    return 0;
  }
  antsLeft[hillNum] = antsLeft[hillNum] - 1;
  pthread_mutex_unlock(&antsLeftLock);

  if(!slurp(aardvarkName, hillNum)) {
    //did not manage to eat an ant
    //increment the counter
    pthread_mutex_lock(&antsLeftLock);
    antsLeft[hillNum] = antsLeft[hillNum] + 1;
    pthread_mutex_unlock(&antsLeftLock);
  }
  
  return 1;
}


void *thread_A(void *input) { 
    char aname = *(char *)input; // name of aardvark, for debugging
    initialize();
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
