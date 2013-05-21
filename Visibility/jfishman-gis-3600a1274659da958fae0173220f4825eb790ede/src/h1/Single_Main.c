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
  int numVisible = 0;

  //define a double infinity (1/0) to find which angles are above and belowe the viewpoint
  double infinity = (double)1.0/0.0;

  int c, r;
  for(c = 0; c < Grid_getNCols(*grid); c++) {
    for(r = 0; r < Grid_getNRows(*grid); r++) {
      //Do not add points that are NODATA
      if(Point_getElev(*Grid_getPoint(grid, c, r))==Grid_getNoDataValue(*grid)){
	continue;
      }

      //if point is directly above or below i.e. have a center angle of inf, it to both arrays
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
      if(Point_getVis(*Grid_getPoint(grid, c, r)) != VISIBLE) {
	printf("cell %d, %d is not marked visible when it should be\n", c, r);
	exit(3);
      }
      numVisible++;
    }
  }

  DEBUG {
    //print out the grid
    printf("Start Angles:\n");
    for(r = 0; r < Grid_getNRows(*grid); r++) {
      for(c = 0; c < Grid_getNCols(*grid); c++) {
	printf("%f ", Point_getStartAngle(*Grid_getPoint(grid, c, r)));
	fflush(stdout);
      }
      printf("\n");
    }
    printf("Center angles:\n");
    for(r = 0; r < Grid_getNRows(*grid); r++) {
      for(c = 0; c < Grid_getNCols(*grid); c++) {
	printf("%f ", Point_getCenterAngle(*Grid_getPoint(grid, c, r)));
	fflush(stdout);
      }
      printf("\n");
    }
    printf("End Angles:\n");
    for(r = 0; r < Grid_getNRows(*grid); r++) {
      for(c = 0; c < Grid_getNCols(*grid); c++) {
	printf("%f ", Point_getEndAngle(*Grid_getPoint(grid, c, r)));
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
/*   qsort(rightPoints, rightPointsLength, sizeof(Point*), PointPointer_compareByDist); */
/*   qsort(leftPoints, leftPointsLength, sizeof(Point*), PointPointer_compareByDist); */
  //depending on if we have more rows or columns, the max dist passed to the sorting function will be different
  if(Grid_getNCols(*grid) > Grid_getNRows(*grid)) {
    PointPointer_sortByDist(rightPoints, rightPointsLength, Grid_getNCols(*grid));
    PointPointer_sortByDist(leftPoints, leftPointsLength, Grid_getNCols(*grid));
  }
  else {
    PointPointer_sortByDist(rightPoints, rightPointsLength, Grid_getNRows(*grid));
    PointPointer_sortByDist(leftPoints, leftPointsLength, Grid_getNRows(*grid));
  }

  rt_start(vis_time);

  //start the visibility recursion - we don't really care about the output in Horizon form.  Once it is done, all the visibility values for all the points in the gridshould be set correctly
  Horizon* right = visibility(0, rightPointsLength, rightPoints, &numVisible,1);
  Horizon* left = visibility(0, leftPointsLength, leftPoints, &numVisible, 0);

  rt_stop(vis_time);
  
 /*  DEBUG{ */
/*     printf("Final horizon:\n"); */
/*     Horizon_print(*h); */
/*     printf("\n"); */
/*     fflush(stdout); */
/*   } */

  //all visibilty values should now be set, so output the file
  //just a reminder - argv[2] is the output path and argv[1] is input path
  //This also prints out the number of visible points
  Grid_outputVisGrid(grid, argv[2], argv[1]);

  printf("There are %d visible points according to visibility\n", numVisible);
  DEBUG{ 
    //manually count the number of visible points
    int visCount = 0;
    for(c = 0; c < Grid_getNCols(*grid); c++) {
      for(r = 0; r < Grid_getNCols(*grid); r++) {
	//if nodata, skip
	if(Grid_getNoDataValue(*grid) == Point_getElev(*Grid_getPoint(grid, c, r))) continue;
	//otherwise, increment the count if necessary
	if(Point_getVis(*Grid_getPoint(grid, c, r)) == VISIBLE) visCount++;
      }
    }
    printf("According to the count, there are %d visible cells\n", visCount);
  }

  //now, free things up
  free(rightPoints);
  free(leftPoints);
  Viewpoint_kill(vp);
  Grid_kill(grid);
  Horizon_kill(right);
  Horizon_kill(left);

  rt_stop(total_time);

  char buf[1000];
  rt_sprint(buf, total_time);
  printf("total time: %s\n", buf);
  rt_sprint(buf, vis_time);
  printf("visibility time: %s\n", buf);

  exit(0);
}
