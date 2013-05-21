#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "radial.h"
#include "event.h"
#include "status_structure.h"
#include "grid.h"


#define RADIAL_DEBUG  if(0)
#define VISIBLE_DEBUG  if(0)

/*compute the visibility of the viewpoint based on the events in the
  eventList and data.  Return the number of visible cells.*/
int sweep_radial(Event* eventList, long nevents, float* data, Viewpoint vp, 
		 Grid* grid) {
  
  assert(eventList && data);

  StatusList *status_struct = create_status_struct();
  assert(status_struct); 

  /* initialize the status struct with the non-null values in data */
  StatusNode sn;
  long i;
  for (i = vp.col +1; i < grid->hd->ncols; i++) {
    if(!is_nodata(grid, data[i])) {
      /*now fill the status node */
      sn.col = i;
      sn.row = vp.row;
      sn.elev = data[i];
      /*calculate distance to vp and Gradient, store them in sn */
      calculate_dist_n_gradient(&sn, &vp);
      /* insert sn into the status structure */
      insert_into_status_struct(sn, status_struct);
    }
  }
  
  
  /* sweep the eventlist */
  int nvis = 0; /*the number of visible cells.  Will be returned later */
  double max; 
  Event* e;
  for (i = 0; i < nevents; i++) {

    /* get out one event at a time and process it according to its type */
    e = &(eventList[i]);
    
    /* skip the viewpoint */
    if(e->col == vp.col && e->row == vp.row) {
      continue;
    }

    sn.col = e->col;
    sn.row = e->row;
    sn.elev = e->elev;
    //sn.dist_to_vp = e->dist; 
    /*calculate the (vertical) gradient wrt vp*/
    // calculate_gradient(&sn, &vp);
    calculate_dist_n_gradient(&sn, &vp);
    RADIAL_DEBUG {
    printf("event %ld:",i);  print_event (*e); 
    printf("sn.dist=%lf, sn.gradient=%lf\n", sn.dist_to_vp, sn.gradient); 
    }
    switch(e->eventType) {

    case ENTERING_EVENT:
      /*insert the node into the status structure */
      insert_into_status_struct(sn, status_struct);
      break;
      
    case EXITING_EVENT:
      /* delete the node into the status structure; should assert that
	 we are deleting the right cell */
      delete_from_status_struct(status_struct, sn.dist_to_vp, sn);
      break;
      
    case CENTER_EVENT:
      /*calculate the visibility */
      max = find_max_gradient_in_status_struct(status_struct, sn.dist_to_vp);
      
      if(max <= sn.gradient) {
	assert(max <= sn.gradient);
	/* the point is visible, so we increment nvis*/
	VISIBLE_DEBUG {
	  printf("\t point (%d, %d) visible  (gradient=%lf, max=%lf\n", 
		 sn.row, sn.col, sn.gradient, max); 
	}
	nvis++;
      } else {
	
	VISIBLE_DEBUG {
	  printf("\t point (%d, %d) INvisible  (d=%lf, gradient=%lf, max=%lf\n", 
		 sn.row, sn.col, sn.dist_to_vp, sn.gradient, max); 
	}
      }
      /*otherwise, we do nothing */
      break;
    }
  }
  return nvis;
}



