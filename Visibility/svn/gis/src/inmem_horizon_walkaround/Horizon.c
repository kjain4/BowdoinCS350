/* Will Richard
 * Horizon.c
 */

#include "Horizon.h"

#define MERGE_DEBUG if(0)
#define FIND_DEBUG if(0)
#define INSERT_DEBUG if(0)

//CONSTRUCT AND DISTROY --------------------------------------------------------
//Create a new horizon section.
HSect* HSect_new(double startAngle, float slope) {

  //malloc and asset the new HSect
  HSect* new = (HSect*) malloc(sizeof(HSect));
  assert(new);

  //store the passed angle and slope
  new->startAngle = startAngle;
  new->slope = slope;

  //return the HSect
  return new;
}

//Fill a HSect
void HSect_fill(HSect* hs, double startAngle, float slope) {
  assert(hs);

  //fill the passed HSect
  hs->startAngle = startAngle;
  hs->slope = slope;
}

//Create a new horizon with no sections, but array size of passed value
Horizon* Horizon_new(int size) {

  //malloc and assert the new horizon
  Horizon* h = (Horizon*) malloc(sizeof(Horizon));
  assert(h);
  
  //set the size of the Horizon.  If the size passed is <=0, use 10
  if(size <= 0) { h->size = 10; }
  else { h->size = size; }

  //set the number of sections in the horizon
  h->numSect = 0;

  //malloc the Horizon array
  h->sections = (HSect*) malloc(sizeof(HSect) * h->size);
  assert(h->sections);

  //return
  return h;
}

//free the passed horizon section.
void HSect_kill(HSect* hs) {
  assert(hs);
  free(hs);
}

//free the passed horizon and all the sections within it.
void Horizon_kill(Horizon* h) {
  assert(h);
  //free the array
  free(h->sections);
  //free the horizon
  free(h);
}

//GETTERS ----------------------------------------------------------------------
double HSect_getStartAngle(HSect hs) {
  return hs.startAngle;
}

float HSect_getSlope(HSect hs) {
  return hs.slope;
}

//get the ith section of the horizon
HSect* Horizon_getSect(Horizon* h, int i) {
  assert(i >=0);
  assert(i < Horizon_getNumSect(*h));
  return &h->sections[i];
}

//get the angle of the ith section of the horizon
double Horizon_getSectAngle(Horizon* h, int i) {
  return HSect_getStartAngle(*Horizon_getSect(h, i));
}

//get the slope if the ith section of the horizon
float Horizon_getSectSlope(Horizon* h, int i) {
  return HSect_getSlope(*Horizon_getSect(h, i));

}

//get the size of the Horizon
int Horizon_getSize(Horizon h) {
  return h.size;
}

int Horizon_getNumSect(Horizon h) {
  return h.numSect;
}

//SETTERS ----------------------------------------------------------------------
//sets the start angle of the passed section
void HSect_setAngle(HSect* hs, double newAngle) {
  assert(hs);
  hs->startAngle = newAngle;
}

//Horizon HELPERS --------------------------------------------------------------

//adds a sections to the horizon - not sure if this will work
void Horizon_addSect(Horizon* h, HSect hs) {
  assert(h);
  //grow the horizon array, if necessary
  if(Horizon_getNumSect(*h)+1 == Horizon_getSize(*h)) { Horizon_grow(h); }

  //if the startAngle of the last section of the horizon is equal to the passed start angle, overwrite the angle currently in the horizon
  if(Horizon_getNumSect(*h) > 0 &&
     HSect_getStartAngle(hs) == Horizon_getSectAngle(h, Horizon_getNumSect(*h)-1)) {
    h->sections[Horizon_getNumSect(*h)-1] = hs;
  }
  else {
    //add the section to the horizon
    h->sections[Horizon_getNumSect(*h)] = hs;
    h->numSect++;
  }
  assert(Horizon_getNumSect(*h) < Horizon_getSize(*h));
}

void Horizon_addSectValues(Horizon* h, double startAngle, float slope) {
  assert(h);
  //grow the horizon array, if necessary
  if(Horizon_getNumSect(*h)+1 == Horizon_getSize(*h)) Horizon_grow(h);

  //if the startAngle of the last section of the horizon is equal to the passed start angle, overwrite the angle currently in the horizon
   if(Horizon_getNumSect(*h) > 0 &&
     startAngle == Horizon_getSectAngle(h, Horizon_getNumSect(*h)-1)) {
    HSect_fill(&(h->sections[Horizon_getNumSect(*h)-1]), startAngle, slope);
  }
  else {
    //fill the section of the horizon with the passed values
    HSect_fill(&(h->sections[Horizon_getNumSect(*h)]), startAngle, slope);
    h->numSect++;
  }
  assert(Horizon_getNumSect(*h) < Horizon_getSize(*h));
}

//grow the horizon array by doubling its size
void Horizon_grow(Horizon* h) {
  assert(h);

  h->sections = (HSect*) realloc(h->sections, Horizon_getSize(*h) * 2 * sizeof(HSect));
  assert(h->sections);

  h->size = Horizon_getSize(*h) * 2;

  fflush(stdout);
}

/* //return the HSect in the horizon that may occulde the passed Point. */
/* HSect* Horizon_findSectionForPoint(Horizon* h, Point p) { */
/*   assert(h); */

/*   FIND_DEBUG{ */
/*     printf("tring to find section for point with angle %f slope %f elev %d dist %f in:\n", Point_getCenterAngle(p), Point_getSlope(p), Point_getElev(p), Point_getDist(p)); */
/*     Horizon_print(*h); */
/*     printf("\n"); */
/*     fflush(stdout); */
/*   } */

/*   //use a binary search to find the correct section */
/*   int low = 0; */
/*   int high = Horizon_getNumSect(*h)-1; */
/*   int mid; */
/*   while(low <= high) { */
/*   //get the midpoint between high and low */
/*     mid = low + ((high-low)/2); */

/*     // If the section at mid is the last in the horizon, then it is the correct section, and should be returned. */
/*     if(mid == Horizon_getNumSect(*h) -1) return Horizon_getSect(*h, mid); */

/*     //if the angle for the section at mid is greater than the point's angle, then there is no way that this could be the point - continue with the binary search using the part of the horizon before mid */
/*     if(Horizon_getSectAngle(*h, mid) > Point_getCenterAngle(p)) { */
/*       high = mid - 1; */
/*   } */
/*     /\*if the section at mid has a lower or equal angle than the point, one of 2 things may be the case. */
/*       1) The section at mid+1 has a greater starting angle than the point, in which case the section at index mid is the correct one, so we should return it.   */
/*       2) The point at mid+1 has starting angle less than the point, in which case we should continue the binary search, with a lower bound of mid */
/*     *\/ */
/*     else { // meaning the section at mid has angle <= the center angle of p */
/*       //case 1 */
/*       if(Horizon_getSectAngle(*h, mid+1) > Point_getCenterAngle(p)) { */
/* 	return Horizon_getSect(*h, mid); */
/*       } */
/*       //case 2 */
/*       else { */
/* 	low = mid + 1; */
/*       } */
/*     }  */
/*   } */
/*   //if things go bad, exit - it should never happen */
/*   printf("search for a point has gone bad\n"); */
/*   exit(2); */
/* } */

//merge the 2 passed horizons, and return the resulting horizon
Horizon* Horizon_merge(Horizon* h1, Horizon* h2) {
  assert(h1);
  assert(h2);

  MERGE_DEBUG{printf("starting merge\n"); fflush(stdout);}

  MERGE_DEBUG{
    printf("h1:\n");
    Horizon_print(h1);
    printf("h2:\n");
    Horizon_print(h2);
    printf("\n");
  }

  //catch the possibility that h1 or h2 is empty
  if(Horizon_getNumSect(*h1) == 0)
    return h2;
  if(Horizon_getNumSect(*h2) == 0)
    return h1;

  Horizon* hNew = Horizon_new(Horizon_getNumSect(*h1) + Horizon_getNumSect(*h2));
  assert(hNew);

  HSect curMax; //stores the section relating to the current max value
  int curMaxH; //stores which horizon that curMax came from
  int h1index = 0, h2index = 0;
  
  //set initial currMax
  if(HSect_getSlope(*Horizon_getSect(h1, 0)) > HSect_getSlope(*Horizon_getSect(h2, 0))) {
    //store the first element for horizon 1's info into curMax
    HSect_fill(&curMax, Horizon_getSectAngle(h1, 0), 
	       HSect_getSlope(*Horizon_getSect(h1, 0)));
    //set curMaxH
    curMaxH = 1;
    //increment h1index, to reflect what should be looked at next
    h1index++;
    //put the information from the first sect of the first horizon into the new horizon
    Horizon_addSect(hNew, curMax);
  }
  else {
    //store the first element for horizon 2's info into curMax
    HSect_fill(&curMax, Horizon_getSectAngle(h2, 0), 
	       HSect_getSlope(*Horizon_getSect(h2, 0)));
    //set curMaxH
    curMaxH = 2;
    //increment h2index, to reflect what should be looked at next
    h2index++;
    //put the information from the first sect of the 2nd horizon into the new horizon
    Horizon_addSect(hNew, curMax);
  }
  MERGE_DEBUG{printf("adding sect with angle = %f and slope = %f\n", HSect_getStartAngle(curMax), HSect_getSlope(curMax)); fflush(stdout);}


  HSect next; //stores the HSect that should be looked next
  int nextH; //stores the horizon from which that HSect originated
  while(h1index < Horizon_getNumSect(*h1) || h2index < Horizon_getNumSect(*h2)) {    
    //if h1 or h2 has reached the end of the horizon, fill next from the other horizon
    if(h1index >= Horizon_getNumSect(*h1)) {
      next = *Horizon_getSect(h2, h2index);
      nextH = 2;
      h2index++;
    }
    else if(h2index >= Horizon_getNumSect(*h2)) {
      next = *Horizon_getSect(h1, h1index);
      nextH = 1;
      h1index++;
    }
    //otherwise, investigate the next lowest angle
    else if(Horizon_getSectAngle(h1, h1index) < Horizon_getSectAngle(h2, h2index)){
      //the next sect is from h1
      next = *Horizon_getSect(h1, h1index);
      nextH = 1;
      //the increment the h1index
      h1index++;
    }
    else {
      //the next sect is from h2
      next = *Horizon_getSect(h2, h2index);
      nextH = 2;
      h2index++;
    }

    MERGE_DEBUG{printf("next sect from h%d: angle = %f, slope = %f\n", nextH, HSect_getStartAngle(next), HSect_getSlope(next)); fflush(stdout);}

    //if the next sect and the curMax are from the same horizon, need to check both horizons to find the new curMax
    if(nextH == curMaxH) {
      //if next is from horizon 1, then next overlaps with HSect[h2index-1]
      //so, in that case, compare those 2 values, and add the higher one
      if(nextH == 1) {
	if(HSect_getSlope(next) >= Horizon_getSectSlope(h2, h2index-1)) {
	  HSect_fill(&curMax, HSect_getStartAngle(next), HSect_getSlope(next));
	  curMaxH = 1;
	  MERGE_DEBUG{printf("adding sect with angle = %f and slope = %f\n", HSect_getStartAngle(curMax), HSect_getSlope(curMax)); fflush(stdout);}
	  Horizon_addSect(hNew, curMax); 
	}
	else { //nextH == 1, but the slope at this angle from h2 is greater than the slope at this angle from h1
	  HSect_fill(&curMax, HSect_getStartAngle(next), Horizon_getSectSlope(h2, h2index-1));
	  curMaxH = 2;
	  MERGE_DEBUG{printf("adding sect with angle = %f and slope = %f\n", HSect_getStartAngle(curMax), HSect_getSlope(curMax)); fflush(stdout);}
	  Horizon_addSect(hNew, curMax); 
	}
      }
      //else, do the same, but switch h1index and h2index
      else { // meaning nextH == 2
	//so, we care about comparing the sect from h2 at h2index and the sect from h1index at h1index -1
	if(HSect_getSlope(next) >= Horizon_getSectSlope(h1, h1index-1)) {
	  HSect_fill(&curMax, HSect_getStartAngle(next), HSect_getSlope(next));
	  curMaxH = 2;
	  MERGE_DEBUG{printf("adding sect with angle = %f and slope = %f\n", HSect_getStartAngle(curMax), HSect_getSlope(curMax)); fflush(stdout);}
	  Horizon_addSect(hNew, curMax); 
	}
	else {//nextH == 2, but the slope at this angle from h1 is greater than the slope at this angle from h2
	  HSect_fill(&curMax, HSect_getStartAngle(next), Horizon_getSectSlope(h1, h1index -1));
	  curMaxH = 1;
	  MERGE_DEBUG{printf("adding sect with angle = %f and slope = %f\n", HSect_getStartAngle(curMax), HSect_getSlope(curMax)); fflush(stdout);}
	  Horizon_addSect(hNew, curMax); 	  
	}
      }
    }
    else { //the curMax and next value are from different horizons, so you only need to check the next versus the curMax
      if(HSect_getSlope(next) > HSect_getSlope(curMax)){
	//since the next value is higher than the curMax, make the curMax the next value
	curMax = next;
	curMaxH = nextH;
	MERGE_DEBUG{printf("adding sect with angle = %f and slope = %f\n", HSect_getStartAngle(curMax), HSect_getSlope(curMax)); fflush(stdout);}
	Horizon_addSect(hNew, curMax);
      }
      else {
	MERGE_DEBUG{printf("not adding any sect\n"); fflush(stdout);}
      } //the curMax is still the max value, so nothing needs to be done
    }
  }  

  MERGE_DEBUG{printf("ending merge\n********************\n"); fflush(stdout);}

  //hNew should be filled completely - return
  return hNew;
}


/* //inserts a section that has a center angle at index <i>, splitting or adjusting sections so the horizon is still correct */
/* void Horizon_insertSect(Horizon* h, int i, double startAngle, float slope, double endAngle) { */
/*   assert(h); */

/*   INSERT_DEBUG{ */
/*     printf("starting Horizon_insertSect on section with start %f, slope %f, end %f, trying to put it in index %d\n", startAngle, slope, endAngle, i); */
/*     fflush(stdout); */
/*   } */

/*   //adjust i so that it corresponds to the section including startAngle */
/*   while(Horizon_getSectAngle(*h, i) > startAngle) { */
/*     INSERT_DEBUG { */
/*       printf("the section angle at index %d (which is %f) is greater than the start angle, %f, so we are decrementing the index\n", i, Horizon_getSectAngle(*h, i), startAngle); */
/*       fflush(stdout); */
/*     } */
/*     i--; */
/*   } */
/*   assert(Horizon_getSectAngle(*h, i) <= startAngle); */

/*   //create a HSect for the section we will insert. */
/*   HSect newSect; */
/*   HSect_fill(&newSect, startAngle, slope); */

/*   //catch the point directly to the right of the viewpoint. */
/*   //we will identify this point because the start angle will be near 2PI, but the end angle will be near 0, or in other words, the start angle will be greater than the end angle */
/*   if(startAngle > endAngle) { */
/*     //adjust the current section at angle 0.0 to start at this point's end angle */
/*     HSect_setAngle(Horizon_getSect(*h, 0), endAngle); */
/*     //now, insert the newSect section, starting at angle 0.0 in index 0 */
/*     HSect_setAngle(&newSect, 0.0); */
/*     Horizon_addAndShift(h, 0, newSect); */

/*     //add a section at the very last segment, starting at the start angle */
/*     HSect_setAngle(&newSect, startAngle); */
/*     Horizon_addAndShift(h, Horizon_getNumSect(*h), newSect); */
/*     return; */
/*   } */

/*   //catch the case where there is only one segment in the horizon, or i is at the last segment.  In that case, we need to add <newSect>, and add a segment with the slope of the one segment starting at <endAngle> */
/*   //if there is only one segment, then */
/*   if(i == Horizon_getNumSect(*h)-1) { */
/*     //add the segment that goes after newSect */
/*     HSect after; */
/*     HSect_fill(&after, endAngle, Horizon_getSectSlope(*h, i)); */
/*     Horizon_addAndShift(h, i+1, after); */
/*     //add newSect */
/*     Horizon_addAndShift(h, i+1, newSect); */
/*   } */
/*   //if the end angle of the section at i is greater than endAngle, then we know the section at i fully blocks <newSect> */
/*   //note, the end angle for the section currently at i is the start angle of the section at i+1 */
/*   else if(Horizon_getSectAngle(*h, i+1) > endAngle) { */
/*     //in this case, we need to add a section with the same slope as the section at i, but starting at the end angle of newSect */
/*     HSect after; */
/*     HSect_fill(&after, endAngle, Horizon_getSectSlope(*h, i)); */
/*     Horizon_addAndShift(h, i+1, after); */
/*     //now, add newSect at index i+1 */
/*     Horizon_addAndShift(h, i+1, newSect); */
/*   } */
/*   //otherwise, we know that <newSect> falls on the border of 2 or more sections */
/*   else { */
/*     //make sure newSect does not cover any sections completely.  If it does, delete those sections */
/*     //we check this my seeing if the end angle of the section at index i+1 (meaning the start angle of the section at i+2) is less than the end angle of our newSect section. */
/*     while (Horizon_getSectAngle(*h, i+2) < endAngle && i+2 < Horizon_getNumSect(*h)) { */
/*       Horizon_deleteAndShift(h, i+1); */
/*     } */

/*     //now, we need to change the startAngle of the section at i+1 to match the end angle of <newSect> */
/*     HSect_setAngle(Horizon_getSect(*h, i+1), endAngle); */
    
/*     //add newSect */
/*     Horizon_addAndShift(h, i+1, newSect); */
/*   } */

/*   //try to remove slivers */
/*   //if the startAngle and the angle of the section at i are equal, delete the section at i */
/*   if(startAngle == Horizon_getSectAngle(*h, i)) */
/*      Horizon_deleteAndShift(h, i);  */
/* } */

/* //adds the passed HSect at index i, then shifts all the rest of the sections right one. */
/* void Horizon_addAndShift(Horizon* h, int i, HSect newSect) { */
/*   assert(h); */
  
/*   //make sure i is valid */
/*   assert(i <= Horizon_getNumSect(*h)); */
/*   assert(i >= 0);   */

/*   //grow the array, if necessary */
/*   if(Horizon_getNumSect(*h) + 1 >= Horizon_getSize(*h)) { */
/*     Horizon_grow(h); */
/*   } */

/*   //increment the number of sections in h */
/*   h->numSect++; */

/*   //we will add and shift by storing the value currently at <i> in <old>, inserting <newSect>, then moving <old> into <newSect> and incrementing <i> */
/*   HSect old; */

/*   for(; i < Horizon_getNumSect(*h); i++) { */
/*     old = *Horizon_getSect(*h, i); */
/*     h->sections[i] = newSect; */
/*     newSect = old; */
/*   } */
/* } */

/* //deletes the section at index i, then shefts the rest of the sections left one. */
/* void Horizon_deleteAndShift(Horizon* h, int i) { */
/*   assert(h); */

/*   //make sure <i> is valid */
/*   assert(i < Horizon_getNumSect(*h)); */
/*   assert(i >=0); */

/*   //for every section, we will move it left one slot, thereby deleting the section at i and shifting everything over 1. */
/*   for(; i < Horizon_getNumSect(*h)-1; i++) { */
/*     h->sections[i] = *Horizon_getSect(*h, i+1); */
/*   } */

/*   //decrement the number of sections */
/*   h->numSect--; */
/* } */


//HSect HELPERS ----------------------------------------------------------------
//print out the horizon
void Horizon_print(Horizon* h) {
  int i;
  for(i = 0; i < Horizon_getNumSect(*h); i++) {
    HSect_print(*Horizon_getSect(h, i));
  }
  printf("\n");
  fflush(stdout);
}


//print out the passed HSect
void HSect_print(HSect hs) {
  printf("%f, %f\t", HSect_getStartAngle(hs), HSect_getSlope(hs));
  fflush(stdout);
}
