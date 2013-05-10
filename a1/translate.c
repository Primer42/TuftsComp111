#include <stdio.h> 
#include <string.h> 
#include <malloc.h>
#include <stdlib.h>
#include <unistd.h>

#define TRUE 1
#define FALSE 0 
#define BUFSIZE 300

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
	int size; 
	int r=0; 
	zero_grid(grid, *rows, *cols); 
	while (! feof(f)) { 
	    int c; 
	    fgets(buffer, BUFSIZE, f); 
	    size = strlen(buffer); 
	    for (c=0; c<BUFSIZE && c<size && c<*cols; c++) { 
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
	fprintf(stderr, "first line does not contain dimensions\n"); 
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

/* The body of the RLE image consists of any number of compressed
runs.  A run consists of a single character describing the kind of run
(optionally preceded by an integer value representing a repeat count,
if other than 1).  The character can be $ representing a new-line, the
letter b representing a blank (empty) cell, or the letter o
representing a living cell. */ 

/* Example of an RLE file
x = 106, y = 109
4bo17bo37bo$5bo17bo35bo$3b3o15b3o35b3o$50bo$49bo$49b3o3$17bobo$18b2o$
18bo5$62bo15b2x24b2x$61bo16b2x24b2x$61b3o22b2x$85bx2bx$6bo79b2x$7bo83b
x$5b3o82bxbx$89bx3bx$90bx2bx2$90bx2bx$90bx3bx$56b3o32bxbx$56bo35bx$57b
o38b2x$95bx2bx$3o93b2x$2bo75b2x24b2x$bo76b2x24b2x4$45b3o$15b3o27bo$17b
o28bo$16bo3$12b3o$14bo$13bo$2b3o35b3o15b3o$4bo35bo17bo$3bo37bo17bo12$
4bo17bo37bo$5bo17bo35bo$3b3o15b3o35b3o$50bo$49bo$49b3o$41bo$41bobo$41b
2o$16bo$17bo$15b3o4$62bo15b2x24b2x$61bo16b2x24b2x$61b3o22b2x$85bx2bx$
6bo79b2x$7bo83bx$5b3o82bxbx$89bx3bx$90bx2bx2$90bx2bx$90bx3bx$56b3o32bx
bx$56bo35bx$57bo38b2x$95bx2bx$3o93b2x$2bo75b2x24b2x$bo76b2x24b2x4$46b
3o$46bo$47bo$21b2o$20bobo$22bo$12b3o$14bo$13bo$2b3o35b3o15b3o$4bo35bo
17bo$3bo37bo17bo!
 */

int **read_rle(FILE *f, int *rows, int *cols) { 
    int **out; 
    char buffer[BUFSIZE]; 
    fgets(buffer, BUFSIZE, f); 
    while (!feof(f) && buffer[0]=='#') fgets(buffer, BUFSIZE, f); 
    if (sscanf(buffer,"x = %d, y = %d",cols,rows)==2) { 
	int r=0; int c=0; 
	out = make_grid(*rows,*cols); 
	zero_grid(out,*rows,*cols); 
	while (!feof(f)) { 
	    char *cp; 
	    int count=1; 
	    fgets(buffer,BUFSIZE, f); 
#ifdef DEBUG
	    fprintf(stderr, "processing line '%s'\n", buffer); 
#endif /* DEBUG */ 
	    cp=buffer; 
	    while (*cp != '\n' && *cp != '\0') {
		if (*cp>='1' && *cp<='9') { 
		    count=(atoi(cp)); 
		    while (*cp>='0' && *cp<='9') cp++; 
#ifdef DEBUG
		fprintf(stderr, "read count %d\n", count); 
#endif /* DEBUG */ 

		} else if (*cp=='o') { 
#ifdef DEBUG
		    fprintf(stderr, "%d live cells\n", count); 
#endif /* DEBUG */ 
		    while (count) { 
			out[r][c++]=1; 
			count--; 
		    } 
		    count=1; 
		    cp++; 
		} else if (*cp=='b') { 
#ifdef DEBUG
		    fprintf(stderr, "%d blank cells\n", count); 
#endif /* DEBUG */ 
		    while (count) { 
			out[r][c++]=0; 
			count--; 
		    } 
		    count=1; 
		    cp++; 
		} else if (*cp=='$') { 
#ifdef DEBUG
		    fprintf(stderr, "%d newlines\n", count); 
#endif /* DEBUG */ 
		    while (count) { 
			r++; 	
			count--; 
		    } 
		    count=1; 
		    c=0; 
		    cp++; 
		} else if (*cp=='!') { 
#ifdef DEBUG
		    fprintf(stderr, "end of run\n", count); 
#endif /* DEBUG */ 
		    return out; 
		} else {  /* default is to make the cell live */ 
#ifdef DEBUG
		    fprintf(stderr, "%d questionable cells\n", count); 
#endif /* DEBUG */ 
		    while (count) { 
			out[r][c++]=1; 
			count--; 
		    } 
		    count=1; 
		    cp++; 
		} 
	    } 
	} 
    } else { 
	fprintf(stderr,"cannot read rle: first line incorrect!\n"); 
	return NULL; 
    } 
} 

/*  read an image and generate a result */ 
int main(int argc, char **argv) { 
   FILE *f; 
   int rows=0; int cols=0; int iterations=0; 
   int i; 
   if (argc>1 || !(grid=read_rle(stdin,&rows,&cols))) { 
	fprintf(stderr,"translate usage: translate <inputfile\n"); 
	exit(1); 
   } 
   print_grid(stdout,grid,rows,cols); 
   free_grid(grid,rows); 
} 

