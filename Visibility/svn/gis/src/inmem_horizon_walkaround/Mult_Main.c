/* Will Richard
 * Walkaround Visibility Algorithm
 * Mult_Main.c
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "Grid.h"
#include "Visibility.h"
#include "Points.h"
#include "rtimer.h"

#define PRINT_PERCENTAGE if(1)

//forward declaration of functions
void writeOutputGrid(char* inputPath, char* outputPath, long** outputGrid, int nRows, int nCols);

int main(int argc, char** argv) {
  //make sure input is valid
  if(argc != 3) {
    printf("Usage incorrect.\nOnly perfect spellers may\nrun algorithm\n\n-- originally by Jason Axley, modified by Will Richard\n");
    printf("Usage: multVis <input file path> <output file path>\n");
    exit(0);
  }

  Rtimer total_time;
  rt_zero(total_time);
  rt_start(total_time);

  //argv[1] is input path, argv[2] is output path

  //create the grid from the input file.
  Grid* grid = Grid_createFromFile(argv[1]);

  //make the short double array that will hold the output data
  long** outputGrid = (long**) malloc(sizeof(long*) * Grid_getNCols(*grid));
  assert(outputGrid);
  int w;
  for(w = 0; w < Grid_getNCols(*grid); w++) {
    outputGrid[w] = (long*) malloc(sizeof(long) * Grid_getNRows(*grid));
    assert(outputGrid[w]);
  }

  //declare variables that will be used in function
  int vc, vr; //the current viewpoint row and col
  Viewpoint* vp = (Viewpoint*) malloc(sizeof(Viewpoint)); //the current viewpoint.  Will be filled later
  long numVis; // the current number of visible points
  int curProgress = -1; //the progress percentage
  
  Rtimer vis_time;//the time spent in the visibility algorithm
  rt_zero(vis_time);

  for(vc = 0; vc < Grid_getNCols(*grid); vc++) {
    for(vr = 0; vr < Grid_getNRows(*grid); vr++) {
      PRINT_PERCENTAGE {
	//write progress percentage
	if((int) (100*((vc*Grid_getNRows(*grid))+vr) / 
		  (Grid_getNCols(*grid) * Grid_getNRows(*grid))) 
	   != curProgress) {
	  curProgress = (int) (100*(vc*Grid_getNRows(*grid)+vr) / 
			       (Grid_getNCols(*grid) * Grid_getNRows(*grid)));
	  printf("%d ", curProgress);
	  fflush(stdout);
	}	
      }

      //if the point at vc, vr is NODATA, write NODATA to output file
      if(Point_getElev(*Grid_getPoint(grid, vc, vr)) == Grid_getNoDataValue(*grid)){
	outputGrid[vc][vr] = Grid_getNoDataValue(*grid);
	continue;
      }

      //set up the viewpoint
      Viewpoint_fill(vp, vc, vr, Point_getElev(*Grid_getPoint(grid, vc, vr)));

      //do the visibility algorithm
      rt_start(vis_time);
      
      numVis = visibility(grid, *vp);

      rt_stop_and_accumulate(vis_time);

      //write the number of visible points to the output grid
      outputGrid[vc][vr] = numVis;
    }
  }

  //print an extra \n, so things are well spaced
  printf("\n");

  //now write the output grid to file
  //reminder, than argv[1] is the input path, argv[2] is the output path
  writeOutputGrid(argv[1], argv[2], outputGrid, Grid_getNRows(*grid), Grid_getNCols(*grid));

  //free/kill stuff
  for(w = 0; w < Grid_getNCols(*grid); w++) {
    free(outputGrid[w]);
  }
  free(outputGrid);
  Viewpoint_kill(vp);
  Grid_kill(grid);

  //stop and print the timers
  rt_stop(total_time);

  char buf[1000];
  rt_sprint_total(buf, vis_time);
  printf("visibility time: %s\n", buf);
  rt_sprint(buf, total_time);
  printf("total time: %s\n", buf);

  exit(1);
}

//writes the output grid of shorts to the output file
void writeOutputGrid(char* inputPath, char* outputPath, long** outputGrid, int nRows, int nCols) {
  assert(outputGrid);
  assert(inputPath);
  assert(outputPath);

  //open the files
  FILE* inputFile = fopen(inputPath, "r+");
  assert(inputFile);
  
  FILE* outputFile = fopen(outputPath, "w");
  assert(outputFile);

  //write the header i.e. the first 6 lines of the inputFile to outputFile
  int i;
  char headerLine[40];
  for(i = 0; i < 6; i++) {
    fputs(fgets(headerLine, 40, inputFile), outputFile);
  }

  //done with the input file, so close it
  fclose(inputFile);
  
  //now go through outputGrid, and write the values to the outputFile
  int c, r;
  for(r = 0; r < nRows; r++) {
    for(c = 0; c < nCols; c++) {
      fprintf(outputFile, "%ld ", outputGrid[c][r]);
    }
    //at the end of each line, write a newline
    fprintf(outputFile, "\n");
  }

  fclose(outputFile);
}

