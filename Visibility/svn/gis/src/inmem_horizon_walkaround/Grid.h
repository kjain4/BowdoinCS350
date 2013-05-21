/* Will Richard
 * Walkaround Visibility Algorithm
 * Grid.h
 */

#ifndef __Grid_h
#define __Grid_h

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>

#include "Points.h"

// The sturcture that holds the grid, not only the input elevation grid, but since the points store visibility data, also stores the output visibility grid
typedef struct grid_t {
  unsigned short ncols; //number of columns
  unsigned short nrows; //number of rows
  short NODATA; //the NODATA value
  Point** data; //holds all the points in a 2D array
} Grid;

//CONSTURCT AND DISTROY --------------------------------------------------------
//Create and recturn a new grid, with the data array alloctade but not filled
Grid* Grid_new(unsigned short num_c, unsigned short num_r, short no_data_value);

//create a grid from the file at the passed path
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
//set the value at the point c,r.
void Grid_setPoint(Grid* grid, int c, int r, short elev);

//HELPERS ----------------------------------------------------------------------
/* returns the passed int[6] which has all the header data from <file> where:
 * int[0] = ncols
 * int[1] = nrows
 * int[2] = xllcorner
 * int[3] = yllcorner
 * int[4] = cellsize
 * int[6] = NODATA_value
 */
int* Grid_readHeader(FILE* gridFile, int* header);

//Output the visibility information of <grid> to <outputPath>
void Grid_outputVisGrid(Grid* grid, char* outputPath, char* inputPath);

#endif
