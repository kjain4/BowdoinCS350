/* Will Richard
 * Visibility.c
 */

#include "Visibility.h"

/* recursivly computes the visibility of set a points.  The points should be ordered by distance, from closest to furthest away.
   BASE CASE:  When there is only one point cosidered, it creates a new Horizon with dummy points surrounding the one point
   RECURSION: Split the range of points (going from index "start" to index "end", not including end) into 2 by distance, and calculate the horizon for each half. It then checks to make sure that any points in the 2nd half that are occluded by the 1st horizon are marked as inivsible.  Finally, it merges the 2 halves together using Horizon_merge (see Horizon.h).
*/
Horizon* visibility(int start, int end, Point** points, int* numVisible) {
  assert(points);
  //BASE CASE - when there is only one point left, create a horizon with it, and return that horizon
  if(end-start <= 1) {
    //if the distance of the point is 0, it is the viewpoint.  Return a horizon that is just a dummy
    if(Point_getDist(*points[start]) == 0.0) {
      Horizon* h = Horizon_new(1);
      Horizon_addSectValues(h, 0.0, DUMMY_SLOPE);
      return h;
    }
    //otherwise, it is a real point - treat it as such
    //make the new horizon with size 3 - 2 dummy sections and 1 real section
    Horizon* h = Horizon_new(3);
    
    /*points were the center angle is at 0 rad need to be split into 2 horizon sections.  One section from 0 to the points's end angle, one section from the point's start angle to 2*PI.*/
    if(Point_getCenterAngle(*points[start]) == 0) {
      Horizon_addSectValues(h, 0.0, 
			    Point_getSlope(*points[start]));
      Horizon_addSectValues(h, Point_getEndAngle(*points[start]),
			    DUMMY_SLOPE);
      Horizon_addSectValues(h, Point_getStartAngle(*points[start]),
			    Point_getSlope(*points[start]));
    }
    //otherwise, add the dummy from 0 until the start angle of the point, then the point starting at start angle of the point, and then the dummy from the end angle of the point
    else {
      Horizon_addSectValues(h, 0.0, DUMMY_SLOPE);
      Horizon_addSectValues(h, Point_getStartAngle(*points[start]), Point_getSlope(*points[start]));
      Horizon_addSectValues(h, Point_getEndAngle(*points[start]), DUMMY_SLOPE);
    }
    return h;
  }
  
  //RECURSION!
  
  //compute the horizons for each half of the passed section 
  Horizon* h1 = visibility(start, (start+end)/2, points, numVisible);
  Horizon* h2 = visibility((start+end)/2, end, points, numVisible);
  
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
  Horizon_kill(h1);
  Horizon_kill(h2);
  return hNew;
}
