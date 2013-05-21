
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


#ifndef _graphics_h_DEFINED
#define _graphics_h_DEFINED

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/glut.h>
#endif


#include "datagrid.h"

typedef struct glut_window_t {
  int index;
  const char *title;
  int x, y;
  int width, height;
} GlutWindow;

typedef struct glColor3i_t {
  GLint r, g, b;
} GLColor;

// initialize and display a GLUT window
GlutWindow initialize_window(const char *title, int x, int y, int w, int h);

// calculate the data point indexes of pixels for each dimension in an image
index_t **build_xy_indexes(index_t nrow, index_t ncol, short w, short h);
// free built index arrays
void free_xy_indexes(index_t **isets);
// initialize an array mapping z indexes to colors
//   uses an array+len of color values.  this doesn't really provide much
//   efficiency over mod'ing the actual z by C{nmap}, multiplying by
//   C{ncol/nmap}, and indexing into C{colors}.  Provided to allow simple
//   non-function-based color mapping usinig the sanem interface.
GLColor *build_col_map_cols(int nmap, const GLColor *colors, int ncol);
// initialize an array mapping z indexes to colors
//   precalculates the color function for each z index and stores the values
//   in the returned C{z_map}
GLColor *build_col_map_func(int nmap, GLColor (*color_func)(int val));
// free the z mapping (equivalent to assert() & free())
void free_col_map(GLColor *col_map);

void display2d(void);
void display3d(void);


#endif

