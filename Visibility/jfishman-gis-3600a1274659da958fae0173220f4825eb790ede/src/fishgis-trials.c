
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
#include "datagrid.h"
#include "flow.h"

int main(int argc, const char **argv)
{
  const char *USAGE =
    "Usage: fishgis-trials INPUT NTRIAL NTHREAD [MAX_THREAD]\n"
    "\n"
    "  NTRIAL     - the number of trials to run; NTRIAL >= 1\n"
    "  NTHREAD    - the number of threads to use; NTHREAD >= 1\n"
    "  MAX_THREAD - when given, run sub-trials with threads from NTHREAD to\n"
    "               MAX_THREAD, inclusive; MAX_THREAD >= NTHREAD";

  DataSet *elev, *flow, *accu;
  int ntrial, nthread, max_thread;
  int itrial, ithread;

  if (argc < 4 || argc > 5) {
    fprintf(stderr, USAGE, argv[0]);
    return -1;
  }

  errno = 0;
  ntrial = strtol(argv[2], NULL, 10);
  if (errno != 0 || ntrial < 1) {
    fprintf(stderr, USAGE, argv[0]);
    return -1;
  }
  nthread = strtol(argv[3], NULL, 10);
  if (errno != 0 || nthread < 1) {
    fprintf(stderr, USAGE, argv[0]);
    return -1;
  }
  max_thread = nthread;
  if (argc == 5) {
    max_thread = strtol(argv[4], NULL, 10);
    if (errno != 0 || max_thread < nthread) {
      fprintf(stderr, USAGE, argv[0]);
      return -1;
    }
  }

  printf("Loading elevation grid...\n");
  elev = dLoad(argv[1], FLOAT);
  flow = NULL;

  for (itrial = 0; itrial < ntrial; itrial++) {
    for (ithread = nthread; ithread < max_thread + 1; ithread++) {
      if (flow)
        // free last flow result
        dFree(flow);
      flow = flow_direction(elev, ithread);
    }
  }
  // don't need the elevation anymore
  dFree(elev);

  for (itrial = 0; itrial < ntrial; itrial++) {
    for (ithread = nthread; ithread < max_thread + 1; ithread++) {
      accu = flow_accumulation_tree(flow, ithread);
      // can simply free this immediately
      dFree(accu);
    }
  }
  // done with the last flow, as well
  dFree(flow);

  return 0;
}

