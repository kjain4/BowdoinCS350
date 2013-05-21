/* Will Richard
 * Points.c
 */

#include "Points.h"

//Construct and Distroy ----------------------------------------------------
//Create a new point.  Elevation of this point and the Pointh that is the viewpoint are passed, 
Point* Point_new(short elev) {
  //allocate the new point, and assert it
  Point* new = (Point*) malloc(sizeof(Point));
  assert(new);

  //set the elevation
  new->elev = elev;

  //all done - return
  return new;
}

//Fills the passed point's elev
void Point_fillElev(Point* p, short elev) {
  assert(p);
  p->elev = elev;
}


//Fills all values but elev of <p> based on the passed Viewpoint.
void Point_fillVp(Point* p, int i, int j, Viewpoint vp, int vi, int vj) {
  assert(p);

  //set visibilty to visible initially
  p->vis = VISIBLE;

  //calculate which radius the point is in
  p->radius = Point_calcRadius(vi, vj, i, j);

  //if point is viewpoint, explicitly set its distance to 0 - we will use that in the visibility function to identify the viewpoint.  Do not set any other value
  if(i == vi && j == vj) {
    p->distance = 0.0;
    return;
  }
  
  if(i > vi) p->toRight = 1;
  else p->toRight = 0;

  //calculate the distance from vp to p.  Needs to be done first - subsequent calculations rely upon it.
  p->distance = Point_calcDist(vi, vj, i, j);

  //calculate slope
  p->slope = Point_calcSlope(vp, *p);

  //calculate angles
  p->center_angle = Point_calcCenterAngle(vi, vj, i, j);
  p->start_angle = Point_calcStartAngle(vi, vj, i, j);
  p->end_angle = Point_calcEndAngle(vi, vj, i, j);
}


//Fill the passed ponit with the passed values.
void Point_fillWithValues(Point* p, short elev, short vis, float slope, float dist, double center_angle, double start_angle, double end_angle) {
  assert(p);
  p->elev = elev;
  p->vis = vis;
  p->slope = slope;
  p->distance = dist;
  p->center_angle = center_angle;
  p->start_angle = start_angle;
  p->end_angle = end_angle;
}



//Create the viewpoint.  Only elevation is passed.  Set to visible, and that is all that is set.
Viewpoint* Viewpoint_new(int elev) {
  //malloc vp
  Viewpoint* vp = (Viewpoint*) malloc(sizeof(Viewpoint));
  assert(vp);

  //set elev
  vp->elev = elev;

  //return vp
  return vp;
}

//fills the passed viewpoint
void Viewpoint_fill(Viewpoint* vp, int elev) {
  assert(vp);
  vp->elev = elev;
}

//free the passed point
void Point_kill(Point* point) {
  free(point);
}

//free the passed viewpoint
void Viewpoint_kill(Viewpoint* vp) {
  free(vp);
}

//GETTERS ----------------------------------------------------------------------
short Point_getElev(Point p) {
  return p.elev;
}

short Point_getVis(Point p) {
  return p.vis;
}

float Point_getSlope(Point p) {
  return p.slope;
}

float Point_getDist(Point p) {
  return p.distance;
}

int Point_getRadius(Point p) {
  return p.radius;
}

double Point_getCenterAngle(Point p) {
  return p.center_angle;
}

double Point_getStartAngle(Point p) {
  return p.start_angle;
}

double Point_getEndAngle(Point p) {
  return p.end_angle;
}

int Point_isRightOfVP(Point p) {
  return p.toRight;
}

short Viewpoint_getElev(Viewpoint vp) {
  return vp.elev;
}

//HELPERS ----------------------------------------------------------------------
//These functions calculate various relations between points

//calculates the slope on the z plane from vp to p.  Uses distance from vp to p, stored in p
float Point_calcSlope(Viewpoint vp, Point p) {
  float deltaZ, deltaX;
  //change in z values is change in elevation between the points.  Sign matters
  deltaZ = Point_getElev(p) - Viewpoint_getElev(vp);
  //change in x values is distance from vp to p.  This is stored in p.dist
  deltaX = Point_getDist(p);
  //slope is deltaZ / deltaX
  if(deltaZ < 0)
    return -1.0 * deltaZ * deltaZ / deltaX;
  else
    return deltaZ * deltaZ / deltaX; 
}

//calculates the distance from vp to p.  Is actually going to be the squared distance, since taking square roots is expensive.  Should be fine, as long as all distances are calculated this way.
float Point_calcDist(int vpi, int vpj, int pi, int pj) {
  return sqrt((pi-vpi) * (pi-vpi)) + ((pj - vpj) * (pj - vpj));
}

//the radius of a point which circle around the viewpoint the point is in, meaning if you draw concentric cirles of points around the viewpoint, the radius is which circle away from the viewpoint this point is in
int Point_calcRadius(int vpi, int vpj, int pi, int pj) {
  int yDiff = abs(vpj - pj);
  int xDiff = abs(vpi - pi);
  if(yDiff > xDiff) return yDiff;
  else return xDiff;
}


//calculates the angle on the x,y plane from vp to the center of p in radians.  This is the angle from vp to p where 0 is directly to the right of vp, PI/2 is directly up from vp, etc. 
double Point_calcCenterAngle(int vpi, int vpj, int pi, int pj){
  //in the case where the point should be at 0, make sure that it is exactly 0
  if(pj == vpj && pi > vpi) {
    return 0.0;
  }

  //return the angle, using atan2
  //atan2 returns in the interval [-PI, PI], so add PI to adjust it correctly
  return atan2(pj-vpj, vpi-pi) + PI;

/*   //return the tangent value - opposite over adjacent */
/*   return (double)(pj - vpj) / (double)(vpi - pi); */
}

//calculate the angle on the x,y plate from the vp to the first corner of p in counter-clockwise order in radians.
double Point_calcStartAngle(int vpi, int vpj, int pi, int pj) {
  //set the i and j values for the start point
  double starti, startj;
  if(pi < vpi) { startj = pj - .5; }
  else if(vpi == pi) { 
    if(pj > vpj) { startj = pj + .5; }
    else { startj = pj - .5; }
  }
  else { startj = pj + .5; }
  
  if(pj < vpj) {starti = pi + .5; }
  else if(vpj == pj) { 
    if(pi > vpi) { starti = pi + .5; }
    else { starti = pi - .5; }
  }
  else { starti = pi - .5; }
  
  //return the angle, using atan2
  //atan2 returns in the interval [-PI, PI], so add PI to adjust it correctly
  return atan2(startj-vpj, vpi-starti) + PI;
  
/*   //return the tangent value - opposite over adjacent */
/*   return (startj - vpj) / (vpi - starti); */
}

//calculate the angle on the x,y plate from the vp to the last corner of p in counter-clockwise order in radians.
double Point_calcEndAngle(int vpi, int vpj, int pi, int pj) {
  //set the i and j values for the end point
  double endi, endj;
  if(vpi > pi) { endj = pj + .5; }
  else if(vpi == pi) { 
    if(pj > vpj) { endj = pj + .5; }
    else { endj = pj - .5; }
  }
  else { endj = pj - .5; }

  if(vpj > pj) { endi = pi - .5; }
  else if(vpj == pj) { 
    if(pi > vpi) { endi = pi + .5; }
    else { endi = pi - .5; }
  }
  else { endi = pi + .5; }

  //return the angle, using atan2
  //atan2 returns in the interval [-PI, PI], so add PI to adjust it correctly
  return atan2(endj-vpj, vpi-endi) + PI;

/*   //return the tangent value - opposite over adjacent */
/*   return (endj - vpj) / (vpi - endi); */
}

//comparitor function for qsort (or bsearch).  In this case, we're going to be sorting Point*
int PointPointer_compareByDist(const void* a, const void* b) {
  assert(a);
  assert(b);

  Point** p1 = (Point**) a;
  Point** p2 = (Point**) b;

  if((*p1)->distance > (*p2)->distance) return 1;
  else if((*p1)->distance == (*p2)->distance) return 0;
  else return -1;
}

//sorts the passed Point* array by distance using a counting sort.  Returns the sorted array.
void PointPointer_sortByDist(Point** points, int pointsLength, int maxDist) {
  assert(points);

  //the count array will keep a count for how many points are at a given radius from the viewpoint.  i.e. count[i] will have the number of points that are distance <i> away from the viewpoint.  For this reason, we need one extra spot
  int* count = (int*) calloc((maxDist+1), sizeof(int));
  assert(count);

  //counter for for loops
  int i;

  //make sure they are all set to 0 - MAYBE NOT NECESSARY
  /* @jfishman - not necessary, since using 'calloc' above
  for(i = 0; i < maxDist+1; i++) {
    count[i] = 0;
  }
  */

  //go through <points> and fill the <count> array.
  for(i = 0; i < pointsLength; i++) {
    assert(points[i]);
    count[Point_getRadius(*points[i])]++;
  }

  //now, update the count array so count[i] holds the starting index for the points with radius i.
  for(i = 1; i < maxDist+1; i++) {
    count[i] = count[i] + count[i-1];
    assert(count[i] <= pointsLength);
  }
  // we need to decrement by exactly 1, since we want to start at index 0
  for (i = 0; i < maxDist + 1; i++) {
    if (count[i]) {
      count[i] --;
      assert(count[i] < pointsLength);
    }
  }

  //make a new array to hold all the points
  Point** sorted = (Point**) malloc(sizeof(Point*) * pointsLength);
  assert(sorted);

  int curRadius;
  //Now, place the each point in the new array
  for(i = 0; i < pointsLength; i++) {
    curRadius = Point_getRadius(*points[i]);
    //place the point in the index stored in count for this radius
    assert(count[curRadius] < pointsLength);
    sorted[count[curRadius]] = points[i];
    //decrement the index for this radius
    count[curRadius] --;
  }

  for(i = 0; i < pointsLength; i++) {
    points[i] = sorted[i];
  }

  free(sorted);
  free(count);
}


//print the passed point
void Point_print(Point p) {
  printf("elev = %d, slope = %f, dist = %f, center = %f, start = %f, end = %f\n", Point_getElev(p), Point_getSlope(p), Point_getDist(p), Point_getCenterAngle(p), Point_getStartAngle(p), Point_getEndAngle(p));
}
