
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
#include <stdio.h>
#include <stdlib.h>

#include "datagrid.h"

int main(int argc, const char **argv)
{
  DataSet *data;
  double *stats;

  if (argc != 2) {
    fprintf(stderr, "Usage: %s  GRID.asc", argv[0]);
    return 1;
  }

  data = dLoad(argv[1], FLOAT);
  if (!data)
    return 1;

  stats = gStatf(&data->grid, STD);
  assert(stats);

  printf("Count:\t%i\n", (int)stats[I_N]);
  printf("Min:\t%f\n", stats[I_MIN]);
  printf("Max:\t%f\n", stats[I_MAX]);
  printf("Avg:\t%f\n", stats[I_AVG]);
  printf("Std:\t%f\n", stats[I_STD]);

  free(stats);
  dFree(data);

  return 0;
}
