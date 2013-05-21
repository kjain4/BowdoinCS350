
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
#include <stdlib.h>
#include <string.h>
#include "vis.h"

#include <stdio.h>

void test_visible_zeros_diag(void);
void test_visible_zeros_vert(void);
void test_visible_zeros_hor (void);

void test_visible_one_hor (void);
void test_visible_one_vert(void);
void test_visible_one_diag(void);

void test_visible_test2_up(void);
void test_visible_test2_diagup(void);
void test_visible_test2_nearvert(void);

void test_visible_test1_center(void);
void test_visible_test1_diag(void);
void test_visible_test1_offleft(void);

void test_brute_viewshed_zeros(void);
void test_brute_viewshed_zeros1(void);
void test_brute_viewshed_ones(void);
void test_brute_viewshed(const char *path);

int main(void)
{
  test_visible_zeros_hor();
  test_visible_zeros_vert();
  test_visible_zeros_diag();

  test_visible_one_hor();
  test_visible_one_vert();
  test_visible_one_diag();

  test_visible_test2_up();
  test_visible_test2_diagup();
  test_visible_test2_nearvert();

  test_visible_test1_center();
  test_visible_test1_diag();
  test_visible_test1_offleft();

  test_brute_viewshed_zeros();
  test_brute_viewshed_zeros1();

  test_brute_viewshed("../data/set1.asc");
  /*
  //test_brute_viewshed("../data/kaweah.asc");
  */

  return 0;
}

DataSet *init_zeros(void)
{
  const int n = 100;
  DataSet *dset = dInit(n, n, UCHAR);
  assert(dset);
  memset(dset->grid.ucData, 0, n * n * sizeof(unsigned char));
  dset->grid.ucNODATA = 2;
  return dset;
}

void test_visible_zeros_hor(void)
{
  DataSet *dset = init_zeros();
  GridPoint p = {0, 0};
  GridPoint q = {0, dset->grid.ncol - 1};
  float fNODATA = getNODATA(dset);

  assert(visible(dset, p, q, fNODATA));
  dFree(dset);
}

void test_visible_zeros_vert(void)
{
  DataSet *dset = init_zeros();
  GridPoint p = {0, 0};
  GridPoint q = {dset->grid.nrow - 1, 0};
  float fNODATA = getNODATA(dset);

  assert(visible(dset, p, q, fNODATA));
  dFree(dset);
}

void test_visible_zeros_diag(void)
{
  DataSet *dset = init_zeros();
  GridPoint p = {0, 0};
  GridPoint q = {dset->grid.nrow - 1, dset->grid.ncol - 1};
  float fNODATA = getNODATA(dset);

  assert(visible(dset, p, q, fNODATA));
  dFree(dset);
}

void test_visible_one_hor(void)
{
  DataSet *dset = init_zeros();
  GridPoint p = {0, 0};
  GridPoint q = {0, dset->grid.ncol - 1};
  float fNODATA = getNODATA(dset);
  
  dSet(dset, 0, dset->grid.ncol/2, 1);

  assert(!visible(dset, p, q, fNODATA));
  dFree(dset);
}

void test_visible_one_vert(void)
{
  DataSet *dset = init_zeros();
  GridPoint p = {0, 0};
  GridPoint q = {dset->grid.nrow - 1, 0};
  float fNODATA = getNODATA(dset);
  
  dSet(dset, dset->grid.nrow/2, 0, 1);

  assert(!visible(dset, p, q, fNODATA));
  dFree(dset);
}

void test_visible_one_diag(void)
{
  DataSet *dset = init_zeros();
  GridPoint p = {0, 0};
  GridPoint q = {dset->grid.nrow - 1, dset->grid.ncol - 1};
  float fNODATA = getNODATA(dset);
  
  dSet(dset, dset->grid.nrow/2, dset->grid.ncol/2, 1);

  assert(!visible(dset, p, q, fNODATA));
  dFree(dset);
}

void test_visible_test2_up(void)
{
  DataSet *dset = dLoad("test2.asc", UCHAR);
  assert(dset);

  GridPoint p = {dset->grid.nrow/2, dset->grid.ncol/2};
  GridPoint q = {1, dset->grid.ncol/2};
  float fNODATA = getNODATA(dset);

  assert(visible(dset, p, q, fNODATA));
  dFree(dset);
}

void test_visible_test2_diagup(void)
{
  DataSet *dset = dLoad("test2.asc", UCHAR);
  assert(dset);

  GridPoint p = {dset->grid.nrow/2, dset->grid.ncol/2};
  GridPoint q = {1, dset->grid.ncol-2};
  float fNODATA = getNODATA(dset);

  assert(visible(dset, p, q, fNODATA));
  dFree(dset);
}

void test_visible_test2_nearvert(void)
{
  DataSet *dset = dLoad("test2.asc", UCHAR);
  assert(dset);

  GridPoint p = {dset->grid.nrow/2, dset->grid.ncol/2};
  GridPoint q = {1, dset->grid.ncol/2 - 1};
  float fNODATA = getNODATA(dset);

  assert(visible(dset, p, q, fNODATA));
  dFree(dset);
}

void test_visible_test1_center(void)
{
  DataSet *dset = dLoad("test1.asc", UCHAR);
  assert(dset);

  GridPoint p = {dset->grid.nrow/2, dset->grid.ncol/2};
  GridPoint q = {1, dset->grid.ncol/2};
  float fNODATA = getNODATA(dset);

  assert(visible(dset, p, q, fNODATA));
  dFree(dset);
}

void test_visible_test1_diag(void)
{
  DataSet *dset = dLoad("test1.asc", UCHAR);
  assert(dset);

  GridPoint p = {dset->grid.nrow/2, dset->grid.ncol/2};
  GridPoint q = {dset->grid.nrow/2-3, dset->grid.ncol/2+3};
  float fNODATA = getNODATA(dset);

  assert(visible(dset, p, q, fNODATA));
  dFree(dset);
}

void test_visible_test1_offleft(void)
{
  DataSet *dset = dLoad("test1.asc", UCHAR);
  assert(dset);

  GridPoint p = {1, 1};
  GridPoint q = {5, 2};
  float fNODATA = getNODATA(dset);

  assert(!visible(dset, p, q, fNODATA));
  dFree(dset);
}

void print_viewshed(DataSet *viewshed)
{
  int r, c, v;
  for (r = 0; r < viewshed->grid.nrow; r++) {
    for (c = 0; c < viewshed->grid.ncol; c++) {
      dGet(viewshed, r, c, v, int);
      printf("%i", v);
    }
    printf("\n");
  }
}

void test_brute_viewshed_zeros(void)
{
  DataSet *terrain, *viewshed;
  GridPoint p;
  double *stats;

  terrain = init_zeros();
  p.r = 0;//terrain->grid.nrow / 2;
  p.c = 0;//terrain->grid.ncol / 2;

  viewshed = brute_viewshed(terrain, p);
  stats = gStatuc(&viewshed->grid, MINMAX);

  assert(stats[I_N] == terrain->grid.nrow * terrain->grid.ncol);
  
  free(stats);
  dFree(viewshed);
  dFree(terrain);
}

void test_brute_viewshed_zeros1(void)
{
  DataSet *terrain, *viewshed;
  GridPoint p;
  double *stats;

  terrain = init_zeros();
  p.r = terrain->grid.nrow / 2;
  p.c = terrain->grid.ncol / 2;

  viewshed = brute_viewshed(terrain, p);
  stats = gStatuc(&viewshed->grid, MINMAX);

  assert(stats[I_N] == terrain->grid.nrow * terrain->grid.ncol);

  free(stats);
  dFree(viewshed);
  dFree(terrain);
}

void test_brute_viewshed(const char *path)
{
  DataSet *terrain, *viewshed;
  GridPoint p;
  double *stats;

  terrain = dLoad(path, FLOAT);
  if (!terrain) {
    fprintf(stderr, "Skipping test_brute_viewshed for %s\n", path);
    return;
  }
  p.r = terrain->grid.nrow / 2;
  p.c = terrain->grid.ncol / 2;

  viewshed = brute_viewshed(terrain, p);
  stats = gStatuc(&viewshed->grid, MINMAX);
  printf("n = %i\n", (int)stats[I_N]);

  dFree(viewshed);
  dFree(terrain);
}
