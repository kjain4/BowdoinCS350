/* Will Richard
 * Single_Main.c
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "Grid.h"
#include "Visibility.h"
#include "Horizon.h"
#include "Points.h"
#include "rtimer.h"

#define DEBUG if(0)

int main(int argc, char** argv) {
  //make sure that the input is valid
  if(argc != 5) {
    printf("Incorrect number of arguments passed\n");
    printf("Usage: oneVis <input file path> <output file path> <viewpoint column> <viwepoint row>");
    exit(0);
  }

  Rtimer total_time, vis_time;
  rt_start(total_time);

  //argv[1] is input path, argv[2] is output path, argv[3] is viewpoint column, argv[4] is viewpoint row

  //create the grid from the input file.  Note, this only fills in the elevation values - other values are filled in later with a call To Grid_fillPointValues.
  Grid* grid = Grid_createFromFile(argv[1]);

  //store the viewpoint row and column
  int vp_col = atoi(argv[3]);
  int vp_row = atoi(argv[4]);

  //make the viewpoint based on Point_VP
  Viewpoint* vp = Viewpoint_new(Point_getElev(*Grid_getPoint(grid, vp_col, vp_row)));

  //fill in other values of the Points in grid based on the vp
  Grid_fillPointValues(grid, *vp, vp_col, vp_row);

  //now that all the point values are set, make the array of Point* that will be used for visibility.

  //allocate space for col*row points, for all the points on the grid
  Point** points = (Point**) malloc((Grid_getNCols(*grid)*(Grid_getNRows(*grid))) * sizeof(Point*));
  assert(points);

  int pointsLength = 0; //the logical size of the array - only adding non-NODATA values, so it will not be completely filled

  int c, r;
  for(c = 0; c < Grid_getNCols(*grid); c++) {
    for(r = 0; r < Grid_getNRows(*grid); r++) {
      //Do not add points that are NODATA
      if(Point_getElev(*Grid_getPoint(grid, c, r))==Grid_getNoDataValue(*grid)){
	continue;
      }

      points[pointsLength] = Grid_getPoint(grid, c, r);
      pointsLength++;
    }
  }

  DEBUG {
    //print out the grid
    printf("Center angles:\n");
    for(r = 0; r < Grid_getNRows(*grid); r++) {
      for(c = 0; c < Grid_getNCols(*grid); c++) {
	printf("%f ", Point_getCenterAngle(*Grid_getPoint(grid, c, r)));
	fflush(stdout);
      }
      printf("\n");
    }
    printf("slopes:\n");
    for(r = 0; r < Grid_getNRows(*grid); r++) {
      for(c = 0; c < Grid_getNCols(*grid); c++) {
	printf("%f ", Point_getSlope(*Grid_getPoint(grid, c, r)));
	fflush(stdout);
      }
      printf("\n");
    }
  }

  //now, sort points by distance
/*   qsort(points, pointsLength, sizeof(Point*), PointPointer_compareByDist); */
  if(Grid_getNCols(*grid) > Grid_getNRows(*grid))
    PointPointer_sortByDist(points, pointsLength, Grid_getNCols(*grid));
  else
    PointPointer_sortByDist(points, pointsLength, Grid_getNRows(*grid));

  DEBUG{
    int i;
    for(i = 0; i < pointsLength; i++) {
      Point_print(*points[i]);
    }
  }

  //pointsLength is the most possible points that could be visible
  int numVisible = pointsLength;

  rt_start(vis_time);

  //start the visibility recursion - we don't really care about the output in Horizon form.  Once it is done, all the visibility values for all the points in the gridshould be set correctly
  Horizon* h = visibility(0, pointsLength, points, &numVisible);

  rt_stop(vis_time);
  
  DEBUG{
    printf("Final horizon:\n");
    Horizon_print(*h);
    printf("\n");
    fflush(stdout);
  }

  //all visibilty values should now be set, so output the file
  //just a reminder - argv[2] is the output path and argv[1] is input path
  //This also prints out the number of visible points
  Grid_outputVisGrid(grid, argv[2], argv[1]); 

  printf("There are %d visible points\n", numVisible);

  //now, free things up
  free(points);
  Viewpoint_kill(vp);
  Grid_kill(grid);
  Horizon_kill(h);

  rt_stop(total_time);

  char buf[1000];
  rt_sprint(buf, total_time);
  printf("total time: %s\n", buf);
  rt_sprint(buf, vis_time);
  printf("visibility time: %s\n", buf);

  exit(1);
}


