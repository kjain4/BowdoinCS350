/* Will Richard
 * Points.h
 */

#ifndef __Point_H
#define __Point_H

#define PI 3.14159

#define VISIBLE 1
#define INVISIBLE 0

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>

//Point structure.  Has all info about a given point on the grid
typedef struct point_t {
  short elev;  //elevation of point
  short vis;  //visibility of point.  0 for inivisible, 1 for visible
  float slope;  //slope from viewshed to point
  float distance;  //distance from viewshed to point
  int radius; //if you draw concentric cirles (or squares) of points around the viewshed, this stores which circle (or square) the point is in - the point is in the <radius>th circle from the viewpoint
  double center_angle;  //angle from viewshed to center of the point, were 0 is horezontal line to the right
  double start_angle; //angle from the viewshed to first corner of the point
  double end_angle; //angle from the viewpoint to the last corner of the point
  int toRight; //1 if point is to the right of viewpoint, 0 if point is to the left of the viewpoint
} Point;

//Viewpoint Structure.  Has all the information about the viewpoint
typedef struct viewpoint_t {
  short elev;
} Viewpoint;

//CONSTRUCT AND DISTROY --------------------------------------------------------
//Create a new point, but only fills in elev.  All other values need to be set uusing Point_fillVp.
Point* Point_new(short elev);

//Fills the passed point's elev
void Point_fillElev(Point* p, short elev);

//Fills all values but elev of <p> based on the passed Viewpoint.
void Point_fillVp(Point* p, int i, int j, Viewpoint vp, int vi, int vj);

//Fill the passed ponit with the passed values.
void Point_fillWithValues(Point* p, short elev, short vis, float slope, float dist, double center_angle, double start_angle, double end_angle);

//Create the viewpoint.  Only elevation is passed.  Set to visible, and that is all that is set.
Viewpoint* Viewpoint_new(int elev);

//fills the passed viewpoint
void Viewpoint_fill(Viewpoint* vp, int elev);

//free the passed point
void Point_kill(Point* point);

//free the passed viewpoint
void Viewpoint_kill(Viewpoint* vp);

//GETTERS ----------------------------------------------------------------------
short Point_getElev(Point p);

short Point_getVis(Point p);

float Point_getSlope(Point p);

float Point_getDist(Point p);

int Point_getRadius(Point p);

double Point_getCenterAngle(Point p);
double Point_getStartAngle(Point p);
double Point_getEndAngle(Point p);

int Point_isRightOfVP(Point p);

short Viewpoint_getElev(Viewpoint vp);

//HELPERS ----------------------------------------------------------------------
//These functions calculate various relations between points

//calculates the slope on the z plane from vp to p.  Uses distance from vp to p, stored in p
float Point_calcSlope(Viewpoint vp, Point p);

//calculates the distance from vp to p.  Is actually going to be the squared distance, since taking square roots is expensive.  Should be fine, as long as all distances are calculated this way.
float Point_calcDist(int vpi, int vpj, int pi, int pj);

//the radius of a point which circle around the viewpoint the point is in, meaning if you draw concentric cirles of points around the viewpoint, the radius is which circle away from the viewpoint this point is in
int Point_calcRadius(int vpi, int vpj, int pi, int pj);

//calculates the angle on the x,y plane from vp to p.  This is the angle from vp to p where 0 is directly to the right of vp, PI/2 is directly up from vp, etc. 
double Point_calcCenterAngle(int vpi, int vpj, int pi, int pj);

//calculate the angle on the x,y plate from the vp to the first corner of p in counter-clockwise order in radians.
double Point_calcStartAngle(int vpi, int vpj, int pi, int pj);

//calculate the angle on the x,y plate from the vp to the last corner of p in counter-clockwise order in radians.
double Point_calcEndAngle(int vpi, int vpj, int pi, int pj);

//comparitor function for qsort (or bsearch).  In this case, we're going to be sorting Point*
int PointPointer_compareByDist(const void* a, const void* b);

//sorts the past Point* array by distance using a counting sort.  Modifies the passed array.
void PointPointer_sortByDist(Point** points, int pointsLength, int maxDist);

//print the passed point
void Point_print(Point p);

#endif
