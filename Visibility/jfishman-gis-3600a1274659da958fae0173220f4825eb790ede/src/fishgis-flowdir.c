
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
#include "flow.h"

int main(int argc, const char **argv)
{
  const char *USAGE =
    "Usage: fishgis-flowdir [-n[ ]NTHREAD] ELEV.asc FLOW.asc";

  DataSet *elev, *flow;
  int i, nthread;

  i = 1;
  argc--;
  nthread = 1;
  if (argc > 2 && strncmp(argv[i], "-n", 2) == 0) {
    // supplied an nthread option
    i++; argc--;
    errno = 0;

    // flowdir -nNTRHEAD ELEV.asc FLOW.asc
    if (strlen(argv[i]) > 2)
      nthread = strtol(argv[i-1] + 2, NULL, 10);
    // flowdir -n NTHREAD ELEV.asc FLOW.asc
    else if (argc > 2) {
      i++; argc--;
      nthread = strtol(argv[i-1], NULL, 10);
    }else
      errno = -1;

    if (errno != 0) {
      fprintf(stderr, "%s\n", USAGE);
      return -1;
    }
  }
  if (argc != 2) {
    // incorrect arg count
    fprintf(stderr, "%s\n", USAGE);
    return -1;
  }

  
  elev = dLoad(argv[i++], FLOAT);
  if (elev) {
    flow = flow_direction(elev, nthread);
    dFree(elev);

    if (flow) {
      dStore(flow, argv[i]);
      dFree(flow);
    }else
      return -1;
  }else
    return -1;

  return 0;
}
