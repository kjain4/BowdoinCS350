/* Will Richard
 * Horizon.c
 */

#include "Horizon.h"

#define MERGE_DEBUG if(0)
#define FIND_DEBUG if(0)

//CONSTRUCT AND DISTROY --------------------------------------------------------
//Create a new horizon section.
HSect* HSect_new(double startAngle, float slope) {

  //malloc and asset the new HSect
  HSect* new = (HSect*) malloc(sizeof(HSect));
  assert(new);

  //store the passed angle and point
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

//free the passed horizon section.  DO NOT free the Point
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
HSect* Horizon_getSect(Horizon h, int i) {
  return &h.sections[i];
}

//get the angle of the ith section of the horizon
double Horizon_getSectAngle(Horizon h, int i) {
  return HSect_getStartAngle(*Horizon_getSect(h, i));
}

//get the slope if the ith section of the horizon
float Horizon_getSectSlope(Horizon h, int i) {
  return HSect_getSlope(*Horizon_getSect(h, i));

}

//get the size of the Horizon
int Horizon_getSize(Horizon h) {
  return h.size;
}

int Horizon_getNumSect(Horizon h) {
  return h.numSect;
}

//Horizon HELPERS --------------------------------------------------------------

//adds a sections to the horizon - not sure if this will work
void Horizon_addSect(Horizon* h, HSect hs) {
  assert(h);
  //grow the horizon array, if necessary
  if(Horizon_getNumSect(*h) == Horizon_getSize(*h)) { Horizon_grow(h); }

  //add the section to the horizon
  h->sections[Horizon_getNumSect(*h)] = hs;
  h->numSect++;
}

void Horizon_addSectValues(Horizon* h, double startAngle, float slope) {
  assert(h);
  //grow the horizon array, if necessary
  if(Horizon_getNumSect(*h) == Horizon_getSize(*h)) Horizon_grow(h);

  //fill the section of the horizon with the passed values
  HSect_fill(&(h->sections[Horizon_getNumSect(*h)]), startAngle, slope);
  h->numSect++;
}

//grow the horizon array by doubling its size
void Horizon_grow(Horizon* h) {
  assert(h);

  h->sections = (HSect*) realloc(h->sections, Horizon_getSize(*h) * 2);
  assert(h->sections);
}

//return the HSect in the horizon that may occulde the passed Point.
HSect* Horizon_findSectionForPoint(Horizon* h, Point p) {
  assert(h);

  FIND_DEBUG{
    printf("trying to find section for point with angle %f slope %f elev %d dist %f in:\n", Point_getCenterAngle(p), Point_getSlope(p), Point_getElev(p), Point_getDist(p));
    Horizon_print(*h);
    printf("\n");
    fflush(stdout);
  }

  //use a binary search to find the correct section
  int low = 0;
  int high = Horizon_getNumSect(*h)-1;
  int mid;
  while(low <= high) {
  //get the midpoint between high and low
    mid = low + ((high-low)/2);

    // If the section at mid is the last in the horizon, then it is the correct section, and should be returned.
    if(mid == Horizon_getNumSect(*h) -1) return Horizon_getSect(*h, mid);

    //if the angle for the section at mid is greater than the point's angle, then there is no way that this could be the point - continue with the binary search using the part of the horizon before mid
    if(Horizon_getSectAngle(*h, mid) > Point_getCenterAngle(p)) {
      high = mid - 1;
  }
    /*if the section at mid has a lower or equal angle than the point, one of 2 things may be the case.
      1) The section at mid+1 has a greater starting angle than the point, in which case the section at index mid is the correct one, so we should return it.  
      2) The point at mid+1 has starting angle less than the point, in which case we should continue the binary search, with a lower bound of mid
    */
    else { // meaning the section at mid has angle <= the center angle of p
      //case 1
      if(Horizon_getSectAngle(*h, mid+1) > Point_getCenterAngle(p)) {
	return Horizon_getSect(*h, mid);
      }
      //case 2
      else {
	low = mid + 1;
      }
    } 
  }
  //if things go bad, exit - it should never happen
  printf("search for a point has gone bad\n");
  exit(2);
}

//merge the 2 passed horizons, and return the resulting horizon
Horizon* Horizon_merge(Horizon* h1, Horizon* h2) {
  assert(h1);
  assert(h2);

  MERGE_DEBUG{printf("starting merge\n"); fflush(stdout);}

  MERGE_DEBUG{
    printf("h1:\n");
    Horizon_print(*h1);
    printf("h2:\n");
    Horizon_print(*h2);
    printf("\n");
  }

  Horizon* hNew = Horizon_new(Horizon_getNumSect(*h1) + Horizon_getNumSect(*h2));
  assert(hNew);

  HSect curMax; //stores the section relating to the current max value
  int curMaxH; //stores which horizon that curMax came from
  int h1index = 0, h2index = 0;
  
  //set initial currMax
  if(HSect_getSlope(*Horizon_getSect(*h1, 0)) > HSect_getSlope(*Horizon_getSect(*h2, 0))) {
    //store the first element for horizon 1's info into curMax
    HSect_fill(&curMax, Horizon_getSectAngle(*h1, 0), 
	       HSect_getSlope(*Horizon_getSect(*h1, 0)));
    //set curMaxH
    curMaxH = 1;
    //increment h1index, to reflect what should be looked at next
    h1index++;
    //put the information from the first sect of the first horizon into the new horizon
    Horizon_addSect(hNew, curMax);
  }
  else {
    //store the first element for horizon 2's info into curMax
    HSect_fill(&curMax, Horizon_getSectAngle(*h2, 0), 
	       HSect_getSlope(*Horizon_getSect(*h2, 0)));
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
  while(h1index < Horizon_getNumSect(*h1) && h2index < Horizon_getNumSect(*h2)) {
    //investigate the next lowest angle
    if(Horizon_getSectAngle(*h1, h1index) < Horizon_getSectAngle(*h2, h2index)){
      //the next sect is from h1
      next = *Horizon_getSect(*h1, h1index);
      nextH = 1;
      //the increment the h1index
      h1index++;
    }
    else {
      //the next sect is from h2
      next = *Horizon_getSect(*h2, h2index);
      nextH = 2;
      h2index++;
    }

    MERGE_DEBUG{printf("next sect from h%d: angle = %f, slope = %f\n", nextH, HSect_getStartAngle(next), HSect_getSlope(next)); fflush(stdout);}

    //if the next sect and the curMax are from the same horizon, need to check both horizons to find the new curMax
    if(nextH == curMaxH) {
      //if next is from horizon 1, then next overlaps with HSect[h2index-1]
      //so, in that case, compare those 2 values, and add the higher one
      if(nextH == 1) {
	if(HSect_getSlope(next) >= Horizon_getSectSlope(*h2, h2index-1)) {
	  HSect_fill(&curMax, HSect_getStartAngle(next), HSect_getSlope(next));
	  curMaxH = 1;
	  MERGE_DEBUG{printf("adding sect with angle = %f and slope = %f\n", HSect_getStartAngle(curMax), HSect_getSlope(curMax)); fflush(stdout);}
	  Horizon_addSect(hNew, curMax); 
	}
	else { //nextH == 1, but the slope at this angle from h2 is greater than the slope at this angle from h1
	  HSect_fill(&curMax, HSect_getStartAngle(next), Horizon_getSectSlope(*h2, h2index-1));
	  curMaxH = 2;
	  MERGE_DEBUG{printf("adding sect with angle = %f and slope = %f\n", HSect_getStartAngle(curMax), HSect_getSlope(curMax)); fflush(stdout);}
	  Horizon_addSect(hNew, curMax); 
	}
      }
      //else, do the same, but switch h1index and h2index
      else { // meaning nextH == 2
	//so, we care about comparing the sect from h2 at h2index and the sect from h1index at h1index -1
	if(HSect_getSlope(next) >= Horizon_getSectSlope(*h1, h1index-1)) {
	  HSect_fill(&curMax, HSect_getStartAngle(next), HSect_getSlope(next));
	  curMaxH = 2;
	  MERGE_DEBUG{printf("adding sect with angle = %f and slope = %f\n", HSect_getStartAngle(curMax), HSect_getSlope(curMax)); fflush(stdout);}
	  Horizon_addSect(hNew, curMax); 
	}
	else {//nextH == 2, but the slope at this angle from h1 is greater than the slope at this angle from h2
	  HSect_fill(&curMax, HSect_getStartAngle(next), Horizon_getSectSlope(*h1, h1index -1));
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

  //now, add whatever points are left
  if(h1index < Horizon_getNumSect(*h1)) {
    for(; h1index < Horizon_getNumSect(*h1); h1index++) {
      MERGE_DEBUG{printf("adding sect with angle = %f and slope = %f\n", Horizon_getSectAngle(*h1, h1index), Horizon_getSectSlope(*h1, h1index)); fflush(stdout);}
      Horizon_addSectValues(hNew, Horizon_getSectAngle(*h1, h1index), Horizon_getSectSlope(*h1, h1index));
    }
  }
  if(h2index < Horizon_getNumSect(*h2)) {
    for(; h2index < Horizon_getNumSect(*h2); h2index++) {
      	MERGE_DEBUG{printf("adding sect with angle = %f and slope = %f\n", Horizon_getSectAngle(*h2, h2index), Horizon_getSectSlope(*h2, h2index)); fflush(stdout);}
      Horizon_addSectValues(hNew, Horizon_getSectAngle(*h2, h2index), Horizon_getSectSlope(*h2, h2index));
    }
  }
  

  MERGE_DEBUG{printf("ending merge\n********************\n"); fflush(stdout);}

  //hNew should be filled completely - return
  return hNew;
}


//HSect HELPERS ----------------------------------------------------------------
//print out the horizon
void Horizon_print(Horizon h) {
  int i;
  for(i = 0; i < Horizon_getNumSect(h); i++) {
    printf("%f, %f\t", HSect_getStartAngle(*Horizon_getSect(h, i)), HSect_getSlope(*Horizon_getSect(h, i)));
  }
  printf("\n");
  fflush(stdout);
}
