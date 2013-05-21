
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


#ifndef _gridpoint_h_DEFINED
#define _gridpoint_h_DEFINED

#ifndef index_t
#  if __GLIBC_HAVE_LONG_LONG
typedef unsigned long long index_t;
#define DGI_FMT "%llu"
#  else
typedef unsigned int index_t;
#define DGI_FMT "%u"
#  endif
#endif

typedef unsigned int dim_t;

typedef struct datagrid_point_t {
  dim_t r, c;
} GridPoint;

int gp_equal(GridPoint p, GridPoint q);
int gp_within(GridPoint p, dim_t r0, dim_t c0, dim_t r1, dim_t c1);

double       gp_dist (GridPoint p, GridPoint q);
unsigned int gp_dist2(GridPoint p, GridPoint q);


#endif /* _gridpoint_h_DEFINED */
