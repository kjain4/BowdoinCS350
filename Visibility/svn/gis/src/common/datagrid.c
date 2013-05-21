
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

#include <assert.h>
#include <errno.h>
#include <float.h>
#include <limits.h>
#ifndef __APPLE__
#  include <malloc.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "rtimer.h"
#include "datagrid.h"

// load functions for each type
int gLoadf (FILE *fp, Grid *grid);
int gLoadi (FILE *fp, Grid *grid);
int gLoadui(FILE *fp, Grid *grid);
int gLoads (FILE *fp, Grid *grid);
int gLoadus(FILE *fp, Grid *grid);
int gLoadc (FILE *fp, Grid *grid);
int gLoaduc(FILE *fp, Grid *grid);

// load data from file into the data set
DataSet* dLoad(const char *path, enum GridDataType type)
{
  static Rtimer rt;
  DataSet* dset;
  Grid *grid;
  FILE *fp;
  int result;

  rt_start(rt);

  // allocate a new DataSet object
  dset = (DataSet*) malloc(sizeof(DataSet));
  if (dset == NULL) {
    perror("Unable to allocate DataSet object");
    return NULL;
  }
  dset->path = (char*) malloc(strlen(path) + 1);
  if (dset->path == NULL) {
    perror("Unable to allocate path string");
    free(dset);
    return NULL;
  }
  strcpy(dset->path, path);
  
  // read grid meta info
  grid = &dset->grid;
  grid->type = type;
  fp = fopen(path, "r");
  if (!fp) {
    fprintf(stderr, "Could not open input file (%s).\n", path);
    perror(NULL);
    free(dset->path);
    free(dset);
    return NULL;
  }

  result = fscanf(fp, "ncols\t%i\nnrows\t%i\n",
                  &grid->hd.ncol, &grid->hd.nrow);
  if (result != 2) {
    perror("Could not read grid size.");
    free(dset->path);
    free(dset);
    fclose(fp);
    return NULL;
  }
  result = fscanf(fp, "xllcorner\t%f\nyllcorner\t%f\ncellsize\t%f\n"
                  "NODATA_value\t%f\n", &grid->hd.xllcorner,
                  &grid->hd.yllcorner, &grid->hd.cellsize,
                  &grid->hd.NODATA_value);
  if (result != 4) {
    printf("%d\n", result);
    perror("Could not read grid meta data");
    free(dset->path);
    free(dset);
    fclose(fp);
    return NULL;
  }

  result = -1;
  switch (type) {
    case FLOAT: result = gLoadf (fp, grid); break;
    case INT:   result = gLoadi (fp, grid); break;
    case UINT:  result = gLoadui(fp, grid); break;
    case SHRT:  result = gLoads (fp, grid); break;
    case USHRT: result = gLoadus(fp, grid); break;
    case CHAR:  result = gLoadc (fp, grid); break;
    case UCHAR: result = gLoaduc(fp, grid); break;
    default: fprintf(stderr, "Invalid type given\n");
  }
  if (result != 0) {
    free(dset->path);
    free(dset);
    fclose(fp);
    return NULL;
  }

  // close input file
  fclose(fp);

  rt_stop(rt);
  static char buf[256];
  rt_sprint(buf, rt);
  printf("dLoad('%s',%d):\t%s\n", path, type, buf);

  return dset;
}

/**
 * Load a grid of floats from the given file pointer.
 */
int gLoadf(FILE *fp, Grid *grid)
{
  index_t div, mod;
  float *p, *end;
#ifndef __APPLE__
  int result;
#endif

  // allocate grid
  grid->data_size = sizeof(float);
#ifdef __APPLE__
  grid->fData = (float*) malloc(grid->hd.nrow * grid->hd.ncol * grid->data_size);
  if (!grid->fData) {
    fprintf(stderr, "Could not allocate data array: [Errno %d] %s\n",
            errno, strerror(errno));
#else
  result = sysconf(_SC_PAGESIZE);
  result = posix_memalign((void**)&grid->fData, result,
                     grid->hd.ncol * grid->hd.nrow * grid->data_size);
  if (result != 0) {
    fprintf(stderr, "Could not allocate data array: [Errno %d] %s\n",
            result, strerror(result));
#endif
    return -1;
  }

  // read grid raw data
  div = (grid->hd.nrow * grid->hd.ncol) / 16;
  mod = (grid->hd.nrow * grid->hd.ncol) % 16;
  p = grid->fData;
  end = grid->fData + grid->hd.nrow * grid->hd.ncol - mod;

  // loop-unroll by 4
  if (div > 0) {
    while (p < end) {
      p += 15;
      if (fscanf(fp, "%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f ",
                     p  , p--, p--, p--, p--, p--, p--, p--,
                     p--, p--, p--, p--, p--, p--, p--, p--) != 16) {
        perror("Error reading grid data");
        free(grid->fData);
          return -1;
      }
      p += 16;
    }
  }

  // finish off the array
  while (mod-- > 0) {
    if (fscanf(fp, "%f ", p++) != 1) {
      perror("Error reading grid data.");
      free(grid->fData);
      return -1;
    }
  }

  return 0;
}

/**
 * Load a grid of integers from the given file pointer.
 */
int gLoadi(FILE *fp, Grid *grid)
{
  index_t div, mod;
  int *p, *end;
#ifndef __APPLE__
  int result;
#endif

  // allocate grid
  grid->data_size = sizeof(int);
#ifdef __APPLE__
  grid->iData = (int*) malloc(grid->hd.nrow * grid->hd.ncol * grid->data_size);
  if (!grid->iData) {
    fprintf(stderr, "Could not allocate data array: [Errno %d] %s\n",
            errno, strerror(errno));
#else
  result = sysconf(_SC_PAGESIZE);
  result = posix_memalign((void**)&grid->iData, result,
                     grid->hd.ncol * grid->hd.nrow * grid->data_size);
  if (result != 0) {
    fprintf(stderr, "Could not allocate data array: [Errno %d] %s\n",
            result, strerror(result));
#endif
    return -1;
  }

  // read grid raw data
  div = (grid->hd.nrow * grid->hd.ncol) / 16;
  mod = (grid->hd.nrow * grid->hd.ncol) % 16;
  p = grid->iData;
  end = grid->iData + grid->hd.nrow * grid->hd.ncol - mod;

  // loop-unroll by 4
  if (div > 0) {
    while (p < end) {
      p += 15;
      if (fscanf(fp, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d ",
                     p  , p--, p--, p--, p--, p--, p--, p--,
                     p--, p--, p--, p--, p--, p--, p--, p--) != 16) {
        perror("Error reading grid data");
        free(grid->iData);
        return -1;
      }
      p += 16;
    }
  }

  // finish off the array
  while (mod-- > 0) {
    if (fscanf(fp, "%d ", p++) != 1) {
      perror("Error reading grid data.");
      free(grid->iData);
      return -1;
    }
  }

  return 0;
}

/**
 * Load a grid of unsigned integers from the given file pointer.
 */
int gLoadui(FILE *fp, Grid *grid)
{
  index_t div, mod;
  unsigned int *p, *end;
#ifndef __APPLE__
  int result;
#endif

  // allocate grid
  grid->data_size = sizeof(unsigned int);
#ifdef __APPLE__
  grid->uiData = (unsigned int*) malloc(grid->hd.nrow * grid->hd.ncol *
                                        grid->data_size);
  if (!grid->uiData) {
    fprintf(stderr, "Could not allocate data array: [Errno %d] %s\n",
            errno, strerror(errno));
#else
  result = sysconf(_SC_PAGESIZE);
  result = posix_memalign((void**)&grid->uiData, result,
                     grid->hd.ncol * grid->hd.nrow * grid->data_size);
  if (result != 0) {
    fprintf(stderr, "Could not allocate data array: [Errno %d] %s\n",
            result, strerror(result));
#endif
    return -1;
  }

  // read grid raw data
  div = (grid->hd.nrow * grid->hd.ncol) / 16;
  mod = (grid->hd.nrow * grid->hd.ncol) % 16;
  p = grid->uiData;
  end = grid->uiData + grid->hd.nrow * grid->hd.ncol - mod;

  // loop-unroll by 4
  if (div > 0) {
    while (p < end) {
      p += 15;
      if (fscanf(fp, "%u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u ",
                     p  , p--, p--, p--, p--, p--, p--, p--,
                     p--, p--, p--, p--, p--, p--, p--, p--) != 16) {
        perror("Error reading grid data");
        free(grid->uiData);
        return -1;
      }
      p += 16;
    }
  }

  // finish off the array
  while (mod-- > 0) {
    if (fscanf(fp, "%u ", p++) != 1) {
      perror("Error reading grid data.");
      free(grid->uiData);
      return -1;
    }
  }

  return 0;
}

/**
 * Load a grid of shorts from the given file pointer.
 */
int gLoads(FILE *fp, Grid *grid)
{
  index_t div, mod;
  short *p, *end;
#ifndef __APPLE__
  int result;
#endif

  // allocate grid
  grid->data_size = sizeof(short);
#ifdef __APPLE__
  grid->sData = (short*) malloc(grid->hd.nrow * grid->hd.ncol * grid->data_size);
  if (!grid->sData) {
    fprintf(stderr, "Could not allocate data array: [Errno %d] %s\n",
            errno, strerror(errno));
#else
  result = sysconf(_SC_PAGESIZE);
  result = posix_memalign((void**)&grid->sData, result,
                     grid->hd.ncol * grid->hd.nrow * grid->data_size);
  if (result != 0) {
    fprintf(stderr, "Could not allocate data array: [Errno %d] %s\n",
            result, strerror(result));
#endif
    return -1;
  }

  // read grid raw data
  div = (grid->hd.nrow * grid->hd.ncol) / 16;
  mod = (grid->hd.nrow * grid->hd.ncol) % 16;
  p = grid->sData;
  end = grid->sData + grid->hd.nrow * grid->hd.ncol - mod;

  // loop-unroll by 4
  if (div > 0) {
    while (p < end) {
      p += 15;
      if (fscanf(fp, "%hd %hd %hd %hd %hd %hd %hd %hd "
                     "%hd %hd %hd %hd %hd %hd %hd %hd ",
                     p  , p--, p--, p--, p--, p--, p--, p--,
                     p--, p--, p--, p--, p--, p--, p--, p--) != 16) {
        perror("Error reading grid data");
        free(grid->sData);
        return -1;
      }
      p += 16;
    }
  }

  // finish off the array
  while (mod-- > 0) {
    if (fscanf(fp, "%hd ", p++) != 1) {
      perror("Error reading grid data.");
      free(grid->sData);
      return -1;
    }
  }

  return 0;
}

/**
 * Load a grid of unsigned shorts from the given file pointer.
 */
int gLoadus(FILE *fp, Grid *grid)
{
  index_t div, mod;
  unsigned short *p, *end;
#ifndef __APPLE__
  int result;
#endif

  // allocate grid
  grid->data_size = sizeof(unsigned short);
#ifdef __APPLE__
  grid->usData = (unsigned short*) malloc(grid->hd.nrow * grid->hd.ncol *
                                          grid->data_size);
  if (!grid->usData) {
    fprintf(stderr, "Could not allocate data array: [Errno %d] %s\n",
            errno, strerror(errno));
#else
  result = sysconf(_SC_PAGESIZE);
  result = posix_memalign((void**)&grid->usData, result,
                     grid->hd.ncol * grid->hd.nrow * grid->data_size);
  if (result != 0) {
    fprintf(stderr, "Could not allocate data array: [Errno %d] %s\n",
            result, strerror(result));
#endif
    return -1;
  }

  // read grid raw data
  div = (grid->hd.nrow * grid->hd.ncol) / 16;
  mod = (grid->hd.nrow * grid->hd.ncol) % 16;
  p = grid->usData;
  end = grid->usData + grid->hd.nrow * grid->hd.ncol - mod;

  // loop-unroll by 4
  if (div > 0) {
    while (p < end) {
      p += 15;
      if (fscanf(fp, "%hu %hu %hu %hu %hu %hu %hu %hu "
                     "%hu %hu %hu %hu %hu %hu %hu %hu ",
                     p  , p--, p--, p--, p--, p--, p--, p--,
                     p--, p--, p--, p--, p--, p--, p--, p--) != 16) {
        perror("Error reading grid data");
        free(grid->usData);
        return -1;
      }
      p += 16;
    }
  }

  // finish off the array
  while (mod-- > 0) {
    if (fscanf(fp, "%hu ", p++) != 1) {
      perror("Error reading grid data.");
      free(grid->usData);
      return -1;
    }
  }

  return 0;
}

/**
 * Load a grid of chars from the given file pointer.
 */
int gLoadc(FILE *fp, Grid *grid)
{
  index_t div, mod;
  char *p, *end;
#ifndef __APPLE__
  int result;
#endif

  // allocate grid
  grid->data_size = sizeof(char);
#ifdef __APPLE__
  grid->cData = (char*) malloc(grid->hd.nrow * grid->hd.ncol *
                                          grid->data_size);
  if (!grid->cData) {
    fprintf(stderr, "Could not allocate data array: [Errno %d] %s\n",
            errno, strerror(errno));
#else
  result = sysconf(_SC_PAGESIZE);
  result = posix_memalign((void**)&grid->cData, result,
                     grid->hd.ncol * grid->hd.nrow * grid->data_size);
  if (result != 0) {
    fprintf(stderr, "Could not allocate data array: [Errno %d] %s\n",
            result, strerror(result));
#endif
    return -1;
  }

  // read grid raw data
  div = (grid->hd.nrow * grid->hd.ncol) / 16;
  mod = (grid->hd.nrow * grid->hd.ncol) % 16;
  p = grid->cData;
  end = grid->cData + grid->hd.nrow * grid->hd.ncol - mod;

  // loop-unroll by 4
  if (div > 0) {
    while (p < end) {
      p += 15;
      if (fscanf(fp, "%hhd %hhd %hhd %hhd %hhd %hhd %hhd %hhd "
                     "%hhd %hhd %hhd %hhd %hhd %hhd %hhd %hhd ",
                     p  , p--, p--, p--, p--, p--, p--, p--,
                     p--, p--, p--, p--, p--, p--, p--, p--) != 16) {
        perror("Error reading grid data");
        free(grid->cData);
        return -1;
      }
      p += 16;
    }
  }

  // finish off the array
  while (mod-- > 0) {
    if (fscanf(fp, "%hhd ", p++) != 1) {
      perror("Error reading grid data.");
      free(grid->cData);
      return -1;
    }
  }

  return 0;
}

/**
 * Load a grid of unsigned chars from the given file pointer.
 */
int gLoaduc(FILE *fp, Grid *grid)
{
  index_t div, mod;
  unsigned char *p, *end;
#ifndef __APPLE__
  int result;
#endif

  // allocate grid
  grid->data_size = sizeof(unsigned char);
#ifdef __APPLE__
  grid->ucData = (unsigned char*) malloc(grid->hd.nrow * grid->hd.ncol *
                                          grid->data_size);
  if (!grid->ucData) {
    fprintf(stderr, "Could not allocate data array: [Errno %d] %s\n",
            errno, strerror(errno));
#else
  result = sysconf(_SC_PAGESIZE);
  result = posix_memalign((void**)&grid->ucData, result,
                     grid->hd.ncol * grid->hd.nrow * grid->data_size);
  if (result != 0) {
    fprintf(stderr, "Could not allocate data array: [Errno %d] %s\n",
            result, strerror(result));
#endif
    return -1;
  }

  // read grid raw data
  div = (grid->hd.nrow * grid->hd.ncol) / 16;
  mod = (grid->hd.nrow * grid->hd.ncol) % 16;
  p = grid->ucData;
  end = grid->ucData + grid->hd.nrow * grid->hd.ncol - mod;

  // loop-unroll by 4
  if (div > 0) {
    while (p < end) {
      p += 15;
      if (fscanf(fp, "%hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu "
                     "%hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu ",
                     p  , p--, p--, p--, p--, p--, p--, p--,
                     p--, p--, p--, p--, p--, p--, p--, p--) != 16) {
        perror("Error reading grid data");
        free(grid->ucData);
        return -1;
      }
      p += 16;
    }
  }

  // finish off the array
  while (mod-- > 0) {
    if (fscanf(fp, "%hhu ", p++) != 1) {
      perror("Error reading grid data.");
      free(grid->ucData);
      return -1;
    }
  }

  return 0;
}


DataSet* dInit(index_t nrow, index_t ncol, enum GridDataType type)
{
  DataSet* dset;
  Grid *grid;
  int align, result;

  // allocate a new DataSet object
  dset = (DataSet*) malloc(sizeof(DataSet));
  if (dset == NULL) {
    perror("Unable to allocate DataSet object");
    return NULL;
  }
  dset->path = (char*) malloc(1);
  if (dset->path == NULL) {
    perror("Unable to allocate path string");
    free(dset);
    return NULL;
  }
  dset->path[0] = '\0';

  grid = &dset->grid;
  grid->hd.nrow = nrow;
  grid->hd.ncol = ncol;
  
  // allocate (empty) data array
  result = -1;
  align = sysconf(_SC_PAGESIZE);
  grid->type = type;
  switch (type) {
    case FLOAT:
      grid->data_size = sizeof(float);
#ifdef __APPLE__
      grid->fData = (float*) malloc(nrow * ncol * grid->data_size);
      if (grid->fData)
        result = 0;
#else
      result = posix_memalign((void**)&grid->fData, align,
                              ncol * nrow * grid->data_size);
#endif
      break;
    case INT:
      grid->data_size = sizeof(int);
#ifdef __APPLE__
      grid->iData = (int*) malloc(nrow * ncol * grid->data_size);
      if (grid->iData)
        result = 0;
#else
      result = posix_memalign((void**)&grid->iData, align,
                              ncol * nrow * grid->data_size);
#endif
      break;
    case UINT:
      grid->data_size = sizeof(unsigned int);
#ifdef __APPLE__
      grid->uiData = (unsigned int*) malloc(nrow * ncol * grid->data_size);
      if (grid->uiData)
        result = 0;
#else
      result = posix_memalign((void**)&grid->uiData, align,
                              ncol * nrow * grid->data_size);
#endif
      break;
    case SHRT:
      grid->data_size = sizeof(short);
#ifdef __APPLE__
      grid->sData = (short*) malloc(nrow * ncol * grid->data_size);
      if (grid->sData)
        result = 0;
#else
      result = posix_memalign((void**)&grid->sData, align,
                              ncol * nrow * grid->data_size);
#endif
      break;
    case USHRT:
      grid->data_size = sizeof(unsigned short);
#ifdef __APPLE__
      grid->usData = (unsigned short*) malloc(nrow * ncol * grid->data_size);
      if (grid->usData)
        result = 0;
#else
      result = posix_memalign((void**)&grid->usData, align,
                              ncol * nrow * grid->data_size);
#endif
      break;
    case CHAR:
      grid->data_size = sizeof(char);
#ifdef __APPLE__
      grid->cData = (char*) malloc(nrow * ncol * grid->data_size);
      if (grid->cData)
        result = 0;
#else
      result = posix_memalign((void**)&grid->cData, align,
                              ncol * nrow * grid->data_size);
#endif
      break;
    case UCHAR:
      grid->data_size = sizeof(unsigned char);
#ifdef __APPLE__
      grid->ucData = (unsigned char*) malloc(nrow * ncol * grid->data_size);
      if (grid->ucData)
        result = 0;
#else
      result = posix_memalign((void**)&grid->ucData, align,
                              ncol * nrow * grid->data_size);
#endif
      break;
  }
  if (result != 0) {
    perror("Could not allocate data");
    free(dset->path);
    free(dset);
    return NULL;
  }

  return dset;
}

// storage functions for each type
int gStoref (FILE *fp, Grid *grid);
int gStorei (FILE *fp, Grid *grid);
int gStoreui(FILE *fp, Grid *grid);
int gStores (FILE *fp, Grid *grid);
int gStoreus(FILE *fp, Grid *grid);
int gStorec (FILE *fp, Grid *grid);
int gStoreuc(FILE *fp, Grid *grid);

/**
 * Store a DataSet containing a grid of any valid type in a file at the given
 * location.
 *
 * Stores data in Arc/Info ASCII Grid format.
 */
int dStore(DataSet *dset, const char *path)
{
  static Rtimer rt;
  FILE *fp;
  Grid *grid;
  char *tmp;
  int result;

  rt_start(rt);

  assert(dset);
  assert(dset->path);

  grid = &dset->grid;
  fp = fopen(path, "w");
  if (!fp) {
    fprintf(stderr, "Could not open input file (%s).\n", path);
    perror(NULL);
    return -1;
  }

  // set path
  if (strlen(dset->path) != strlen(path)) {
    tmp = realloc(dset->path, strlen(path));
    if (!tmp) {
      perror("Couldn't reallocate path string");
      fclose(fp);
      return -1;
    }
    dset->path = tmp;
  }
  strcpy(dset->path, path);

  // store meta data
  fprintf(fp, "ncols         %i\n", grid->hd.ncol);
  fprintf(fp, "nrows         %i\n", grid->hd.nrow);
  fprintf(fp, "xllcorner     %f\n", grid->hd.xllcorner);
  fprintf(fp, "yllcorner     %f\n", grid->hd.yllcorner);
  fprintf(fp, "cellsize      %f\n", grid->hd.cellsize);

  result = -1;
  switch (grid->type) {
    case FLOAT: result = gStoref (fp, grid); break;
    case INT:   result = gStorei (fp, grid); break;
    case UINT:  result = gStoreui(fp, grid); break;
    case SHRT:  result = gStores (fp, grid); break;
    case USHRT: result = gStoreus(fp, grid); break;
    case CHAR:  result = gStorec (fp, grid); break;
    case UCHAR: result = gStoreuc(fp, grid); break;
    default: fprintf(stderr,
                     "Invalid type set in DataSet argument to dStore()?\n");
  }
  if (result != 0) {
    fclose(fp);
    return -1;
  }

  //close output file
  fclose(fp);

  rt_stop(rt);
  static char buf[256];
  rt_sprint(buf, rt);
  printf("dStore(grid,'%s'):\t%s\n", path, buf);

  return 0;
}


int gStoref(FILE *fp, Grid *grid)
{
  index_t div, mod, i, c;
  float *p, *end;

  // read grid raw data
  div = grid->hd.ncol / 16;
  mod = grid->hd.ncol % 16;
  c = 0;
  p = grid->fData;
  end = grid->fData + grid->hd.nrow * grid->hd.ncol - mod;

  while (p < end) {
    // loop-unroll by 4
    if (div > 0) {
      c = 0;
      i = grid->hd.ncol - mod;
      while (c < i) {
        p += 15;
        if (fprintf(fp, "%f %f %f %f %f %f %f %f "
                        "%f %f %f %f %f %f %f %f ",
                        *p  , *p--, *p--, *p--, *p--, *p--, *p--, *p--,
                        *p--, *p--, *p--, *p--, *p--, *p--, *p--, *p--) <= 0) {
          perror("Error writing grid data");
          return -1;
        }
        c += 16;
        p += 16;
      }
    }
    // finish off the row
    i = mod;
    while (i-- > 0) {
      if (fprintf(fp, "%f ", *p++) <= 0) {
        perror("Error writing grid data.");
        return -1;
      }
    }
    // print endline character
    if (fprintf(fp, "\n") <= 0) {
      perror("Error writing grid data.");
      return -1;
    }
  }

  return 0;
}

int gStorei(FILE *fp, Grid *grid)
{
  index_t div, mod, i, c;
  int *p, *end;

  // read grid raw data
  div = grid->hd.ncol / 16;
  mod = grid->hd.ncol % 16;
  c = 0;
  p = grid->iData;
  end = grid->iData + grid->hd.nrow * grid->hd.ncol - mod;

  while (p < end) {
    // loop-unroll by 4
    if (div > 0) {
      c = 0;
      i = grid->hd.ncol - mod;
      while (c < i) {
        p += 15;
        if (fprintf(fp, "%d %d %d %d %d %d %d %d "
                        "%d %d %d %d %d %d %d %d ",
                        *p  , *p--, *p--, *p--, *p--, *p--, *p--, *p--,
                        *p--, *p--, *p--, *p--, *p--, *p--, *p--, *p--) <= 0) {
          perror("Error writing grid data");
          return -1;
        }
        c += 16;
        p += 16;
      }
    }
    // finish off the row
    i = mod;
    while (i-- > 0) {
      if (fprintf(fp, "%d ", *p++) <= 0) {
        perror("Error writing grid data.");
        return -1;
      }
    }
    // print endline character
    if (fprintf(fp, "\n") <= 0) {
      perror("Error writing grid data.");
      return -1;
    }
  }

  return 0;
}

int gStoreui(FILE *fp, Grid *grid)
{
  index_t div, mod, i, c;
  unsigned int *p, *end;

  // read grid raw data
  div = grid->hd.ncol / 16;
  mod = grid->hd.ncol % 16;
  c = 0;
  p = grid->uiData;
  end = grid->uiData + grid->hd.nrow * grid->hd.ncol - mod;

  while (p < end) {
    // loop-unroll by 4
    if (div > 0) {
      c = 0;
      i = grid->hd.ncol - mod;
      while (c < i) {
        p += 15;
        if (fprintf(fp, "%u %u %u %u %u %u %u %u "
                        "%u %u %u %u %u %u %u %u ",
                        *p  , *p--, *p--, *p--, *p--, *p--, *p--, *p--,
                        *p--, *p--, *p--, *p--, *p--, *p--, *p--, *p--) <= 0) {
          perror("Error writing grid data");
          return -1;
        }
        c += 16;
        p += 16;
      }
    }
    // finish off the row
    i = mod;
    while (i-- > 0) {
      if (fprintf(fp, "%u ", *p++) <= 0) {
        perror("Error writing grid data.");
        return -1;
      }
    }
    // print endline character
    if (fprintf(fp, "\n") <= 0) {
      perror("Error writing grid data.");
      return -1;
    }
  }

  return 0;
}

int gStores(FILE *fp, Grid *grid)
{
  index_t div, mod, i, c;
  short *p, *end;

  // read grid raw data
  div = grid->hd.ncol / 16;
  mod = grid->hd.ncol % 16;
  c = 0;
  p = grid->sData;
  end = grid->sData + grid->hd.nrow * grid->hd.ncol - mod;

  while (p < end) {
    // loop-unroll by 4
    if (div > 0) {
      c = 0;
      i = grid->hd.ncol - mod;
      while (c < i) {
        p += 15;
        if (fprintf(fp, "%hd %hd %hd %hd %hd %hd %hd %hd "
                        "%hd %hd %hd %hd %hd %hd %hd %hd ",
                        *p  , *p--, *p--, *p--, *p--, *p--, *p--, *p--,
                        *p--, *p--, *p--, *p--, *p--, *p--, *p--, *p--) <= 0) {
          perror("Error writing grid data");
          return -1;
        }
        c += 16;
        p += 16;
      }
    }
    // finish off the row
    i = mod;
    while (i-- > 0) {
      if (fprintf(fp, "%hd ", *p++) <= 0) {
        perror("Error writing grid data.");
        return -1;
      }
    }
    // print endline character
    if (fprintf(fp, "\n") <= 0) {
      perror("Error writing grid data.");
      return -1;
    }
  }

  return 0;
}

int gStoreus(FILE *fp, Grid *grid)
{
  index_t div, mod, i, c;
  unsigned short *p, *end;

  // read grid raw data
  div = grid->hd.ncol / 16;
  mod = grid->hd.ncol % 16;
  c = 0;
  p = grid->usData;
  end = grid->usData + grid->hd.nrow * grid->hd.ncol - mod;

  while (p < end) {
    // loop-unroll by 4
    if (div > 0) {
      c = 0;
      i = grid->hd.ncol - mod;
      while (c < i) {
        p += 15;
        if (fprintf(fp, "%hu %hu %hu %hu %hu %hu %hu %hu "
                        "%hu %hu %hu %hu %hu %hu %hu %hu ",
                        *p  , *p--, *p--, *p--, *p--, *p--, *p--, *p--,
                        *p--, *p--, *p--, *p--, *p--, *p--, *p--, *p--) <= 0) {
          perror("Error writing grid data");
          return -1;
        }
        c += 16;
        p += 16;
      }
    }
    // finish off the row
    i = mod;
    while (i-- > 0) {
      if (fprintf(fp, "%hu ", *p++) <= 0) {
        perror("Error writing grid data.");
        return -1;
      }
    }
    // print endline character
    if (fprintf(fp, "\n") <= 0) {
      perror("Error writing grid data.");
      return -1;
    }
  }

  return 0;
}

int gStorec(FILE *fp, Grid *grid)
{
  index_t div, mod, i, c;
  char *p, *end;

  // read grid raw data
  div = grid->hd.ncol / 16;
  mod = grid->hd.ncol % 16;
  c = 0;
  p = grid->cData;
  end = grid->cData + grid->hd.nrow * grid->hd.ncol - mod;

  while (p < end) {
    // loop-unroll by 4
    if (div > 0) {
      c = 0;
      i = grid->hd.ncol - mod;
      while (c < i) {
        p += 15;
        if (fprintf(fp, "%hhd %hhd %hhd %hhd %hhd %hhd %hhd %hhd "
                        "%hhd %hhd %hhd %hhd %hhd %hhd %hhd %hhd ",
                        *p  , *p--, *p--, *p--, *p--, *p--, *p--, *p--,
                        *p--, *p--, *p--, *p--, *p--, *p--, *p--, *p--) <= 0) {
          perror("Error writing grid data");
          return -1;
        }
        c += 16;
        p += 16;
      }
    }
    // finish off the row
    i = mod;
    while (i-- > 0) {
      if (fprintf(fp, "%hhd ", *p++) <= 0) {
        perror("Error writing grid data.");
        return -1;
      }
    }
    // print endline character
    if (fprintf(fp, "\n") <= 0) {
      perror("Error writing grid data.");
      return -1;
    }
  }

  return 0;
}

int gStoreuc(FILE *fp, Grid *grid)
{
  index_t div, mod, i, c;
  unsigned char *p, *end;

  // read grid raw data
  div = grid->hd.ncol / 16;
  mod = grid->hd.ncol % 16;
  c = 0;
  p = grid->ucData;
  end = grid->ucData + grid->hd.nrow * grid->hd.ncol - mod;

  while (p < end) {
    // loop-unroll by 4
    if (div > 0) {
      c = 0;
      i = grid->hd.ncol - mod;
      while (c < i) {
        p += 15;
        if (fprintf(fp, "%hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu "
                        "%hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu ",
                        *p  , *p--, *p--, *p--, *p--, *p--, *p--, *p--,
                        *p--, *p--, *p--, *p--, *p--, *p--, *p--, *p--) <= 0) {
          perror("Error writing grid data");
          return -1;
        }
        c += 16;
        p += 16;
      }
    }
    // finish off the row
    i = mod;
    while (i-- > 0) {
      if (fprintf(fp, "%hhu ", *p++) <= 0) {
        perror("Error writing grid data.");
        return -1;
      }
    }
    // print endline character
    if (fprintf(fp, "\n") <= 0) {
      perror("Error writing grid data.");
      return -1;
    }
  }

  return 0;
}


void dFree(DataSet *dset)
{
  assert(dset);
  assert(dset->path);

  switch (dset->grid.type) {
    case FLOAT: free(dset->grid.fData);  break;
    case INT:   free(dset->grid.iData);  break;
    case UINT:  free(dset->grid.uiData); break;
    case SHRT:  free(dset->grid.sData);  break;
    case USHRT: free(dset->grid.usData); break;
    case CHAR:  free(dset->grid.cData);  break;
    case UCHAR: free(dset->grid.ucData); break;
  }

  free(dset->path);
  free(dset);
}


// Calculate Grid statistics - min, max, average, standard deviation

const int INITIAL_LEN = I_AVG;

double *gStatf(Grid *grid, int level)
{
  double *info, dev;
  float *p, *pend;

  assert(grid->type == FLOAT);
  assert(grid->hd.nrow * grid->hd.ncol >= 1);

  info = (double*) malloc((INITIAL_LEN + level) * sizeof(double));
  assert(info);
  switch (level) {
    case STD:
      info[I_STD] = 0;
    case AVG:
      info[I_AVG] = 0;
    default:
      info[I_N] = 0;
      info[I_MIN] = FLT_MAX;
      info[I_MAX] = -FLT_MAX;
      info[I_MINI] = info[I_MAXI] = 0;
  }

  p = grid->fData;
  pend = p + grid->hd.nrow * grid->hd.ncol;
  for (; p < pend; p++) {
    if (*p == grid->hd.NODATA_value)
      continue;

    // increment the count of actual data points
    info[I_N]++;

    if (level >= AVG) {
      // perform a mean update
      dev = *p - info[I_AVG];
      info[I_AVG] += dev / info[I_N];
    }
    if (level == STD)
      // perform a square-difference update
      info[I_STD] += dev * (*p - info[I_AVG]);

    if (*p < info[I_MIN]) {
      info[I_MIN] = *p;
      info[I_MINI] = p - grid->fData;
    }
    if (*p > info[I_MAX]) {
      info[I_MAX] = *p;
      info[I_MAXI] = p - grid->fData;
    }
  }

  if (level == STD && info[I_N] > 1)
    info[I_STD] /= info[I_N] - 1;

  return info;
}

double *gStati(Grid *grid, int level)
{
  double *info, dev;
  int *p, *pend;

  assert(grid->type == INT);
  assert(grid->hd.nrow * grid->hd.ncol >= 1);

  info = (double*) malloc((INITIAL_LEN + level) * sizeof(double));
  assert(info);
  switch (level) {
    case STD:
      info[I_STD] = 0;
    case AVG:
      info[I_AVG] = 0;
    default:
      info[I_N] = 0;
      info[I_MIN] = FLT_MAX;
      info[I_MAX] = -FLT_MAX;
      info[I_MINI] = info[I_MAXI] = 0;
  }

  p = grid->iData;
  pend = p + grid->hd.nrow * grid->hd.ncol;
  for (; p < pend; p++) {
    if (*p == (int)grid->hd.NODATA_value)
      continue;

    // increment the count of actual data points
    info[I_N]++;

    if (level >= AVG) {
      // perform a mean update
      dev = *p - info[I_AVG];
      info[I_AVG] += dev / info[I_N];
    }
    if (level == STD)
      // perform a square-difference update
      info[I_STD] += dev * (*p - info[I_AVG]);

    if (*p < info[I_MIN]) {
      info[I_MIN] = *p;
      info[I_MINI] = p - grid->iData;
    }
    if (*p > info[I_MAX]) {
      info[I_MAX] = *p;
      info[I_MAXI] = p - grid->iData;
    }
  }

  if (level == STD && info[I_N] > 1)
    info[I_STD] /= info[I_N] - 1;

  return info;
}

double *gStatui(Grid *grid, int level)
{
  double *info, dev;
  unsigned int *p, *pend;

  assert(grid->type == UINT);
  assert(grid->hd.nrow * grid->hd.ncol >= 1);

  info = (double*) malloc((INITIAL_LEN + level) * sizeof(double));
  assert(info);
  switch (level) {
    case STD:
      info[I_STD] = 0;
    case AVG:
      info[I_AVG] = 0;
    default:
      info[I_N] = 0;
      info[I_MIN] = FLT_MAX;
      info[I_MAX] = -FLT_MAX;
      info[I_MINI] = info[I_MAXI] = 0;
  }

  p = grid->uiData;
  pend = p + grid->hd.nrow * grid->hd.ncol;
  for (; p < pend; p++) {
    if (*p == (unsigned int)grid->hd.NODATA_value)
      continue;

    // increment the count of actual data points
    info[I_N]++;

    if (level >= AVG) {
      // perform a mean update
      dev = *p - info[I_AVG];
      info[I_AVG] += dev / info[I_N];
    }
    if (level == STD)
      // perform a square-difference update
      info[I_STD] += dev * (*p - info[I_AVG]);

    if (*p < info[I_MIN]) {
      info[I_MIN] = *p;
      info[I_MINI] = p - grid->uiData;
    }
    if (*p > info[I_MAX]) {
      info[I_MAX] = *p;
      info[I_MAXI] = p - grid->uiData;
    }
  }

  if (level == STD && info[I_N] > 1)
    info[I_STD] /= info[I_N] - 1;

  return info;
}

double *gStats(Grid *grid, int level)
{
  double *info, dev;
  short *p, *pend;

  assert(grid->type == SHRT);
  assert(grid->hd.nrow * grid->hd.ncol >= 1);

  info = (double*) malloc((INITIAL_LEN + level) * sizeof(double));
  assert(info);
  switch (level) {
    case STD:
      info[I_STD] = 0;
    case AVG:
      info[I_AVG] = 0;
    default:
      info[I_N] = 0;
      info[I_MIN] = FLT_MAX;
      info[I_MAX] = -FLT_MAX;
      info[I_MINI] = info[I_MAXI] = 0;
  }

  p = grid->sData;
  pend = p + grid->hd.nrow * grid->hd.ncol;
  for (; p < pend; p++) {
    if (*p == (short)grid->hd.NODATA_value)
      continue;

    // increment the count of actual data points
    info[I_N]++;

    if (level >= AVG) {
      // perform a mean update
      dev = *p - info[I_AVG];
      info[I_AVG] += dev / info[I_N];
    }
    if (level == STD)
      // perform a square-difference update
      info[I_STD] += dev * (*p - info[I_AVG]);

    if (*p < info[I_MIN]) {
      info[I_MIN] = *p;
      info[I_MINI] = p - grid->sData;
    }
    if (*p > info[I_MAX]) {
      info[I_MAX] = *p;
      info[I_MAXI] = p - grid->sData;
    }
  }

  if (level == STD && info[I_N] > 1)
    info[I_STD] /= info[I_N] - 1;

  return info;
}

double *gStatus(Grid *grid, int level)
{
  double *info, dev;
  unsigned short *p, *pend;

  assert(grid->type == USHRT);
  assert(grid->hd.nrow * grid->hd.ncol >= 1);

  info = (double*) malloc((INITIAL_LEN + level) * sizeof(double));
  assert(info);
  switch (level) {
    case STD:
      info[I_STD] = 0;
    case AVG:
      info[I_AVG] = 0;
    default:
      info[I_N] = 0;
      info[I_MIN] = FLT_MAX;
      info[I_MAX] = -FLT_MAX;
      info[I_MINI] = info[I_MAXI] = 0;
  }

  p = grid->usData;
  pend = p + grid->hd.nrow * grid->hd.ncol;
  for (; p < pend; p++) {
    if (*p == (unsigned short)grid->hd.NODATA_value)
      continue;

    // increment the count of actual data points
    info[I_N]++;

    if (level >= AVG) {
      // perform a mean update
      dev = *p - info[I_AVG];
      info[I_AVG] += dev / info[I_N];
    }
    if (level == STD)
      // perform a square-difference update
      info[I_STD] += dev * (*p - info[I_AVG]);

    if (*p < info[I_MIN]) {
      info[I_MIN] = *p;
      info[I_MINI] = p - grid->usData;
    }
    if (*p > info[I_MAX]) {
      info[I_MAX] = *p;
      info[I_MAXI] = p - grid->usData;
    }
  }

  if (level == STD && info[I_N] > 1)
    info[I_STD] /= info[I_N] - 1;

  return info;
}

double *gStatc(Grid *grid, int level)
{
  double *info, dev;
  char *p, *pend;

  assert(grid->type == CHAR);
  assert(grid->hd.nrow * grid->hd.ncol >= 1);

  info = (double*) malloc((INITIAL_LEN + level) * sizeof(double));
  assert(info);
  switch (level) {
    case STD:
      info[I_STD] = 0;
    case AVG:
      info[I_AVG] = 0;
    default:
      info[I_N] = 0;
      info[I_MIN] = FLT_MAX;
      info[I_MAX] = -FLT_MAX;
      info[I_MINI] = info[I_MAXI] = 0;
  }

  p = grid->cData;
  pend = p + grid->hd.nrow * grid->hd.ncol;
  for (; p < pend; p++) {
    if (*p == (char)grid->hd.NODATA_value)
      continue;

    // increment the count of actual data points
    info[I_N]++;

    if (level >= AVG) {
      // perform a mean update
      dev = *p - info[I_AVG];
      info[I_AVG] += dev / info[I_N];
    }
    if (level == STD)
      // perform a square-difference update
      info[I_STD] += dev * (*p - info[I_AVG]);

    if (*p < info[I_MIN]) {
      info[I_MIN] = *p;
      info[I_MINI] = p - grid->cData;
    }
    if (*p > info[I_MAX]) {
      info[I_MAX] = *p;
      info[I_MAXI] = p - grid->cData;
    }
  }

  if (level == STD && info[I_N] > 1)
    info[I_STD] /= info[I_N] - 1;

  return info;
}

double *gStatuc(Grid *grid, int level)
{
  double *info, dev;
  unsigned char *p, *pend;

  assert(grid->type == UCHAR);
  assert(grid->hd.nrow * grid->hd.ncol >= 1);

  info = (double*) malloc((INITIAL_LEN + level) * sizeof(double));
  assert(info);
  switch (level) {
    case STD:
      info[I_STD] = 0;
    case AVG:
      info[I_AVG] = 0;
    default:
      info[I_N] = 0;
      info[I_MIN] = FLT_MAX;
      info[I_MAX] = -FLT_MAX;
      info[I_MINI] = info[I_MAXI] = 0;
  }

  p = grid->ucData;
  pend = p + grid->hd.nrow * grid->hd.ncol;
  for (; p < pend; p++) {
    if (*p == (unsigned char)grid->hd.NODATA_value)
      continue;

    // increment the count of actual data points
    info[I_N]++;

    if (level >= AVG) {
      // perform a mean update
      dev = *p - info[I_AVG];
      info[I_AVG] += dev / info[I_N];
    }
    if (level == STD)
      // perform a square-difference update
      info[I_STD] += dev * (*p - info[I_AVG]);

    if (*p < info[I_MIN]) {
      info[I_MIN] = *p;
      info[I_MINI] = p - grid->ucData;
    }
    if (*p > info[I_MAX]) {
      info[I_MAX] = *p;
      info[I_MAXI] = p - grid->ucData;
    }
  }

  if (level == STD && info[I_N] > 1)
    info[I_STD] /= info[I_N] - 1;

  return info;
}

