
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <assert.h>

#include "grid.h"


/* ------------------------------------------------------------ */
/*read header from file; */
void read_header_from_arcascii_filename(GridHeader* hd, char* fname)
{
  FILE *fp;

  assert(fname && hd);
  fp = fopen(fname, "r");
  assert(fp);
  read_header_from_arcascii_file(hd, fp);
  fclose(fp);
}



/* ------------------------------------------------------------ */
/*read header from file  */
void read_header_from_arcascii_file(GridHeader* hd, FILE* fp)
{
  int nrows, ncols;
  int result;

  assert(fp && hd);
  rewind(fp);

  result = fscanf(fp, "%*s%d\n", &ncols);
  assert(result == 1);
  result = fscanf(fp, "%*s%d\n", &nrows);
  assert(result == 1);
  /*check that you dont lose precision */
  if (nrows <= maxDimension && ncols <= maxDimension) {
    hd->nrows = (dimensionType) nrows;
    hd->ncols = (dimensionType) ncols;
  }
  else {
    fprintf(stderr, "grid dimension too big for current precision\n");
    fprintf(stderr, "  nrows=%i ncols=%i\n", nrows, ncols);
    printf("change type and re-compile\n");
    exit(1);
  }
  result = fscanf(fp, "%*s%f\n", &(hd->xllcorner));
  assert(result == 1);
  result = fscanf(fp, "%*s%f\n", &(hd->yllcorner));
  assert(result == 1);
  result = fscanf(fp, "%*s%f\n", &(hd->cellsize));
  assert(result == 1);
  result = fscanf(fp, "%*s%f\n", &(hd->nodata_value));
  assert(result == 1);

}



/* ------------------------------------------------------------ */
GridHeader* create_empty_header() {
  GridHeader *hd = (GridHeader *) malloc(sizeof(GridHeader));
  assert(hd);
  hd->ncols=hd->nrows=0;
  hd->cellsize=0;
  hd->nodata_value=0;
  return hd;
}


/* ------------------------------------------------------------ */
/*copy from b to a */
void copy_header(GridHeader * a, GridHeader b)
{
    assert(a);
    a->nrows = b.nrows;
    a->ncols = b.ncols;
    a->xllcorner = b.xllcorner;
    a->yllcorner = b.yllcorner;
    a->cellsize = b.cellsize;
    a->nodata_value = b.nodata_value;
    return;
}


/* ------------------------------------------------------------ */
/*print header */
void print_grid_header(GridHeader * hd)
{

    assert(hd);
    fprint_grid_header(stdout, hd);
    return;
}


/* ------------------------------------------------------------ */
void fprint_grid_header(FILE * fp, GridHeader * hd)
{
    assert(fp && hd);
    fprintf(fp, "ncols\t%d\n", hd->ncols);
    fprintf(fp, "nrows\t%d\n", hd->nrows);
    fprintf(fp, "xllcorner\t%f\n", hd->xllcorner);
    fprintf(fp, "yllcorner\t%f\n", hd->yllcorner);
    fprintf(fp, "cellsize\t%f\n", hd->cellsize);
    fprintf(fp, "NODATA_value\t%f\n", hd->nodata_value);
    return;
}




/* ------------------------------------------------------------ */
/*returns 1 if value is Nodata, 0 if it is not */
int is_header_nodata(GridHeader * hd, float value)
{
    assert(hd);
    if (fabs(value - hd->nodata_value) < 0.000001) {
	return 1;
    }
    return 0;
}


/* ------------------------------------------------------------ */
/*returns 1 if value is Nodata, 0 if it is not */
int is_nodata(Grid * grid, float value)
{
    assert(grid);
    return is_header_nodata(grid->hd, value);
}


/* ------------------------------------------------------------ */
/* return 1 iff grid(i,j) is Nodata; 0 otherwise */
int is_nodata_at(Grid* grid, dimensionType i, dimensionType j) {

  assert(grid && i< grid->hd->nrows && j<grid->hd->ncols);
  return is_header_nodata(grid->hd, grid->grid_data[i][j]);
}


/* ------------------------------------------------------------ */
/* return the value at (i,j) */
float get(Grid* grid, dimensionType i, dimensionType j) {

  assert(grid && i< grid->hd->nrows && j<grid->hd->ncols);
  return grid->grid_data[i][j];
}


/* ------------------------------------------------------------ */
/* set data[i][j] in the grid to this value */
void set(Grid* grid, dimensionType i, dimensionType j, float value) {
  assert(grid && i< grid->hd->nrows && j<grid->hd->ncols);
  grid->grid_data[i][j] = value;
}


/* ------------------------------------------------------------ */
/* set data[i][j] in the grid to NODATA */
void set_nodata(Grid* grid, dimensionType i, dimensionType j) {
  assert(grid && i< grid->hd->nrows &&  j<grid->hd->ncols);
  grid->grid_data[i][j] = grid->hd->nodata_value;
}



/* ------------------------------------------------------------ */
/* create an empty grid with an empty header and return it. The header
   is all 0 and data is set to NULL.  */
Grid *create_empty_grid()
{
  Grid *ptr_grid = (Grid *) malloc(sizeof(Grid));
  assert(ptr_grid);

  /*initialize structure */
  ptr_grid->hd = create_empty_header();
  ptr_grid->grid_data = NULL;

#ifdef _DEBUG_ON
  printf("**DEBUG: createEmptyGrid \n");
  fflush(stdout);
#endif

    return ptr_grid;
}




/* ------------------------------------------------------------ */
/* allocate memroy for grid_data; grid must have a header that gives
   the dimensions */
void alloc_grid_data(Grid * pgrid)
{
    dimensionType i;

    assert(pgrid);
    assert(pgrid->hd);
    pgrid->grid_data = (float **)malloc(pgrid->hd->nrows * sizeof(float *));
    assert(pgrid->grid_data);

    for (i = 0; i < pgrid->hd->nrows; i++) {
      pgrid->grid_data[i] =
	(float *)malloc(pgrid->hd->ncols * sizeof(float));
      assert(pgrid->grid_data[i]);
    }

#ifdef _DEBUG_ON
    printf("**DEBUG: allocGridData\n");
    fflush(stdout);
#endif
    return;
}

/* ------------------------------------------------------------ */
/*reads header and data from file */
Grid *read_grid_from_arcascii_file(char *filename)
{
    FILE *fp;
    Grid *grid;
    dimensionType i, j;
    int first_flag, result;
    float value;

    assert(filename);
    fp = fopen(filename, "r");
    assert(fp);

    grid = create_empty_grid();

    read_header_from_arcascii_file(grid->hd, fp);
    alloc_grid_data(grid);

    /*READ DATA */
    first_flag = 1;
    value = 0;

    for (i = 0; i < grid->hd->nrows; i++) {
	for (j = 0; j < grid->hd->ncols; j++) {
	    result = fscanf(fp, "%f", &value);
		assert(result == 1);
	    grid->grid_data[i][j] = value;

	    if (first_flag) {
		if (is_nodata(grid, value))
		    continue;
		grid->minvalue = grid->maxvalue = value;
		first_flag = 0;
	    }
	    else {
		if (is_nodata(grid, value))
		    continue;
		if (value > grid->maxvalue)
		    grid->maxvalue = value;
		if (value < grid->minvalue)
		    grid->minvalue = value;
	    }
	}
	result = fscanf(fp, "\n");
	assert(result == 0);
    }

    fclose(fp);

#ifdef DEBUG_ON
    printf("**DEBUG: readGridFromArcasciiFile():\n");
    fflush(stdout);
#endif

    return grid;
}





/* ------------------------------------------------------------ */
/*destroy the structure and reclaim all memory allocated */
void destroy_grid(Grid * grid)
{
    assert(grid);
    /*free grid data if its allocated */
    if (grid->grid_data) {
      dimensionType i;

      for (i = 0; i < grid->hd->nrows; i++) {
	if (!grid->grid_data[i])
	  free((float *)grid->grid_data[i]);
      }

      free((float **)grid->grid_data);
    }

    assert(grid->hd);
    free(grid->hd);
    free(grid);

#ifdef _DEBUG_ON
    printf("**DEBUG: grid destroyed.\n");
    fflush(stdout);
#endif

    return;
}





/* ------------------------------------------------------------ */
/*save the grid into an arcascii file.  Loops through all elements x
  in row-column order and writes fun(x) to file */
void
save_grid_to_arcascii_file(Grid * grid, char *filename) {

  FILE *outfile, *fp;
  int ret;
  dimensionType i, j;

  assert(filename && grid);
  printf("saving grid to %s\n", filename);
  fflush(stdout);

  outfile = fopen(filename, "r");

  if (outfile) {		/*outfile already exists */
    printf("The output file already exists. It will be overwritten\n");
    fclose(outfile);
    ret = remove(filename);	/*delete the existing file */

    if (ret != 0) {
      printf("unable to overwrite the existing output file.\n!");
      exit(1);
    }
  }
  fp = fopen(filename, "a");
  assert(fp);

  /*print header */
  fprint_grid_header(fp, grid->hd);

  /*print data */
  for (i = 0; i < grid->hd->nrows; i++) {
    for (j = 0; j < grid->hd->ncols; j++) {

      /*  call fun() on this element and write it to file  */
      fprintf(fp, "%.1f ",  grid->grid_data[i][j]);
    }
    fprintf(fp, "\n");
  }

#ifdef _DEBUG_ON
  printf("**DEBUG: saveGridToArcasciiFile: saved to %s\n", filename);
  fflush(stdout);
#endif

  return;
}



/* ------------------------------------------------------------ */
/*save the grid into an arcascii file.  Loops through all elements x
  in row-column order and writes fun(x) to file */
void
save_grid_to_arcascii_file_fun(Grid * grid, char *filename,
			   float(*fun)(float)) {
  FILE *outfile, *fp;
  int ret;
  dimensionType i, j;

  assert(filename && grid);
  printf("saving grid to %s\n", filename);
  fflush(stdout);

  outfile = fopen(filename, "r");

  if (outfile) {		/*outfile already exists */
    printf("The output file already exists. It will be overwritten\n");
    fclose(outfile);
    ret = remove(filename);	/*delete the existing file */

    if (ret != 0) {
      printf("unable to overwrite the existing output file.\n!");
      exit(1);
    }
  }
  fp = fopen(filename, "a");
  assert(fp);

  /*print header */
  fprint_grid_header(fp, grid->hd);

  /*print data */
  for (i = 0; i < grid->hd->nrows; i++) {
    for (j = 0; j < grid->hd->ncols; j++) {

      /*  call fun() on this element and write it to file  */
      fprintf(fp, "%.1f ",  fun(grid->grid_data[i][j]) );

    }
    fprintf(fp, "\n");
  }

#ifdef _DEBUG_ON
  printf("**DEBUG: saveGridToArcasciiFile: saved to %s\n", filename);
  fflush(stdout);
#endif

  return;
}
