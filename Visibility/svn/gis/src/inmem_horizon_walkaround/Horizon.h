/* Will Richard
 * Horizon.h
 */

#ifndef __Horizon_H
#define __Horizon_H

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>

#include "Points.h"

//Horizon section structure.
typedef struct horizon_sec_t {
  double startAngle;  //the start angle of this section of the horizon
  float slope;  // the slope of the point that this section corresponds to.
} HSect;

typedef struct horizon_t {
  HSect* sections; //array of sections that make up the horizon
  int numSect; //number of sections currently filled
  int size; // the size of the array
} Horizon;

//CONSTRUCT AND DISTROY --------------------------------------------------------
//Create a new horizon section.
HSect* HSect_new(double startAngle, float slope);

//Fill a HSect
void HSect_fill(HSect* hs, double startAngle, float slope);

//Create a new horizon with no sections, but array size of passed value
Horizon* Horizon_new(int size);

//free the passed horizon section.
void HSect_kill(HSect* hs);

//free the passed horizon and all the sections within it.
void Horizon_kill(Horizon* h);

//GETTERS ----------------------------------------------------------------------
double HSect_getStartAngle(HSect hs);

float HSect_getSlope(HSect hs);

//get the ith section of the horizon
HSect* Horizon_getSect(Horizon* h, int i);

//get the angle of the ith section of the horizon
double Horizon_getSectAngle(Horizon* h, int i);

//get the slope if the ith section of the horizon
float Horizon_getSectSlope(Horizon* h, int i);

//get the size of the Horizon
int Horizon_getSize(Horizon h);

//get the number of sections in the Horizon
int Horizon_getNumSect(Horizon h);

//SETTERS ----------------------------------------------------------------------
//sets the start angle of the passed section
void HSect_setAngle(HSect* hs, double newAngle);

//Horizon HELPERS --------------------------------------------------------------
//adds a sections to the horizon
void Horizon_addSect(Horizon* h, HSect hs);
void Horizon_addSectValues(Horizon* h, double startAngle, float slope);

//grow the horizon array
void Horizon_grow(Horizon* h);

/* //return the HSect in the horizon that may occulde the passed Point. */
/* HSect* Horizon_findSectionForPoint(Horizon* h, Point p); */

//merge the 2 passed horizons, and return the resulting horizon
Horizon* Horizon_merge(Horizon* h1, Horizon* h2);

/* //inserts a section at index <i>, splitting or adjusting sections so the horizon is still correct */
/* void Horizon_insertSect(Horizon* h, int i, double startAngle, float slope, double endAngle); */

/* //adds the passed HSect at index i, then shifts all the rest of the sections right one. */
/* void Horizon_addAndShift(Horizon* h, int i, HSect new); */

/* //deletes the section at index i, then shefts the rest of the sections left one. */
/* void Horizon_deleteAndShift(Horizon* h, int i); */

//HSect HELPERS ----------------------------------------------------------------
//print out the horizon
void Horizon_print(Horizon* h);

//print out the passed HSect
void HSect_print(HSect hs);

#endif
