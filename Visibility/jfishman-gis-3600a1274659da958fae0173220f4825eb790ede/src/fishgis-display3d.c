
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

int main(int argc, const char **argv)
{
  const char *USAGE = "Usage: fishgis-display3d GRID.asc";

  GlutWindow win;
  DataSet *dset;

  if (argc != 2) {
    fprintf(stderr, "%s\n", USAGE);
    return -1;
  }

  dset = dLoad(argv[1], FLOAT);
  if (!dset)
    return -1;

  win = initialize_window("FishGis - Display 3D", 0, 0, 600, 500);
  glutMainLoop();

  return 0;
}
