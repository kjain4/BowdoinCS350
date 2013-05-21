
/**
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */



#include <assert.h>
#include <malloc.h>
#include <math.h>
#include <stdio.h>
#include "pqheap.h"
#include "rbbst.h"
#include "radial2.h"
#include "rtimer.h"


/* turn on to calculate event angles using atan2.  otherwise, uses tangents */
//#define USE_ATAN


DataSet* radial2_viewshed_terrain(DataSet *terrain)
{
  static Rtimer rt;
  DataSet *vcount;
  GridPoint vp;
  unsigned int count;

  rt_start(rt);

  vcount = dInit(terrain->grid.hd.nrow, terrain->grid.hd.ncol, UINT);
  vcount->grid.hd.NODATA_value = 0;

  vp.r = 0; vp.c = 0;
  for (vp.r = 0; vp.r < terrain->grid.hd.nrow; vp.r++) {
    for (vp.c = 0; vp.c < terrain->grid.hd.ncol; vp.c++) {
      count = 1;
      count += radial2_viewshed_quadrant(&terrain->grid, vp, QUADRANT_I,
                                         NULL);
      count += radial2_viewshed_quadrant(&terrain->grid, vp, QUADRANT_II,
                                         NULL);
      count += radial2_viewshed_quadrant(&terrain->grid, vp, QUADRANT_III,
                                         NULL);
      count += radial2_viewshed_quadrant(&terrain->grid, vp, QUADRANT_IV,
                                         NULL);

      dg_set(vcount->grid, vp, count);
    }
  }

  rt_stop(rt);
  static char buf[256];
  rt_sprint(buf, rt);
  printf("radial2_viewshed('%s',start={r=%u,c=%u}):\t%s\n",
         terrain->path, vp.r, vp.c, buf);

  return vcount;
}


DataSet* radial2_viewshed(DataSet *terrain, GridPoint vp)
{
  static Rtimer rt;
  int count;
  DataSet* dset;

  rt_start(rt);
  
  dset = dInit(terrain->grid.hd.nrow, terrain->grid.hd.ncol, UINT);
  dset->grid.hd.NODATA_value = 0;

  count = 1;
  count += radial2_viewshed_quadrant(&terrain->grid, vp, QUADRANT_I,
                                     &dset->grid);
  count += radial2_viewshed_quadrant(&terrain->grid, vp, QUADRANT_II,
                                     &dset->grid);
  count += radial2_viewshed_quadrant(&terrain->grid, vp, QUADRANT_III,
                                     &dset->grid);
  count += radial2_viewshed_quadrant(&terrain->grid, vp, QUADRANT_IV,
                                     &dset->grid);

  printf("count = %i\n", count);

  rt_stop(rt);
  static char buf[256];
  rt_sprint(buf, rt);
  printf("radial2_viewshed('%s',start={r=%u,c=%u}):\t%s\n",
         terrain->path, vp.r, vp.c, buf);

  return dset;
}

int radial2_viewshed_cnt(Grid terrain, GridPoint vp)
{
  int count;

  count = 0;
  count += radial2_viewshed_quadrant(&terrain, vp, QUADRANT_I,   NULL);
  count += radial2_viewshed_quadrant(&terrain, vp, QUADRANT_II,  NULL);
  count += radial2_viewshed_quadrant(&terrain, vp, QUADRANT_III, NULL);
  count += radial2_viewshed_quadrant(&terrain, vp, QUADRANT_IV,  NULL);

  return count;
}

int radial2_viewshed_quadrant(Grid *terrain, GridPoint vp, int quadrant,
                         Grid *viewshed)
{
  SweepEvent ev;
  PQueue *pq;
  TreeNode *node;
  TreeValue tv;
  RBTree *as;

  int count, result;
  int dr, dc;
  double h0, h, maxGradient;

  assert(quadrant == QUADRANT_I   || quadrant == QUADRANT_II ||
         quadrant == QUADRANT_III || quadrant == QUADRANT_IV);

  dg_get(*terrain, vp, h0, double);
  assert(h0 != terrain->hd.NODATA_value);

  /*printf("Processing quadrant %s\n", quadrant_name(quadrant));*/

  /* Initialize PQ */
  pq = PQ_initialize();
  ev.p = vp;
  dr = (quadrant == QUADRANT_II) | -(quadrant == QUADRANT_IV );
  dc = (quadrant == QUADRANT_I ) | -(quadrant == QUADRANT_III);
  while (gp_within(ev.p, 0, 0, terrain->hd.nrow, terrain->hd.ncol)) {
    if (gp_equal(vp, ev.p)) {
      ev.p.r += dr; ev.p.c += dc;
      continue;
    }
    /* insert new initial event into the priority queue */
    ev.type = ENTER_EVENT;
    ev.dist = gp_dist(vp, ev.p);
    /* set angle to ordered negative values to ensure early processing
     * and avoid angle issues on the axes */
    ev.angle = ev.dist - (terrain->hd.nrow + terrain->hd.ncol);
    PQ_insert(pq, ev);
    /* continue to next point */
    ev.p.r += dr; ev.p.c += dc;
  }

  /* Initialize AS */
  tv.key = 0;
  tv.gradient = SMALLEST_GRADIENT;
  as = create_tree(tv);

  /* Sweep events in the quadrant */
  count = 0;
  dr = (quadrant == QUADRANT_I   || quadrant == QUADRANT_IV) ? 1 : -1;
  dc = (quadrant == QUADRANT_III || quadrant == QUADRANT_IV) ? 1 : -1;
  while (!PQ_isEmpty(pq)) {

    /* get next event */
    result = PQ_extractMin(pq, &ev);
    assert(result);
    assert(ev.type == ENTER_EVENT || ev.type == SIGHT_EVENT ||
           ev.type == LEAVE_EVENT);
    assert(gp_within(ev.p, 0, 0, terrain->hd.nrow, terrain->hd.ncol) &&
          (calculate_GridPoint_quadrant(vp, ev.p) & quadrant));

    if (ev.type == ENTER_EVENT) {
      /* insert this obstacle in AS */
      tv.key = ev.dist;
      dg_get(*terrain, ev.p, h, double);
      if (h == terrain->hd.NODATA_value)
        /* skip NODATA points */
        continue;
      tv.gradient = (h - h0) / ev.dist;
      insert_into(as, tv);
      if (calculate_GridPoint_quadrant(vp, ev.p) & ((quadrant << 1) % 15))
        /* stop adding events for points on the next axis */
        continue;
      /* insert corresponding CENTER and EXIT events in PQ */
      ev.type = SIGHT_EVENT;
#ifdef USE_ATAN
      ev.angle = calculate_SweepEvent_angle(ev, vp);
#else
      ev.angle = calculate_SweepEvent_tangent(ev, vp, quadrant);
#endif
      PQ_insert(pq, ev);
      ev.type = LEAVE_EVENT;
#ifdef USE_ATAN
      ev.angle = calculate_SweepEvent_angle(ev, vp);
#else
      ev.angle = calculate_SweepEvent_tangent(ev, vp, quadrant);
#endif
      PQ_insert(pq, ev);
      /* insert _next_ point's ENTER event in PQ */
      ev.p.r += dr;
      ev.p.c += dc;
      if (gp_within(ev.p, 0, 0, terrain->hd.nrow, terrain->hd.ncol)) {
        ev.type = ENTER_EVENT;
        ev.dist = gp_dist(vp, ev.p);
#ifdef USE_ATAN
        ev.angle = calculate_SweepEvent_angle(ev, vp);
#else
        ev.angle = calculate_SweepEvent_tangent(ev, vp, quadrant);
#endif
        PQ_insert(pq, ev);
      }

    }else if (ev.type == LEAVE_EVENT)
      /* delete this event from AS */
      delete_from(as, ev.dist);

    else { /* SIGHT_EVENT */
      /* determine visibility of GridPoint */
      node = search_for_node_with_key(as, ev.dist);
      assert(notNIL(node));
      maxGradient = find_max_gradient_within_node(node);
      if (maxGradient <= node->value.gradient)
        /* visible! */
        count++;
      if (viewshed != NULL)
        dg_set(*viewshed, ev.p, maxGradient <= node->value.gradient);
    }
  }

  /*printf("count = %i\n", count);*/

  return count;
}
