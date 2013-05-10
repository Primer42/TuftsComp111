#include <pthread.h>
#include <sys/timeb.h>
#include <stdlib.h>
#include <stdio.h>


typedef struct _ts_ring_buffer {
  int head;
  int end;
  struct timeb* elements;
  int numElements;
  pthread_mutex_t modLock;
} RingBuffer;

void rbCreate(int bufSize, RingBuffer* rb);
void rbDelete(RingBuffer* rb);

struct timeb enqueue(RingBuffer* rb, int bufNum);
struct timeb dequeue(RingBuffer* rb, struct timeb expectedHeadValue, int bufNum);

struct timeb peek(RingBuffer rb);
