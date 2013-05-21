/* Will Richard
 * Mult_Main.c
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "Grid.h"
#include "Visibility.h"
#include "Horizon.h"
#include "Points.h"
#include "rtimer.h"


//forward declarations of functions
void writeOutputGrid(char* inputPath, char* outputPath, short** outputGrid, int nRows, int nCols);

int main(int argc, char** argv) {
  //make sure the input is valid
  if(argc != 3) {
    printf("Incorrect number of arguments passed\n");
    printf("Usage: multVis <input file path> <output file path>\n");
    exit(0);
  }

  Rtimer total_time;
  rt_start(total_time);

  //argv[1] is input path, argv[2] is output path

  //create the grid from the input file.  Note, this only fills in the elevation values - other values are filled in later with a call To Grid_fillPointValues and depending on viewpoint
  Grid* grid = Grid_createFromFile(argv[1]);

  //create a double array of shorts to hold the number of visible points from each point
  short** outputGrid = (short**) malloc(sizeof(short*) * Grid_getNCols(*grid));
  assert(outputGrid);
  int w;
  for(w = 0; w < Grid_getNCols(*grid); w++) {
    outputGrid[w] = (short*) malloc(sizeof(short) * Grid_getNRows(*grid));
    assert(outputGrid[w]);
  }


  //now, make 2 Point* arrays, one to hold the points to the right of the viewpoint, one to hold the points to the left of the viewpoint
  Point** rightPoints = (Point**) malloc((Grid_getNCols(*grid)*Grid_getNRows(*grid)) * sizeof(Point*));
  assert(rightPoints);
  Point** leftPoints = (Point**) malloc((Grid_getNCols(*grid)*Grid_getNRows(*grid)) * sizeof(Point*));
  assert(leftPoints);
  //the logical size of the points array
  int rightPointsLength = 0;
  int leftPointsLength = 0;
  //this number keeps track of the number of points on the boder of horizons, that are stored in both halves, so we get an accurate number of visible points
  int numPointsInBoth = 0;

  //the Viewpoint* for the viewpoint
  Viewpoint* vp = (Viewpoint*) malloc(sizeof(Viewpoint));

  //go through all points on the grid, and compute visibility with current point as viewpoint
  int vc, vr;
  /*   int i, j; */
  int c, r;
  int numVis;
  int curProgress = -1;
  for(vc = 0; vc < Grid_getNCols(*grid); vc++) {
    //for each new column, the points to the right and left of the viewpoint change, so refill the right and left arrays
    //reset the counters
    rightPointsLength = 0;
    leftPointsLength = 0;
    numPointsInBoth = 0;
    for(c = 0; c < Grid_getNCols(*grid); c++) {
      //if the current column (c) is equal to the viewpoint column (vc) add to both arrays
      if(c == vc) {
	for(r = 0; r < Grid_getNRows(*grid); r++) {
	  //Do not add points that are NODATA
	  if(Point_getElev(*Grid_getPoint(grid, c, r))==Grid_getNoDataValue(*grid))
	    continue;
	  //otherwise, add the point to the correct array
	  rightPoints[rightPointsLength] = Grid_getPoint(grid, c, r);
	  rightPointsLength++;
	  leftPoints[leftPointsLength] = Grid_getPoint(grid, c, r);
	  leftPointsLength++;
	  //these points are in both halves, so increment the counter for that
	  numPointsInBoth++;
	}
      }
      //if c is less than vc, add to the left set of points
      else if(c < vc) {
	for(r = 0; r < Grid_getNRows(*grid); r++) {
	  	  //Do not add points that are NODATA
	  if(Point_getElev(*Grid_getPoint(grid, c, r))==Grid_getNoDataValue(*grid))
	    continue;
	  //otherwise, add the point to the correct array
	  leftPoints[leftPointsLength] = Grid_getPoint(grid, c, r);
	  leftPointsLength++;
	}
      }
      //finally, in this case c > vc, so add to the right set of points
      else {
	for(r = 0; r < Grid_getNRows(*grid); r++) {
	  //Do not add points that are NODATA
	  if(Point_getElev(*Grid_getPoint(grid, c, r))==Grid_getNoDataValue(*grid))
	    continue;
	  //otherwise, add the point to the correct array
	  rightPoints[rightPointsLength] = Grid_getPoint(grid, c, r);
	  rightPointsLength++;
	}
      }
    }

    //now, go down the column, computing viewshed for each row of the column
    for(vr = 0; vr < Grid_getNRows(*grid); vr++) {
      //write progress percentage
      if((int) 100*(vc*Grid_getNCols(*grid)+vr%Grid_getNRows(*grid)) / (Grid_getNCols(*grid) * Grid_getNRows(*grid)) != curProgress) {
	curProgress = (int)100*(vc*Grid_getNCols(*grid)+vr%Grid_getNRows(*grid)) / (Grid_getNCols(*grid) * Grid_getNRows(*grid));
	printf("%d ", curProgress);
	fflush(stdout);
      }
      
      //if the point at vc, vr is NODATA, write NODATA to output file
      if(Point_getElev(*Grid_getPoint(grid,vc,vr))==Grid_getNoDataValue(*grid)){
	outputGrid[vc][vr] = Grid_getNoDataValue(*grid);
	continue;
      }
      //create a viewpoint at vc, vr
      Viewpoint_fill(vp, Point_getElev(*Grid_getPoint(grid, vc, vr)));
      
      //fill the other vales of the Points in the grid based on the viewpoint
      Grid_fillPointValues(grid, *vp, vc, vr);

      //now, sort the arrays by distance
      qsort(rightPoints, rightPointsLength, sizeof(Point*), PointPointer_compareByDist);
      qsort(leftPoints, leftPointsLength, sizeof(Point*), PointPointer_compareByDist);

      //the maximum number of points that may be visible is the number of points in the right half + the number of points in the left half, minus the number in both.  This number will be decremented by visibility.  
      numVis = rightPointsLength + leftPointsLength - numPointsInBoth;

      //start the visibility recursion - we don't really care about the output horizon.  Once it is done, the visibility values for all the points in the grid should be set correctly.
      visibility(0, rightPointsLength, rightPoints, &numVis, 1);
      visibility(0, leftPointsLength, leftPoints, &numVis, 0);

      //write the number of visible points to the grid
      outputGrid[vc][vr] = numVis;
    }
  }

  //now, write the output grid to file
  //reminder that argv[1] is input path and argv[2] is output path
  writeOutputGrid(argv[1], argv[2], outputGrid, Grid_getNRows(*grid), Grid_getNCols(*grid));

  //free and kill stuff
  free(rightPoints);
  free(leftPoints);
  for(w = 0; w < Grid_getNCols(*grid); w++) {
    free(outputGrid[w]);
  }
  free(outputGrid);
  free(vp);
  Grid_kill(grid);  

  //stop and print the timer
  rt_stop(total_time);
  
  char buf[1000];
  rt_sprint(buf, total_time);
  printf("\ntotal time: %s\n", buf);

  exit(1);
}

//writes the output grid of shorts to the output file
void writeOutputGrid(char* inputPath, char* outputPath, short** outputGrid, int nRows, int nCols) {
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
      fprintf(outputFile, "%hi ", outputGrid[c][r]);
    }
    //at the end of each line, write a newline
    fprintf(outputFile, "\n");
  }

  fclose(outputFile);
}
