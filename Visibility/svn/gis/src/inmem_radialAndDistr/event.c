
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>


#include "event.h"
#include "grid.h" 


/* comparison function.  Compares two events based on their angle. */
int compare_events_angle(const void* a, const void* b) {
  assert(a && b);
  Event* e1 = (Event*) a;
  Event* e2 = (Event*) b;
  if(e1->angle < e2->angle) {
    return -1;
  }
  if(e1->angle > e2->angle) {
     return 1;
  }
  
  /* they are equal */  
  /* then order by distance */
  if (e1->dist < e2->dist) return -1; 
  if (e1->dist > e2->dist) return 1; 

  /* equal angle and equal distance from vp. Either they are events of
     the viewpoint cell, or they represent the same point shared by 2
     cells, one is EXIT evelt, one is ENTER.  */
  
  assert(
	 (e1->dist ==0 && e2->dist==0) ||
	 (e1->eventType == ENTERING_EVENT && e2->eventType==EXITING_EVENT) ||
	 (e1->eventType == EXITING_EVENT && e2->eventType==ENTERING_EVENT)
	 );

  /*  in this case the order should be: EXIT, ENTER, QUERY. Basically
     we want EXITS to be processed before ENTERS. Because EXIT deletes
     an event with that distance from the tree, assuming that there is
     no other node with the same distance (it could check, but it
     would need to store i and j for teh cell). If the ENTER event
     with the same distance is in the tree, then it may delete that
     one.  This may cause subtle bugs and differences in
     visibility. */
  if (e1->eventType == EXITING_EVENT && e2->eventType == ENTERING_EVENT)
    //then e1 comes first.
    return -1; 

  if (e2->eventType == EXITING_EVENT && e1->eventType == ENTERING_EVENT)
    //then e1 comes second
    return 1; 

  //this should only happen when both events correspond to the
  //viewpoint cell; this cell will be ignored, so order of its events
  //does not matter.
  return 0;
}

/*comparison function.  Compares two events based on their distance for the viewpoint */
int compare_events_dist(const void* a, const void* b) {
  assert(a && b);
  Event* e1 = (Event*) a;
  Event* e2 = (Event*) b;

  if(e1->dist < e2->dist) {
    return -1;
  }
  if(e1->dist > e2->dist) {
    return 1;
  }

  /* they are equal */
  return 0;

}


/* ------------------------------------------------------------ 
   compute the gradient of the CENTER of this event wrt viewpoint. For
   efficiency it does not compute the gradient, but the square of the
   arctan of the gradient. Assuming all gradients are computed the same
   way, this is correct. */
double calculate_center_gradient(Event * e, Viewpoint * vp)
{

    assert(e && vp);
    double gradient, sqdist;

    /*square of the distance from the center of this event to vp */
    sqdist = (e->row - vp->row) * (e->row - vp->row) +
	(e->col - vp->col) * (e->col - vp->col);

    gradient = (e->elev - vp->elev) * (e->elev - vp->elev) / sqdist;
    /*maintain sign */
    if (e->elev < vp->elev)
	gradient = -gradient;
    return gradient;
}





/* ------------------------------------------------------------ 
   //calculate the angle at which the event is. Return value is the angle.

   angle quadrants:
   2 1
   3 4 
   ----->x
   |
   |
   |
   V y

 */

/*/////////////////////////////////////////////////////////////////////
   //return the angle from this event wrt viewpoint; the type of the
   //event is taken into position to compute a different amngle for each
   //event associated with a cell */
double calculate_event_angle(Event * e, Viewpoint * vp)
{

    assert(e && vp);
    double ex, ey;

    calculate_event_position(*e, vp->row, vp->col, &ey, &ex);
    return calculate_angle(ex, ey, vp->col, vp->row);
}


/*/////////////////////////////////////////////////////////////////////
   //calculate the exit angle corresponding to this cell */
double
calculate_exit_angle(int row, int col, Viewpoint * vp)
{
    Event e;
    double x, y;

    e.eventType = EXITING_EVENT;
    e.angle = e.elev = 0;
    e.row = row;
    e.col = col;
    calculate_event_position(e, vp->row, vp->col, &y, &x);
    return calculate_angle(x, y, vp->col, vp->row);
}


/*/////////////////////////////////////////////////////////////////////
   //calculate the enter angle corresponding to this cell */
double
calculate_enter_angle(int row, int col, Viewpoint * vp)
{
    Event e;
    double x, y;

    e.eventType = ENTERING_EVENT;
    e.angle = e.elev = 0;
    e.row = row;
    e.col = col;
    calculate_event_position(e, vp->row, vp->col, &y, &x);
    return calculate_angle(x, y, vp->col, vp->row);
}



/* ------------------------------------------------------------ */
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
void
calculate_event_position(Event e, int viewpointRow, int viewpointCol, double *y, double *x)
{

    assert(x && y);
    *x = 0;
    *y = 0;

    if (e.eventType == CENTER_EVENT) {
	/*FOR CENTER_EVENTS */
	*y = e.row;
	*x = e.col;
	return;
    }

    if (e.row < viewpointRow && e.col < viewpointCol) {
	/*first quadrant */
	if (e.eventType == ENTERING_EVENT) {
	    /*if it is ENTERING_EVENT */
	    *y = e.row - 0.5;
	    *x = e.col + 0.5;
	}
	else {
	    /*otherwise it is EXITING_EVENT */
	    *y = e.row + 0.5;
	    *x = e.col - 0.5;
	}

    }
    else if (e.col == viewpointCol && e.row < viewpointRow) {
	/*between the first and second quadrant */
	if (e.eventType == ENTERING_EVENT) {
	    /*if it is ENTERING_EVENT */
	    *y = e.row + 0.5;
	    *x = e.col + 0.5;
	}
	else {
	    /*otherwise it is EXITING_EVENT */
	    *y = e.row + 0.5;
	    *x = e.col - 0.5;
	}

    }
    else if (e.col > viewpointCol && e.row < viewpointRow) {
	/*second quadrant */
	if (e.eventType == ENTERING_EVENT) {
	    /*if it is ENTERING_EVENT */
	    *y = e.row + 0.5;
	    *x = e.col + 0.5;
	}
	else {			/*otherwise it is EXITING_EVENT */
	    *y = e.row - 0.5;
	    *x = e.col - 0.5;
	}

    }
    else if (e.row == viewpointRow && e.col > viewpointCol) {
	/*between the second and the fourth quadrant */
	if (e.eventType == ENTERING_EVENT) {
	    /*if it is ENTERING_EVENT */
	    *y = e.row + 0.5;
	    *x = e.col - 0.5;
	}
	else {
	    /*otherwise it is EXITING_EVENT */
	    *y = e.row - 0.5;
	    *x = e.col - 0.5;
	}

    }
    else if (e.col > viewpointCol && e.row > viewpointRow) {
	/*fourth quadrant */
	if (e.eventType == ENTERING_EVENT) {
	    /*if it is ENTERING_EVENT */
	    *y = e.row + 0.5;
	    *x = e.col - 0.5;
	}
	else {
	    /*otherwise it is EXITING_EVENT */
	    *y = e.row - 0.5;
	    *x = e.col + 0.5;
	}

    }
    else if (e.col == viewpointCol && e.row > viewpointRow) {
	/*between the third and fourth quadrant */
	if (e.eventType == ENTERING_EVENT) {
	    /*if it is ENTERING_EVENT */
	    *y = e.row - 0.5;
	    *x = e.col - 0.5;
	}
	else {
	    /*otherwise it is EXITING_EVENT */
	    *y = e.row - 0.5;
	    *x = e.col + 0.5;
	}

    }
    else if (e.col < viewpointCol && e.row > viewpointRow) {
	/*third quadrant */
	if (e.eventType == ENTERING_EVENT) {
	    /*if it is ENTERING_EVENT */
	    *y = e.row - 0.5;
	    *x = e.col - 0.5;
	}
	else {
	    /*otherwise it is EXITING_EVENT */
	    *y = e.row + 0.5;
	    *x = e.col + 0.5;
	}

    }
    else if (e.row == viewpointRow && e.col < viewpointCol) {
	/*between first and third quadrant */
	if (e.eventType == ENTERING_EVENT) {	/*if it is ENTERING_EVENT */
	    *y = e.row - 0.5;
	    *x = e.col + 0.5;
	}
	else {
	    /*otherwise it is EXITING_EVENT */
	    *y = e.row + 0.5;
	    *x = e.col + 0.5;
	}
    }
    else {
	/*must be the viewpoint cell itself */
	assert(e.row == viewpointRow && e.col == viewpointCol);
	*x = e.col;
	*y = e.row;
    }

    assert(fabs(*x - e.col) < 1 && fabs(*y - e.row) < 1);

    return;
}





double
calculate_angle(double eventX, double eventY,
		double viewpointX, double viewpointY)
{

    /*M_PI is defined in math.h to represent 3.14159... */
    if (viewpointY == eventY && eventX > viewpointX) {
	return 0;		/*between 1st and 4th quadrant */
    }
    else if (eventX > viewpointX && eventY < viewpointY) {
	/*first quadrant */
	return atan((viewpointY - eventY) / (eventX - viewpointX));

    }
    else if (viewpointX == eventX && viewpointY > eventY) {
	/*between 1st and 2nd quadrant */
	return (M_PI) / 2;

    }
    else if (eventX < viewpointX && eventY < viewpointY) {
	/*second quadrant */
	return ((M_PI) / 2 +
		atan((viewpointX - eventX) / (viewpointY - eventY)));

    }
    else if (viewpointY == eventY && eventX < viewpointX) {
	/*between 1st and 3rd quadrant */
	return M_PI;

    }
    else if (eventY > viewpointY && eventX < viewpointX) {
	/*3rd quadrant */
	return (M_PI + atan((eventY - viewpointY) / (viewpointX - eventX)));

    }
    else if (viewpointX == eventX && viewpointY < eventY) {
	/*between 3rd and 4th quadrant */
	return (M_PI * 3.0 / 2.0);
    }
    else if (eventX > viewpointX && eventY > viewpointY) {
	/*4th quadrant */
	return (M_PI * 3.0 / 2.0 +
		atan((eventX - viewpointX) / (eventY - viewpointY)));
    }
    assert(eventX == viewpointX && eventY == viewpointY);
    return 0;
}

/* calculate the distance form the event to the viewpoint */
double calculate_dist(double eventX, double eventY, double viewpointX, double viewpointY) {

  return (eventX - viewpointX) * (eventX - viewpointX) + (eventY - viewpointY) * (eventY - viewpointY);

}





/* This function is called once, before knowing the actual
  viewpoint. Fills the eventList with all the necessary events. Each
  event is filled with row, column, elevation and type.  Angle will be
  filled later for each viewpoint. The eventlist is allocated (outside
  this function) to hold the maximum number of events possible.
  Returns the number of events.*/
long init_event_list (Event* eventList, Grid* g) {
  
  assert(eventList && g);
  
  printf("Initializing events.\n"); 
  int nrows, ncols, row, col;
  nrows = g->hd->nrows;
  ncols = g->hd->ncols;

  int nevents = 0;
  Event e;
  e.angle = e.dist = e.elev = -1; 

  for(row = 0; row < nrows; row++) {
    for(col = 0; col < ncols; col++) {
    
      /* if point is nodata, continue */
      if (is_nodata_at(g, (dimensionType)row, (dimensionType)col)) continue; 
      
      e.row = row;
      e.col = col;
      e.elev = get(g, row, col); 

      /*add 3 events to the event list, one of each type */
      e.eventType = ENTERING_EVENT;
      eventList[nevents] = e;
      nevents++;

      e.eventType = CENTER_EVENT;
      eventList[nevents] = e;
      nevents++;

      e.eventType = EXITING_EVENT;
      eventList[nevents] = e;
      nevents++;
    }
  }
  
  printf("Done initializing events.\n");
  return nevents;  
}




/*sets each event in the event list with the correct angle from the
  viewpoint.  It also fills data with the values for the row of the
  viewpoint, to be used to fill the status structure later */
void set_event_list_angles_and_dist (int nevents, Event* eventList, Viewpoint* vp){

  assert(eventList && vp);

  /*temporary storage of the event's position */
  double ax, ay;

  /*go through each event */
  int i;
  for(i = 0; i < nevents; i++) {

    /* an event cannot be nodata */
    //assert(!is_nodata(eventList[i].elev)); is_nodata needs the grid,
    //which is not passed to this function. if you really want this
    //assert here yuo need to change the function to taje the grid as
    //parameter */

    /* skip the viewpoint */
    if(eventList[i].row == vp->row && eventList[i].col == vp->col) {
      eventList[i].dist = 0; 
      eventList[i].angle = 0; 
      continue;
    }
    
    /*calculate the position, and then the angle*/
    calculate_event_position(eventList[i], vp->row, vp->col, &ay, &ax);
    eventList[i].angle = calculate_angle(ax, ay, vp->col, vp->row);
    eventList[i].dist = calculate_dist(ax, ay, vp->col, vp->row);

    // print_event(eventList[i]); printf("ax=%f, ay=%f",ax, ay); printf("\n");
  }

  return;
  
}


void print_event( Event e) {
  printf("e=[row=%d, col =%d, elev=%lf, angle=%f, dist=%f, ", 
	 e.row, e.col, e.elev, e.angle, e.dist); 
  if (e.eventType == -1) printf("type=EXIT] "); 
  if (e.eventType == 1) printf("type=ENTER] "); 
  if (e.eventType == 0) printf("type=Q] "); 

}
