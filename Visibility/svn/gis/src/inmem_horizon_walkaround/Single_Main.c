/* Will Richard
 * Walkaround Visibility Algorithm
 * Single_Main.c
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>

#include "Grid.h"
#include "Visibility.h"
#include "Points.h"
#include "rtimer.h"

int main(int argc, char** argv) {
  //make sure thi input is valid
  if(argc != 5) {
    printf("Usage incorrect.\nOnly perfect spellers may\nrun algorithm\n\n-- originally by Jason Axley, modified by Will Richard\n");
    printf("Correct Usage: oneVis <input file path> <output file path> <viewpoint column> <viewpoint row>\n");
    exit(0);
  }

  Rtimer total_time, vis_time;
  rt_start(total_time);

  //argv[1] is input path, argv[2] is output path, argv[3] is viewpoint column, argv[4] is viewpoint row
  
  //create the grid from the input file.  Note, this only fills in the elevation values.
  Grid* grid = Grid_createFromFile(argv[1]);

  //store the viewpoint row and column
  int vp_col = atoi(argv[3]);
  int vp_row = atoi(argv[4]);

  //make sure the viewpoint row and column are valid
  if(vp_col < 0 || vp_row < 0 || vp_col >= Grid_getNCols(*grid) || vp_row >= Grid_getNRows(*grid)) {
    printf("The row or column\nyou put into the program\nare not possible.\n\n--Will Richard\n");
    Grid_kill(grid);
    exit(0);
  }

  //make the viewpoint based on Point_VP
  Viewpoint* vp = Viewpoint_new(vp_col, vp_row, Point_getElev(*Grid_getPoint(grid, vp_col, vp_row)));

  if(Viewpoint_getElev(*vp) == Grid_getNoDataValue(*grid)) {
    printf("The viewpoint you chose\nDoes not exist on the map\nIt is no data\n\n--Will Richard\n");
    Grid_kill(grid);
    Viewpoint_kill(vp);
    exit(0);
  }

  rt_start(vis_time);

  //start the visibilty algorithm.  We don't really care about the output horizon.
  unsigned int numVis = visibility(grid, *vp);

  rt_stop(vis_time);

  //all visibility values should now be set, so output the file
  //just a reminder - argv[2] is the output path and argv[1] is input path
  Grid_outputVisGrid(grid, argv[2], argv[1]);

  //now, free things up
  Grid_kill(grid);
  Viewpoint_kill(vp);
  
  rt_stop(total_time);

  printf("there are %d visible points\n", numVis);

  //print out the timer information
  char buf[1000];
  rt_sprint(buf, vis_time);
  printf("visibility time: %s\n", buf);
  rt_sprint(buf, total_time);
  printf("total time: %s\n", buf);

  exit(0);
}
