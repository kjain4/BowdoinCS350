
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

#ifndef index_t
#  if __GLIBC_HAVE_LONG_LONG
typedef unsigned long long index_t;
#define DGI_FMT "%llu"
#  else
typedef unsigned int index_t;
#define DGI_FMT "%u"
#  endif
#endif

enum GridDataType {
    FLOAT,
    INT,
    UINT,
    SHRT,
    USHRT,
    CHAR,
    UCHAR,
};

// struct representing a grid of data values
typedef struct grid_t {
  index_t nrow;
  index_t ncol;
  float xllcorner;
  float yllcorner;
  float cellsize;
  enum GridDataType type;  
  size_t data_size;
  union {
      float fNODATA;
      int iNODATA;
      unsigned int uiNODATA;
      short sNODATA;
      unsigned short usNODATA;
      char cNODATA;
      unsigned char ucNODATA;
  };
  union {
      float *fData;
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



// get a (r,c) entry in a data set
//   performs correct type-math  at runtime, but does not check coersion
#define dGet(dset, r, c, val, cast) { \
  switch (dset->grid.type) { \
    case FLOAT: \
      val = (cast)*(dset->grid.fData + r*dset->grid.ncol + c); break; \
    case INT: \
      val = (cast)*(dset->grid.iData + r*dset->grid.ncol + c); break; \
    case UINT: \
      val = (cast)*(dset->grid.uiData + r*dset->grid.ncol + c); break; \
    case SHRT: \
      val = (cast)*(dset->grid.sData + r*dset->grid.ncol + c); break; \
    case USHRT: \
      val = (cast)*(dset->grid.usData + r*dset->grid.ncol + c); break; \
    case CHAR: \
      val = (cast)*(dset->grid.cData + r*dset->grid.ncol + c); break; \
    case UCHAR: \
      val = (cast)*(dset->grid.ucData + r*dset->grid.ncol + c); break; \
    default: \
      val = (cast)*((int*)NULL); /* intentionally cause failure */ \
  } \
}
// set a (r,c) entry in a data set
//   performs correct type-math  at runtime, but does not check coersion
#define dSet(dset, r, c, val) { \
  switch (dset->grid.type) { \
    case FLOAT: \
      *(dset->grid.fData + r*dset->grid.ncol + c) = val; break; \
    case INT: \
      *(dset->grid.iData + r*dset->grid.ncol + c) = val; break; \
    case UINT: \
      *(dset->grid.uiData + r*dset->grid.ncol + c) = val; break; \
    case SHRT: \
      *(dset->grid.sData + r*dset->grid.ncol + c) = val; break; \
    case USHRT: \
      *(dset->grid.usData + r*dset->grid.ncol + c) = val; break; \
    case CHAR: \
      *(dset->grid.cData + r*dset->grid.ncol + c) = val; break; \
    case UCHAR: \
      *(dset->grid.ucData + r*dset->grid.ncol + c) = val; break; \
    default: \
      *((int*)NULL) = val; /* intentionally cause failure */ \
  } \
}

#endif /* _datagrid_h_DEFINED */

