
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


#include <stdio.h>
#include <assert.h>
#ifdef __APPLE__
#  include <stdlib.h>
#else
#  include <malloc.h>
#endif

#include "graphics.h"

GlutWindow initialize_window(const char *title, int x, int y, int w, int h)
{
  GlutWindow win;

  win.index = 0;
  win.title = title;
  win.x = x;
  win.y = y;
  win.width = w;
  win.height = h;
  

  glutInit(&win.index, NULL);

  glutInitWindowSize(w, h);
  glutInitWindowPosition(x, y);
  win.index = glutCreateWindow(win.title);


  glClearColor(0.8, 0.8, 0.8, 0.8);

  return win;
}

index_t **build_xy_indexes(index_t nrow, index_t ncol, short w, short h)
{
  double r, c, dr, dc;
  index_t *ip, *iend, **isets;

  // instantiate X and Y index grid
  isets = (index_t**) malloc(2 * sizeof(index_t*));
  assert(isets);
  isets[0] = (index_t*) malloc((w + 1) * sizeof(index_t));
  isets[1] = (index_t*) malloc((h + 1) * sizeof(index_t));
  assert(isets[0]); assert(isets[1]);

  dr = ((float)nrow) / h;
  dc = ((float)ncol) / w;

  // set width (col) indexes
  isets[0][0] = w;
  ip = &isets[0][1];
  iend = ip + w;
  for (c = 0; ip < iend; c+=dc)
    *ip++ = c < ncol - 1 ? c : ncol - 1;

  // set height (row) indexes
  isets[1][0] = h;
  ip = &isets[1][1];
  iend = ip + h;
  for (r = 0; ip < iend; r+=dr)
    *ip++ = r < nrow - 1 ? r : nrow - 1;
  /*
  r = nrow - 1; // reverse order
  for (r = nrow - 1; ip < iend; r-=dr)
    *ip++ = r > 0 ? r : 0;
    */

  return isets;
}

void free_xy_indexes(index_t **isets)
{
  assert(isets);
  assert(isets[0]);
  assert(isets[1]);

  free(isets[1]);
  free(isets[0]);
  free(isets);
}

GLColor *build_col_map_cols(int nmap, const GLColor *colors, int ncol)
{
  GLColor *col_map;
  GLint v;
  int i;

  col_map = (GLColor*) malloc((nmap + 1) * sizeof(GLColor));
  assert(col_map);

  col_map[0].r = nmap;
  for (i = 1; i <= nmap; i++) {
    v = ((i-1) % nmap) * ncol / nmap;
    col_map[i].r = v;
    col_map[i].g = v;
    col_map[i].b = v;
  }

  return col_map;
}

GLColor *build_col_map_func(int nmap, GLColor (*color_func)(int val))
{
  GLColor *col_map;
  int i;

  col_map = (GLColor*) malloc((nmap + 1) * sizeof(GLColor));
  assert(col_map);

  col_map[0].r = nmap;
  for (i = 1; i <= nmap; i++)
    col_map[i] = (*color_func)(i-1);

  return col_map;
}

void free_col_map(GLColor *col_map)
{
  assert(col_map);
  free(col_map);
}


/*********************
 * Display functions *
 *********************/


void draw2D(DataSet *dset, int x, int y, int w, int h, int offset,
            int **isets, GLColor *col_map, int fillmode)
{
  int i, j, ncols;
  index_t r0, r1, c0, c1;
  int v0, v1, v2, v3;
  GLColor *col0, *col1, *col2, *col3;

  if (fillmode) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  } else {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  }

  ncols = col_map[0].r;
  
  for (j = y; i < h - 1; j++) {
    r0 = isets[1][j];
    r1 = isets[1][j+1];
    for (i = x; i < w - 1; i++) {
      c0 = isets[0][i];
      c1 = isets[0][i];
      // load data values
      dGet(dset, r0, c0, v0, int);
      dGet(dset, r0, c1, v1, int);
      dGet(dset, r1, c1, v2, int);
      dGet(dset, r1, c0, v3, int);
      // apply offset
      v0 += offset;
      v1 += offset;
      v2 += offset;
      v3 += offset;
      // map colors
      col0 = &col_map[(v0 % ncols) + 1];
      col1 = &col_map[(v1 % ncols) + 1];
      col2 = &col_map[(v2 % ncols) + 1];
      col3 = &col_map[(v3 % ncols) + 1];

      // draw rectangle
      glBegin(GL_POLYGON);
      glColor3iv(&col0->r);
      glVertex2i(i, h - j);
      glColor3iv(&col1->r);
      glVertex2i(i + 1, h - j);
      glColor3iv(&col2->r);
      glVertex2i(i + 1, h - j - 1);
      glColor3iv(&col3->r);
      glVertex2i(i, h - j - 1);
      glEnd();
    }
  }
}

void draw3D(DataSet *dset, int x, int y, int z, int w, int h, int s,
            int offset, float scale, int **isets, GLColor *col_map,
            int fillmode)
{
  int i, j, ncols;
  index_t r0, r1, c0, c1;
  int v0, v1, v2, v3;
  int z0, z1, z2, z3;
  GLColor *col0, *col1, *col2, *col3;

  if (fillmode) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  } else {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  }

  ncols = col_map[0].r;
  
  for (j = y; i < h - 1; j++) {
    r0 = isets[1][j];
    r1 = isets[1][j+1];
    for (i = x; i < w - 1; i++) {
      c0 = isets[0][i];
      c1 = isets[0][i];
      // load data values
      dGet(dset, r0, c0, v0, int);
      dGet(dset, r0, c1, v1, int);
      dGet(dset, r1, c1, v2, int);
      dGet(dset, r1, c0, v3, int);
      // apply offset
      v0 += offset;
      v1 += offset;
      v2 += offset;
      v3 += offset;
      // map colors
      col0 = &col_map[(v0 % ncols) + 1];
      col1 = &col_map[(v1 % ncols) + 1];
      col2 = &col_map[(v2 % ncols) + 1];
      col3 = &col_map[(v3 % ncols) + 1];
      // map z values
      z0 = z + v0 * scale;
      z1 = z + v1 * scale;
      z2 = z + v2 * scale;
      z3 = z + v3 * scale;

      // draw triangle 1
      glBegin(GL_POLYGON);
      glColor3iv(&col0->r);
      glVertex3i(i, h - j, z);
      glColor3iv(&col1->r);
      glVertex3i(i + 1, h - j, z);
      glColor3iv(&col2->r);
      glVertex3i(i + 1, h - j - 1, z);
      glEnd();
      // draw triangle 1
      glBegin(GL_POLYGON);
      glColor3iv(&col1->r);
      glVertex3i(i + 1, h - j, z);
      glColor3iv(&col2->r);
      glVertex3i(i + 1, h - j - 1, z);
      glColor3iv(&col3->r);
      glVertex3i(i, h - j - 1, z);
      glEnd();
    }
  }
}

