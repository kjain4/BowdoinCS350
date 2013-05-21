
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
#include "gridpoint.h"

int gp_equal(GridPoint p, GridPoint q)
{
  return p.r == q.r && p.c == q.c;
}

int gp_within(GridPoint p, dim_t r0, dim_t c0, dim_t r1, dim_t c1)
{
  return p.r >= r0 && p.c >= c0 && p.r < r1 && p.c < c1;
}

double gp_dist(GridPoint p, GridPoint q)
{
  return sqrt((q.r - p.r) * (q.r - p.r) + (q.c - p.c) * (q.c - p.c));
}

unsigned int gp_dist2(GridPoint p, GridPoint q)
{
  return (q.r - p.r) * (q.r - p.r) + (q.c - p.c) * (q.c - p.c);
}

