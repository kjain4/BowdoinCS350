#ifndef __INMEMDISTRIBUTE_H
#define __INMEMDISTRIBUTE_H

#include "event.h"
#define EPSILON .00000001          /* used for comparing doubles */



/* distribute the eventList into sectors, dropping events that are
   hidden by long events and computes visibility in each sector
   recursively. Returns the number of visible cells. Returns the
   number of visible cells.  Assumes the eventList has already been
   sorted by distance. */
int distribute_and_sweep(Event* eventList, int nevents,
			 int NUM_SECTORS, int  BASECASE_THRESHOLD,  
			 Viewpoint* vp, int* dropped);


/* recursively distribute each sector, solving it in memory if it is
   small enough, otherwise split the sector and distribute the events
   into it */
int distribute_sector(Event* eventList, int nevents, 
		      int MAX_SECTOR_FACTOR,
		      int NUM_SECTORS, int  BASECASE_THRESHOLD,  
		      Event* enterBndEvents, int enterBnd_length, 
		      Viewpoint* vp, double start_angle, double end_angle, 
		      int deleteEventList, int* dropped);


/* base case of distribution.  */
int distribute_basecase(Event* eventList, int nevents, 
			Event* enterBndEvents,  int enterBnd_length, 
			double start_angle,  double end_angle,
			Viewpoint* vp, int deleteEventList);


/* bndEvents is an array of events that cross into the sector's
   (first) boundary; they must be distributed to the boundary streams
   of the sub-sectors of this sector. Note: the boundary streams of
   the sub-sectors may not be empty; as a result, events get appended
   at the end, and they will not be sorted by distance from the vp. */
void distribute_bnd_events(Event* bndEvents, int bndEvents_length, 
			   int NUM_SECTORS, int MAX_SECTOR_SIZE,
			   Event** sectorBnd, int* secotrBnd_length, 
			   Viewpoint * vp, double start_angle,double end_angle,
			   double *high, int* dropped_events);


/* computes the sector that contains this angle.  If the angle falls
   is not between start_angle and end_angle, return -1 */
int get_event_sector(double angle, double start_angle, double end_angle, 
		     int NUM_SECTORS);


/* determines if an event with angle is almost on a boundary, to deal
   with prescion issues */
int is_almost_on_boundary(double angle, int sec, double start_angle, 
			  double end_angle, int NUM_SECTORS);


/* helper for is_almost_on_boundary.  Returns 1 if the angle is within
   epsion from boundary angle */
int is_almost_on_boundry_helper(double angle, double boundary_angle);

/* insert event e into the sector if it is not occuded by high_s */
void insert_event_in_sector(Event e, int s, Event* sector, int* sector_length, 
			    int MAX_SECTOR, double high_s, Viewpoint* vp, int* dropped);


/* returns 1 if the center of event is occluded by the gradient, which
   is assumed to be in line with the event  */
int is_center_gradient_occluded(Event e, double gradient, Viewpoint * vp);


/* the event e spans sectors from start_s to end_s; Action: update
   high[] for each spanned sector.
 */
void process_long_cell(int start_sec, int end_sec, Viewpoint * vp, Event e,
		       double *high, int NUM_SECTORS);


/* print the passed stats */
void print_stats(Viewpoint* vp, int insertCount, int dropCount, 
		 int bndInsertCount, int bndDropCount, int totalCount,
		 int NUM_SECTORS, int BASECASE_THRESHOLD);


#endif
