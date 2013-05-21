/* Will Richard
 * Single_Main.c
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>

#include "Grid.h"
#include "Visibility.h"
#include "Horizon.h"
#include "Points.h"
#include "rtimer.h"

int main(int argc, char** argv) {
  //make sure that the input is valid
  if(argc != 5) {
    printf("Incorrect number of arguments passed\n");
    printf("Usage: oneVis <input file path> <output file path> <viewpoint column> <viwepoint row>");
    exit(0);
  }

  Rtimer total_time, vis_time, sort_time;
  rt_start(total_time);

  //argv[1] is input path, argv[2] is output path, argv[3] is viewpoint column, argv[4] is viewpoint row

  //create the grid from the input file.  Note, this only fills in the elevation values - other values are filled in later with a call To Grid_fillPointValues.
  Grid* grid = Grid_createFromFile(argv[1]);

  //store the viewpoint row and column
  int vp_col = atoi(argv[3]);
  int vp_row = atoi(argv[4]);

  //make sure the viewpoint row and column are valid
  if(vp_col < 0 || vp_row < 0 || vp_col >= Grid_getNCols(*grid) || vp_row >= Grid_getNRows(*grid)) {
    printf("invalid viewpoint row or column.\n");
    exit(0);
  }

  //make the viewpoint based on Point_VP
  Viewpoint* vp = Viewpoint_new(Point_getElev(*Grid_getPoint(grid, vp_col, vp_row)));

  //fill in other values of the Points in grid based on the vp
  Grid_fillPointValues(grid, *vp, vp_col, vp_row);

  //now that all the point values are set, make the array of Point* that will be used for visibility.

  //allocate space for col*row points, for all the points on the grid
  //rightPoints stores the points to the right of the viewpoint, leftPoints stores the points to the left of the viewpoint
  Point** rightPoints = (Point**) malloc((Grid_getNCols(*grid)*(Grid_getNRows(*grid))) * sizeof(Point*));
  assert(rightPoints);
  Point** leftPoints = (Point**) malloc((Grid_getNCols(*grid)*(Grid_getNRows(*grid))) * sizeof(Point*));
  assert(leftPoints);

  //the logical size of the array - only adding non-NODATA values, so it will not be completely filled
  int rightPointsLength = 0; 
  int leftPointsLength = 0;
  //this number keeps track of the maximum number of points that might be visible.  This number will be decremented by visibility
  long numVisible = 0;

  //need to be defined to determine which points are directly above or below the viewpoint
  double infinity = 1.0/0.0;

  int c, r;
  for(c = 0; c < Grid_getNCols(*grid); c++) {
    for(r = 0; r < Grid_getNRows(*grid); r++) {
      //Do not add points that are NODATA
      if(Point_getElev(*Grid_getPoint(grid, c, r))==Grid_getNoDataValue(*grid)){
	continue;
      }

      //if point is directly above or below i.e. have a center angle of infinity, add it to both arrays
      if(Point_getCenterAngle(*Grid_getPoint(grid, c, r)) == infinity ||
	 Point_getCenterAngle(*Grid_getPoint(grid, c, r)) == -1 * infinity) {
	rightPoints[rightPointsLength] = Grid_getPoint(grid,c,r);
	rightPointsLength++;
	leftPoints[leftPointsLength] = Grid_getPoint(grid, c, r);
	leftPointsLength++;
      }
      //else, if the point is to the right, add it to the right array, else add it to the left array
      else if (Point_isRightOfVP(*Grid_getPoint(grid, c, r))) {
	rightPoints[rightPointsLength] = Grid_getPoint(grid, c, r);
	rightPointsLength++;
      }
      else {
	leftPoints[leftPointsLength] = Grid_getPoint(grid, c, r);
	leftPointsLength++;
      }

      numVisible++;
    }
  }

  //now, sort points by distance
  rt_start(sort_time);

  //depending on if we have more rows or columns, the max dist passed to the sorting function will be different
  if(Grid_getNCols(*grid) > Grid_getNRows(*grid)) {
    PointPointer_sortByDist(rightPoints, rightPointsLength, Grid_getNCols(*grid));
    PointPointer_sortByDist(leftPoints, leftPointsLength, Grid_getNCols(*grid));
  }
  else {
    PointPointer_sortByDist(rightPoints, rightPointsLength, Grid_getNRows(*grid));
    PointPointer_sortByDist(leftPoints, leftPointsLength, Grid_getNRows(*grid));
  }

  rt_stop(sort_time);

  rt_start(vis_time);

  //start the visibility recursion - we don't really care about the output in Horizon form.  Once it is done, all the visibility values for all the points in the gridshould be set correctly
  Horizon* right = visibility(0, rightPointsLength, rightPoints, &numVisible, 1, 0);
  Horizon* left = visibility(0, leftPointsLength, leftPoints, &numVisible, 0, 0);

  rt_stop(vis_time);
  
  //all visibilty values should now be set, so output the file
  //just a reminder - argv[2] is the output path and argv[1] is input path
  Grid_outputVisGrid(grid, argv[2], argv[1]);

  printf("There are %ld visible points according to visibility\n", numVisible);

  //now, free things up
  free(rightPoints);
  free(leftPoints);
  Viewpoint_kill(vp);
  Grid_kill(grid);
  Horizon_kill(right);
  Horizon_kill(left);

  rt_stop(total_time);

  char buf[1000];
  rt_sprint(buf, sort_time);
  printf("sort time: %s\n", buf);
  rt_sprint(buf, vis_time);
  printf("visibility time: %s\n", buf);
  rt_sprint(buf, total_time);
  printf("total time: %s\n", buf);

  exit(0);
}
