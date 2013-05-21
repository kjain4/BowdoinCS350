#ifndef __RADIAL_H
#define __RADIAL_H


#include "event.h"
#include "status_structure.h"
#include "grid.h"


/*compute the visibility of the viewpoint based on the events in the
  eventList and data.  Return the number of visible cells.*/
int sweep_radial(Event* eventList, long nevents, float* data, Viewpoint vp, 
		 Grid* grid); 
  
#endif
