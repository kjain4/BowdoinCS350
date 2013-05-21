
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


#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "datagrid.h"
#include "graphics.h"

const int WIDTH  = 600;
const int HEIGHT = 500;

GLColor color_function(int zval)
{
  GLColor col;
  return col;
}

int main(int argc, const char **argv)
{
  const char *USAGE = "Usage: fishgis-display2d GRID.asc";

  GlutWindow win;
  DataSet *dset;
  index_t **isets;
  GLColor *col_map;
  double *info, range;

  if (argc != 2) {
    fprintf(stderr, "%s\n", USAGE);
    return -1;
  }

  // Load data and calculate important values
  dset = dLoad(argv[1], FLOAT);
  if (!dset)
    return -1;
  printf("NODATA == %f\n", dset->grid.fNODATA);
  info = gStatf(&dset->grid, MINMAX);
  range = info[I_MAX] - info[I_MIN];
  printf("Range: [%f, %f]\n", info[I_MIN], info[I_MAX]);

  win = initialize_window("FishGis - Display 2D", 0, 0, WIDTH, HEIGHT);

  // initialize drawing parameters
  isets = build_xy_indexes(dset->grid.nrow, dset->grid.ncol, WIDTH, HEIGHT);
  col_map = build_col_map_func((int)range, color_function);
  

  //glutMainLoop();

  // garbage collect
  free_col_map(col_map);
  free_xy_indexes(isets);
  free(info);

  return 0;
}
