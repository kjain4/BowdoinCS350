/* Will Richard
 * Walkaround algroithm
 * Points.h
 */

#ifndef __Points_H
#define __Points_H

#define PI 3.14159

#define VISIBLE 1
#define INVISIBLE 0

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>

//Point structure.  Has information about a Grid point
typedef struct point_t {
  short elev; //elevation of the point
  short vis; //if the point is VISIBLE or INVISIBLE
/*   float slope; //the slope from the vp to the point. */
/*   double centerAngle; //the center angle of the point */
/*   //these variables should only be filled if the point is visible */
/*   double startAngle; // the start angle of this point. */
/*   double endAngle; //the end angle of this point. */
} Point;


//basic viewpoint structure
typedef struct viewpoint_t {
  short col;
  short row;
  short elev;
} Viewpoint;

//CONSTURCT AND DISTROY --------------------------------------------------------
//Create a new point, filling its elev
Point* Point_new(short elev);

//Create a new viewpoint
Viewpoint* Viewpoint_new(short col, short row, short elev);

//fills the passed viewpoint
void Viewpoint_fill(Viewpoint* vp, short col, short row, short elev);

//free the passed Point
void Point_kill(Point* point);

//free the passed viewpoint
void Viewpoint_kill(Viewpoint* vp);

//GETTERS ----------------------------------------------------------------------
short Point_getElev(Point p);
short Point_getVis(Point p);
/* float Point_getSlope(Point p); */
/* double Point_getCenterAngle(Point p); */
/* double Point_getStartAngle(Point p); */
/* double Point_getEndAngle(Point p); */

short Viewpoint_getElev(Viewpoint vp);
short Viewpoint_getCol(Viewpoint vp);
short Viewpoint_getRow(Viewpoint vp);

//SETTERS ----------------------------------------------------------------------
void Point_setElev(Point* p, short elev);
void Point_setVis(Point* p, short vis);
/* void Point_setSlopeAndCenterAngle(Point* p, short pCol, short pRow, Viewpoint vp); */
/* void Point_setStartAndEndAngle(Point* p, short pCol, short pRow, Viewpoint vp); */


//HELPERS ----------------------------------------------------------------------
//these are functions that calculate relationships between the Viewpoints and Points

//calculates the slope on the z plane form vp to p.
float Point_calcSlope(Viewpoint vp, Point p, short pCol, short pRow);

//Calculates the angle from the viewpoint to the point on the x-y plane.  This angle is equal to 0 if the point is directly to the right of the viewpoint, PI/2 if the point is directly above the viewpoint, etc.
double Point_calcCenterAngle(Viewpoint vp, short pCol, short pRow);

//Calculates the angle of the corner of the point that is reached first if you move in counterclockwise order around the viewpoint in the x-y plane
double Point_calcStartAngle(Viewpoint vp, short pCol, short pRow);

//Calculates the angle of the last corner of the point that is reached if you move around the viewpoint in counter clockwise order in the x-y plane
double Point_calcEndAngle(Viewpoint vp, short pCol, short pRow);

#endif 
