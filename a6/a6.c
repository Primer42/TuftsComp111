/*************************************************
 * William Richard (wrichard)
 * Comp 111
 * Fall 2011
 * Assignment 6
 *
 * NOTE:
 * Please build using the command:
 * gcc a6.c -lrt
 *
 * Also, please make sure the constant
 * TESTFILE_DAT_LOC points to a large file
 * such as the test file of length 1000000000 bytes
 * provided.
 *
 * This program will measure both read and write
 * speed from disk cache, as well as read speed directly
 * from disk.
 * 
 * Measure write speed directly to disk is nearly impossible
 * as every write will be cached, and written to disk after
 * the write command returns.
 *
 **************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>

//make sure this is pointing to a LARGE file
#define TESTFILE_DAT_LOC "/comp/111/assign/a6/testfile.dat"

#define READFILES_LOC "./readFiles/"

#define NUM_ITERATIONS 1000
#define ONE_BILLION 1000000000
#define BUF_SIZE 100000000

//Use a standard function to consume CPU cycles
int consumeCycles(long int i, long int j) {
  return i * rand() + j / 42;
}

//just a little function that computes the difference between two
//struct timespecs.
long int subtractTimes(struct timespec* t1, struct timespec* t2) {
  struct timespec *early, *late;
  if(t1->tv_sec == t2->tv_sec) {
    if(t1->tv_nsec > t2->tv_nsec) {
      early = t2;
      late = t1;
    } else {
      early = t1;
      late = t2;
    }
  } else if(t1->tv_sec > t2->tv_sec) {
    early = t2;
    late = t1;
  } else {
    early = t1;
    late = t2;
  }

  return (late->tv_nsec + (ONE_BILLION * late->tv_sec)) - (early->tv_nsec + (ONE_BILLION * early->tv_sec));
}

/*
 * Computes the amount of time that running a for loop
 * of our consumeCycles function takes per iteration
 * This will allow us to remove this time per cycle later
 * so we are measure just the read time, as opposed to the
 * read time along with the overhead of the loop.
 */
long int getBaseLoopTime() {
  long int i,j;
  struct timespec startTime, endTime;
  clock_gettime(CLOCK_REALTIME, &startTime);

  for(i = 0; i < NUM_ITERATIONS; ++i) {
    //waste CPU cycles, so we can measure the time the loop plus wasting takes
    j = consumeCycles(i,j);
  }

  clock_gettime(CLOCK_REALTIME, &endTime);

  return subtractTimes(&startTime, &endTime) / NUM_ITERATIONS;
}

/* Calculate the rate of reading from disk cache
 *
 * This is achieved by first opening a file,
 * doing one read from it to move it into disk cache
 * then timing how long it takes to read many bytes from it.
 */
long int getDiskCacheReadRate(char* buf) {
  long int i,j;
  struct timespec startTime, endTime;
  
  //First, open the file and do the first read
  //so we know it is in the cache and not on disk
  int fd = open(TESTFILE_DAT_LOC, O_RDONLY);

  //do a read, so we know the file is in disk cache
  read(fd, buf, BUF_SIZE);
  //now, seek back to the beginning of the file
  lseek(fd, 0, SEEK_SET);

  //Start the clock
  clock_gettime(CLOCK_REALTIME, &startTime);
  //do our read
  ssize_t numRead = read(fd, buf, BUF_SIZE);
  //Stop the clock
  clock_gettime(CLOCK_REALTIME, &endTime);
  
  //close the file
  close(fd);

  //calculate the speed of access
  //divide the time taken (in nanoseconds) by the number of bytes
  //that were read, to get the time taken to read one byte
  //units are nanoseconds / byte
  return numRead * 1000.0 / subtractTimes(&startTime, &endTime);
}

/* Calculate how the rate at which we can write to the disk cache
 * This is achieve by first opening a file
 * then writing to it, as every write will be directly to cache.
 * The changes are written out of cache at a later time
 * after the write command returns
 */
double getDiskCacheWriteRate(char* buf) {
  long int i, j;
  struct timespec startTime, endTime;

  //first, open the file and do a write
  //to get it into the disk cache
  int fd = open("./writeout.dat", O_WRONLY | O_CREAT);
  
  //start the clock
  clock_gettime(CLOCK_REALTIME, &startTime);
  //write a whole bunch of stuff from memory
  ssize_t numWritten = write(fd, buf, BUF_SIZE);
  //stop the clock
  clock_gettime(CLOCK_REALTIME, &endTime);

  close(fd);
  
  //delet the file we made
  unlink("./writeout.dat");

  //return (subtractTimes(&startTime, &endTime) - (overheadPerLoop * NUM_ITERATIONS)) / NUM_ITERATIONS;
  return 1000.0 * numWritten / subtractTimes(&startTime, &endTime);
}

/* This returns the rate at which we can read directly from disk
 * This is achieved by first creating many files, and writing a lot to them.
 * This pushes any information that was in the cache out.
 * When we then proceed to read from those files, they will have
 * to be accessed from disk.
 * This means we'll be able to time how long it takes to read a byte from disk
 * A few other tricks:
 * - Keep all of the files open before reading. Opening the files does not
     move them into cache, and then we don't need to time it takes to open
     each file.
*/
double getDiskReadRate(char* buf, long int overheadPerLoop) {
  //just to be nice, make sure buf ends in a '\0'
  buf[BUF_SIZE-1] = '\0';

  //make a directory for our files
  mkdir(READFILES_LOC, 0774);

  //make a lot of new files in that directory
  long int i,j;
  int fd;
  char fileLoc[1024];
  for(i = 0; i < NUM_ITERATIONS; ++i) {
    sprintf(fileLoc, "%s%ld.dat", READFILES_LOC, i);
    fd = open(fileLoc, O_WRONLY | O_CREAT);
    write(fd, buf, BUF_SIZE/10);
    close(fd);
  }

  struct timespec startTime, endTime;
  
  //re-open all of the files
  int fds[NUM_ITERATIONS];
  for(i = 0; i < NUM_ITERATIONS; ++i) {
    sprintf(fileLoc, "%s%ld.dat", READFILES_LOC, i);
    fds[i] = open(fileLoc, O_RDONLY);
  }
  //do our reads
  char* a;
  clock_gettime(CLOCK_REALTIME, &startTime);
  for(i = 0; i < NUM_ITERATIONS; ++i) {
    j = consumeCycles(i,j);
    read(fds[i], a, 1);
  }
  clock_gettime(CLOCK_REALTIME, &endTime);

  //close and delete all of the files
  for(i = 0; i < NUM_ITERATIONS; ++i) {
    close(fds[i]);
    sprintf(fileLoc, "%s%ld.dat", READFILES_LOC, i);
    unlink(fileLoc);
  }

  //remove the directory
  rmdir(READFILES_LOC);

  //return the result
  //make sure we remove the time it takes to conume the extra cycles
  //so we ONLY measure the reads, and not the overhead of the for loop.
  return 1000.0 * NUM_ITERATIONS / (subtractTimes(&startTime, &endTime) - (overheadPerLoop * NUM_ITERATIONS));
}


int main(int argc, char** argv) {

  char* buffer = (char*) malloc(BUF_SIZE);
  
  long int loopTime = getBaseLoopTime();

  double readCacheRate = getDiskCacheReadRate(buffer);

  double writeCacheRate = getDiskCacheWriteRate(buffer);

  double diskReadRate = getDiskReadRate(buffer, loopTime);

  printf("Cache read rate is %f MB / s\n", readCacheRate);
  printf("Cache write rate is %f MB / s\n", writeCacheRate);
  printf("Disk read time is %f MB / s\n", diskReadRate);

}
