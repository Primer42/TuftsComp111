#include <stdio.h> 
#include <string.h> 
#include <malloc.h>
#include <stdlib.h>
#include <unistd.h>

#define TRUE 1
#define FALSE 0 
#define BUFSIZE 300

// #define DEBUG 1

/* global variables contain grid data */ 
int **grid; 
int **neighbors; 

/* make a grid of cells for the game of life */ 
int **make_grid(rows,cols) { 
    int **out = (int **)malloc (rows*sizeof (int *)); 
    int r; 
    for (r=0; r<rows; r++) {
	out[r] = (int *)malloc (cols * sizeof(int)); 
// if (!out[r]) fprintf(stderr,"allocation of row %d failed\n",r); 
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

/* print a grid in a form that can be read back in */ 
void print_grid(FILE *fp, int **g,int rows,int cols) { 
    int r, c; 
    fprintf(fp, "x = %d, y = %d\n", cols, rows); 
    for (r=0; r<rows; r++) {
	for (c=0; c<cols; c++) { 
	    fprintf(fp,(g[r][c]?"*":".")); 
	}
	printf("\n"); 
    } 
} 

void free_grid(int **grid, int rows) { 
    int i; 
    for (i=0; i<rows; i++) 
        free(grid[i]); 
    free(grid); 
} 

int **grid = NULL; 
int **neighbors = NULL; 

void next(int **cells, int rows, int cols) { 
   int r, c; 
   for (r=0; r<rows; r++) {
      for (c=0; c<cols; c++) { 
	  int n = 0; 
	  if (c>0 && cells[r][c-1]) n++; 
	  if (r>0 && c>0 && cells[r-1][c-1]) n++; 
	  if (r>0 && cells[r-1][c]) n++; 
	  if (r>0 && c<cols-1 && cells[r-1][c+1]) n++; 
	  if (c<cols-1 && cells[r][c+1]) n++; 
	  if (r<rows-1 && c<cols-1 && cells[r+1][c+1]) n++; 
	  if (r<rows-1 && cells[r+1][c]) n++; 
	  if (r<rows-1 && c>0 && cells[r+1][c-1]) n++; 
	  neighbors[r][c]=n; 
      } 
   } 
   for (r=0; r<rows; r++) {
      for (c=0; c<cols; c++) { 
	  /* any live cell with < 2 or > 3 neighbors dies */ 
	  if (cells[r][c] && neighbors[r][c]<2 || neighbors[r][c]>3) 
	      cells[r][c] = FALSE; 
          /* any dead cell with three neighbors lives */ 
          else if (!cells[r][c] && neighbors[r][c]==3) 
	      cells[r][c] = TRUE; 
      } 
   } 
} 

/* read an image and generate a result */ 
int main(int argc, char **argv) {
   int rows=0; int cols=0; int iterations=0; int i; 
   if (argc<2 || !(grid=read_grid(stdin,&rows,&cols)) 
     || (iterations=atoi(argv[1]))<0) {
	fprintf(stderr,"life usage:  life iterations <inputfile\n");
	exit(1);
   }
#ifdef DEBUG
   printf("input:\n"); print_grid(stdout,grid,rows,cols);
#endif /* DEBUG */ 
   neighbors = make_grid(rows,cols); 
   for (i=0; i<iterations; i++) {
	next(grid, rows, cols);
#ifdef DEBUG
        printf("next\n"); print_grid(stdout,grid,rows,cols);
#endif /* DEBUG */ 
   } 
   print_grid(stdout,grid,rows,cols); free_grid(grid,rows);
   free_grid(neighbors,rows);
} 
