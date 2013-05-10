/*****************************************
 * William Richard
 * Comp111
 * a3
 *
 * To Compile - simply execute make to run the included Makefile.
 * This will compile watch, 1, 2, 3, and 4.
 *
 * Watch handles each of the cases in the following ways:
 * Child Death:
 * Child death will send a SIGCHLD signal to the parent
 * This is caught by the childReaper function.
 * The number of lines printed is set by the loop that reads from the pipe
 * The total runtime is measured by starting a timer just after the fork() command
 * and then looking at how much time has passed since the fork() command was
 * executed.
 * The time of death is simply measured by making a call to time(NULL).
 * This will not work if the child manages to not send a SIGCHLD signal when
 * it quits.  I do not think this is possible.
 *
 * Too much stack memory or CPU usage.
 * These are handled by setting a resource limit in the child process
 * before calling execl.
 * To my knowledge, there is no way to avoid dying because of a
 * resource limit.
 *
 * Printing more than 1000 lines:
 * This is handled using a pipe and dup.
 * The parent has the read pipe, and the child's stdout
 * is dup'd to be the write side of the pipe.
 * Every line the child writes will (eventually) be read on the parent's
 * side of the pipe.
 * The problem with this is that pipes, like every other file, are
 * page buffered, so unless the child is nice and calls fflush after every
 * printf, the parent will not see the output until either an entire page is
 * written or the child exits.
 * Calling fflush is not a guaranteed way for the parent to immediately see the output.
 * The prints are so quick that the parent does not have enough time to read 
 * one line before the child exits.
 * Therefore, the child will exit before the parent gets the first line via the pipe,
 * even if fflush is called.
 * This can be called by sleeping between each print, but that does not seem 
 * like a viable option.
 *
 * The child may also avoid being killed for printing too many lines 
 * by dup-ing it's own stdout.  The lines will not be printed to the terminal,
 * but it will be able to "printf" as much as it wants, and not be assassinated.
 */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define FALSE 0
#define TRUE 1

int done = FALSE;

time_t childStartTime;

int numLinesChildPrinted = 0;

void childReaper(int sig) {
  time_t timeOfDeath = time(NULL);
  int status;
  struct rusage usage;
  pid_t childPid = wait3(&status, 0, &usage);
  printf("Child %d has died with status %d\n", childPid,  status);
  printf("Total runtime = %ld seconds\n",  (time(NULL) - childStartTime));
  printf("Used %ld.%ld seconds of CPU time total\n", (usage.ru_utime.tv_sec + usage.ru_stime.tv_sec), (usage.ru_utime.tv_usec + usage.ru_stime.tv_sec));
  printf("Printed %d lines\n", numLinesChildPrinted);
  printf("Time of death = %s\n", ctime(&timeOfDeath));
  done = TRUE;
}


int main(int argc, char** argv) {
  if(argc != 2) {
    printf("Usage: %s <executable to watch>\n", argv[0]);
    printf("Current, this cannot handle arguments for the watched program\n");
    exit(1);
  }
  signal(SIGCHLD, childReaper);

  int fd[2];
  pipe(fd);

  int childPid;
  if((childPid = fork())) {
    //parent
    childStartTime = time(NULL);
    //start reading from our pipe
    int bufSize = 1024;
    char* buf = (char*) malloc(sizeof(char) * bufSize);
    //open the read side of the pipe, and close the write side
    FILE* readPipe = fdopen(fd[0], "r");
    close(fd[1]);
    //start waiting for input
    while((fgets(buf, bufSize, readPipe)) != NULL) {
      ++numLinesChildPrinted;
      printf("%d: %s", numLinesChildPrinted, buf);
      if(numLinesChildPrinted >= 1000) {
	//bad program printed too much
	//make it quit
	printf("Making it quit\n");
	kill(childPid, SIGKILL); //SIGKILL cannot be caught and not die
      }
    }

    while(!done) {
      sleep(1);
    }    

  } else {
    //child
    //set the resource limits
    //start with stack memory
    //max of 4 MB = 4000000 B
    struct rlimit l;
    l.rlim_cur = 4000000;
    l.rlim_max = 4000000;
    setrlimit(RLIMIT_STACK, &l);
    //CPU limit
    //max of 1 sec
    l.rlim_cur = 1;
    l.rlim_max = 1;
    setrlimit(RLIMIT_CPU, &l);

    //start sending anything it sends to stdout to the write side of our pipe
    close(1); //closes 1 = stdout
    dup(fd[1]); //copy the write side of the pipe into 1
    close(fd[0]); //close the other side of the pipe for the child.
  
    execl(argv[1], argv[1], NULL);
    printf("We should never get here\n");
  }

  exit(0);

}
