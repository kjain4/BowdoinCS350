/* Will Richard
 * Angle.c
 */

#include "Angle.h"

#define COMPARE_DEBUG if(0)


//CONSTURCT AND DISTROY --------------------------------------------------------
//create a new angle from x1,y1 to x2,y2
Angle* Angle_new(double x1, double y1, double x2, double y2) {
  //in this case, quadrant is the quadrant, value is opposite/adjacent or delta(y) over delta(x)
  Angle* nAngle = (Angle*) malloc(sizeof(Angle));
  assert(nAngle);

  // since, in this case, +y values are down and not up, but we are keeping the same arangement of quadrants and angles (with 1 in the top left and 4 in the bottom right) the computation of angle is slightly different than you would usually expect 
  nAngle->value = (y1- y2) / (x2-x1);
  //just for simplicity, make sure that the value is positive
/*   if(nAngle->value < 0) nAngle->value = nAngle->value * (-1); */

  if(y2 <= y1) { // either quadrant 1 or 2
    if(x2 >= x1) // pt 2 is up and to the left of pt 1 -> quadrant 1
      nAngle->quadrant = 1;
    else 
      nAngle->quadrant = 2;
  }
  else { // either in quadrant 3 or 4
    if(x2 >= x1) // pt 2 is down and to the lft of pt 1 -> quadrant 4
      nAngle->quadrant = 4;
    else
      nAngle->quadrant = 3;
  }

  return nAngle;
}

//fills in the passed angle with the angle from x1,y1 to x2,y2
void Angle_fill(Angle* a, double x1, double y1, double x2, double y2) {
  assert(a);

  // since, in this case, +y values are down and not up, but we are keeping the same arangement of quadrants and angles (with 1 in the top left and 4 in the bottom right) the computation of angle is slightly different than you would usually expect 
  a->value = (y1-y2) / (x2-x1);
  //just for simplicity, make sure that the value is positive
/*   if(a->value < 0) a->value = a->value * (-1); */

  if(y2 <= y1) { // either quadrant 1 or 2
    if(x2 >= x1) // pt 2 is up and to the left of pt 1 -> quadrant 1
      a->quadrant = '1';
    else 
      a->quadrant = '2';
  }
  else { // either in quadrant 3 or 4
    if(x2 >= x1) // pt 2 is down and to the lft of pt 1 -> quadrant 4
      a->quadrant = '4';
    else
      a->quadrant = '3';
  }
  
}

//kills the passed angle
void Angle_kill(Angle* a) {
  assert(a);
  free(a);
}

/**/

//UP - TANGENT VALUE - QUADRANT IMPLIMENTATION
//ALTERNATIVES.  EITHER THE CODE ABOVE THIS OR BELOW SHOULD BE UNCOMMENTED
//NOT BOTH.  
//DOWN - ACTUAL ANGLE VALUE IMPLIMENATION

/*
//CONSTURCT AND DISTROY --------------------------------------------------------
//create a new angle from x1,y1 to x2,y2
Angle* Angle_new(double x1, double y1, double x2, double y2) {
  Angle* nAngle = (Angle*) malloc(sizeof(Angle));
  assert(nAngle);

  // since, in this case, +y values are down and not up, but we are keeping the same arangement of quadrants and angles (with 1 in the top left and 4 in the bottom right) the computation of angle is slightly different than you would usually expect 

  //atan2 returns in the interval [-PI, PI], so add PI to adjust it correctly
  nAngle->value = atan2(y2-y1, x1-x1) + PI;

  //compute tangent based on value
  if(Angle_getValue(*nAngle) < PI/2)
    nAngle->quadrant = 1;
  else if(Angle_getValue(*nAngle) < PI)
    nAngle->quadrant = 2;
  else if(Angle_getValue(*nAngle) < 3*PI / 2)
    nAngle->quadrant = 3;
  else
    nAngle->quadrant = 4;

  return nAngle;

}

//fills in the passed angle with the angle from x1,y1 to x2,y2
void Angle_fill(Angle* a, double x1, double y1, double x2, double y2) {
  assert(a);
  
    // since, in this case, +y values are down and not up, but we are keeping the same arangement of quadrants and angles (with 1 in the top left and 4 in the bottom right) the computation of angle is slightly different than you would usually expect 

  //atan2 returns in the interval [-PI, PI], so add PI to adjust it correctly
  a->value = atan2(y2-y1, x1-x1) + PI;

  //compute tangent based on value
  if(Angle_getValue(*a) < PI/2)
    a->quadrant = '1';
  else if(Angle_getValue(*a) < PI)
    a->quadrant = '2';
  else if(Angle_getValue(*a) < 3*PI / 2)
    a->quadrant = '3';
  else
    a->quadrant = '4';
}

//kills the passed angle
void Angle_kill(Angle* a) {
  assert(a);
  free(a);
}

/**/

/******************************************************************************/
//getters and helpers are the same for both implimentations

//GETTERS ----------------------------------------------------------------------
int Angle_getQuad(Angle a) {
  return a.quadrant;
}

double Angle_getValue(Angle a) {
  return a.value;
}

//HELPERS ----------------------------------------------------------------------
//returns value >0 if a1 > a2, value =0 if a1 <= a2
int Angle_compare(Angle a1, Angle a2) {
  //if they are in different quadrants, then just compare quadrants
  COMPARE_DEBUG{
    printf("a1 = ");
    Angle_print(a1);
    printf("\na2 = ");
    Angle_print(a2);
    printf("\n");
    fflush(stdout);
  }
  //compare quadrants
  if(Angle_getQuad(a1) > Angle_getQuad(a2)) {
    COMPARE_DEBUG{printf("returning 1\n"); fflush(stdout);}
    return 1;
  }
  else if(Angle_getQuad(a1) == Angle_getQuad(a2)){ 
    //if quadrants are equal, compare angles
    if(Angle_getValue(a1) > Angle_getValue(a2)) {
      COMPARE_DEBUG{printf("returning 1\n"); fflush(stdout);}
      return 1;
    }
    else {
      COMPARE_DEBUG{printf("returning 0\n"); fflush(stdout);}
      return 0; 
    }   
  }
  else {
    COMPARE_DEBUG{printf("returning 0\n"); fflush(stdout);}
    return 0;
  }
}

//returns a value >0 if a1 == a2, 0 if a1 != a2
int Angle_equal(Angle a1, Angle a2) {
  return Angle_getQuad(a1) == Angle_getQuad(a2) && 
    Angle_getValue(a1) == Angle_getValue(a2);
}

//prints the passed angle
void Angle_print(Angle a) {
  printf("quadrant %c, value %f", Angle_getQuad(a), Angle_getValue(a));
}
