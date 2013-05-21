
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
#include <float.h>
#include <limits.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>

#include "rtimer.h"
#include "runthreads.h"
#include "rbbst.h"
#include "vector.h"
#include "vis.h"

const double epsilon = 0.0000001;

int visible(DataSet *terrain, GridPoint start, GridPoint end, float fNODATA)
{
  GridPoint p;
  float x0, y0, x1, y1, y_start;
  float dx, dy, dydx;
  int idx;
  float h0, h1;
  double dzds;

  assert(start.r >= 0 && start.c >= 0);
  assert(end.r >= 0 && end.c >= 0);
  assert(start.r < terrain->grid.nrow && start.c < terrain->grid.ncol);
  assert(end.r < terrain->grid.nrow && end.c < terrain->grid.ncol);

  if (gpEqual(start, end)) {
    gpSlope(terrain, start, end, h0, h1, dzds);
    if (h0 == fNODATA || h1 == fNODATA)
      return 0;
    else
      return 1;
  }

  // calculate slope in x and y
  gpCenter(start, x0, y0);
  gpCenter(end, x1, y1);
  gpDiff(start, end, dx, dy);
  if (dx == 0) {
    idx = 0;
    dydx = 0; // not used
  }else {
    // idx is +1 or -1
    idx = (int)(dx / fabs(dx));
    dydx = dy / dx;
  }

  // calculate the z gradient
  gpSlope(terrain, start, end, h0, h1, dzds);
  if (h0 == fNODATA || h1 == fNODATA)
    return 0;
  y_start = y1 = y0;
  x1 = x0;
  p = start;

  // run through columns
  while (p.c >= 0 && p.c < terrain->grid.ncol &&
         p.r >= 0 && p.r < terrain->grid.nrow) {
    // increment x1 by a column, or half a column
    if (p.c == start.c || p.c == end.c)
      x1 += .5 * idx;
    else
      x1 += idx;
    // calculate next y intercept
    y0 = y1;
    if (idx == 0)
      y1 = y0 + dy;
    else
      y1 = y_start + dydx * (x1 - x0);
    // translate y intercept to row index
    p.r = dy >= 0 ? (int)y0 : (int)(ceilf(y0) - 1);
    // check column visibility
    if (gpBefore(p, end, dx, dy) &&
        !columnVisible(terrain, start, p, dzds, y0, y1, fNODATA))
      return 0;
    // proceed to next column
    p.c += idx;
  }

  return 1;
}

int columnVisible(DataSet *terrain, GridPoint start, GridPoint base,
                  double goal_dzds, float y0, float y1, float fNODATA)
{
  GridPoint p;
  float ydir = y1 - y0;
  float h0, h1;
  double dzds;

  p = base;
  while ((y0 == y1 && p.r == base.r ) ||
         (p.r >= 0 && p.r < terrain->grid.nrow &&
           ((ydir > 0 && p.r < y1      ) ||
            (ydir < 0 && (p.r + 1) > y1)))) {
    if (!gpEqual(start, p)) {
      gpSlope(terrain, start, p, h0, h1, dzds);
      if (h1 == fNODATA || dzds > goal_dzds + epsilon)
        return 0;
    }
    p.r += ydir > 0 ? 1 : -1;
  }
  return 1;
}

float getNODATA(DataSet *terrain)
{
  switch (terrain->grid.type) {
    case FLOAT: return (float)terrain->grid.fNODATA;
    case INT:   return (float)terrain->grid.iNODATA;
    case UINT:  return (float)terrain->grid.uiNODATA;
    case SHRT:  return (float)terrain->grid.sNODATA;
    case USHRT: return (float)terrain->grid.usNODATA;
    case CHAR:  return (float)terrain->grid.cNODATA;
    case UCHAR: return (float)terrain->grid.ucNODATA;
    default:    assert(0); return 0;
  }
}

DataSet *brute_viewshed(DataSet *terrain, GridPoint start)
{
  static Rtimer rt;
  DataSet *viewshed;
  GridPoint p;
  float fNODATA, h;

  rt_start(rt);

  fNODATA = getNODATA(terrain);
  gpHeight(terrain, start, h);
  if (h == fNODATA)
    return NULL;

  viewshed = dInit(terrain->grid.nrow, terrain->grid.ncol, UCHAR);
  viewshed->grid.ucNODATA = 0;

  for (p.r = 0; p.r < viewshed->grid.nrow; p.r++)
    for (p.c = 0; p.c < viewshed->grid.ncol; p.c++)
      dSet(viewshed, p.r, p.c, visible(terrain, start, p, fNODATA));

  rt_stop(rt);
  static char buf[256];
  rt_sprint(buf, rt);
  printf("brute_viewshed('%s',start={r=" DGI_FMT ",c=" DGI_FMT "}):\t%s\n",
         terrain->path, start.r, start.c, buf);

  return viewshed;
}

unsigned int brute_viewshed_cnt(DataSet *terrain, GridPoint start)
{
  unsigned int count;
  GridPoint p;
  float fNODATA, h;

  fNODATA = getNODATA(terrain);
  gpHeight(terrain, start, h);
  if (h == fNODATA)
    return 0;

  count = 0;
  for (p.r = 0; p.r < terrain->grid.nrow; p.r++)
    for (p.c = 0; p.c < terrain->grid.ncol; p.c++)
      if (visible(terrain, start, p, fNODATA))
        count ++;

  return count;
}

// thread band info structure
typedef struct viewshed_thread_t {
  int nthread;
  int index;
  DataSet *terrain, *vmap;
  float fNODATA;
  unsigned int (*vcount)(DataSet *terrain, GridPoint p);
} ViewshedBand;
// forward declaration
void* viewshed_terrain_sub(void *closure);

DataSet *run_viewshed_terrain(DataSet *terrain, int nthread,
    unsigned int (*vcount) (DataSet *terrain, GridPoint p))
{
  static Rtimer rt;
  DataSet *vmap;
  Vector *thread_bands;
  ViewshedBand band;
  int i;
  float fNODATA;

  rt_start(rt);

  assert(nthread > 0);

  fNODATA = getNODATA(terrain);

  vmap = dInit(terrain->grid.nrow, terrain->grid.ncol, UINT);
  assert(vmap);
  vmap->grid.uiNODATA = 0;

  thread_bands = vinit2(sizeof(ViewshedBand), nthread);
  for (i = 0; i < nthread; i++) {
    band.nthread = nthread;
    band.index = i;
    band.terrain = terrain;
    band.vmap = vmap;
    band.fNODATA = fNODATA;
    band.vcount = vcount;
    vappend(thread_bands, &band);
  }

  run_threads(nthread, viewshed_terrain_sub, thread_bands);

  rt_stop(rt);
  static char buf[256];
  rt_sprint(buf, rt);
  printf("run_viewshed_terrain('%s'):\t%s\n",
         terrain->path, buf);

  return vmap;
}

void* viewshed_terrain_sub(void *closure)
{
  ViewshedBand band;
  GridPoint p;
  unsigned int *ptr, *end;
  float h;

  assert(closure);
  band = *(ViewshedBand*)closure;
  assert(band.nthread > 0);
  assert(band.index >= 0 &&  band.index < band.nthread);

  printf("thread %i of %i\n", band.index + 1, band.nthread);
  fflush(stdout);
  ptr = band.vmap->grid.uiData + band.index;
  end = band.vmap->grid.uiData + band.vmap->grid.nrow * band.vmap->grid.ncol;
  p.r = 0;
  p.c = band.index;
  while (ptr < end) {
    // run viewshed alg.
    gpHeight(band.terrain, p, h);
    if (h == band.fNODATA)
      *ptr = band.vmap->grid.uiNODATA;
    else
      *ptr = band.vcount(band.terrain, p);
    // advance to next point
    p.c += band.nthread;
    ptr += band.nthread;
    if (p.c >= band.vmap->grid.ncol) {
      printf("thread %i advancing to row " DGI_FMT "\n", band.index, p.r+1);
      fflush(stdout);
      p.c %= band.vmap->grid.ncol;
      p.r++;
    }
  }

  pthread_exit(NULL);
}


DataSet *brute_viewshed_terrain(DataSet *terrain, int nthread)
{
  return run_viewshed_terrain(terrain, nthread, brute_viewshed_cnt);
}

typedef struct rectangle_t {
  GridPoint p, q;
} Rectangle;

void brute_viewshed_subterrain(DataSet *terrain, const Rectangle a,
                                   const Rectangle b, float fNODATA,
                                   DataSet *vshed_cnt)
{
  unsigned int count;
  GridPoint p, q;
  unsigned int *aptr, *bptr;
  const index_t nrow = vshed_cnt->grid.nrow;
  const index_t ncol = vshed_cnt->grid.ncol;

  count = 0;
  for (p.r = a.p.r; p.r < a.q.r && p.r < nrow; p.r++) {
    aptr = vshed_cnt->grid.uiData + p.r * ncol + a.p.c;
    for (p.c = a.p.c; p.c < a.q.c && p.c < ncol; p.c++, aptr++) {
      for (q.r = b.p.r; q.r < b.q.r && q.r < nrow; q.r++) {
        bptr = vshed_cnt->grid.uiData + q.r * ncol + b.p.c;
        for (q.c = b.p.c; q.c < b.q.c && q.c < ncol; q.c++, bptr++) {
          if (visible(terrain, p, q, fNODATA)) {
            (*aptr)++;
            if (!gpEqual(a.p, b.p))
              (*bptr)++;
          }
        }
      }
    }
  }
}

DataSet *brute_viewshed_terrain2(DataSet *terrain)
{
  DataSet *vshed_cnt;
  Rectangle a, b;
  float fNODATA;
  const index_t nrow = terrain->grid.nrow;
  const index_t ncol = terrain->grid.ncol;
  const index_t w = 50;
  const index_t h = 50;
  static Rtimer rt;

  //start timing
  rt_start(rt);

  // grab NODATA value, make sure this is a valid point
  fNODATA = getNODATA(terrain);
  // initialize viewshed count grid
  vshed_cnt = dInit(terrain->grid.nrow, terrain->grid.ncol, UINT);
  assert(vshed_cnt);

  // step through 50x50 block in the grid, computing visibility count
  for (a.p.r = 0; a.p.r < nrow; a.p.r += h) {
    a.q.r = a.p.r + h;
    for (a.p.c = 0; a.p.c < ncol; a.p.c += w) {
      a.q.c = a.p.c + w;
      printf("SubGrid (" DGI_FMT "," DGI_FMT ")\n", a.p.r, a.p.c);
      for (b.p.r = a.p.r; b.p.r < nrow; b.p.r += h) {
        b.q.r = b.p.r + h;
        for (b.p.c = 0; b.p.c < ncol; b.p.c += w) {
          if (a.p.r == b.p.r && a.p.c > b.p.c)
            continue;
          b.q.c = b.p.c + w;
          printf("  Target (" DGI_FMT "," DGI_FMT ")\n", b.p.r, b.p.c);
          brute_viewshed_subterrain(terrain, a, b, fNODATA, vshed_cnt);
        }
      }
    }
  }

  // stop timing
  rt_stop(rt);
  static char buf[256];
  rt_sprint(buf, rt);
  printf("brute_viewshed_terrain2('%s'):\t%s\n", terrain->path, buf);

  return vshed_cnt;
}


typedef struct viewshed_rb_tree_event
{
  GridPoint p;
  char event;
  double angle;
  double dist;
} SweepEvent;

#define ENTER_EVENT 0
#define SIGHT_EVENT 1
#define LEAVE_EVENT 2

#define CMP_FUNC int(*)(const void*, const void*)

int compare_SweepEvent_angle(const SweepEvent *a, const SweepEvent *b)
{
  if (fabs(a->angle - b->angle) < epsilon) return 0;
  if (a->angle < b->angle) return -1;
  return 1;
}

/**
 * Calculate the angle at which a line sweep would first intercept the bounding
 * box of the target point, moving in a clockwise direction about the base
 * point.
 */
double enterAngle(GridPoint base, GridPoint target)
{
  double x, y;
  gpCenter(base, x, y);

  if (target.r > base.r) {
    if (target.c >= base.c) {
      // QUADRANT I
      return atan2(target.r - y,     target.c - x + 1);
    }else {
      // QUADRANT II
      return atan2(target.r - y + 1, target.c - x + 1);
    }
  }else if (target.r < base.r) {
    if (target.c <= base.c) {
      // QUADRANT III
      return atan2(target.r - y + 1, target.c - x    ) + 2 * M_PI;
    }else {
      // QUADRANT IV
      return atan2(target.r - y    , target.c - x    ) + 2 * M_PI;
    }
  }else {
    if (target.c >= base.c) {
      // also QUADRANT IV  -----  i.e. the start row or positive x axis
      //   the initial row recieves heavy negative weighting, in
      //   order, proceeding positively away from the start point
      return (double)INT_MIN + target.c - base.c;
      //return atan2(target.r - y    , target.c - x    ) + 2 * M_PI;
    }else
      // also QUADRANT II
      return atan2(target.r - y + 1, target.c - x + 1);
  }
}

/**
 * Calculate the angle at which a line sweep would intercept the center of the
 * target point, moving in a clockwise direction about the base point.
 */
double sightAngle(GridPoint base, GridPoint target)
{
  double x0, y0, x1, y1;
  gpCenter(base, x0, y0);
  gpCenter(target, x1, y1);


  if (y1 < y0)
    // QUADRANTS III and IV - atan2 will be negative (-PI, 0)
    return atan2(y1 - y0, x1 - x0) + 2 * M_PI;
  if (y1 > y0 || x1 < x0)
    // QUADRANTS I and II - atan2 will be positive [0, PI]
    return atan2(y1 - y0, x1 - x0);

  // y1 == y0 && x1 >= x0  -----  i.e. the start row or positive x axis
  //   the initial row recieves heavy negative weighting, in
  //   order, proceeding positively away from the start point
  return INT_MIN + x1 - x0 + .5;
}

/**
 * Calculate the angle at which a line sweep would no longer intercept the
 * bounding box of the target point, moving in a clockwise direction about the
 * base point.
 */
double leaveAngle(GridPoint base, GridPoint target)
{
  double x, y;
  gpCenter(base, x, y);

  if (target.r > base.r) {
    if (target.c > base.c) {
      // QUADRANT I
      return atan2(target.r - y + 1, target.c - x    );
    }else {
      // QUADRANT II
      return atan2(target.r - y    , target.c - x    );
    }
  }else if (target.r < base.r) {
    if (target.c < base.c) {
      // QUADRANT III
      return atan2(target.r - y    , target.c - x + 1) + 2 * M_PI;
    }else {
      // QUADRANT IV
      return atan2(target.r - y + 1, target.c - x + 1) + 2 * M_PI;
    }
  }else {
    if (target.c > base.c) {
      // also QUADRANT I
      return atan2(target.r - y + 1, target.c - x    );
    }else if (target.c < base.c) {
      // also QUADRANT III
      return atan2(target.r - y    , target.c - x + 1) + 2 * M_PI;
    }else {
      return INT_MAX;
    }
  }
}

void build_event_list(DataSet *terrain, GridPoint start, float fNODATA,
                      SweepEvent **output_list, size_t *output_len)
{
  SweepEvent *event_list, *eptr;
  SweepEvent event;
  GridPoint p;
  float h;
  size_t len;
  const size_t nmemb = 3 * terrain->grid.nrow * terrain->grid.ncol - 1;
  const size_t size = sizeof(SweepEvent);
  
  // allocate event list
  event_list = (SweepEvent*) malloc(nmemb * size);
  assert(event_list);
  // fill list with events, calculating the intercept angles
  len = 0;
  eptr = event_list;
  for (p.r = 0; p.r < terrain->grid.nrow; p.r++) {
    for (p.c = 0; p.c < terrain->grid.ncol; p.c++) {
      if (gpEqual(start, p))
        continue;
      gpHeight(terrain, p, h);
      if (h == fNODATA)
        continue;

      event.p = p;
      event.event = ENTER_EVENT;
      event.angle = enterAngle(start, p);
      event.dist = gpDist(start, p);
      *eptr++ = event;

      event.event = SIGHT_EVENT;
      event.angle = sightAngle(start, p);
      *eptr++ = event;

      event.event = LEAVE_EVENT;
      event.angle = leaveAngle(start, p);
      *eptr++ = event;

      len += 3;
    }
  }
  // sort the event list by intercept angle
  qsort(event_list, len, size, (CMP_FUNC)compare_SweepEvent_angle);

  *output_list = event_list;
  *output_len = len;
}

unsigned int process_event_list(DataSet *terrain, GridPoint start,
                                SweepEvent *event_list, size_t len,
                                DataSet *vshed)
{
  SweepEvent *eptr, *eend;
  RBTree *tree;
  TreeNode *node;
  TreeValue value;
  unsigned int count;
  double h0, h1;
  
  // retrieve height of start point
  gpHeight(terrain, start, h0);
  // initialiaze tree
  value.key = 0;
  value.gradient = SMALLEST_GRADIENT;
  tree = createTree(value);

  // process list
  count = 0;
  eptr = event_list;
  eend = event_list + len;
  while (eptr < eend) {
    if (eptr->event == ENTER_EVENT) {
      // set key to node distance
      value.key = eptr->dist;
      // calculate slope in z
      gpHeight(terrain, eptr->p, h1);
      value.gradient = (h1 - h0) / eptr->dist;
      // insert the node into the active list
      insertInto(tree, value);
      
    }else if (eptr->event == SIGHT_EVENT) {
      // check the active list if the current node is visible
      // query for the current node
      node = searchForNodeWithKey(tree, eptr->dist);
      assert(notNIL(node));
      h1 = findMaxGradientWithinKey(tree, eptr->dist);
      if (vshed != NULL)
        dSet(vshed, eptr->p.r, eptr->p.c, (node->value.gradient >= h1));
      count += (node->value.gradient >= h1);

    }else { // LEAVE_EVENT 
      // remove the current node
      deleteFrom(tree, eptr->dist);

    }
    eptr++;
  }
  // garbage collect
  deleteTree(tree);

  return count;
}

DataSet *sweep_viewshed(DataSet *terrain, GridPoint start)
{
  static Rtimer rt;
  DataSet *viewshed;
  float fNODATA, h;
  SweepEvent *event_list;
  size_t len;

  // start timing
  rt_start(rt);
  // grab NODATA value, make sure this is a valid point
  fNODATA = getNODATA(terrain);
  gpHeight(terrain, start, h);
  if (h == fNODATA)
    return NULL;
  
  // initialize viewshed terrain
  viewshed = dInit(terrain->grid.nrow, terrain->grid.ncol, UCHAR);
  viewshed->grid.ucNODATA = 0;

  // build list of start and end events
  build_event_list(terrain, start, fNODATA, &event_list, &len);
  // iterate event list, marking visible points in viewshed terrain
  process_event_list(terrain, start, event_list, len, viewshed);

  // garbage collect
  free(event_list);
  // stop timing
  rt_stop(rt);
  static char buf[256];
  rt_sprint(buf, rt);
  printf("sweep_viewshed('%s',start={r=" DGI_FMT ",c=" DGI_FMT "}):\t%s\n",
         terrain->path, start.r, start.c, buf);

  return viewshed;
}

unsigned int sweep_viewshed_cnt(DataSet *terrain, GridPoint start)
{
  float fNODATA, h;
  SweepEvent *event_list;
  size_t len;
  unsigned int count;

  // grab NODATA value, make sure this is a valid point
  fNODATA = getNODATA(terrain);
  gpHeight(terrain, start, h);
  if (h == fNODATA)
    return 0;

  // build list of start and end events
  build_event_list(terrain, start, fNODATA, &event_list, &len);
  // iterate event list, counting visible points
  count = process_event_list(terrain, start, event_list, len, NULL);

  // garbage collect
  free(event_list);

  return count;
}

DataSet *sweep_viewshed_terrain(DataSet *terrain, int nthread)
{
  return run_viewshed_terrain(terrain, nthread, sweep_viewshed_cnt);
}
