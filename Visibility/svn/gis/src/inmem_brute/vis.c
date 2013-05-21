
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
  assert(start.r < terrain->grid.hd.nrow && start.c < terrain->grid.hd.ncol);
  assert(end.r < terrain->grid.hd.nrow && end.c < terrain->grid.hd.ncol);

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
  while (p.c >= 0 && p.c < terrain->grid.hd.ncol &&
         p.r >= 0 && p.r < terrain->grid.hd.nrow) {
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
         (p.r >= 0 && p.r < terrain->grid.hd.nrow &&
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

DataSet *brute_viewshed(DataSet *terrain, GridPoint start)
{
  static Rtimer rt;
  DataSet *viewshed;
  GridPoint p;
  float fNODATA, h;

  rt_start(rt);

  fNODATA = terrain->grid.hd.NODATA_value;
  gpHeight(terrain, start, h);
  if (h == fNODATA)
    return NULL;

  viewshed = dInit(terrain->grid.hd.nrow, terrain->grid.hd.ncol, UCHAR);
  viewshed->grid.hd.NODATA_value = 0;

  for (p.r = 0; p.r < viewshed->grid.hd.nrow; p.r++)
    for (p.c = 0; p.c < viewshed->grid.hd.ncol; p.c++)
      dSet(viewshed, p.r, p.c, visible(terrain, start, p, fNODATA));

  rt_stop(rt);
  static char buf[256];
  rt_sprint(buf, rt);
  printf("brute_viewshed('%s',start={r=%u,c=%u}):\t%s\n",
         terrain->path, start.r, start.c, buf);

  return viewshed;
}

unsigned int brute_viewshed_cnt(DataSet *terrain, GridPoint start)
{
  unsigned int count;
  GridPoint p;
  float fNODATA, h;

  fNODATA = terrain->grid.hd.NODATA_value;
  gpHeight(terrain, start, h);
  if (h == fNODATA)
    return 0;

  count = 0;
  for (p.r = 0; p.r < terrain->grid.hd.nrow; p.r++)
    for (p.c = 0; p.c < terrain->grid.hd.ncol; p.c++)
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

  fNODATA = terrain->grid.hd.NODATA_value;

  vmap = dInit(terrain->grid.hd.nrow, terrain->grid.hd.ncol, UINT);
  assert(vmap);
  vmap->grid.hd.NODATA_value = 0;

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
  end = band.vmap->grid.uiData + band.vmap->grid.hd.nrow *
                                 band.vmap->grid.hd.ncol;
  p.r = 0;
  p.c = band.index;
  while (ptr < end) {
    // run viewshed alg.
    gpHeight(band.terrain, p, h);
    if (h == band.fNODATA)
      *ptr = (unsigned int)band.vmap->grid.hd.NODATA_value;
    else
      *ptr = band.vcount(band.terrain, p);
    // advance to next point
    p.c += band.nthread;
    ptr += band.nthread;
    if (p.c >= band.vmap->grid.hd.ncol) {
      printf("thread %i advancing to row %u\n", band.index, p.r+1);
      fflush(stdout);
      p.c %= band.vmap->grid.hd.ncol;
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
  const index_t nrow = vshed_cnt->grid.hd.nrow;
  const index_t ncol = vshed_cnt->grid.hd.ncol;

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
  const index_t nrow = terrain->grid.hd.nrow;
  const index_t ncol = terrain->grid.hd.ncol;
  const index_t w = 50;
  const index_t h = 50;
  static Rtimer rt;

  //start timing
  rt_start(rt);

  // grab NODATA value, make sure this is a valid point
  fNODATA = terrain->grid.hd.NODATA_value;
  // initialize viewshed count grid
  vshed_cnt = dInit(terrain->grid.hd.nrow, terrain->grid.hd.ncol, UINT);
  assert(vshed_cnt);

  // step through 50x50 block in the grid, computing visibility count
  for (a.p.r = 0; a.p.r < nrow; a.p.r += h) {
    a.q.r = a.p.r + h;
    for (a.p.c = 0; a.p.c < ncol; a.p.c += w) {
      a.q.c = a.p.c + w;
      printf("SubGrid (%u,%u)\n", a.p.r, a.p.c);
      for (b.p.r = a.p.r; b.p.r < nrow; b.p.r += h) {
        b.q.r = b.p.r + h;
        for (b.p.c = 0; b.p.c < ncol; b.p.c += w) {
          if (a.p.r == b.p.r && a.p.c > b.p.c)
            continue;
          b.q.c = b.p.c + w;
          printf("  Target (%u,%u)\n", b.p.r, b.p.c);
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


typedef struct viewshed_point_t
{
  GridPoint p;
  double eventAngles[3];
  double gradient;
  double dist;
} ViewshedPoint;

typedef struct viewshed_sweep_event
{
  char event;
  ViewshedPoint *vp;
} SweepEvent;

#define ENTER_EVENT 0
#define SIGHT_EVENT 1
#define LEAVE_EVENT 2

#define CMP_FUNC int(*)(const void*, const void*)

int compare_SweepEvent_angle(const SweepEvent *a, const SweepEvent *b)
{
  const double angleA = a->vp->eventAngles[(int)a->event];
  const double angleB = b->vp->eventAngles[(int)b->event];

  if (fabs(angleA - angleB) < epsilon) {
    if (a->event == b->event)
      return (a->vp->dist < b->vp->dist) ? (-1) : (a->vp->dist > b->vp->dist);
    if (a->event < b->event)
      return -1;
    return 1;
  }
  if (angleA < angleB)
    return -1;
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
  ViewshedPoint *vptr;
  GridPoint p;
  float h0, h;
  size_t len;
  const size_t nmemb = terrain->grid.hd.nrow * terrain->grid.hd.ncol - 1;
  
  assert(terrain);
  assert(output_list);
  assert(output_len);
  // allocate event list
  event_list = (SweepEvent*) malloc(3 * nmemb * sizeof(SweepEvent) +
                                    nmemb * sizeof(ViewshedPoint));
  assert(event_list);
  // retrieve height of terrain at viewpoint
  gpHeight(terrain, start, h0);
  assert(h0 != fNODATA);
  // fill list with events, calculating the intercept angles
  len = 0;
  vptr = (ViewshedPoint*)(event_list + 3 * nmemb);
  eptr = event_list;
  for (p.r = 0; p.r < terrain->grid.hd.nrow; p.r++) {
    for (p.c = 0; p.c < terrain->grid.hd.ncol; p.c++) {
      // skip viewpoint itself, and NODATA points
      if (gpEqual(start, p))
        continue;
      gpHeight(terrain, p, h);
      if (h == fNODATA)
        continue;
      // initialize viewshed data point
      vptr->p = p;
      vptr->eventAngles[ENTER_EVENT] = enterAngle(start, p);
      vptr->eventAngles[SIGHT_EVENT] = sightAngle(start, p);
      vptr->eventAngles[LEAVE_EVENT] = leaveAngle(start, p);
      vptr->dist = gpDist(start, p);
      vptr->gradient = (h - h0) / vptr->dist;
      // initialize three events for each point
      eptr->event = ENTER_EVENT;
      eptr->vp = vptr;
      eptr++;
      eptr->event = SIGHT_EVENT;
      eptr->vp = vptr;
      eptr++;
      eptr->event = LEAVE_EVENT;
      eptr->vp = vptr;
      eptr++;
      // keep track of number of events
      vptr++;
      len += 3;
    }
  }
  // sort the event list by intercept angle
  qsort(event_list, len, sizeof(SweepEvent),
        (CMP_FUNC)compare_SweepEvent_angle);
  // set return values-by-reference
  *output_list = event_list;
  *output_len = len;
}

unsigned int process_event_list(DataSet *terrain, GridPoint start,
                                SweepEvent *event_list, size_t len,
                                DataSet *vshed)
{
  SweepEvent *eptr, *eend;
  RBTree *tree;
  TreeValue value;
  unsigned int count;
  //double h0, h1;
  double h;
  
  // initialiaze tree
  value.key = 0;
  value.gradient = SMALLEST_GRADIENT;
  tree = create_tree(value);
  assert(tree);
  assert(terrain);
  assert(event_list);

  // process list
  count = 0;
  eptr = event_list;
  eend = event_list + len;
  while (eptr < eend) {
    if (eptr->event == ENTER_EVENT) {
      // set key to node distance
      value.key = eptr->vp->dist;
      value.gradient = eptr->vp->gradient;
      // insert the node into the active list
      insert_into(tree, value);
      
    }else if (eptr->event == SIGHT_EVENT) {
      //assert(notNIL(search_for_node_with_key(tree, eptr->vp->dist)));
      assert(search_for_node_with_key(tree, eptr->vp->dist)->value.gradient ==
                eptr->vp->gradient);
      // check the active list for nodes bloocking the current node
      h = find_max_gradient_within_key(tree, eptr->vp->dist);
      if (vshed != NULL)
        dSet(vshed, eptr->vp->p.r, eptr->vp->p.c, (eptr->vp->gradient >= h));
      count += (eptr->vp->gradient >= h);

    }else { // LEAVE_EVENT 
      //assert(notNIL(search_for_node_with_key(tree, eptr->vp->dist)));
      assert(search_for_node_with_key(tree, eptr->vp->dist)->value.gradient ==
                eptr->vp->gradient);
      // remove the current node
      delete_from(tree, eptr->vp->dist);
    }
    eptr++;
  }
  // garbage collect
  delete_tree(tree);

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
  fNODATA = terrain->grid.hd.NODATA_value;
  gpHeight(terrain, start, h);
  if (h == fNODATA)
    return NULL;
  
  // initialize viewshed terrain
  viewshed = dInit(terrain->grid.hd.nrow, terrain->grid.hd.ncol, UCHAR);
  viewshed->grid.hd.NODATA_value = 0;

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
  printf("sweep_viewshed('%s',start={r=%u,c=%u}):\t%s\n",
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
  fNODATA = terrain->grid.hd.NODATA_value;
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
