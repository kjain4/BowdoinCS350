
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


#ifndef _vis_h_DEFINED
#define _vis_h_DEFINED

#include "gridpoint.h"
#include "datagrid.h"

int visible(DataSet *terrain, GridPoint start, GridPoint end, float fNODATA);
int columnVisible(DataSet *terrain, GridPoint start, GridPoint base,
                  double goal_dzds, float y0, float y1, float fNODATA);

DataSet *brute_viewshed(DataSet *terrain, GridPoint start);
DataSet *brute_viewshed_terrain(DataSet *terrain, int nthread);
DataSet *brute_viewshed_terrain2(DataSet *terrain);
unsigned int brute_viewshed_cnt(DataSet *terrain, GridPoint start);

DataSet *sweep_viewshed(DataSet *terrain, GridPoint start);
DataSet *sweep_viewshed_terrain(DataSet *terrain, int nthread);
unsigned int sweep_viewshed_cnt(DataSet *terrain, GridPoint start);


// Useful macros and functions //

// are two GridPoints equal
#define gpEqual(p, q) ((p).r == (q).r && (p).c == (q).c)
// calculate the (x,y) center of a GridPoint
#define gpCenter(p, x, y) { x = (p).c + .5; y = (p).r + .5; }
// calculate the difference in x and y between two points
#define gpDiff(p, q, dx, dy) { \
  dx = (float)(q).c - (p).c; \
  dy = (float)(q).r - (p).r; \
}
// apply a slope to a value over a given period
#define gpApply(y, dydx, dx, res) (res = y + (dydx * dx))
// retrieve the float height value of a DataSet at a GridPoint
#define gpHeight(dset, p, h) dGet(dset, (p).r, (p).c, h, float)
// calculate the Euclidean distance between two points
#define gpDist(p, q) ( \
  sqrt(pow((double)(q).r - (p).r, 2) + pow((double)(q).c - (p).c, 2)) )
// calculate the slope in the Z (height) plane, ignoring dy & dx
#define gpSlope(dset, p, q, hp, hq, dzds) { \
  gpHeight(dset, p, hp); \
  gpHeight(dset, q, hq); \
  dzds = (hq-hp) / gpDist(p, q); \
}
// determine if a point lies before another in a given direction
#define gpBefore(p, q, dx, dy) ((dx > 0 && p.c < q.c) || \
                                (dx < 0 && p.c > q.c) || \
                                (dy > 0 && p.r < q.r) || \
                                (dy < 0 && p.r > q.r))

float getNODATA(DataSet *terrain);


#endif /* vis_h_DEFINED */
