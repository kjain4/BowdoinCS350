/* Will Richard
 * Angle.h
 */

#ifndef __Angle_H
#define __Angle_H

#define PI 3.14159

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>

//ONLY ONE ANGLE STRUCTURE AT ANY TIME SHOULD BE UNCOMMENTED

//Angle sturcture.  Exists to try to aviod computing real angle with arctan, but still have and compare angle
typedef struct angle_t {
  char quadrant; //storest which quadrant the angle is in
  double value; //the value of tangent of the angle.  In one implimentation, this is i.e. opposite / adjacent, in the other its the actual angle value, computed with arctangent
} Angle;

//CONSTURCT AND DISTROY --------------------------------------------------------
//create a new angle from x1,y1 to x2,y2
Angle* Angle_new(double x1, double y1, double x2, double y2);

//fills in the passed angle with the angle from x1,y1 to x2,y2
void Angle_fill(Angle* a, double x1, double y1, double x2, double y2);

//kills the passed angle
void Angle_kill(Angle* a);

//HELPERS ----------------------------------------------------------------------
//returns 1 if a1 > a2, 0 if a1 == a2, -1 if a1 < a2
int Angle_compare(Angle a1, Angle a2);

//returns 1 if a1 == a2, 0 if a1 != a2
int Angle_equal(Angle a1, Angle a2);

//prints the passed angle
void Angle_print(Angle a);

//GETTERS ----------------------------------------------------------------------
int Angle_getQuad(Angle a);
double Angle_getValue(Angle a);

#endif
