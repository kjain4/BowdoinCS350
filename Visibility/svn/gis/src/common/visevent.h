
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


#ifndef _visevent_h_DEFINED
#define _visevent_h_DEFINED

#include <sys/types.h>
#include <stddef.h>

#include "gridpoint.h"

#define ENTER_EVENT 1
#define SIGHT_EVENT 0
#define LEAVE_EVENT -1

#define QUADRANT_I    1
#define QUADRANT_II   2
#define QUADRANT_III  4
#define QUADRANT_IV   8


typedef struct rad_swp_evt_t
{
  double angle;
  double dist;
  char type;
  GridPoint p;
} SweepEvent;


/**
 * Compare two SweepEvents, first by angle, then by distance, then by event
 * type.
 */
int compare_SweepEvent(SweepEvent a, SweepEvent b);

/**
 * Retrieve a human-readable name for a quadrant bitmask
 */
const char* quadrant_name(int quadrant);

/**
 * Determine what quadrant of a grid a point lies with respect to the viewpoint
 * as origin.  Quadrants are as mathematical quadrants of the xy-plane, with y
 * and rows equivalent, x and columns equivalent.
 *
 * A graphical example:
 *              |
 *     III      |      IV
 *              |
 *              |
 *  -----------VP-------------
 *              |
 *              |
 *     II       |      I
 *              |
 *
 * Points on the axes are more difficult.  By convention, all axis points are
 * associated with the quadrant following in clockwise order.
 */
int calculate_GridPoint_quadrant(GridPoint vp, GridPoint p);

/**
 * Calculate the angle at which a sweep line about the viewpoint would
 * intercept the given event.  The sweep line is assumed to progress in a
 * clockwise direction around the viewpoint.  Quadrants are determined by
 * calculate_GridPoint_quadrant().
 */
double calculate_SweepEvent_angle(SweepEvent ev, GridPoint vp);

/**
 * Calculate the tangent of the SweepEvent angle.  As the tangent is not unique
 * this methods requires specification of which quadrant the tangent is to be
 * calculated relative to.  For example, if quadrant I is specified, all points
 * within quadrant I will return tangents between zero and infinity.
 * Points in quadrant IV will return tangents less than zero, and those in
 * quadrants II and III will reflect those in IV and I, respectively.
 */
double calculate_SweepEvent_tangent(SweepEvent ev, GridPoint vp, int quadrant);


#endif /* _visevent_h_DEFINED */
