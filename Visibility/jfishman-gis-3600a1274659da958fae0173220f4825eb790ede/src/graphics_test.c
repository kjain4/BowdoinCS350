
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
#include <stdio.h>
#include "datagrid.h"
#include "graphics.h"

#define DEBUG_PRINT 1

void test_xy_indexes(void);
void test_col_map_cols(void);
void test_col_map_func(void);

int main(int argc, const char **argv)
{

  test_xy_indexes();
  test_col_map_cols();
  test_col_map_func();

  return 0;
}

void test_xy_indexes(void)
{
  index_t **isets;
  int i;

  isets = build_xy_indexes(10000, 10000, 300, 200);
#if DEBUG_PRINT
  // print arrays
  for (i = 0; i < 11; i++)
    printf("%i ", isets[0][i]);
  printf("\n");
  for (i = 0; i < 11; i++)
    printf("%i ", isets[1][i]);
  printf("\n");
#endif
  // assert correct values
  assert(isets[0][0] == 300);
  for (i = 1; i < 11; i++)
    assert(isets[0][i] == (int)(100.0 / 3 * (i-1)));
  assert(isets[1][0] == 200);
  for (i = 1; i < 11; i++)
    assert(isets[1][i] == 50 * (i-1));
  // free arrays
  free_xy_indexes(isets);
}

void test_col_map_cols(void)
{
  int i;
  const GLColor colors[] = {{0,0,0}, {1,1,1}, {2,2,2}, {3,3,3}, {4,4,4}};
  GLColor *col_map;
  
  col_map = build_col_map_cols(30, &colors[0], 5);
#if DEBUG_PRINT
  // print array
  for (i = 0; i < 31; i++)
    printf("%i ", col_map[i].r);
  printf("\n");
#endif
  // assert correct values
  assert(col_map[0].r == 30);
  for (i = 1; i < 31; i++)
    assert(col_map[i].r == (i-1) * 5 / 30);
  // free array
  free_col_map(col_map);
}

GLColor color_func(int index)
{
  GLColor c;
  c.r = c.g = c.b = (index % 30) * 5 / 30;
  return c;
}

void test_col_map_func(void)
{
  int i;
  GLColor *col_map;
  
  col_map = build_col_map_func(30, color_func);
#if DEBUG_PRINT
  // print array
  for (i = 0; i < 31; i++)
    printf("%i ", col_map[i].r);
  printf("\n");
#endif
  // assert correct values
  assert(col_map[0].r == 30);
  for (i = 1; i < 31; i++)
    assert(col_map[i].r == (i-1) * 5 / 30);
  // free array
  free_col_map(col_map);
}
