
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


#ifndef _radial2_vis_sweep_h_DEFINED
#define _radial2_vis_sweep_h_DEFINED

#include "gridpoint.h"
#include "datagrid.h"
#include "visevent.h"

DataSet* radial2_viewshed_terrain(DataSet *terrain);
DataSet* radial2_viewshed(DataSet *terrain, GridPoint vp);
int radial2_viewshed_cnt(Grid terrain, GridPoint vp);
int radial2_viewshed_quadrant(Grid *terrain, GridPoint vp, int quadrant,
                         Grid *viewshed);

#endif /* _radial2_vis_sweep_h_DEFINED */
