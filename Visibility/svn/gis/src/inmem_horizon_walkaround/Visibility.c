/* Will Richard
 * Walkaround Visibility Algorithm
 * Visibility.c
 */

#include "Visibility.h"

#define VIS_DEBUG if(0)


/* //move the col value left one, if allowed */
/* void moveColLeft(int* curValue, Grid g) { */
/*   assert(curValue); */
/*   if(*curValue - 1 >= 0)  */
/*     (*curValue)--; */
/* } */
/* //move the col value right one, if allowed */
/* void moveColRight(int* curValue, Grid g) { */
/*   assert(curValue); */
/*   if(*curValue + 1 < Grid_getNCols(g)); */
/*      (*curValue)++; */
/* } */
/* //move the row value up one, if allowed */
/* void moveRowUp(int* curValue, Grid g) { */
/*   assert(curValue); */
/*   if(*curValue - 1 >= 0) */
/*     (*curValue)--; */
/* } */
/* //move the row value down one, if allowed */
/* void moveRowDown(int* curValue, Grid g) { */
/*   assert(curValue); */
/*   if(*curValue + 1 < Grid_getNRows(g)) */
/*     (*curValue)++; */
/* } */

//see if the col-row combination is valid, i.e. on the grid
int colRowValid(int col, int row, Grid g) {
  return ((col >=0) && (col < Grid_getNCols(g)) && (row >=0) && (row < Grid_getNRows(g)));
}


/* computes the visibility of all the points on the grid from the passed viewpoint.
 * Has a for loop that goes out in layers from the viewpoint, where each layer is a set of points that create a concentric cirle around the viewpoint.  
 * At each layer, we go around the viewpoint from the point directly to the right of the viewpoint (at center angle 0/2PI) around the whole layer, back to the same point, checking to see if each point is occulded or not by the horizon created by the layers that are closer to the viewpoint.  
 * If a point is blocked by the horizon so far, it is marked invisible and we continue walking around the layer.  
 * If it is not blocked, it is marked as visible and added to a temporary array of points.  Once the walk around the layer is completed, all points in the tempory array are added to the horizon.
 * It then moves on to the next layer, until all layers have been walked, at which point the number of visible points in returned.
 */
long visibility(Grid* grid, Viewpoint vp) {
  assert(grid);

  int layer; // keep track of which layer we're in
  int curCol, curRow; //once we're in a layer, keep track of which row and column we should be in.
  //curCol and curRow will keep track of which row and column we *should* be in, if the grid was infinate, so that the edges of the grid do not interfere with the loop defined by a layer.  We will only check if a point is visible if it is a valid point. 
  int hIndex; //within a layer, keep track of what horizon index we're looking at

  //mark the viewpoint visible
  Point_setVis(Grid_getPoint(grid, Viewpoint_getCol(vp), Viewpoint_getRow(vp)), VISIBLE);

  long numVisible = 1; //a count of the number of visible points

  //figure out what the max layer value is by finding the max dimention from the viewpoint to an edge of the grid.  Start by assuming the dimention to the right of the viewpoint is the max.
  int maxLayer = Grid_getNCols(*grid)-1 - Viewpoint_getCol(vp);
  //check the dimention above
  if(Viewpoint_getRow(vp) > maxLayer) maxLayer = Viewpoint_getRow(vp);
  //see if the dimention to the left of the viewpoint is bigger
  if(Viewpoint_getCol(vp) > maxLayer) maxLayer = Viewpoint_getCol(vp);
  //see if the dimention down and to the left of the viewpoint is bigger
  if(Grid_getNRows(*grid)-1 - Viewpoint_getRow(vp) > maxLayer) 
    maxLayer = Grid_getNRows(*grid)-1 - Viewpoint_getRow(vp);

  VIS_DEBUG {
    printf("max layer = %d\n", maxLayer);
    fflush(stdout);
  }

  //create the overall Horizon using Horizon_new.  
  Horizon* h = Horizon_new(100);
  //Initialize this horizon with 1 section with slope DUMMY_SLOPE.
  Horizon_addSectValues(h, 0.0, DUMMY_SLOPE);

  int counter;
  int firstPointVis;

  //go through all the layers, one by one
  for(layer = 1; layer <= maxLayer; layer++) {
    VIS_DEBUG{ 
      printf("starting layer %d\n", layer);
      fflush(stdout);
    }

    //first, create the horizon for this layer.
    Horizon* layerHorizon = Horizon_new((layer*4)+4);
    //initialize layerHorizon with a dummy section
    Horizon_addSectValues(layerHorizon, 0.0, DUMMY_SLOPE);

    //set the hIndex to 0, so we start at the beginning of h
    hIndex = 0;
    //start with the point directly to the right of the viewpoint
    curCol = Viewpoint_getCol(vp) + layer;
    curRow = Viewpoint_getRow(vp);
    //keep track of if the first point, directly to the right of vp, was visible
    firstPointVis = 0;
    //handle the point, storing if it was visible
    if(colRowValid(curCol, curRow, *grid))
      firstPointVis = pointVisible(h, &hIndex, layerHorizon, &numVisible, vp, grid, curCol, curRow);
    //now, move up layer rows & check all points
    for(counter = 0; counter < layer; counter++) {
      curRow--;
      if(colRowValid(curCol, curRow, *grid))
	pointVisible(h, &hIndex, layerHorizon, &numVisible, vp, grid, curCol, curRow);
    }
    //move left 2*layer columns, checking each point
    for(counter = 0; counter < 2*layer; counter++) {
      curCol--;
      if(colRowValid(curCol, curRow, *grid))
	pointVisible(h, &hIndex, layerHorizon, &numVisible, vp, grid, curCol, curRow);
    }
    //move down 2*layer rows, checking each point
    for(counter = 0; counter < 2*layer; counter++) {
      curRow++;
      if(colRowValid(curCol, curRow, *grid))
	pointVisible(h, &hIndex, layerHorizon, &numVisible, vp, grid, curCol, curRow);
    }
    //move right 2*layer columns, checking each point
    for(counter = 0; counter < 2*layer; counter++) {
      curCol++;
      if(colRowValid(curCol, curRow, *grid))
	pointVisible(h, &hIndex, layerHorizon, &numVisible, vp, grid, curCol, curRow);
    }
    //move up layer-1 rows, checking each point
    for(counter = 0; counter < layer-1; counter++) {
      curRow--;
      if(colRowValid(curCol, curRow, *grid))
	pointVisible(h, &hIndex, layerHorizon, &numVisible, vp, grid, curCol, curRow);
    }

    //if the first point was visible, add it to the end of the horizon
    if(firstPointVis) {
      curRow--;
      Horizon_addSectValues(layerHorizon, Point_calcStartAngle(vp, curCol, curRow), Point_calcSlope(vp, *Grid_getPoint(grid, curCol, curRow), curCol, curRow));
      VIS_DEBUG {
	printf("the first point was visible, so adding its other half to the end of the horizon\nLayer Horizon is now:\n");
	Horizon_print(layerHorizon);
	fflush(stdout);
      }
    }

    //merge the horizons
    Horizon* newH = Horizon_merge(h, layerHorizon);

    //kill layerHorizon
    Horizon_kill(layerHorizon);

    //also, we want to kill what h was pointing to, and set h=newH
    Horizon* temp;
    temp = h;
    h = newH;
    Horizon_kill(temp);


    VIS_DEBUG {
      printf("done with layer %d.  Horizon is:\n", layer);
      Horizon_print(h);
      printf("*********************\n\n");
      fflush(stdout);
    }
  }

  //kill everything
  Horizon_kill(h);
  
  VIS_DEBUG {
    printf("done with visibility\n");
    fflush(stdout);
    printf("returning that there are %ld points\n", numVisible);
    fflush(stdout);
  }
  //all done, so return the number of visible points;
  return numVisible;
}

/*determines if the point at pCol, pRow is visible based on the horizon, and handles it accordingly by adding it to h.
Also, updates hIndex correctly, if it needs to be incremented as we move around the horizon.
 */
int pointVisible(Horizon* h, int* hIndex, Horizon* layerH, long* numVis, Viewpoint vp, Grid* grid, int pCol, int pRow) {
  assert(h);
  assert(hIndex);
  assert(layerH);
  assert(numVis);
  assert(grid);

  int returnValue;

  VIS_DEBUG {
    printf("starting pointVisible on %d, %d\n", pCol, pRow);
    fflush(stdout);
  }

  //catch the viewpoint - don't do anything on the viewpoint
  if(pCol == Viewpoint_getCol(vp) && pRow == Viewpoint_getRow(vp)) {
    VIS_DEBUG {
      printf("on viewpoint - ending pointVisible.\n");
      fflush(stdout);
    }
    return 0;
  }

  //get the point in question
  Point* p = Grid_getPoint(grid, pCol, pRow);

  //if the point is NODATA, skip it
  if(Point_getElev(*p) == Grid_getNoDataValue(*grid)) {
    VIS_DEBUG {
      printf("point is NODATA.\n");
      fflush(stdout);
    }
    return 0;
  }

  //fill the points center angle and slope, to determine if its visible
/*   Point_setSlopeAndCenterAngle(p, pCol, pRow, vp); */
  double pCenterAngle = Point_calcCenterAngle(vp, pCol, pRow);
  float pSlope = Point_calcSlope(vp, *p, pCol, pRow);

  VIS_DEBUG {
    printf("%d, %d has angle %f and slope %f\n", pCol, pRow, pCenterAngle, pSlope);
    fflush(stdout);
  }

  VIS_DEBUG{ 
    printf("current section at index %d is %f, %f\n", *hIndex, Horizon_getSectAngle(h, *hIndex), Horizon_getSectSlope(h, *hIndex));
    fflush(stdout);
  }

  //make sure the section at hIndex is in front of the center angle, by checking that its start angle is less than the center angle, but its end angle is greater than the center angle.  If it is not the case, go on to the next section.
  //we find the end angle of the section it hIndex by looking at the start angle of the section with index hIndex+1
  while(*hIndex+1 < Horizon_getNumSect(*h) && 
	doubleLessThan(Horizon_getSectAngle(h, *hIndex+1), pCenterAngle)) {
    (*hIndex)++;
    VIS_DEBUG {
      printf("hIndex incremented - now %d\n", *hIndex);
      printf("new current section is %f, %f\n", Horizon_getSectAngle(h, *hIndex), Horizon_getSectSlope(h, *hIndex));
      fflush(stdout);
    }
  }

  //the section at hIndex should now be in front of p, so compare the slopes
  //if the slope is equal, mark the point VISIBLE, but don't do anything with temp, since the horizon doesn't really change.
  if(doubleEqual(pSlope, Horizon_getSectSlope(h, *hIndex))) {
    VIS_DEBUG {
      printf("point at %d,%d has an equal slope, so it is visible\n", pCol, pRow);
      fflush(stdout);
    }
    Point_setVis(p, VISIBLE);
    //also, update visibility count
    (*numVis)++;
    //set return value to 0, because though the point is visible, it was not added to layerH
    returnValue = 0;
  }
  //if the point's slope is greater, mark the point visible, and add a corresponding section to layerH
  else if(doubleGreaterThan(pSlope, Horizon_getSectSlope(h, *hIndex))) {
    //mark as visible
    Point_setVis(p, VISIBLE);
    //update visibility count
    (*numVis)++;
    VIS_DEBUG {
      printf("point at %d,%d is visible - inserting into horizon\n", pCol, pRow);
      fflush(stdout);
    }
    //set returnValue to 1, since the point was added to the horizon
    returnValue = 1;

    //catch the first section at center angle 0.0
    if(pCenterAngle == 0.0) {
      //just insert one section from 0.0, then a dummy section starting at the end angle
      Horizon_addSectValues(layerH, 0.0, pSlope);
      Horizon_addSectValues(layerH, Point_calcEndAngle(vp, pCol, pRow), DUMMY_SLOPE); 
    }
    else {
      //now, add an HSect to layerH, then a dummy section
      Horizon_addSectValues(layerH, Point_calcStartAngle(vp, pCol, pRow), pSlope);
      Horizon_addSectValues(layerH, Point_calcEndAngle(vp, pCol, pRow), DUMMY_SLOPE);
    }
/*     Horizon_insertSect(h, *hIndex, Point_calcStartAngle(vp, pCol, pRow), pSlope, Point_calcEndAngle(vp, pCol, pRow)); */
  }
  //otherwise, the point is not visible, so mark it invisible
  else {
    VIS_DEBUG {
      printf("point at %d,%d is invisible\n", pCol, pRow);
      fflush(stdout);
    }
    Point_setVis(p, INVISIBLE);
    //set returnValue to 0, since its invisible
    returnValue = 0;
  }

  VIS_DEBUG {
    printf("Done with pointVisible\n");
    printf("overall horizon is:\n");
    Horizon_print(h);
    printf("layer horizon is now:\n");
    Horizon_print(layerH);
    printf("\n");
    fflush(stdout);
  }

  return returnValue;
}
