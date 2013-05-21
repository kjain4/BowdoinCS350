
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
    "Usage: fishgis-flowaccu [-n NTHREAD] [ELEV.asc] FLOW.asc ACCU.asc";

  DataSet *flow, *accu;
  int i, nthread;

  i = 1; argc--;
  nthread = 1;
  if (argc > 2 && strncmp(argv[i], "-n", 2) == 0) {
    // supplied an nthread option
    i++; argc--;
    errno = 0;

    // flowaccu -nNTRHEAD [ELEV.asc] FLOW.asc ACCU.asc
    if (strlen(argv[i]) > 2)
      nthread = strtol(argv[i-1] + 2, NULL, 10);
    // flowaccu -n NTRHEAD [ELEV.asc] FLOW.asc ACCU.asc
    else if (argc > 2) {
      i++; argc--;
      nthread = strtol(argv[i-1], NULL, 10);
    }else
      errno = -1;

    if (errno != 0) {
      printf("bad conversion\n");
      fprintf(stderr, "%s\n", USAGE);
      return -1;
    }
  }
  if (argc > 3) {
    printf("too many args\n");
    // too many args
    fprintf(stderr, "%s\n", USAGE);
    return -1;
  }else if (argc == 3) {
    // skip ELEV.asc argument
    i++;
    argc--;
  }

  flow = dLoad(argv[i++], UCHAR);
  if (flow) {
    accu = flow_accumulation_tree(flow, nthread);
    dFree(flow);

    if (accu) {
      dStore(accu, argv[i]);
      dFree(accu);
    }else
      return -1;
  }else
    return -1;

  return 0;
}
