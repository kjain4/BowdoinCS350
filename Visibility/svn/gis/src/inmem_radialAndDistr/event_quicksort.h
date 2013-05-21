#ifndef __quicksort_h_
#define __quicksort_h_


/* sort the event list in radial order */
void event_quicksort_radial(Event* eventlist, size_t nevents); 


/* sort the event list in distance order */
void event_quicksort_distance(Event* eventlist, size_t nevents); 


#define compare_values(a, b) ((a < b) ? (-1) : (a > b))


#endif
