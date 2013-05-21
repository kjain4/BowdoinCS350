#ifndef __EVENTLIST_H
#define __EVENTLIST_H

#include "grid.h"

#define EXITING_EVENT -1
#define ENTERING_EVENT 1
#define CENTER_EVENT 0



typedef struct viewpoint_ {
  int row, col; 
  float elev; 
} Viewpoint; 


typedef struct event_ {
  int row, col;         /* location of the center of the cell */
  float elev;           /* elevation of the event*/
  double angle;         /* the angle from the viewpoint to the event */
  double dist;           /* the distance from the viewpoint to the event */
  char eventType;       /* the type of event - ENTERING_EVENT,
			   EXITING_EVENT, CENTER_EVENT */

} Event;

/* Compares two events based on their angle wrt viewpoint. */
int compare_events_angle(const void* a, const void* b);


/* Compares two events based on their distance for the viewpoint */
int compare_events_dist(const void* a, const void* b);


/* compute the gradient of the CENTER of this event wrt viewpoint. For
   efficiency it does not compute the gradient, but the square of the
   tan of the gradient. Assuming all gradients are computed the same
   way, this is correct. */
double calculate_center_gradient(Event * e, Viewpoint * vp);


/*calculate the exit angle corresponding to this cell */
double calculate_exit_angle(int row, int col, Viewpoint * vp);


/*calculate the enter angle corresponding to this cell */
double calculate_enter_angle(int row, int col, Viewpoint * vp);


/* calculate the exact position of the given event, and store them in x
   and y.
   quadrants:  1 2
   3 4
   ----->x
   |
   |
   |
   V y
 */
void calculate_event_position(Event e, int viewpointRow, int viewpointCol, 
			      double *y, double *x);


/* calculate the angle of the event with the passed x and y*/
double calculate_angle(double eventX, double eventY, double viewpointX, 
		       double viewpointY);

/* calculate the distance form the event to the viewpoint */
double calculate_dist(double eventX, double eventY, double viewpointX, 
		     double viewpointY);


/* returns an eventList that has all the events in it, and all events
   are filled with row, column, elevation and type.  Angle will be
   filled later for each viewpoint. */
long  init_event_list (Event* eventList, Grid* g);



/*sets each event in the event list with the correct angle from the viewpoint */
void set_event_list_angles_and_dist (int nevents, Event* eventList, 
				     Viewpoint* vp);

void print_event( Event e);
#endif
