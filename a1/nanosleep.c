#define _POSIX_C_SOURCE 199309 
#include <time.h>
#include <stdio.h> 
// int nanosleep(const struct timespec *req, struct timespec *rem);

int main(int argc, char **argv) { 
    struct timespec request, remainder; 
    int result; 
    request.tv_sec=20; 
    request.tv_nsec=25;
    result=nanosleep(&request, &remainder); 
    printf("result is %d\n", result); 
} 
