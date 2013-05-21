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
  rt_init(total_time);
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


  //now, make the Point* array to hold all the points for the visibilty algorithm
  Point** points = (Point**) malloc((Grid_getNCols(*grid)*(Grid_getNRows(*grid))) * sizeof(Point*));
  assert(points);
  //the logical size of the points array
  int pointsLength = 0;

  //fill the points array
  int c, r;
  for(c = 0; c < Grid_getNCols(*grid); c++) {
    for(r = 0; r < Grid_getNRows(*grid); r++) {
      //Do not add points that are NODATA
      if(Point_getElev(*Grid_getPoint(grid, c, r))==Grid_getNoDataValue(*grid))
	continue;
      //otherwise, add the point
      points[pointsLength] = Grid_getPoint(grid, c, r);
      pointsLength++;
    }
  }

  //the Viewpoint* for the viewpoint
  Viewpoint* vp = (Viewpoint*) malloc(sizeof(Viewpoint));

  //go through all points on the grid, and compute visibility with current point as viewpoint
  int vc, vr;
  Rtimer sort_time;
  Rtimer vis_time;
  rt_init(sort_time);
  rt_init(vis_time);
/*   int i, j; */
  int numVis;
  int curProgress = -1;
  for(vc = 0; vc < Grid_getNCols(*grid); vc++) {
    for(vr = 0; vr < Grid_getNRows(*grid); vr++) {
      //write progress
      if((int) 100*(vc*Grid_getNRows(*grid)+vr)
/ 
(Grid_getNCols(*grid) * Grid_getNRows(*grid)) != curProgress) {
	curProgress = 
(int)100*(vc*Grid_getNRows(*grid)+vr) / 
(Grid_getNCols(*grid) * Grid_getNRows(*grid));
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

      rt_start(sort_time);

      //now, sort by distance
/*       qsort(points, pointsLength, sizeof(Point*), PointPointer_compareByDist); */
      if(Grid_getNCols(*grid) > Grid_getNRows(*grid))
	PointPointer_sortByDist(points, pointsLength, Grid_getNCols(*grid));
      else
	PointPointer_sortByDist(points, pointsLength, Grid_getNRows(*grid));

      rt_stop(sort_time);      

      //pointsLength is the maximum number of possible visible points
      numVis = pointsLength;

      rt_start(vis_time);

      //start the visibility recursion - we don't really care about the output horizon.  Once it is done, the visibility values for all the points in the grid should be set correctly.
      visibility(0, pointsLength, points, &numVis);

      rt_stop(vis_time);

      //write the number of visible points to the grid
      outputGrid[vc][vr] = numVis;
    }
  }

  //now, write the output grid to file
  //reminder that argv[1] is input path and argv[2] is output path
  writeOutputGrid(argv[1], argv[2], outputGrid, Grid_getNRows(*grid), Grid_getNCols(*grid));

  //free and kill stuff
  free(points);
  for(w = 0; w < Grid_getNCols(*grid); w++) {
    free(outputGrid[w]);
  }
  free(outputGrid);
  free(vp);
  Grid_kill(grid);

  //stop and print the timer
  rt_stop(total_time);
  
  char buf[1000];
  rt_sprint_total(buf, sort_time);
  printf("\nsort time: %s\n", buf);
  rt_sprint_total(buf, vis_time);
  printf("visibility time: %s\n", buf);
  rt_sprint_total(buf, total_time);
  printf("total time: %s\n", buf);

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
