
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
#include <getopt.h>
#include <stdio.h>
#include "rtimer.h"
#include "vis.h"

int main(int argc, char * const argv[])
{
  const char *USAGE =
    "Usage: bvshed [options] ELEV.asc VMAP.asc\n"
    "\n"
    "Options:\n"
    "  -t, --threads    number of threads to use, if possible\n"
    "  -p, --points     number of points to process.\n"
    "                      0 - implies all points.  same as no option.\n"
    "                      1 - point requires '-r' and '-c' indexes.\n"
    "                          same as no option argument.\n"
    "  -r, --row        for single point viewshed, the row index.\n"
    "  -c, --col        for single point viewshed, the column index.\n"
    "\n"
    "   All option arguments must be >=0, except nthread must be >0\n";

  const struct option options[] = {
    {"threads", 1, NULL, 't'},
    {"points",  2, NULL, 'p'},
    {"row",     1, NULL, 'r'},
    {"col",     1, NULL, 'c'},
    /* sentinel */
    {0, 0, 0, 0}
  };

  DataSet *terrain, *vmap;
  int nthread, npoint;
  GridPoint p;
  char c;

  /* Rarse options */
  p.r = p.c = 0;
  npoint = 0;
  nthread = 1;
  opterr = 1; /* ensure that bad options return error codes */
  while ((c = getopt_long(argc, argv, "t:p::r:c:", options, NULL)) >= 0) {
    switch (c) {
      case 't':
        /* options supplied number of thread */
        nthread = strtol(optarg, NULL, 10);
        break;
      case 'p':
        /* options supplied number of points */
        if (optarg)
          npoint = strtol(optarg, NULL, 10);
        else
          npoint = 1;
        break;
      case 'r':
        /* options supplied point row index */
        p.r = strtol(optarg, NULL, 10);
        break;
      case 'c':
        /* options supplied point column index */
        p.c = strtol(optarg, NULL, 10);
        break;
      case '?':
        /* bad option */
        fprintf(stderr, "%s\n", USAGE);
        return -1;
      default:
        assert(NULL);
    }
  }
  assert(c == -1);

  /* Check option results */
  if (argc - optind != 2) {
    /* incorrect argument count */
    fprintf(stderr, "Invalid number of arguments (expected 2, got %i)\n",
            argc - optind);
    fprintf(stderr, "%s\n", USAGE);
    return -1;
  }
  if (npoint < 0 || nthread <= 0 || p.r < 0 || p.c < 0) {
    fprintf(stderr, "Invalid option argument\n");
    fprintf(stderr, "%s\n", USAGE);
    return 1;
  }

  /* Load terrain file */
  terrain = dLoad(argv[optind++], FLOAT); 
  if (!terrain)
    return 1;
  if (npoint == 1 && (p.r > terrain->grid.hd.nrow ||
                      p.c > terrain->grid.hd.ncol)) {
    fprintf(stderr, "Specified point is not in grid (%i, %i)\n", p.r, p.c);
    return -1;
  }

  /* Compute requested viewsheds */
  if (npoint == 1)
    vmap = brute_viewshed(terrain, p);
  else if (npoint == 0)
    vmap = brute_viewshed_terrain(terrain, nthread);
  else
    vmap = brute_viewshed_terrain(terrain, nthread);
  assert(vmap);

  /* Store viewshed and exit */
  return dStore(vmap, argv[optind]);
}
