#ifndef __B_Map_h
#define __B_Map_h

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "Elev_type.h"

//short for basic map - just has your basic map structure
typedef struct b_map_t {
  unsigned short ncols;
  unsigned short nrows;
  elev_type NODATA;
  elev_type minElev;
  elev_type maxElev;
/*   unsigned short maxFlow; //max fa value */
  elev_type* elev_data; 
} B_Map;

//Construct and Distroy ------------------------------------------------
//create and return a new map, with the data array allocated but not filled
B_Map* B_Map_new(int num_c, int num_r, elev_type no_data_value);

//create a map from file
B_Map* B_Map_createFromFile(char* grid_path);

//free up the map
void B_Map_kill(B_Map* map);

//HELPERS --------------------------------------------------
/*returns a float[6] which has all the header data, where
 float[0] = ncols, float[1] = nrows, float[2] = xllcorner, float[3] = yllcorner, float[4] = cellsize and float[5] = NODATA_value*/
void B_Map_readHeader(FILE* file, float headerData[6]);

//GETTERS --------------------------------------------------
unsigned short B_Map_getNRows(B_Map map);

unsigned short B_Map_getNCols(B_Map map);

elev_type B_Map_getNoDataValue(B_Map map);

elev_type B_Map_getMinElev(B_Map map);

elev_type B_Map_getMaxElev(B_Map map);

unsigned short B_Map_getMaxFlow(B_Map map);

//returns the value stored at i,j
elev_type B_Map_getValue(B_Map map, unsigned short i, unsigned short j);

//SETTERS ----------------------------------------------------------
//set the value at i, j
void B_Map_setValue(B_Map* map, int c, int r, elev_type value);

//set the min and max elev
void B_Map_setMinElev(B_Map* map, elev_type value);
void B_Map_setMaxElev(B_Map* map, elev_type value);

void B_Map_setMaxFlow(B_Map* map, unsigned short value);

#endif
