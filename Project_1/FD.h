#ifndef __FD_Map_h
#define __FD_Map_h

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>


#include "Elev_type.h"
#include "B_Map.h"
#include "rtimer.h"

typedef struct fd_map_t {
  B_Map* b_map;
  short* fd_data;
} FD_Map;

//Construct, Output and Distroy
//create FD_Map from file
FD_Map* FD_Map_createFromFile(char* grid_path);

//write the FD map to file
void FD_Map_writeMap(FD_Map map, char* in_path, char* out_path);

//free up the FD_Map
void FD_Map_kill(FD_Map* map);

//GETTERS --------------------------------------------------
B_Map* FD_Map_getBMap(FD_Map map);

short FD_get_value(FD_Map map, unsigned short i, unsigned short j);

//SETTERS ----------------------------------------------------------
void FD_Map_setValue(FD_Map* map, unsigned short i, unsigned short j, unsigned short value);

//HELPERS ----------------------------------------------------------
//given a map, point and direction, return the elev at the point in that directino
elev_type FD_elevInDir(B_Map map, unsigned short c, unsigned short r, unsigned short dir);


//WORKHORSE ----------------------------------------------------------
//actually does the work for the FD algorithm - passed a FD grid, fills the FD grid
void FD_fill(FD_Map* map);

#endif
