
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.


#ifndef _datagrid_h_DEFINED
#define _datagrid_h_DEFINED

#include <sys/types.h>
#include <stddef.h>
#include "gridpoint.h"

enum GridDataType {
    FLOAT,
    INT,
    UINT,
    SHRT,
    USHRT,
    CHAR,
    UCHAR,
};

typedef struct grid_header_t
{
  dim_t nrow;
  dim_t ncol;
  float xllcorner;
  float yllcorner;
  float cellsize;
  float NODATA_value;
} GridHeader;

// struct representing a grid of data values
typedef struct grid_t
{
  GridHeader hd;

  enum GridDataType type;  
  size_t data_size;
  union { float *fData;
          int *iData;
          unsigned int *uiData;
          short *sData;
          unsigned short *usData;
          char *cData;
          unsigned char *ucData;
  };
} Grid;

// struct representing a data set, loaded
// and stored from a given location on disk
typedef struct dataset_t {
  char* path;
  struct grid_t grid;
} DataSet;

// load data from file into the data set
DataSet* dLoad(const char *path, enum GridDataType type);

// initialize an empty data grid
DataSet* dInit(index_t nrow, index_t ncol, enum GridDataType type);

// store a data set to file
int dStore(DataSet *dset, const char* path);

// free the memory allocated for a data set
void dFree(DataSet *dset);


// calculate certain important information about a data grid
//   runs through the data array and calculates at the following levels
//     MINMAX = 0  -  only the minimum and maximum
//     AVG    = 1  -  include the average
//     STD    = 2  -  include the standard deviation
double *gStatf (Grid *data, int level);
double *gStati (Grid *data, int level);
double *gStatui(Grid *data, int level);
double *gStats (Grid *data, int level);
double *gStatus(Grid *data, int level);
double *gStatc (Grid *data, int level);
double *gStatuc(Grid *data, int level);
// processing level option
enum {
  MINMAX = 0,
  AVG,
  STD
};
// index of results in info arrays
enum {
  I_N = 0,
  I_MIN,
  I_MINI,
  I_MAX,
  I_MAXI,
  I_AVG,
  I_STD
};


/* get a (r,c) entry in a data grid */
#define dg_get(grid, p, val, cast) { \
  switch ((grid).type) { \
    case FLOAT: \
      val = (cast)*((grid).fData  + (p).r*(grid).hd.ncol + (p).c); break; \
    case INT: \
      val = (cast)*((grid).iData  + (p).r*(grid).hd.ncol + (p).c); break; \
    case UINT: \
      val = (cast)*((grid).uiData + (p).r*(grid).hd.ncol + (p).c); break; \
    case SHRT: \
      val = (cast)*((grid).sData  + (p).r*(grid).hd.ncol + (p).c); break; \
    case USHRT: \
      val = (cast)*((grid).usData + (p).r*(grid).hd.ncol + (p).c); break; \
    case CHAR: \
      val = (cast)*((grid).cData  + (p).r*(grid).hd.ncol + (p).c); break; \
    case UCHAR: \
      val = (cast)*((grid).ucData + (p).r*(grid).hd.ncol + (p).c); break; \
    default: \
      val = (cast)*((int*)NULL); /* intentionally cause failure */ \
  } \
}
/* set a (r,c) entry in a data grid */
#define dg_set(grid, p, val) { \
  switch ((grid).type) { \
    case FLOAT: \
      *((grid).fData  + (p).r*(grid).hd.ncol + (p).c) = val; break; \
    case INT: \
      *((grid).iData  + (p).r*(grid).hd.ncol + (p).c) = val; break; \
    case UINT: \
      *((grid).uiData + (p).r*(grid).hd.ncol + (p).c) = val; break; \
    case SHRT: \
      *((grid).sData  + (p).r*(grid).hd.ncol + (p).c) = val; break; \
    case USHRT: \
      *((grid).usData + (p).r*(grid).hd.ncol + (p).c) = val; break; \
    case CHAR: \
      *((grid).cData  + (p).r*(grid).hd.ncol + (p).c) = val; break; \
    case UCHAR: \
      *((grid).ucData + (p).r*(grid).hd.ncol + (p).c) = val; break; \
    default: \
      *((int*)NULL) = val; /* intentionally cause failure */ \
  } \
}

// get a (r,c) entry in a data set
//   performs correct type-math  at runtime, but does not check coersion
#define dGet(dset, r, c, val, cast) { \
  switch (dset->grid.type) { \
    case FLOAT: \
      val = (cast)*(dset->grid.fData + r*dset->grid.hd.ncol + c); break; \
    case INT: \
      val = (cast)*(dset->grid.iData + r*dset->grid.hd.ncol + c); break; \
    case UINT: \
      val = (cast)*(dset->grid.uiData + r*dset->grid.hd.ncol + c); break; \
    case SHRT: \
      val = (cast)*(dset->grid.sData + r*dset->grid.hd.ncol + c); break; \
    case USHRT: \
      val = (cast)*(dset->grid.usData + r*dset->grid.hd.ncol + c); break; \
    case CHAR: \
      val = (cast)*(dset->grid.cData + r*dset->grid.hd.ncol + c); break; \
    case UCHAR: \
      val = (cast)*(dset->grid.ucData + r*dset->grid.hd.ncol + c); break; \
    default: \
      val = (cast)*((int*)NULL); /* intentionally cause failure */ \
  } \
}
// set a (r,c) entry in a data set
//   performs correct type-math  at runtime, but does not check coersion
#define dSet(dset, r, c, val) { \
  switch (dset->grid.type) { \
    case FLOAT: \
      *(dset->grid.fData + r*dset->grid.hd.ncol + c) = val; break; \
    case INT: \
      *(dset->grid.iData + r*dset->grid.hd.ncol + c) = val; break; \
    case UINT: \
      *(dset->grid.uiData + r*dset->grid.hd.ncol + c) = val; break; \
    case SHRT: \
      *(dset->grid.sData + r*dset->grid.hd.ncol + c) = val; break; \
    case USHRT: \
      *(dset->grid.usData + r*dset->grid.hd.ncol + c) = val; break; \
    case CHAR: \
      *(dset->grid.cData + r*dset->grid.hd.ncol + c) = val; break; \
    case UCHAR: \
      *(dset->grid.ucData + r*dset->grid.hd.ncol + c) = val; break; \
    default: \
      *((int*)NULL) = val; /* intentionally cause failure */ \
  } \
}

#endif /* _datagrid_h_DEFINED */

