// How to game the OS into not counting your computation: 
// Strategy is to do a little work and then sleep until the next tick. 
// this works because nanosleep(zero,NULL) sleeps until the next 
// accounting tick! So you can do a little work and then 
// sleep until *after* the clock has ticked! 

#define _POSIX_C_SOURCE 199309 
#include <time.h>
#include <sys/time.h> 
#include <stdio.h> 

struct timespec zero = {0,0}; // sleep for zero nanoseconds=after next tick!
typedef unsigned long cycle_t; 

// a sneaky trick to get the number of elapsed cycles of the high-resolution
// clock really quickly by dropping into assembler. Much faster than 
// clock_gettime(2) system call. 
inline cycle_t get_cycles()
{
    cycle_t ret;
    asm volatile("rdtsc" : "=A" (ret));
    return ret;
}

// calculate the cycles of the high resolution clock per accounting clock tick, 
// by waiting through 1000 ticks and dividing. 
cycle_t cycles_per_tick()
{
    int i; 
    unsigned long finish; 
    unsigned long start; 
    unsigned long elapsed; 
    const unsigned long hundred = 100; 
    nanosleep(&zero,NULL); // sync with tick
    start = get_cycles();
    for(i=0 ; i<100 ; i++)
        nanosleep(&zero,NULL);
    finish = get_cycles(); 
    elapsed = finish - start; 
    elapsed &= 0xffffffff; // wrap limit
    return (unsigned long)elapsed/(unsigned long)hundred;
}

int main() { 
    int i; 
    cycle_t stuff; 
    for (i=0; i<1000; i++) { 
        stuff = cycles_per_tick(); 
	fprintf(stderr,"cycles_per_tick() is %lu\n",stuff); 
    } 
} 
