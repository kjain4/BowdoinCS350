#ifndef __FA_H
#define __FA_H

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "Elev_type.h"
#include "B_Map.h" //use the read_header method from B_Map
#include "rtimer.h"

//FA Point structure
typedef struct fa_point_t {
  unsigned short col;
  unsigned short row;
  elev_type elev;
  unsigned short fa;
  struct fa_point_t* fd;
} FA_Point;

//FA Map Struct
typedef struct fa_map_t {
  unsigned short ncols;
  unsigned short nrows;
  elev_type NODATA;
  FA_Point** fa_data;
} FA_Map;

//Construct and Distroy ------------------------------------------------
//create and return a new FA_Map, with its fa_data array allocated but not filled
FA_Map* FA_Map_new(unsigned short num_c, unsigned short num_r, elev_type no_data_value);

//create an FA_Map from file
FA_Map* FA_Map_createFromFile(char* grid_path, char* fd_path);

//write the FA map to file.  in_path is passed so we can copy the header from it
void FA_Map_writeMap(FA_Map map, char* in_path, char* out_path);

//free the FA_Map
void FA_Map_kill(FA_Map* map);

//GETTERS --------------------------------------------------
//Map Getters
unsigned short FA_Map_getNRows(FA_Map map);
unsigned short FA_Map_getNCols(FA_Map map);
elev_type FA_Map_getNoDataValue(FA_Map map);
//get the FA_Point stored at i,j
FA_Point* FA_Map_getPoint_ij(FA_Map map, unsigned short i, unsigned short j);
//get the FA_Point storted at index i
FA_Point* FA_Map_getPoint_index(FA_Map map, unsigned short i);

//FA_Point getters
unsigned short FA_Point_getCol(FA_Point p);
unsigned short FA_Point_getRow(FA_Point p);
elev_type FA_Point_getElev(FA_Point p);
unsigned short FA_Point_getFAValue(FA_Point p);
FA_Point* FA_Point_getFD(FA_Point p);

//SETTERS ----------------------------------------------------------
//initialze the FA_Point at c,r with the passed values
void FA_setPoint(FA_Map* map, unsigned short c, unsigned short r, elev_type elev, int fd);
//increment the FA value of the passed FA_Point* by value
void FA_IncrementValue(FA_Point* p, unsigned short value);

//HELPERS ----------------------------------------------------------
//given an FA_Map, a point and a direction, return the FA_Point in that direction using the stadard set in the FD files
FA_Point* FA_PointInDir(FA_Map map, unsigned short c, unsigned short r, unsigned short dir);

//WORKHORSE ----------------------------------------------------------
//actually does the work for the FA algorithm - passed a FA grid, fills the FA grid
void FA_fill(FA_Map* map);

//SORTERS FOR QSORT----------------------------------------------------------
//sort by row column
int sort_by_rc(const void * a, const void * b);
//sort by elevation
int sort_by_elev(const void* a, const void* b);

#endif
