#include "ring_buffer.h"

#define TRUE 1
#define FALSE 0
#define DEBUG FALSE

void rbCreate(int bufSize, RingBuffer* rb) {

  /* RingBuffer* rb = (RingBuffer*) malloc(sizeof(RingBuffer)); */
  //always keep one slot open, so we need to allocate an extra slot
  ++bufSize;
  rb->numElements = bufSize;
  rb->elements = (void *) malloc(sizeof(struct timeb) * bufSize);
  rb->head = 0;
  rb->end = 0;
  pthread_mutex_init(&(rb->modLock), NULL);

  /* return rb; */
}

void rbDelete(RingBuffer* rb) {
  //free the element array
  free(rb->elements);
  //destroy the lock
  pthread_mutex_destroy(&(rb->modLock));
  //free the ring buffer
  free(rb);
}

int full(RingBuffer rb) { return ((rb.end + 1) % rb.numElements) == rb.head;}
int empty(RingBuffer rb) { return rb.end == rb.head; }

struct timeb enqueue(RingBuffer* rb, int bufNum) {
  //don't let an enqueue go if the queue is full
  pthread_mutex_lock(&(rb->modLock));
  struct timeb returnVal;
  if(full(*rb)) {
    returnVal.time = 0;
    returnVal.millitm = 0;
    if(DEBUG)
      printf("%d: enqueue failed. Queue full.", bufNum);
  } else {
    //add the current time to the queue, and return that value
    ftime(&returnVal);
    rb->elements[rb->end] = returnVal;
    rb->end = (rb->end + 1) % rb->numElements;
    if(DEBUG)
      printf("%d: just enqueued.", bufNum);
  }
  
  if(DEBUG) {
    printf(" Buffer is now {");
    int i;
    for(i = 0; i < rb->numElements; ++i) {
      if(i == rb->head)
	printf("H");
      if(i == rb->end)
	printf("E");
      printf("[%ld.%hd] ", rb->elements[i].time, rb->elements[i].millitm);
    }
    printf("}\n");
  }

  pthread_mutex_unlock(&(rb->modLock));
  return returnVal;
}

struct timeb dequeue(RingBuffer* rb, struct timeb expectedHeadValue, int bufNum) {
  pthread_mutex_lock(&(rb->modLock));
  if(DEBUG)
    printf("Starting dequeue\n");

  struct timeb returnValue;

  if(empty(*rb)) {
    if(DEBUG)
      printf("%d: failed to dequeue because it is empty. ", bufNum);
    returnValue.time = 0;
    returnValue.millitm = 0;
  } else {
    returnValue = rb->elements[rb->head];
    if(expectedHeadValue.time != returnValue.time && expectedHeadValue.millitm != returnValue.millitm) {
      returnValue.time = 0;
      returnValue.millitm = 0;
    } else {
  
      rb->head = (rb->head + 1) % rb->numElements;
      if(DEBUG)
	printf("%d: just dequeued. ", bufNum);
    }
  }
  if(DEBUG) {
    printf("Buffer is now:{");
    int i;
    for(i = 0; i < rb->numElements; ++i) {
      if(i == rb->head)
	printf("H");
      if(i == rb->end)
	printf("E");
      printf("[%ld.%hd] ", rb->elements[i].time, rb->elements[i].millitm);
    }
    printf("}\n");
  }

  pthread_mutex_unlock(&(rb->modLock));

  return returnValue;
}

struct timeb peek(RingBuffer rb) {
  pthread_mutex_lock(&(rb.modLock));
  struct timeb peekedVal;
  if(empty(rb)) {
    peekedVal.time = 0;
    peekedVal.millitm = 0;
  }
  else {
    peekedVal = rb.elements[rb.head];
  }
  pthread_mutex_unlock(&(rb.modLock));
  return peekedVal;
}
