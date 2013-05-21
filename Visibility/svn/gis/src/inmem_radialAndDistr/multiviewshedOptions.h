#ifndef multiviewshedOptions_h_
#define multiviewshedOptions_h_



typedef enum {
  SWEEP_DISTRIBUTE = 0, 
  SWEEP_RADIAL = 1
} SweepMode;




typedef struct _multiviewOptions {

  /* the name of the input raster */
  char input_name [100]; 

  /* the name of the output raster */
  char output_name[100];

  /* verbose mode */
  int verbose; 

  /* number of viewpoints: compute this many viewsheds in total  */
  int NVIEWSHEDS; 

  /* if NVIEWSHEDS=1, this is the (row,col) of the viewpoint */
  int vr, vc; 

  /* the sweep mode: radial or distribute */
  SweepMode SWEEP_MODE; 

  /* the point where the recursion stops (used only in DISTRIBUTE mode) */
  int BASECASE_THRESHOLD; 

  /* the fanout of the recursion (used only in DISTRIBUTE mode)  */
  int NUM_SECTORS;

  float maxDist; 
  /* points that are farther than this distance from the viewpoint are
     not visible  */ 

  int doCurv; 
  /*determines if the curvature of the earth should be considered
    when calculating.  Only implemented for GRASS version. */
  double ellps_a; /* the parameter of the ellipsoid */
  
  float cellsize; /* the cell resolution */
  
  int memSize; /* main memory size */

} MultiviewOptions; 

#endif
