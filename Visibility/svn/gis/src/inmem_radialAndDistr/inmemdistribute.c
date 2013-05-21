#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>


#include "inmemdistribute.h"
#include "radial.h"
#include "event.h"
#include "status_structure.h"

#define PRINT_DISTRIBUTE if(0)
#define BND_DEBUG if(0)
#define ALLOC_DEBUG if(0)

#define FALSE 0
#define TRUE 1



/* distribute the eventList dropping events that are hidden by long
   events and computes visibility in each sector recursively. Returns
   the number of visible cells.  Assumes the eventList has already
   been sorted by distance. returns the number of cells visible from
   the viewpoint, and sets the number of dropped cells */
int distribute_and_sweep(Event* eventList, int nevents, 
			 int NUM_SECTORS,  int  BASECASE_THRESHOLD,  
			 Viewpoint* vp, int* dropped) {
  
  assert(eventList && vp && dropped);

  /* 1 means each sector is allocated large enough to hold its
	 theoretical worst-case size; 2 means it is half the theoretical
	 worst-case size, etc */ 
  //int  MAX_SECTOR_FACTOR= 1;
  int  MAX_SECTOR_FACTOR= NUM_SECTORS/100 + 1;
  //  printf("MAX_SECTOR=%d \n", MAX_SECTOR_FACTOR);
  
  ALLOC_DEBUG {
    double size =  (double)(nevents * sizeof(Event)/MAX_SECTOR_FACTOR); 
    printf("allocated %dx2 sectors size %d, total %.1f MB\n", 
	   NUM_SECTORS, (int) size,  2*NUM_SECTORS*size/(1024.0 * 1024.0)); 
    fflush(stdout); 
  }
  return  distribute_sector(eventList, nevents, 
			    MAX_SECTOR_FACTOR, NUM_SECTORS,BASECASE_THRESHOLD, 
			    NULL, 0, vp, 0, 2 * M_PI, FALSE, dropped);
}





/* recursively distribute each sector, solving it in memory if it is
   small enough, otherwise split the sector and distribute the events
   into it */
int distribute_sector(Event* eventList, int nevents, 
		      int MAX_SECTOR_FACTOR,
		      int NUM_SECTORS, int  BASECASE_THRESHOLD,  
		      Event* enterBndEvents,  int enterBnd_length, 
		      Viewpoint* vp, double start_angle,  double end_angle, 
		      int deleteEventList, int* dropped_events) {
  
  
  assert(eventList &&  vp && dropped_events);
  
  PRINT_DISTRIBUTE {
    printf("***DISTRIBUTE sector [%.4f, %.4f]***   ", start_angle, end_angle); 
    printf("nevents=%d,bnd-events=%d BASECASE_THRESHOLD=%d, NUM_SECTORS=%d\n", 
	   nevents, enterBnd_length,BASECASE_THRESHOLD,NUM_SECTORS); 
    fflush(stdout); 
  }

  /* this is the largest sector size that we'll provide for; in theory
     teh max is nevents; hopefully in practice it will be closer to
     nevents/NUM_SECTORS */
  int MAX_SECTOR =  (nevents+enterBnd_length)/MAX_SECTOR_FACTOR; 
  
  int nvis;
  
  //*******************************************************
  //BASE CASE
  //*******************************************************
  if(nevents < BASECASE_THRESHOLD) {
    /* dropped does not change */
    nvis =  distribute_basecase(eventList, nevents, enterBndEvents, 
				enterBnd_length, start_angle, end_angle, 
				vp, deleteEventList);
    return nvis;
  }



  //*******************************************************/
  /* otherwise, recurse  */
  //*******************************************************/

  /* sector[i] will hold all the events in the i-th sector */
  Event** sector;
  sector = (Event**) malloc (NUM_SECTORS * sizeof(Event*));
  assert(sector);
  int i;
  for(i = 0; i < NUM_SECTORS; i++){
    /*  allocate enough space for each sector */
    //sector[i] = (Event*)malloc((nevents+enter Bnd_length)*sizeof(Event));
    /* LT: it turns out that malloc fails for a large number of
       sectors; naturally, for 200+ sectors and 500k events total, it
       is a waste to allocate each sector of max size */ 
    
    sector[i] = (Event*)malloc(MAX_SECTOR * sizeof(Event));
    assert(sector[i]);
  }
  
  /* keep an array of ints that holds the number of events in a given
     sector.  sector_length[i] is the number of events in sector i */
  int* sector_length;
  sector_length = (int*) malloc(NUM_SECTORS * sizeof(int));
  assert(sector_length); 
  /*initialzie to 0*/
  for(i = 0; i < NUM_SECTORS; i++) {
    sector_length[i] = 0;
  }

  /*the array of gradient values, one for each sector; the gradient is
    the gradient of the center of a cell that spans the sector
    completely.  Will be used to occlude invisible events from
    sub-sectors */
  double* high = (double*) malloc (NUM_SECTORS * sizeof(double));
  assert(high); 
  /*initialize <high> with the smallest gradient value from rbbst */
  for(i = 0; i < NUM_SECTORS; i++) {
    high [i] = SMALLEST_GRADIENT;
  }

  /*make an array of events, one for each boundary, to hold events for
    cells on the boundary of sectors. sectorBnd[i] will keep all the
    cells crossing into sector i and below. */
  Event** sectorBnd = (Event**) malloc (NUM_SECTORS * sizeof(Event*));
  assert(sectorBnd); 
  /* again, make sure we have enough space to hold nevent events */
  for(i=0; i< NUM_SECTORS; i++) {
    //sectorBnd[i] = (Event*) malloc ((nevents + enterBnd_length) * sizeof(Event));
	sectorBnd[i] = (Event*) malloc (MAX_SECTOR * sizeof(Event));
    assert(sectorBnd[i]);
  }

  /* keep track of sectorBnd length as well */
  int* sectorBnd_length;
  sectorBnd_length = (int*) malloc(NUM_SECTORS * sizeof(int));
  assert(sectorBnd_length); 
  /*initialize to 0 */
  for(i = 0; i < NUM_SECTORS; i++) {
    sectorBnd_length[i] = 0;
  }
  
  /* keep stats for each sector */
  /*   long* total = (long*) malloc(NUM_SECTORS * sizeof(long)); */
  /*   long* insert = (long*) malloc(NUM_SECTORS * sizeof(long)); */
  /*   long* drop = (long*) malloc(NUM_SECTORS * sizeof(long)); */
  /*   long* bndInsert = (long*) malloc(NUM_SECTORS * sizeof(long)); */
  /*   long* bndDrop = (long*) malloc(NUM_SECTORS * sizeof(long)); */
  
  /*   /\* set all of the stats to 0 *\/ */
  /*   for (i = 0; i < NUM_SECTORS; i++){ */
  /*     total[i] = 0; */
  /*     insert[i] = 0; */
  /*     drop[i] = 0; */
  /*     bndInsert[i] = 0; */
  /*     bndDrop[i] = 0; */
  /*   } */
  
  
  /* keep a counter of long events in this sector */
  int longEvents = 0;

  /****************************************************************
  CONCENTRIC SWEEP
  *****************************************************************/
  Event e;
  double exit_angle, enter_angle;
  int exit_sec, enter_sec, sec;
  int boundaryEvents = 0;

  for(i = 0; i < nevents; i++){
    e = eventList[i];

    /* skip the viewpoint */
    if(e.row == vp->row && e.col == vp->col) {
      continue;
    }

    sec = get_event_sector(e.angle, start_angle, end_angle, NUM_SECTORS);
    /* detect boundary cases */
    if (is_almost_on_boundary(e.angle, sec, start_angle, end_angle, NUM_SECTORS)) {
      boundaryEvents ++;
    }

    /* make sure sec is not -1 */
    assert(sec >=0 && sec < NUM_SECTORS);

    /* increment the number of events in the sector */
   /*  total[sec]++; */

    /* insert event into the sector, if it is not occulded */
    insert_event_in_sector(e, sec, sector[sec], sector_length, MAX_SECTOR, high[sec], vp, dropped_events);

    /* handle the corresponding events of this event */
    switch(e.eventType) {
    case CENTER_EVENT:
      break;

    case ENTERING_EVENT:
      /* find the corresponding exit event and its sector */
      exit_angle = calculate_exit_angle(e.row, e.col, vp);
      exit_sec = get_event_sector(exit_angle, start_angle, end_angle, NUM_SECTORS);

      /* keep in mind that exit_sec can be -1 (outside this sector) */
      if(exit_sec == sec) {
	/*short event - fits in sector */
      }
      else if (exit_sec == (sec + 1) % NUM_SECTORS || (exit_sec + 1) % NUM_SECTORS == sec) {
	/* semi short event: insert in sector sec and in sector
	   boundary s+1 NOTE: to avoid prescion issues, the events are
	   inserted when processing EXIT_EVENT
	*/
      }
      else {
	/* long event; insert in sector sec , and in sector boundary exit_sec */
	process_long_cell(sec, exit_sec, vp, e, high, NUM_SECTORS);
	longEvents++;
	/* again, all insertions occur when processing EXIT_EVENT */
      }
      break;

    case EXITING_EVENT:
      /*find its corresponding enter event and its sector */
      enter_angle = calculate_enter_angle(e.row, e.col, vp);
      enter_sec = get_event_sector(enter_angle, start_angle, end_angle, NUM_SECTORS);

      /* keep in mind that enter_sec can be -1 (outside) */

      if(enter_sec == sec) {
	/*short event, fit in sector */
      }
      else if (enter_sec == (sec+1) % NUM_SECTORS || (enter_sec +1)%NUM_SECTORS == sec) {
	/*semi-short event 
	  the corresonding ENTER event must be inserted in secterBnd[sec] */
	e.eventType = ENTERING_EVENT;
	BND_DEBUG {printf("BND event "); print_event(e); printf("in bndSector %d\n", sec); fflush(stdout);}
	insert_event_in_sector(e, sec, sectorBnd[sec], sectorBnd_length, MAX_SECTOR,  high[sec], vp, dropped_events);
      }
      else {
	/* long event */
	process_long_cell(enter_sec, sec, vp, e, high, NUM_SECTORS);
	longEvents++;

	/* the corresponding ENTER event must insert itself in sectorBnd[sec] */
	e.eventType = ENTERING_EVENT;
	BND_DEBUG {printf("BND event "); print_event(e); printf("in bndSector %d\n", sec); fflush(stdout);}
	insert_event_in_sector(e, sec, sectorBnd[sec], sectorBnd_length, MAX_SECTOR, high[sec], vp, dropped_events);
      }
      break;

    } /* switch event-type */
  
  } /* for event i */

    /* distribute the border events */
  if(enterBndEvents)
    distribute_bnd_events(enterBndEvents, enterBnd_length,  NUM_SECTORS, MAX_SECTOR, sectorBnd, 
			  sectorBnd_length, vp, start_angle, end_angle, high, 
			  dropped_events);

  /* save some memory before recursion */
  /*if the flag is set, delete the eventList */
  if(deleteEventList) free(eventList);
  /* delete boundary events  */
  if(enterBndEvents) free(enterBndEvents);


  /*recursively solve each sector */
  nvis = 0; 
  for(i=0; i < NUM_SECTORS; i++) {
    nvis += distribute_sector(sector[i], sector_length[i], 
			      MAX_SECTOR_FACTOR, NUM_SECTORS, BASECASE_THRESHOLD, 
			      sectorBnd[i], sectorBnd_length[i], vp, 
			      start_angle+i*((end_angle-start_angle)/NUM_SECTORS), 
			      start_angle+(i+1)*((end_angle-start_angle)/NUM_SECTORS), 
			      TRUE, dropped_events);
  }
  
  
  /* clean up allocated space */
  /* the sectors are passed as event lists recursively into the
     function, and they are deleted above */ 
  free(sector);
  free(sector_length);
    /* the sector boundaries are passed as event lists recursively into
     the function, and they are deleted above */ 
  free(sectorBnd);
  free(sectorBnd_length);
  free(high); 

  PRINT_DISTRIBUTE {
    printf("Distribute sector [ %.4f, %.4f] done.\n", start_angle, end_angle);
    fflush(stdout); 
  }
  /*all done - return */
  return nvis;

}





/* bndEvents is an array of events that cross into the sector's
   (first) boundary; they must be distributed to the boundary streams
   of the sub-sectors of this sector. Note: the boundary streams of
   the sub-sectors may not be empty; as a result, events get appended
   at the end, and they will not be sorted by distance from the vp. */
void distribute_bnd_events(Event* bndEvents, int bndEvents_length, 
			   int NUM_SECTORS, int MAX_SECTOR, 
			   Event** sectorBnd, int* sectorBnd_length, 
			   Viewpoint * vp, double start_angle, 
			   double end_angle, double *high, int* dropped_events) {

  assert(bndEvents && sectorBnd && vp && high);
  //PRINT_DISTRIBUTE {
    //printf("Distribute boundary of sector [ %.4f, %.4f]  nevents=%d\n ",
    //start_angle, end_angle, bndEvents_length);
    //fflush(stdout); 
  // }

  Event e;
  double exit_angle;
  int exit_sec;
  int i;
  for(i = 0; i < bndEvents_length; i++) {
    
    /* get the i-th event */
    e = bndEvents[i];
    
    /*     printf("in dist_bnd_events, event %d at %d,%d has type =
	   %d\n", i, e.row, e.col, e.eventType); fflush(stdout); */
    /* make sure it is an ENTER event that falls in a different sector
       than its EXIT */
    assert(e.eventType == ENTERING_EVENT);
    
    /*find its corresponding exit event and its sector */
    exit_angle = calculate_exit_angle(e.row, e.col, vp);
    exit_sec = get_event_sector(exit_angle, start_angle, end_angle, NUM_SECTORS);

    /* exit_sec cannot be outside sector; though we have to be careful
       with prescion */
    assert(exit_sec >= 0 && exit_sec < NUM_SECTORS);

    /*insert this event in the boundary stream of this sector */
    insert_event_in_sector(e, exit_sec, sectorBnd[exit_sec],
			   sectorBnd_length, MAX_SECTOR, high[exit_sec], 
			   vp, dropped_events);
    
  }
  return;
}





/* computes the sector that contains this angle.  If the angle falls
   is not between start_angle and end_angle, return -1 */
int get_event_sector(double angle, double start_angle, double end_angle,  
		     int NUM_SECTORS) {

  int s = -1;

  /*first, protect against rounding angles in the last sector */
  if(fabs(angle - end_angle) < EPSILON)
    return NUM_SECTORS - 1;

  /*same for events in the first sector */
  if(fabs(angle - start_angle) < EPSILON)
    return 0;

  double sect_size = fabs(start_angle - end_angle) / NUM_SECTORS;

  s = (int) ((angle - start_angle) / sect_size);

  if(s < 0 || s >= NUM_SECTORS) {
    s = -1;
  }

  return s;

}




int is_almost_on_boundry_helper(double angle, double boundary_angle) {

  return (fabs(angle - boundary_angle) < EPSILON) || 
    (fabs(angle - 2 * M_PI - boundary_angle) < EPSILON);

}




int is_almost_on_boundary(double angle, int sec, double start_angle, 
			  double end_angle,  int NUM_SECTORS) {
  /* the boundaries of sector s */
  double sect_size = (end_angle - start_angle) / NUM_SECTORS;

  return is_almost_on_boundry_helper(angle, (sec * sect_size)) || 
    is_almost_on_boundry_helper(angle, ((sec+1)* sect_size));

}



/* insert event e into the sector if it is not occuded by high_s */
void insert_event_in_sector(Event e, int s, Event* sector, int* sector_length, 
			    int MAX_SECTOR, 
			    double high_s, Viewpoint* vp, int* dropped) {
  
  assert(sector && sector_length && vp && dropped);
  
  /* sector is not dropped - add it to the sector and increment the
     count and the sector length */
  if(!(is_center_gradient_occluded(e, high_s, vp))) {
	/* insert in sector */
	if (sector_length[s] >= MAX_SECTOR) {
	  /* no more space in this sector */
	  printf("insert_event_in_sector:  sector is full\n");
	  exit(1); 
	}
    sector[sector_length[s]] = e;
    sector_length[s]++;
    BND_DEBUG{printf("inserted at sector %d  sector_length=%d\n", 
		     s, sector_length[s]);}
  }
  /* the sector has been dropped - increment the count */
  else {
    *dropped =  *dropped + 1; 
    BND_DEBUG{printf("dropped\n");}
  }


  return;

} 




/***********************************************************************
 returns 1 if the center of event is occluded by the gradient, which
   is assumed to be in line with the event  */
int is_center_gradient_occluded(Event e, double gradient, Viewpoint * vp) {
  assert(vp);
  double eg = calculate_center_gradient(&e, vp);
  return (eg < gradient);
}






/***********************************************************************
 the event e spans sectors from start_s to end_s; Action: update
   high[] for each spanned sector. start_s and both_s can be -1, which
   means outside given sector---in that case long cell spans to the
   boundary of the sector.
 */
void process_long_cell(int start_sec, int end_sec, Viewpoint * vp,
		       Event e, double *high, int NUM_SECTORS) {

  double ctrgrad = calculate_center_gradient(&e, vp);

  /* ENTER event is outside */
  if (start_sec == -1) {
    assert(e.eventType == EXITING_EVENT);
    assert(end_sec >=0 && end_sec < NUM_SECTORS);
    /*span from 0 to end_s */
    int j;
    for(j=0; j< end_sec; j++){
      if(high[j] < ctrgrad) {
	high[j] = ctrgrad;
      }
    }
    return;
  }

  /* EXIT event is outside */
  if(end_sec == -1) {
    assert(e.eventType == ENTERING_EVENT);
    assert(start_sec >= 0 && start_sec < NUM_SECTORS);
    /*span from start_sec to NUM_SECTORS */
    int j;
    for (j = start_sec +1; j < NUM_SECTORS; j++) {
      if(high[j] < ctrgrad) {
	high[j] = ctrgrad;
      }
    }
    return;
  }

  /* normal scenario - both inside sector */
  if(start_sec < end_sec) {
    /*we must update high[] in start_sec+1 ... end_sec -1 */
    int j;
    for(j = start_sec + 1; j < end_sec; j++) {
      if(high[j] < ctrgrad) {
	high[j] = ctrgrad;
      }
    }
    return;
  }
  else {
    /*start_sec > end_sec: we must insert in start_sec ... NUM_SECTORS and 0 ... end_sec */
    int j;
    for(j=  start_sec +1; j < NUM_SECTORS; j++) {
      if(high[j] < ctrgrad) {
	high[j] = ctrgrad;
      }
    }
    for(j = 0; j < end_sec; j++) {
      if(high[j] < ctrgrad) {
	high[j] = ctrgrad;
      }
    }
  }

  return;

}





/* print the passed stats */
void print_stats(Viewpoint* vp, int insertCount, int dropCount, 
		 int bndInsertCount, int bndDropCount, int totalCount, 
		 int NUM_SECTORS, int BASECASE_THRESHOLD) {


  printf("distribution sweep: BASECASE THRESHOLD = %d NUM_SECTORS = %d", 
	    BASECASE_THRESHOLD, NUM_SECTORS);
  printf("****STATS*****");
  printf("viewpoint (%d, %d):", vp->row, vp->col);
  printf("inserted:\t%d", insertCount);
  printf("dropped:\t%d", dropCount);
  printf("border inserted:\t%d", bndInsertCount);
  printf("border dropped:/t%d", bndDropCount);
  printf("total:/t%d", totalCount);
  return;  
}






/* base case of distribution.  */
int distribute_basecase(Event* eventList, int nevents, 
			Event* enterBndEvents,  int enterBnd_length, 
			double start_angle,  double end_angle,
			Viewpoint* vp, int deleteEventList) {
  
  assert(eventList && enterBndEvents && vp);
  PRINT_DISTRIBUTE {
    printf("solve basecase, nevents=%d, nbnd events=%d.. ", 
	   nevents, enterBnd_length); 
    fflush(stdout);
  }


  /* if there is no event in this sector, then nothing to do */
  if (nevents ==0) {
    if(deleteEventList) free(eventList);
    if  (enterBndEvents)  free(enterBndEvents);
    PRINT_DISTRIBUTE {
      printf("basecase done. Total visible cells=0\n"); 
      fflush(stdout); 
    }
    return 0; 
  }

  
  /* sort the eventList by angle */
  qsort(&eventList[0], nevents, sizeof(Event), compare_events_angle);

  /*create the status structure */
  StatusList* status_struct = create_status_struct();

  /* initialize the status structuure with all ENTER events whose EXIT
     events are inside this sector */
  Event e;
  StatusNode sn;
  int i;
  double max; 
  for(i = 0; i < enterBnd_length; i++) {
    e = enterBndEvents[i];
    sn.col = e.col;
    sn.row = e.row;
    sn.elev = e.elev;
    calculate_dist_n_gradient(&sn, vp);
    insert_into_status_struct(sn, status_struct);
  }
  

  /*sweep the event list */
  int nvis = 0; 
  for(i = 0; i < nevents; i++) {
    e = eventList[i];
    sn.col = e.col;
    sn.row = e.row;
    sn.elev = e.elev;
    calculate_dist_n_gradient(&sn, vp);

    switch(e.eventType) {
    case ENTERING_EVENT:
      insert_into_status_struct(sn, status_struct);
      break;

    case EXITING_EVENT:
      delete_from_status_struct(status_struct, sn.dist_to_vp, sn);
      break;
      
    case CENTER_EVENT:
      /* calculate visibility.  If it is visible, increment nvis,
	 otherwise do nothing */
      max = find_max_gradient_in_status_struct(status_struct, sn.dist_to_vp);
      if(max <= sn.gradient) {
	/* this point is visible */
	nvis++;
      }
      break;
    }
  } /* for each event */

  /* cleanup */
  delete_status_structure(status_struct);
  if(deleteEventList)  free(eventList);
  if(enterBndEvents) free(enterBndEvents);

  PRINT_DISTRIBUTE {
    printf("basecase done. Total visible cells=%d\n", nvis); 
    fflush(stdout); 
  }
  return nvis;
}
