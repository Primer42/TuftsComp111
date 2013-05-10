/********************************************
 * Comp 111 Assignment 1
 * William Richard (wrichard)
 * This code runs Conway's Game of Life, 
 * but does not appear to use any CPU time.
 ********************************************/

#include <stdio.h> 
#include <string.h> 
#include <malloc.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/times.h>
#include <time.h>

#define TRUE 1
#define FALSE 0 
#define BUFSIZE 300

// #define DEBUG 1

/* global variables contain grid data */ 
int **grid; 
int **neighbors;
int rows; int cols;
//global variables for which cell we are on
int r; int c;


/* make a grid of cells for the game of life */ 
int **make_grid(int rows, int cols) { 
    int **out = (int **)malloc (rows*sizeof (int *)); 
    int a; 
    for (a=0; a<rows; a++) {
	out[a] = (int *)malloc (cols * sizeof(int)); 
    } 

    return out; 
} 

/* make all cells non-living */ 
void zero_grid(int **cells, int rows, int cols) { 
    int r, c; 
    for (r=0; r<rows; r++) 
	for (c=0; c<cols; c++) 
	    cells[r][c]=0; 
} 

/* read a grid of cells from a text file */ 
int **read_grid(FILE *f, int *rows, int *cols) { 
   char buffer[BUFSIZE]; 
    fgets(buffer, BUFSIZE, f); 
    while (buffer[0]=='#') fgets(buffer, BUFSIZE, f); 
    if (sscanf(buffer,"x = %d, y = %d",cols,rows)==2) { 
	int **grid = make_grid(*rows, *cols); 
	int r=0; 
	zero_grid(grid, *rows, *cols); 
	while (! feof(f) && r<(*rows)) { 
	    int c; 
	    fgets(buffer, BUFSIZE, f); 
	    for (c=0; c<BUFSIZE && c<(*cols) 
              && buffer[c] != '\n' && buffer[c] != '\0'; ++c) { 
		if (buffer[c]=='.' || buffer[c]==' ') { 
		    grid[r][c]=0; 
		} else { 
		    grid[r][c]=1; 
		} 
	    } 
	    ++r; 
	} 
	return grid; 
    } else { 
	fprintf(stderr, "first line does not contain grid dimensions\n"); 
	return NULL; 
    } 
} 

void print_grid_info(FILE* fp, int rows, int cols) {
    fprintf(fp, "x = %d, y = %d\n", cols, rows);   
}

//int print_next(FILE* fp, int** grid, int rows, int cols) {
int print_next(FILE* fp) {
	//start with a bounds check
	//just check if c==cols - r will always reset before r == rows
	if(r == rows) return FALSE;

	//print r,c
	fprintf(fp,(grid[r][c]?"*":".")); 

 	//update r and c
	if(++c == cols) {c = 0; ++r; fprintf(fp, "\n");}
	return TRUE;
}

void free_grid(int **grid, int rows) { 
    int i; 
    for (i=0; i<rows; i++) 
        free(grid[i]); 
    free(grid); 
} 

int **grid = NULL; 
int **neighbors = NULL; 
enum FUNCTION_TO_CALL { Count, UpMort, Print };
int nextFunction;

//int countNeighbors(int** cells, const int rows, const int cols) {
int countNeighbors() {
  //count the neighbors for r,c
  //start with a bounds check
  //just check if c==cols - r will always reset before r == rows
  if(c == cols)
    return FALSE;
  
  int n = 0; 
  if (c>0 && grid[r][c-1]) n++; 
  if (r>0 && c>0 && grid[r-1][c-1]) n++; 
  if (r>0 && grid[r-1][c]) n++; 
  if (r>0 && c<cols-1 && grid[r-1][c+1]) n++; 
  if (c<cols-1 && grid[r][c+1]) n++; 
  if (r<rows-1 && c<cols-1 && grid[r+1][c+1]) n++; 
  if (r<rows-1 && grid[r+1][c]) n++; 
  if (r<rows-1 && c>0 && grid[r+1][c-1]) n++; 
  neighbors[r][c]=n; 

  //update r & c
  if(++r == rows) {r = 0; ++c;}
  return TRUE;
}

//int updateCellMortality(int** cells, const int rows, const int cols) {
int updateCellMortality() {
  //start with a bounds check
  //just check if c==cols - r will always reset before r == rows
  if(c == cols) return FALSE;
  
  //Kill any live cell with <2 or >3 neighbors
  if(grid[r][c] && neighbors[r][c]<2 || neighbors[r][c]>3)
    grid[r][c] = FALSE;
  //Animate any dead cell with 3 neighbors
  if(!grid[r][c] && neighbors[r][c] == 3)
    grid[r][c] = TRUE;

  //update r and c
  if(++r == rows) {r = 0; ++c;}
  return TRUE;
}

/*******************************************
Cheater code is here!!!
Based on the paper "Secreting Monopolizing the CPU Without Superuser Privileges"
by Tsafrir, Etsoin, and Feitelson.
********************************************/
struct timespec zero = {0,0};  //When we call nanosleep, we want it to leep for 0 nanoseconds. This defaults to restarting after the next tick.
typedef unsigned long cycle_t; //used for counting how many cycles have passed

/* 2 tricks used to get the number of elapsed cycles at a high resolution.
 * First is calling this function as an inline function, hinting to the 
 * compliler to run it as fast as possible.
 * Second is drop into assemly language. Much faster than any other call.
 */
inline cycle_t get_cycles() {
  cycle_t ret;
  asm volatile("rdtsc" : "=A" (ret));
  return ret;
}

/* Calculate the cycles of a clock tick in high resolution using the get_cycles function
 * by waiting through a bunch of ticks and dividing
 */
cycle_t calc_cycles_per_tick()
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


/* read an image and generate a result 
 * Use the cheating methods to do it without the OS or any users knowing
 */ 
int main(int argc, char **argv) {
   //int rows=0; int cols=0; 
	rows = 0; cols = 0;
	int iterations=0; int i = 0;
   r = 0; c = 0;

   //self-regulate the amount of a tick we take up.
   //start small - will get bigger later
   float startWorkPercentage = .25;

   //set up how many cycles we should work, as a percentage of the cycles per tick.
   cycle_t numCyclesPerTick = calc_cycles_per_tick();
   cycle_t numCyclesToWork = numCyclesPerTick * startWorkPercentage;
   cycle_t numCyclesToWorkInc = numCyclesToWork * .05;
   int adjustNumCyclesToWork = TRUE;
   cycle_t tickStart, cyclesSinceLastTick;

   //check the arguments and read in the grid
   //this may take too long - possibly don't do it all in one go
   if (argc<2 || !(grid=read_grid(stdin,&rows,&cols)) 
     || (iterations=atoi(argv[1]))<0) {
	fprintf(stderr,"life usage:  life iterations <inputfile\n");
	exit(1);
   }
   //make the neighbors grid
   //also may take too long - possibly don't do in one go
   neighbors = make_grid(rows,cols);
   cycle_t timingStart; cycle_t timingEnd;

   //start running the simulation
   //sync with start of tick
   nanosleep(&zero, 0);
   tickStart = get_cycles();

   int done = FALSE;
   while(!done) {
     //check if we still have time to run the next step
     cyclesSinceLastTick = get_cycles() - tickStart;
     if(cyclesSinceLastTick >= numCyclesToWork) {
       //make sure we haven't overstayed our welcome
       if(cyclesSinceLastTick >= numCyclesPerTick) {
	 //decrease the work amount
	 numCyclesToWork -= 3*numCyclesToWorkInc;
	 if(numCyclesToWork <= 0) numCyclesToWork = 1;
	 //stop adjusting the work percentage
	 adjustNumCyclesToWork = FALSE;
       }
       
       //if we haven't already found our happy place, increase the work percentage
       if(adjustNumCyclesToWork) {
	 numCyclesToWork = numCyclesToWork + numCyclesToWorkInc;
	 if(numCyclesToWork >= numCyclesPerTick) numCyclesToWork = numCyclesPerTick * .95;
       }
       
       //avoid the bill
       while(nanosleep(&zero, 0) < 0) {};
       tickStart = get_cycles();
     }

     timingStart = get_cycles();
	
     //do some work
     //switch depending on if we're counting neighbors or killing/breeding cells, or printing the grid
     if(nextFunction == Count) {
	if(!countNeighbors()) {
	 //done counting neighbors
	 //so reset r and c so we can start updating cell mortality
	 r = 0;
	 c = 0;
	 nextFunction = UpMort;
       }
     }
     else if (nextFunction == UpMort) {
	if(!updateCellMortality()) {
	 //done updating cell mortality
	 //reset for starting to count neighbors again
	 //also, increment the iteration count, because we are done with this iteration
	 r = 0;
	 c = 0;
	 ++i;
	 //set the next function based on if we are done iterating
	 if(i < iterations)
	   nextFunction = Count;
	 else
	   nextFunction = Print;
       }
     }
     else if (nextFunction == Print) {
	if(r == 0 && c == 0)
	  print_grid_info(stdout, rows, cols);
	if(! print_next(stdout)) {
	  done = TRUE;
	}
     }

     timingEnd = get_cycles();
   }

   //all done - clean up.
   free_grid(grid,rows);
   free_grid(neighbors,rows);
} 
