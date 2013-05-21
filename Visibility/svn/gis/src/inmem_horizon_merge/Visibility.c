/* Will Richard
 * Visibility.c
 */

#include "Visibility.h"

#define PRINT_HORIZON_STATS if(0) //MAKE SURE THIS IS SET TO 0 FOR MULTIMAIN!!!

/* recursivly computes the visibility of set a points.  The points should be ordered by distance, from closest to furthest away.
   BASE CASE:  When there is only one point cosidered, it creates a new Horizon with dummy points surrounding the one point
   RECURSION: Split the range of points (going from index "start" to index "end", not including end) into 2 by distance, and calculate the horizon for each half. It then checks to make sure that any points in the 2nd half that are occluded by the 1st horizon are marked as inivsible.  Finally, it merges the 2 halves together using Horizon_merge (see Horizon.h).
   The passed int, rightSide is 1 if visibility is working on the right-hand side of the grid (with x greater than viewpoint x), 0 if it is working on the left hand side of the grid (with x value less than viewpoint x)
   Pass the level of recursion and viewpoint as well, for horizon information purposes - they are not used, just printed.

*/
Horizon* visibility(int start, int end, Point** points, long* numVisible, int rightSide, short recursionLevel) {
  assert(points);

  //BASE CASE - when there is only one point left, create a horizon with it, and return that horizon
  if(end-start <= 1) {

  //define this now - will be used a lot soon
  double infinity = 1.0/0.0;

    //if the distance of the point is 0, it is the viewpoint.  Return a horizon that is just a dummy
    if(Point_getDist(*points[start]) == 0.0) {
      Horizon* h = Horizon_new(1);
      Horizon_addSectValues(h, -1* infinity, DUMMY_SLOPE);
      return h;
    }
    //otherwise, it is a real point - treat it as such
    //make the new horizon with size 3 - 2 dummy sections and 1 real section
    Horizon* h = Horizon_new(3);

    /*points were the center angle is at infinity are on the edge of the horizon, and need to be treated specially.  They need to just add one real section in the right place, and one dummy section before/after the real one, depending on which edge of the horizion it is on.*/
    if(Point_getCenterAngle(*points[start]) == infinity) {
      //this is a point directly below the viewpoint
      if(rightSide) {
	//we are on the right side of the grid, so add this point at the beginning of the horizon, then a dummy
	Horizon_addSectValues(h, -1 * infinity, Point_getSlope(*points[start]));
	Horizon_addSectValues(h, Point_getEndAngle(*points[start]),
			      DUMMY_SLOPE);
      }
      else {
	//we are on the left side of the grid, so put the point at the end of the horizon
	Horizon_addSectValues(h, -1 * infinity, DUMMY_SLOPE);
	Horizon_addSectValues(h, Point_getStartAngle(*points[start]),
			      Point_getSlope(*points[start]));
	
      }
    }
    else if(Point_getCenterAngle(*points[start]) == -1*infinity) {
      //this is a point directly above the viewpoint
      if(rightSide) {
	//we are on the right side of the grid, so add this point at the end of the horizon
	Horizon_addSectValues(h, -1 * infinity, DUMMY_SLOPE);
	Horizon_addSectValues(h, Point_getStartAngle(*points[start]),
			      Point_getSlope(*points[start]));
      }
      else {
	//we are on the left side of the grid, so add this point to the beginning of the horizon
	Horizon_addSectValues(h, -1 * infinity, Point_getSlope(*points[start]));
	Horizon_addSectValues(h, Point_getEndAngle(*points[start]),
			      DUMMY_SLOPE);
      }
    }
    //otherwise, add the dummy from 0 until the start angle of the point, then the point starting at start angle of the point, and then the dummy from the end angle of the point.
    else {
      Horizon_addSectValues(h, -1*infinity, DUMMY_SLOPE);
      Horizon_addSectValues(h, Point_getStartAngle(*points[start]), Point_getSlope(*points[start]));
      Horizon_addSectValues(h, Point_getEndAngle(*points[start]), DUMMY_SLOPE);
    }
    //the horizon has been set up, so return it
    return h;
  }
  
  //RECURSION!
  
  //compute the horizons for each half of the passed section 
  Horizon* h1 = visibility(start, (start+end)/2, points, numVisible, rightSide, recursionLevel + 1);
  Horizon* h2 = visibility((start+end)/2, end, points, numVisible, rightSide, recursionLevel + 1);
  
  //check all the points in h2 and make sure they are not occulded by h1
  int i;
  HSect* possibleOccluder;
  for(i = (start+end)/2; i < end; i++) {
    if(Point_getVis(*points[i]) == VISIBLE) {
      possibleOccluder = Horizon_findSectionForPoint(h1, *points[i]);
      if(HSect_getSlope(*possibleOccluder) > Point_getSlope(*points[i])) {
	//in this case, part of h1 occludes the point, so mark it as invisible
	points[i]->vis = INVISIBLE;
	//also, decrement the number of visible points
	(*numVisible)--;
      }
    }
    else {} //don't do anything with invisible points
  }
  
  Horizon* hNew = Horizon_merge(h1, h2);

  //print Horizon size, along with level of recursion and viewpoint
  //the columns for the horizon stats are
  //viewpoint col    viewpoint row     recursion level    size of merged horizon
  PRINT_HORIZON_STATS {
    printf("%d\t%d\n", recursionLevel, Horizon_getNumSect(*hNew));
    fflush(stdout);
  }

  Horizon_kill(h1);
  Horizon_kill(h2);
  return hNew;
}
