/* Will Richard
 * Walkaround Visibility Algorithm
 * Visibility.h
 */

#ifndef __Visibility_h
#define __Visibility_h

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <float.h>

#include "Points.h"
#include "Horizon.h"
#include "Grid.h"
#include "compareDouble.h"

#define DUMMY_SLOPE -1*FLT_MAX

/* //move the col value left one, if allowed */
/* void moveColLeft(int* curValue, Grid g); */
/* //move the col value right one, if allowed */
/* void moveColRight(int* curValue, Grid g); */
/* //move the row value up one, if allowed */
/* void moveRowUp(int* curValue, Grid g); */
/* //move the row value down one, if allowed */
/* void moveRowDown(int* curValue, Grid g); */

//see if the col-row combination is valid, i.e. on the grid
int colRowValid(int col, int row, Grid g);

/* computes the visibility of all the points on the grid from the passed viewpoint.
 * Has a for loop that goes out in layers from the viewpoint, where each layer is a set of points that create a concentric cirle around the viewpoint.  At each layer, we go around it from the point directly to the right of the viewpoint (at center angle 0/2PI) around the whole layer, back to the same point, checking to se e if each point is occulded or not by the horizon created by the layers that are closer to the viewpoint.  If a point is blocked by the horizon so far, it is marked invisible and we continue walking around the layer.  If it is not blocked, it is marked as visible and added to the Horizon.  The walkaround in each layer is performed in decreasing order, so segments that are added do not interfere with any segments on the same level. This is continued until all layers have been walked, at which point the number of visible points in returned.
 */
long visibility(Grid* grid, Viewpoint vp);

/*determines if the point at pCol, pRow is visible based on the horizon, and handles it accordingly by adding it to h.
Also, updates hIndex correctly, if it needs to be incremented as we move around the horizon.
returns 1 if the point is visible and was added to layerH, 0 otherwise
 */
int pointVisible(Horizon* h, int* hIndex, Horizon* layerH, long* numVis, Viewpoint vp, Grid* grid, int pCol, int pRow);

#endif
