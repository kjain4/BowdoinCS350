/* Will Richard
 * Grid.h
 */

#ifndef __Grid_h
#define __Grid_h

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>

#include "Points.h"

typedef struct grid_t {
  unsigned short ncols; //number of columns
  unsigned short nrows; //number of rows
  short NODATA; //the NODATA value
  Point** data; //holds all the points in a 2D array
} Grid;

//CONSTRUCT AND DISTROY --------------------------------------------------------
//create and return a new grid, with the data array allocated but not filled
Grid* Grid_new(unsigned short num_c, unsigned short num_r, short no_data_value);

//create a grid from file
Grid* Grid_createFromFile(char* grid_path);

//free the grid
void Grid_kill(Grid* grid);

//GETTERS ----------------------------------------------------------------------
unsigned short Grid_getNCols(Grid grid);
unsigned short Grid_getNRows(Grid grid);
short Grid_getNoDataValue(Grid grid);
//return a pointer to the point at i,j
Point* Grid_getPoint(Grid* grid, int i, int j);

//SETTERS ----------------------------------------------------------------------
//set the value at point i,j.  Makes a Point for c,r, but only fills in elevation.  Later, when the viewpoint is known, Grid_fillPointValues will be used to fill the rest of the values.
void Grid_setElevValue(Grid* grid, int c, int r, short value);

//Now that we know the viewpoint, go through all of the points again, setting the other values
void Grid_fillPointValues(Grid* grid, Viewpoint vp, int vpC, int vpR);

//HELPERS ----------------------------------------------------------------------
/* returns the passed int[6] which has all the header data from <file> where:
 * int[0] = ncols
 * int[1] = nrows
 * int[2] = xllcorner
 * int[3] = yllcorner
 * int[4] = cellsize
 * int[6] = NODATA_value
 */
int* Grid_readHeader(FILE* file, int* header);

//Output the visibility information of <grid> to <outputPath>
void Grid_outputVisGrid(Grid* grid, char* outputPath, char* inputPath);

#endif
