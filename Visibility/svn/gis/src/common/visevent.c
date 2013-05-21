
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

#include <math.h>
#include "visevent.h"

#define EPSILON 0.0000000001

#define compare_values(a, b) ((a) < (b) ? -1 : ((a) > (b)))

/**
 * Compare two SweepEvents, first by angle, then by distance, then by event
 * type.
 */
int compare_SweepEvent(SweepEvent a, SweepEvent b)
{
  if (fabs(a.angle - b.angle) < EPSILON) {
    if (a.type == b.type)
      return compare_values(a.dist, b.dist);
    return compare_values(a.type, b.type);
  }
  return compare_values(a.angle, b.angle);
}


/**
 * Array of human-readable names for each quadrant
 */
const char* QUADRANT_NAMES[] = {
    "?", "I", "II", "I & II", "III", "I & III", "II & III", "I & II & III",
	"IV", "I & IV", "II & IV", "I & II & IV", "III & IV", "I & III & IV",
	"II & III & IV", "I & II & III & IV"
};

const char* quadrant_name(int quadrant)
{
  return QUADRANT_NAMES[quadrant];
}


/**
 * Determine what quadrant of a grid a point lies with respect to the viewpoint
 * as origin.  Quadrants are as mathematical quadrants of the xy-plane, with y
 * and rows equivalent, x and columns equivalent.
 */
int calculate_GridPoint_quadrant(GridPoint vp, GridPoint p)
{
  int quadrant;

  quadrant = 0;

  if (p.r >= vp.r) {
    if (p.c >= vp.c) {
      quadrant = QUADRANT_I;
      if (p.r == vp.r)
        /* on the X axis */
        quadrant |= QUADRANT_IV;
      if (p.c == vp.c)
        /* on the Y axis */
         quadrant |= QUADRANT_II;
    }else {
      quadrant = QUADRANT_II;
      if (p.r == vp.r)
        /* on the X axis */
        quadrant |= QUADRANT_III;
     }
  }else {
    if (p.c >= vp.c) {
      quadrant = QUADRANT_IV;
      if (p.c == vp.c)
        quadrant |= QUADRANT_III;
    }else
      quadrant = QUADRANT_III;
  }

  return quadrant;
}

/* event angle offsets - in clockwise sweep order
    values are located in index 1, 2, 4, and 8, per the QUADRANT bitmasks.
    values in other locations represent point on an axis, the logical OR of two
    neighboring QUADRANT bitmasks, namely entries 3 (I & II), 6 (II & III),
    9 (IV & I), and 12 (III & IV) */
const double ENTER_row_offsets[] =
  {   0, -.5,  .5, -.5,  .5,   0,  .5,   0, -.5, -.5,   0,   0,  .5};
const double ENTER_col_offsets[] =
  {   0,  .5,  .5,  .5, -.5,   0,  .5,   0, -.5, -.5,   0,   0, -.5};
const double LEAVE_row_offsets[] =
  {   0,  .5, -.5, -.5, -.5,   0, -.5,   0,  .5,  .5,   0,   0,  .5};
const double LEAVE_col_offsets[] =
  {   0, -.5, -.5, -.5,  .5,   0,  .5,   0,  .5, -.5,   0,   0,  .5};


/**
 * Calculate the angle at which a sweep line about the viewpoint would
 * intercept the given event.  The sweep line is assumed to progress in a
 * clockwise direction around the viewpoint.  Quadrants are determined by
 * calculate_GridPoint_quadrant().
 */
double calculate_SweepEvent_angle(SweepEvent ev, GridPoint vp)
{
  int quadrant;
  double x, y, angle;
  const double TWO_PI = 2 * M_PI;

  quadrant = calculate_GridPoint_quadrant(vp, ev.p);
  y = ev.p.r;
  x = ev.p.c;
  if (ev.type == ENTER_EVENT) {
    y += ENTER_row_offsets[quadrant];
    x += ENTER_col_offsets[quadrant];
  } else if (ev.type == LEAVE_EVENT) {
    y += LEAVE_row_offsets[quadrant];
    x += LEAVE_col_offsets[quadrant];
  }
  
  angle = atan2(y - vp.r, x - vp.c);
  if (angle < 0)
    angle += TWO_PI;
  return angle;
}

/**
 * Calculate the tangent of the SweepEvent angle.  As the tangent is not unique
 * this methods requires specification of which quadrant the tangent is to be
 * calculated relative to.  For example, if quadrant I is specified, all points
 * within quadrant I will return tangents between negative infinity and zero.
 * Points in quadrant II will return tangents between zero and positive
 * infinity.
 */
double calculate_SweepEvent_tangent(SweepEvent ev, GridPoint vp, int target)
{
  int quadrant;
  double x, y;

  quadrant = calculate_GridPoint_quadrant(vp, ev.p);
  y = ev.p.r;
  x = ev.p.c;
  if (ev.type == ENTER_EVENT) {
    y += ENTER_row_offsets[quadrant];
    x += ENTER_col_offsets[quadrant];
  } else if (ev.type == LEAVE_EVENT) {
    y += LEAVE_row_offsets[quadrant];
    x += LEAVE_col_offsets[quadrant];
  }

  if (target == QUADRANT_I || target == QUADRANT_III)
    /* dy / dx */
    return (y - vp.r) / (x - vp.c);
  else /* QUADRANT_II && QUADRANT_IV */
    /* -dx / dy */
    return -(x - vp.c) / (y - vp.r);
}

