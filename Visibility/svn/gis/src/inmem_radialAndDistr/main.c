#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <ctype.h>

#include "grid.h"
#include "event.h"
#include "status_structure.h"
#include "radial.h"
#include "inmemdistribute.h"
#include "rtimer.h"
#include "multiviewshedOptions.h"
#include "event_quicksort.h"


//#define SYSTEM_SORT
//if this flag is defined the sweep uses system qsort; otherwise it
//uses a (hopefully faster) quicksort defined in event_quicksort.h


/* ********************************************************************** */
/* ********************************************************************** */
/* 
   INTERPOLATE_RESULT: This flag should be on only if the VC grid is
   not computed for every single viewpoint. In other words, if
   NVIEWSHEDS < nrows* ncols.

   If this is on, the output VC grid is interpolated at the points
   where it was not computed using nearest-neighbor. Every point in
   the VC output grid for which the viewshed was not computed is thus
   assigned the values of teh closest viewpoint for which teh viewshed
   was computed. 
   
   It is outside reported timing, so when defined will not make a
   difference to running time.
   
*/
//#define INTERPOLATE_RESULT
#ifdef INTERPOLATE_RESULT
/* this is needed for interpolating the output grid */
typedef struct _nvis {
  int row, col; 
  int nvis; 
} Nvis;

//the arrays that will contain the computed viewsheds
Nvis* viewsheds;
int nvp=0; //the actual number of viewsheds

void interpolate_raster(Grid* outgrid, char* output_name);
#endif
/* ************************************************************************* */
/* ************************************************************************* */






/* ************************************************************ */
/*  read user parameters and store them in options */ 
void parse_args(int argc,char* argv[], MultiviewOptions* options); 

void record_args(MultiviewOptions opt);

void print_usage(); 

/* set viewpoint */
void set_viewpoint(Viewpoint* vp, int row, int col, float elev);

/* compute the viewshed using a distribution sweep */
void compute_multiviewshed_distribution(MultiviewOptions opt, int DO_EVERY, 
					Grid* ingrid,Grid* outgrid,
					int nevents, Event* eventlist);

/* compute the viewshed using a radial sweep */
void compute_multiviewshed_radial(MultiviewOptions opt, int DO_EVERY,  
				  Grid* ingrid, Grid* outgrid,
				  int nevents, Event* eventlist);


void print_init_timings(Rtimer initTime);




void print_usage() {
  printf("usage:\nmultiviewshed -i <inputname> -o <outputname> -v <nbviewpoints> -s <sweepmode> -b <basecase> -f <fanout> -r <row> -c <col> -w\n");
  printf("OPTIONS:\n");
  printf("\t-i input map name.\n"); 
  printf("\t-o output map name.\n"); 
  printf("\t-v number of viewpoints to compute viewsheds for [default: all].\n"); 
  printf("\t-r row of viewpoint [relevant only if NVIEWSHEDS=1]\n"); 
  printf("\t-c col of viewpoint [relevant only if NVIEWSHEDS=1]\n"); 
  printf("\t-s sweep mode.[radial or distribute]. \n"); 
  printf("\t-b basecase [relevant only if mode=distribute].\n"); 
  printf("\t-f fanout [relevant only if mode=distribute].\n"); 
  printf("\t-w verbose.\n"); 
}



/* ************************************************************ */
/* read user parameters and store them in options */
void  parse_args(int argc,char* argv[], MultiviewOptions *options) {
  
  assert(options);
  //set some default values
  options->NVIEWSHEDS = 0; 
  options->BASECASE_THRESHOLD = 0; 
  options->NUM_SECTORS = 0; 
  options->verbose=0;
  options->vc = options->vr = -1; 

  int gotinput=0, gotoutput=0, gotmode=0;
  char c; 
  while ((c = getopt(argc, argv, "i:o:v:s:b:f:r:c:w")) != -1) {
    switch (c) {
    case 'i':
      /* inputfile name */
      //*inputfname = optarg;
      strcpy(options->input_name, optarg);
      gotinput = 1; 
      break;
    case 'o':
      /* outputfile name */
      //*outputfname = optarg;
      strcpy(options->output_name, optarg);
      gotoutput= 1; 
      break;
    case 'v': 
      /* number of viewsheds */
      options->NVIEWSHEDS = atoi(optarg);
      break; 
    case 'r': 
      /* if NVIEWSHEDS=1, thsi is teh row of teh viewpoint */
      options->vr = atoi(optarg); 
      break; 
    case 'c': 
      /* if NVIEWSHEDS=1, thsi is teh col of teh viewpoint */
      options->vc = atoi(optarg); 
      break; 
    case 's': 
      /* THE SWEEP MODE */      
      if(strcmp(optarg,"radial")==0)
	options->SWEEP_MODE = SWEEP_RADIAL; 
      else if (strcmp(optarg,"distribute")==0)
	options->SWEEP_MODE = SWEEP_DISTRIBUTE; 
      else {
	printf("unknown option %s: use  -s: [radial|distribute]\n", optarg); 
	exit(1);
      }
      gotmode=1;
      break; 
    case 'b': 
      /* BASECASE THRESHOLD */
      options->BASECASE_THRESHOLD = atoi(optarg); 
      break; 
    case 'f': 
      /* fanout/NUM_SECTORS */
      options->NUM_SECTORS = atoi(optarg); 
      break; 
    case 'w': 
      options->verbose = 1; 
      break;
    case '?':
        if (optopt == 'i' || optopt == 'o' || optopt == 'n' ||
	    optopt == 's' || optopt == 'b' || optopt == 'f')
	  fprintf(stderr, "Option -%c requires an argument.\n", optopt);
	else if (isprint(optopt)) 
	  fprintf(stderr, "Unknown option '-%c'.\n", optopt);
	else
	  fprintf(stderr, "Unknown option character '\\x%x.\n", optopt);
	print_usage();
	exit(1);
    } 
  }//while getopt 

  if (!gotinput || !gotoutput || !gotmode) {
    printf("Not all required options set.\n");
    print_usage();
    exit(1);
  }
  
  //check parameters 
  if (options->NVIEWSHEDS < 0) {
    printf("NVIEWSHEDS cannot be <0\n");
    exit(1); 
  }
  if (options->NVIEWSHEDS == 0) {
    //will set this later to be nrows * ncols
  }
  if (options->NVIEWSHEDS==1) {
    //user must have entered coordinates of viewpoint 
    if(options->vr <0 || options->vc <0) {
      printf("vp coord cannot be <0\n");
      exit(1); 
    }
  }
  if (options->BASECASE_THRESHOLD < 0) {
    printf("BASECASE cannot be <0\n");
    exit(1); 
  }
  if (options->NUM_SECTORS < 0) {
    printf("NUM_SECTORS cannot be <0\n");
    exit(1); 
  }
  
  if (options->SWEEP_MODE ==SWEEP_DISTRIBUTE) 
    assert(options->BASECASE_THRESHOLD > 0 &&   options->NUM_SECTORS > 0);
    
}



/************************************************************/
void record_args(MultiviewOptions opt) {
  printf("------------------------");
 printf("multiIOviewshed:  nb.viewpoints=%d\n", opt.NVIEWSHEDS);
  if (opt.NVIEWSHEDS==1) 
    printf("vp= (%d,%d)\n", opt.vr, opt.vc);
  
  if (opt.SWEEP_MODE == SWEEP_DISTRIBUTE)
    printf("MODE: distribution sweep\n");
  else 
    printf("MODE: radial sweep, base=%d, fanout=%d\n", 
	   opt.BASECASE_THRESHOLD,opt.NUM_SECTORS);
#ifdef SYSTEM_SORT
  printf("using system qsort\n");
#else 
  printf("using own sort\n");
#endif
 printf("------------------------");
}



/* ************************************************************ */
void print_init_timings(Rtimer initTime) {
  char timeused[100];
  printf("INIT events:\n");
  rt_sprint_safe(timeused, initTime);
  printf("%20s: %s\n", "init total", timeused);
  fflush(stdout);
  return;
}




/************************************************************/
int main(int argc, char* argv[]) {

 //this variable collects all user options
  MultiviewOptions options;

  parse_args(argc, argv, &options); 
  record_args(options);

#ifdef INTERPOLATE_RESULT 
  viewsheds = (Nvis*) malloc((options->NVIEWSHEDS+10)*sizeof(Nvis)); 
  assert(viewsheds);
#endif 


  //read input raster 
  printf("reading input grid %s ", options.input_name); 
  Grid *ingrid = read_grid_from_arcascii_file(options.input_name);
  assert(ingrid); 
  printf("..done\n");

  //number of rows and columns in the grid 
  int nrows, ncols;  
  nrows = ingrid->hd->nrows; 
  ncols = ingrid->hd->ncols; 
  printf("grid: rows = %d, cols = %d\n", nrows, ncols);

  if (options.NVIEWSHEDS ==0) 
    options.NVIEWSHEDS = nrows * ncols; 

  //create an output grid 
  Grid* outgrid = create_empty_grid(); 
  assert(outgrid);
  //outgrid->hd = create_empty_header(); 
  //the header is allocated in create_empty_grid()
  copy_header(outgrid->hd, *(ingrid->hd)); 
  alloc_grid_data(outgrid); 


  /* **************************************** */
  /* INITIALIZE EVENT LIST */
  /* **************************************** */

  /*allocate the eventlist to hold the maximum number of events possible*/
  Event* eventList;
  eventList = (Event*) malloc(ncols * nrows * 3 * sizeof(Event));
  assert(eventList);
  
  /*initialize the eventList with the info common to all viewpoints */
  long  nevents;
  Rtimer initTime; 
  rt_start(initTime);
  nevents  = init_event_list(eventList, ingrid );
  printf("nb events = %ld\n", nevents);
  rt_stop(initTime); 
  print_init_timings(initTime); 
  
 

  /* ****************************** */   
  /* compute the viewshed of the i % DO_EVERY point  */
  /* ****************************** */   
  
  assert(options.NVIEWSHEDS > 0);
  int DO_EVERY = nrows * ncols/ options.NVIEWSHEDS; 
  /* start going through the data and considering each point, in turn,
     as a viewshed */
 
  if (options.SWEEP_MODE == SWEEP_DISTRIBUTE)  {
    assert(options.BASECASE_THRESHOLD >0 && options.NUM_SECTORS >0);
    compute_multiviewshed_distribution(options, DO_EVERY,
				       ingrid, outgrid,  nevents, eventList); 
  }
  else { 
    compute_multiviewshed_radial(options, DO_EVERY, ingrid, outgrid, 
				 nevents, eventList);
  }


  /* ****************************** */
  /*all sweeping and computing done - clean up */
  free(eventList);

  //write output grid to file 
  save_grid_to_arcascii_file(outgrid, options.output_name); 

  //clean up 
  destroy_grid(ingrid); 
  destroy_grid(outgrid); 


#ifdef INTERPOLATE_RESULT
  //for NVIEWSHEDS small, the resulting map is basically all empty;
  //the interpolate function extends each non-empty output viewshed
  //value to a ball centered at that point
  interpolate_raster(outgrid, output_name); 
#endif

  exit(0); 
}









/* ************************************************************ */
void compute_multiviewshed_distribution(MultiviewOptions opt, int DO_EVERY, 
					Grid* ingrid, Grid* outgrid, 
					int nevents, Event* eventlist) {

  
  assert(ingrid && outgrid && eventlist); 
  printf("\n----------------------------------------\n");
  printf("Starting distribute sweep"); 
  printf("total %d viewpoints, BASECASE_THRESHOLD=%d NUM_SECTORS=%d\n", 
	 opt.NVIEWSHEDS, opt.BASECASE_THRESHOLD,opt.NUM_SECTORS); 

  Viewpoint vp; 
  int nvis; 
  int dropped, total_dropped=0;   /*   dropped cells during distribution */
  int nviewsheds = 0;
  Rtimer sweepTotalTime;
  int nrows, ncols, row, col;

   /* get raster rows and cols */
  nrows = ingrid->hd->nrows;
  ncols = ingrid->hd->ncols;



  /* ************************************************************ */
  //compute just one viewshed 
  if (opt.NVIEWSHEDS == 1) {
    rt_start(sweepTotalTime);
    //compute just one viewshed count
    set_viewpoint(&vp, opt.vr, opt.vc,  get(ingrid, opt.vr, opt.vc)); 
    
    if (is_nodata_at(ingrid, opt.vr, opt.vc))  
      printf("point at (%5d,%5d): NODATA\n",opt.vr, opt.vc); 
    else {
      /*set the viewpoint to be this point */
      float crt_elev = get(ingrid, opt.vr, opt.vc); 
      set_viewpoint(&vp, opt.vr, opt.vc, crt_elev); 
      
      /*set the angles for all the events in the eventlist*/
      set_event_list_angles_and_dist(nevents, eventlist, &vp);
      
      /*sort the eventlist*/
#ifdef SYSTEM_SORT
      qsort(eventlist, nevents, sizeof(Event), compare_events_angle);
#else 
      event_quicksort_radial(eventlist, nevents);
#endif
      /*compute the visibility of the viewpoint */
      dropped = 0; 
      nvis = distribute_and_sweep(eventlist, nevents, opt.NUM_SECTORS, 
				  opt.BASECASE_THRESHOLD, &vp, &dropped);
      
      /* update total number of drpped cells */
      total_dropped += dropped; 
      
      /* write nvis to output raster */
      set(outgrid, opt.vr, opt.vc, nvis);
      
      rt_stop(sweepTotalTime); 
      printf("v=(%5d,%5d): nvis=%10d\n", opt.vr, opt.vc, nvis); fflush(stdout); 
      printf("cells dropped = %d\n", total_dropped);
      char timeused[100];
      printf("TOTAL time: \n");
      rt_sprint_safe_average(timeused, sweepTotalTime, 1); 
      printf("%20s: %s\n", "total", timeused);

    }
    return; 
  }

 
 /* ************************************************************ */
  //else  compute VC for many/all viewpoints

  for(row = 0; row < nrows; row++) {
    for(col = 0; col < ncols; col++) {
      
      if(((row*ncols)+col) % DO_EVERY != 0){
	/* do not compute the viewshed of this point: set this point
	   in the output as NODATA */
	set_nodata(outgrid, row, col);
	continue;
      }
      
      /* check if this point is nodata.  If it is, write it as
	 such in output raster and continue*/
      if (is_nodata_at(ingrid, row, col)) {
	set_nodata(outgrid, row, col);
	if (opt.NVIEWSHEDS < 10) 
	  //don't print unless very few viewsheds
	  printf("point at (%5d,%5d): NODATA; ignoring\n",row, col); 
	fflush(stdout);  
	continue; 
      }
      
      /* compute the viewshed of this point */
      nviewsheds++; 
      
      /* print progress to user, one dot per viewpoint */
      //printf("."); fflush(stdout); 
     

      /*set the viewpoint to be this point */
      float crt_elev = get(ingrid, row, col); 
      set_viewpoint(&vp, row, col, crt_elev); 
      
      /*set the angles for all the events in the eventlist*/
      set_event_list_angles_and_dist(nevents, eventlist, &vp);
      
      /*sort the eventList by distance */
      //printf("\nsorting concentrically.."); fflush(stdout);
#ifdef SYSTEM_SORT
      qsort(eventlist, nevents, sizeof(Event), compare_events_dist);
#else 
      event_quicksort_distance(eventlist, nevents);
#endif
      /*distribute and sweep */
      dropped = 0; 
      nvis = distribute_and_sweep(eventlist, nevents, opt.NUM_SECTORS, 
				  opt.BASECASE_THRESHOLD, &vp, &dropped);
      
      /* update total number of drpped cells */
      total_dropped += dropped; 
      
      /* write nvis to output raster */
      set(outgrid, row, col, nvis);
      
#ifdef INTERPOLATE_RESULT 
      //insert this poitn in the array of computed viewsheds 
      Nvis x = {row,col,nvis}; 
      viewsheds[nvp] = x; 
      nvp++;
#endif

      // print this point 
      if (opt.verbose) {
	printf("point at (%5d,%5d): ",row, col); fflush(stdout);  
	printf("nvis=%10d, dropped=%10d\n", nvis, dropped); fflush(stdout);
      }

    } /* for col */
  } /* for row */
  rt_stop(sweepTotalTime);
  printf("\ndone.");
  
  printf("DISTRIBUTION SWEEP: BASECASE_THRESHOLD=%d NUM_SECTORS=%d\n",
	 opt.BASECASE_THRESHOLD,opt.NUM_SECTORS);  
  printf("total nviewsheds=%d\n", nviewsheds);
  if (nviewsheds==0) {
    printf("%20s: 0\n", "total");
  } else {
    float avg_dropped;
    avg_dropped = (float)total_dropped / (float)nviewsheds;
    printf("average nb. cells dropped = %f (%.2f %%)\n", 
	   avg_dropped, avg_dropped/(float)nevents * 100);
    
    char timeused[100];
    rt_sprint_safe_average(timeused, sweepTotalTime, nviewsheds); 
    printf("AVERAGE TIME PER VIEWPOINT: \n"); 
    printf("\t%20s: %s\n", "viewshed average total", timeused);
    
    printf("TOTAL time: \n");
    rt_sprint_safe_average(timeused, sweepTotalTime, 1); 
    printf("%20s: %s\n", "total", timeused);
  }
  
  return;
} 




/* ************************************************************ */
/* fill vp with all the data passed */
void set_viewpoint(Viewpoint* vp, int row, int col, float elev) {
  
  assert(vp);
  vp->row = row;
  vp->col = col;
  vp->elev = elev;
  return;
}


 

         


/* ************************************************************ */
/* ingrid is the input grid that contains the header and data. outgrid
   is the ouutput grid, it has a valid header and data is allocated
   but has no values yet. the eventlist contains all events for any
   viewpoint. */
void compute_multiviewshed_radial(MultiviewOptions opt, int DO_EVERY, 
				  Grid* ingrid, Grid* outgrid, 
				  int nevents, Event* eventlist) {
  assert(ingrid && outgrid && eventlist); 
  printf("\n----------------------------------------\n");
  printf("Starting radial sweep, total %d viewpoints.\n", opt.NVIEWSHEDS); 
  
  int nvis, nviewsheds=0;
  Rtimer sweepTotalTime;
  int nrows, ncols, row, col;
  Viewpoint vp; 

  /* get raster rows and cols */
  nrows = ingrid->hd->nrows;
  ncols = ingrid->hd->ncols;
  
   
  /* ************************************************************ */
  //compute just one viewshed 
  if (opt.NVIEWSHEDS == 1) {
    rt_start(sweepTotalTime);

    //compute just one viewshed count
    set_viewpoint(&vp, opt.vr, opt.vc,  get(ingrid, opt.vr, opt.vc)); 

    if (is_nodata_at(ingrid, opt.vr, opt.vc))  
      printf("point at (%5d,%5d): NODATA\n",opt.vr, opt.vc); 
    else {
      /*set the angles for all the events in the eventlist*/
      set_event_list_angles_and_dist(nevents, eventlist, &vp);
      
      /*sort the eventlist*/
#ifdef SYSTEM_SORT
      qsort(eventlist, nevents, sizeof(Event), compare_events_angle);
#else 
      event_quicksort_radial(eventlist, nevents);
#endif
      /*compute the visibility of the viewpoint */
      nvis = sweep_radial(eventlist,nevents, ingrid->grid_data[opt.vr],
		     vp,ingrid);
      rt_stop(sweepTotalTime); 

      printf("v=(%5d,%5d): nvis=%10d\n", opt.vr, opt.vc, nvis); fflush(stdout); 
      printf("TOTAL time: \n");
      char timeused[100];
      rt_sprint_safe_average(timeused, sweepTotalTime, 1); 
      printf("%20s: %s\n", "total", timeused); 
    }

    return; 
  }


  /* ************************************************************ */
  //else  compute VC for many/all viewpoints
  rt_start(sweepTotalTime);
  for(row = 0; row < nrows; row++) {
    for (col = 0; col < ncols; col++) {
      
      
      if (((row*ncols)+col) % DO_EVERY !=0){
	/* do not compute the viewshed of this point: set this point
	   in the output as NODATA */
	set_nodata(outgrid, row, col);
	continue;
      }
      
      /*check if this point is nodata.  If it is, write it as such in
	output raster and continue*/
      if (is_nodata_at(ingrid, row, col)) {
	set_nodata(outgrid, row, col);
	if (opt.NVIEWSHEDS < 10) 
	  //don't print unless very few viewsheds
	  printf("point at (%5d,%5d): NODATA; ignoring\n",row, col); 
	fflush(stdout);  
	continue; 
      }
      
      /* compute the viewshed of this point */
      nviewsheds++; 
      
      /* print progress to the user: one dot per viewshed computed*/
      //printf("."); fflush(stdout); 
      //printf("point at (%5d,%5d): ",row, col); fflush(stdout);  
      
      /*set the viewpoint to be this point */
      float crt_elev = get(ingrid, row, col); 
      set_viewpoint(&vp, row, col, crt_elev); 
      
      /*set the angles for all the events in the eventlist*/
      set_event_list_angles_and_dist(nevents, eventlist, &vp);
      
	
      /*sort the eventlist*/
      /* note: this should be done with an optimized quicksort */
      qsort(eventlist, nevents, sizeof(Event), compare_events_angle);
      
      /*compute the visibility of the viewpoint */
      nvis = sweep_radial(eventlist,nevents, ingrid->grid_data[row],vp,ingrid);
	
      /* write nvis to the output raster */
      set(outgrid, row, col, nvis);
      
#ifdef INTERPOLATE_RESULT 
      //insert this poitn in the array of computed viewsheds 
      Nvis x = {row,col,nvis}; 
      viewsheds[nvp] = x; 
      nvp++;
#endif

      //print this point 
      if (opt.verbose) printf("v=(%5d,%5d): nvis=%10d\n", row, col, nvis);

      } /* for col */
  } /* for row */
  rt_stop(sweepTotalTime);
  
  printf("\ndone.\n");
  printf("----------------------------------------\n");
  printf("RADIAL sweep:\n"); 
  printf("total nviewsheds=%d\n", nviewsheds);
  if (nviewsheds==0) {
    printf("%20s: 0\n", "total");
  } else {
    char timeused[100];
    rt_sprint_safe_average(timeused, sweepTotalTime, nviewsheds); 
    printf("AVERAGE viewshed time per viewpoint: \n");
    printf("%20s: %s\n", "total", timeused);
    
    printf("TOTAL time: \n");
    rt_sprint_safe_average(timeused, sweepTotalTime, 1); 
    printf("%20s: %s\n", "total", timeused);
  }
  return;
}







/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */
#ifdef INTERPOLATE_RESULT

float dist(int i1, int j1, int i2, int j2) {
  return (i1-i2)*(i1-i2) + (j1-j2)*(j1-j2);
} 

//this functon is used to interpolate the value of a point in teh
//viewshed count grid. it returns the size of the closest viewshed */
int findClosestViewpoint(int r, int c) {

  int i;
  float mindist = dist(r,c,viewsheds[0].row, viewsheds[0].col);
  int minnvis = viewsheds[0].nvis;

  float d; 
  for (i=1; i< nvp; i++) {
    d = dist(r,c,viewsheds[i].row, viewsheds[i].col);
    if (d < mindist) {
      mindist = d;
      minnvis = viewsheds[i].nvis;
    }
  }
  return minnvis; 
}

//for NVIEWSHEDS small, the resulting map is basically all empty;
//the interpolate function extends each non-empty output viewshed
//value to a ball centered at that point
void interpolate_raster(Grid* outgrid, char* output_name) {
  
  assert(outgrid);
  printf("\n-----\n");
  printf("Multiviewshed done, interpolating result grid."); 
  printf("total  %d viewsheds: ", nvp); 

  /*create the interpolated output raster */
  Grid* intgrid = create_empty_grid(); 
  assert(intgrid); 
  copy_header(intgrid->hd, outgrid->hd); 
  alloc_grid_data(intgrid); 

  int nrows, ncols, row, col;
  nrows = intgrid->hd->nrows; 
  ncols = intgrid->hd->ncols; 
  
  int nvis; 
  for(row = 0; row < nrows; row++) {
    for(col = 0; col < ncols; col++) {  
      
      nvis = findClosestViewpoint(row,col);
      //printf("%d %d :%d\n", row, col, nvis); 
      set(intgrid, row, col,  nvis); 
    }//for col
  } //for row 
  
  /* write grid to file */
  char* interp_name = (char*)malloc((strlen(output_name+8))*sizeof(char)); 
  assert(interp_name); 
  strcpy(interp_name,output_name);
  strcat(interp_name, "-interp"); 
  printf("creating interpolated raster: %s\n", interp_name); 
  save_grid_to_arcascii_file(intgrid, interp_name); 

  destroy_grid(intgrid);
}



#endif
