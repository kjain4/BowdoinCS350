/* Will Richard
 * Walkaround Algorithm
 * compareDouble.h
 */

#ifndef __compareDouble_H
#define __compareDouble_H

#define EPSILON .000001

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

//sees if 2 doubles are equal, meaning within EPSILON of eachother.
int doubleEqual(double d1, double d2);

//sees if d1 is greater than d2, within a margin of EPSILON
int doubleGreaterThan(double d1, double d2);

//sees if d1 is greater than or equal to d2, within a margin of EPSILON
int doubleGreaterThanOrEqual(double d1, double d2);

//sees if d1 is Less than d2, within a margin of EPSILON
int doubleLessThan(double d1, double d2);

//sees if d1 is Less than or equal to d2, within a margin of EPSILON
int doubleLessThanOrEqual(double d1, double d2);

#endif
