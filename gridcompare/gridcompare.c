#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h>
#include <ctype.h> 

#include "grid.h"

//write out the differences; can be set by the user
int writeoutput = 0; 

/* forward declarations */
void parse_args(int argc, char *argv[], char** gname1, char** gname2);
void print_usage(); 
int compatible_header(GridHeader *hd1,  GridHeader *hd2); 
double* compute_differences(Grid *g1, Grid *g2);
double* compute_percentage_differences(Grid *g1, Grid *g2);
void compute_distance(double* delta, double* pdelta, int n); 



/* ------------------------------------------------------------ */
int main(int argc, char *argv[]) {

  /* the name of the grids */
  char *gname1, *gname2; 
  parse_args(argc, argv, &gname1, &gname2); 
  
  Grid *g1, *g2;  
  g1 = read_grid_from_arcascii_file(gname1); 
  g2 = read_grid_from_arcascii_file(gname2); 
  assert(g1 && g2); 

  printf("grid1: (rows=%d, cols=%d)\n", g1->hd->nrows, g1->hd->ncols);fflush(stdout);
  printf("grid2: (rows=%d, cols=%d)\n", g2->hd->nrows, g2->hd->ncols);fflush(stdout);
  
  if (!compatible_header(g1->hd, g2->hd)) {
    printf("headers not compatible.\n"); 
    exit(0); 
  } else {
    printf("headers compatible.\n"); 
  }

  double* delta = compute_differences(g1, g2); 
  double* pdelta = compute_percentage_differences(g1, g2); 
  compute_distance(delta, pdelta, g1->hd->nrows * g1->hd->ncols); 

  destroy_grid(g1);
  destroy_grid(g2);
  return 0;
}


/* ------------------------------------------------------------ */
//delta represents a difference vector of size n.  pdelta represents
//percentage differences. compute the norm of these vectors.
void compute_distance(double* delta, double* pdelta, int n) {

  assert(delta); 
  int i, nonzero=0; 
  double max = 0, sum=0, sumsq=0, sumper=0, maxper=0; 
  for (i=0; i<n; i++) {
    
    if(delta[i] != 0) nonzero++; 
    if (fabs(delta[i]) > max) max = fabs(delta[i]); 
    if (pdelta[i] >maxper) maxper = pdelta[i];
    sum += fabs(delta[i]); 
    sumsq += delta[i]*delta[i];
    sumper += pdelta[i]; 
  }
  
  printf("total points: %d\n", n); 
  printf("\tmatching: %d, nonmatching: %d\n", n-nonzero, nonzero);
  printf("\tmax       difference: %.2f\n", max); 
  printf("\tavg       difference: %.2f\n", ((float)sum)/n); 
  printf("\tsum       difference: %.2f\n", sum);
  printf("\teuclidian difference: %.2f\n", sqrt(sumsq));
  printf("\taverage percentatge difference: %.2f\n", sumper/n);
  printf("\tmax percentatge difference: %.2f\n", maxper);
}

/* ------------------------------------------------------------ */
double* compute_differences(Grid *g1, Grid *g2) {

  assert(g1 && g2); 
  int i, j, nrows, ncols;
  double* delta;    //the difference array 
  
  nrows =  g1->hd->nrows;
  ncols =  g1->hd->ncols;
  delta = (double*) malloc(nrows*ncols*sizeof(double));
  assert(delta); 
  for (i=0; i<nrows; i++) {
    for (j=0; j<ncols; j++) {
      if (is_nodata(g1->hd, g1->grid_data[i][j]) || is_nodata(g2->hd, g2->grid_data[i][j]))
	{
	  //this is a nodata point; skip 
	  delta[i*ncols+j] = 0; 
	  continue;
	} 
      delta[i*ncols+j] =  g1->grid_data[i][j] - g2->grid_data[i][j]; 
      if (writeoutput &&   (delta[i*ncols+j] != 0)) 
	printf("(%3d,%3d): %5.1f [1] %5.1f [2]\tdelta: %5.1f\n",  
	       i, j, g1->grid_data[i][j], g2->grid_data[i][j], delta[i*ncols+j]);
     }
  }
  return delta; 
}


/* ------------------------------------------------------------ */
double* compute_percentage_differences(Grid *g1, Grid *g2) {

  assert(g1 && g2); 
  int i, j, nrows, ncols;
  double* delta;    //the difference array 
  
  nrows =  g1->hd->nrows;
  ncols =  g1->hd->ncols;
  delta = (double*) malloc(nrows*ncols*sizeof(double));
  assert(delta); 
  for (i=0; i<nrows; i++) {
    for (j=0; j<ncols; j++) {

      if (is_nodata(g1->hd, g1->grid_data[i][j]) || is_nodata(g2->hd, g2->grid_data[i][j]))
	{
	  //this is a nodata point; skip 
	  delta[i*ncols+j] = 0; 
	}
      else {
	if (g1->grid_data[i][j] == 0 && g2->grid_data[i][j]==0)
	  delta[i*ncols+j] = 0; 
	else if (g1->grid_data[i][j] == 0) {
	  printf("warning: point (%d,%d): 0 [1] %.2f [2]\n",i,j,g2->grid_data[i][j]); 
	  delta[i*ncols+j]  = 100; //100% errorq
	} else {
	  delta[i*ncols+j] =  fabs(g1->grid_data[i][j] - g2->grid_data[i][j])/fabs(g1->grid_data[i][j]);
	}
      }
    } //for j
  } //for i

  return delta; 
}



/* ------------------------------------------------------------ */
int compatible_header(GridHeader *hd1,  GridHeader *hd2) {
  
  assert(hd1 && hd2); 
  if (hd1->nrows != hd2->nrows) return 0;
  if (hd1->ncols != hd2->ncols) return 0;
  if (hd1->xllcorner != hd2->xllcorner) {
    printf("warning: xllcorner does not match\n"); 
  }
  if (hd1->yllcorner != hd2->yllcorner) {
    printf("warning: yllcorner does not match\n"); 
  }
  if (hd1->cellsize != hd2->cellsize) {
    printf("warning: cellsize does not match\n"); 
  }
  if (hd1->nodata_value != hd2->nodata_value) {
    printf("warning: nodata value does not match\n"); 
  }
  return 1;
}


/* ------------------------------------------------------------ */
void print_usage() {
  printf("\nusage: gridcompare -1 <grid1name> -2 <grid2name> -o\n"); 
  printf("\tgrid1 is the reference grid\n"); 
  printf("\tgrid2 is the grid to compare\n"); 
  printf("\t-o prints to stdout the differences\n"); 
}


/* ------------------------------------------------------------ */
void parse_args(int argc, char *argv[], char** gname1, char** gname2)  {
  
  int gotname1 = 0, gotname2=0; 
  int c;
  
  /*deal with flags for the output using getopt */
  while ((c = getopt(argc, argv, "1:2:o")) != -1) {
	switch (c) {
	case '1':
	  /* grid1 name */
	  *gname1 = malloc(strlen(optarg)+1);
	  strcpy(*gname1, optarg);
	  gotname1 = 1; 
	  break;
	case '2':
	   /* grid2 name */
	  *gname2 = malloc(strlen(optarg)+1); 
	  strcpy(*gname2, optarg);
	  gotname2 = 1; 
	  break;
	case 'o': 
	  writeoutput = 1; 
	  break; 
	case '?':
	  if (optopt == '1' || optopt == '2')
		fprintf(stderr, "Option -%c requires an argument.\n", optopt);
	  else if (isprint(optopt)) 
		fprintf(stderr, "Unknown option '-%c'.\n", optopt);
	  else
		fprintf(stderr, "Unknown option character '\\x%x.\n", optopt);
	  print_usage();
	  exit(1);
	}
  } /* while getopt */
  
    /*check to make sure the required options are set.*/
  if(!(gotname1 && gotname2)) {
	printf("Not all required options set.\n");
	print_usage();
	exit(1);
  }
  printf("gname1=%s, gname2=%s writediff = %d\n", *gname1, *gname2, writeoutput);
  return;
}






