#include <assert.h>
#include <stdlib.h>


#include "event.h"
#include "event_quicksort.h"


static int MIN_LEN = 20; 



/* ************************************************************ */
/*      SORT BY RADIAL ANGLE
 */
/* ************************************************************ */


/* comparison function.  Compares two events based on their angle. */
int event_compare_radial(Event e1, Event e2) {


  if(e1.angle < e2.angle) {
    return -1;
  }
  if(e1.angle > e2.angle) {
     return 1;
  }
  
  /* they are equal */  
  /* then order by distance */
  if (e1.dist < e2.dist) return -1; 
  if (e1.dist > e2.dist) return 1; 

  /* equal angle and equal distance from vp. Either they are events of
     the viewpoint cell, or they represent the same point shared by 2
     cells, one is EXIT evelt, one is ENTER.  */
  
  assert(
	 (e1.dist ==0 && e2.dist==0) ||
	 (e1.eventType == ENTERING_EVENT && e2.eventType==EXITING_EVENT) ||
	 (e1.eventType == EXITING_EVENT && e2.eventType==ENTERING_EVENT)
	 );

  /*  in this case the order should be: EXIT, ENTER, QUERY. Basically
     we want EXITS to be processed before ENTERS. Because EXIT deletes
     an event with that distance from the tree, assuming that there is
     no other node with the same distance (it could check, but it
     would need to store i and j for teh cell). If the ENTER event
     with the same distance is in the tree, then it may delete that
     one.  This may cause subtle bugs and differences in
     visibility. */
  if (e1.eventType == EXITING_EVENT && e2.eventType == ENTERING_EVENT)
    //then e1 comes first.
    return -1; 

  if (e2.eventType == EXITING_EVENT && e1.eventType == ENTERING_EVENT)
    //then e1 comes second
    return 1; 

  //this should only happen when both events correspond to the
  //viewpoint cell; this cell will be ignored, so order of its events
  //does not matter.
  return 0;
}
#define event_compare_radial(e1, e2) (\
((e1).angle < (e2).angle) ? (-1) : ( \
  ((e1).angle > (e2).angle) ? (1) : ( \
  ((e1).dist < (e2).dist) ? (-1) : ( \
  ((e1).dist > (e2).dist) ? (1) : ( \
  (((e1).eventType ^ (e2).eventType) == -2) ? ((e1).eventType) : (0) )))))




size_t partition_radial(Event *data, size_t n) {

  Event *ptpart, tpart;
  Event *p, *q;
  Event t0;
  size_t pivot;
    
    // Try to get a good partition value and avoid being bitten by
    // already sorted input.  //ptpart = data + (random() % n);
  ptpart = data + (random() % n);
  
  //swap with data[0]
  tpart = *ptpart;
  *ptpart = data[0];
  data[0] = tpart;
    
  // Walk through the array and partition it.
  for (p = data - 1, q = data + n; ; ) {
    
    do {
      q--;
    } while (event_compare_radial(*q, tpart) > 0);

    do {
      p++;
    } while (event_compare_radial(*p, tpart) < 0);
    
    if (p < q) {
      t0 = *p;
      *p = *q;
      *q = t0;
    } else {
      pivot = q - data;            
      break;
    }
  }
  return pivot;
}

void insertionsort_radial(Event *data, size_t n) {
  Event *p, *q, test;
  
  for (p = data + 1; p < data + n; p++) {
    for (q = p - 1, test = *p; event_compare_radial(*q, test) > 0; q--) {
      *(q+1) = *q;
      if (q==data) {
	q--; // to make assignment below correct
	break;
      }
    }
    *(q+1) = test;
  }
}

/* sort the event list in radial order */
void event_quicksort_radial(Event* eventlist, size_t n) {

 size_t pivot;
 if (n < MIN_LEN) {
   insertionsort_radial(eventlist, n);
   return;
 }
 //else
 pivot = partition_radial(eventlist, n);
 event_quicksort_radial(eventlist, pivot + 1);
 event_quicksort_radial(eventlist + pivot + 1, n - pivot - 1);
}






/* ************************************************************ */
/*      SORT BY DISTANCE 
 */
/* ************************************************************ */


/*comparison function.  Compares two events based on their distance
  for the viewpoint */
int event_compare_distance(Event e1, Event e2) {
  //assert(e1 && e2);

  if(e1.dist < e2.dist) {
    return -1;
  }
  if(e1.dist > e2.dist) {
    return 1;
  }

  /* they are equal */
  return 0;

}
#define event_compare_distance(e1, e2) compare_values((e1).dist, (e2).dist)

size_t partition_distance(Event *data, size_t n) {

  Event *ptpart, tpart;
  Event *p, *q;
  Event t0;
  size_t pivot;
    
    // Try to get a good partition value and avoid being bitten by
    // already sorted input.  //ptpart = data + (random() % n);
  ptpart = data + (random() % n);
  
  //swap with data[0]
  tpart = *ptpart;
  *ptpart = data[0];
  data[0] = tpart;
    
  // Walk through the array and partition it.
  for (p = data - 1, q = data + n; ; ) {
    
    do {
      q--;
    } while (event_compare_distance(*q, tpart) > 0);

    do {
      p++;
    } while (event_compare_distance(*p, tpart) < 0);
    
    if (p < q) {
      t0 = *p;
      *p = *q;
      *q = t0;
    } else {
      pivot = q - data;            
      break;
    }
  }
  return pivot;
}

void insertionsort_distance(Event *data, size_t n) {
  Event *p, *q, test;
  
  for (p = data + 1; p < data + n; p++) {
    for (q = p - 1, test = *p; event_compare_distance(*q, test) > 0; q--) {
      *(q+1) = *q;
      if (q==data) {
	q--; // to make assignment below correct
	break;
      }
    }
    *(q+1) = test;
  }
}

/* sort the event list in distance order */
void event_quicksort_distance(Event* eventlist, size_t n) {

 size_t pivot;
 if (n < MIN_LEN) {
   insertionsort_distance(eventlist, n);
   return;
 }
 //else
 pivot = partition_distance(eventlist, n);
 event_quicksort_distance(eventlist, pivot + 1);
 event_quicksort_distance(eventlist + pivot + 1, n - pivot - 1);
}


