/* Will Richard
 * Walkaround Algorithm
 * compareDouble.c
 */

#include "compareDouble.h"


//compares 2 doubles.  If they are within EPSILON of eachother, returns true.  Otherwise, returns false
int doubleEqual(double d1, double d2) {
  return fabs(d1-d2) <= EPSILON;
}

//sees if d1 is greater than d2, within a margin of EPSILON
int doubleGreaterThan(double d1, double d2) {
  return d1+EPSILON > d2;
}

//sees if d1 is greater than or equal to d2, within a margin of EPSILON
int doubleGreaterThanOrEqual(double d1, double d2) {
  return doubleEqual(d1, d2) || doubleGreaterThan(d1, d2);
}

//sees if d1 is Less than d2, within a margin of EPSILON
int doubleLessThan(double d1, double d2) {
  return d1-EPSILON < d2;
}

//sees if d1 is Less than or equal to d2, within a margin of EPSILON
int doubleLessThanOrEqual(double d1, double d2) {
  return doubleEqual(d1, d2) || doubleLessThan(d1, d2);
}
