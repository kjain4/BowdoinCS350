#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <assert.h>


#include "grid.h"


/* ------------------------------------------------------------ */
/*read header from file and return it; */
GridHeader *read_header_from_arcascii_file(char* fname)
{

  assert(fname);
  FILE *fp;
  fp = fopen(fname, "r");
  if (!fp) {
	printf("could not open file %s\n", fname);
	exit(1);
  }
  GridHeader  *hd = fread_header_from_arcascii_file(fp); 
  fclose(fp); 
  return hd;
}



/* ------------------------------------------------------------ */
/*read header from file and return it; */
GridHeader *fread_header_from_arcascii_file(FILE* fp)
{
    assert(fp);
	rewind(fp); 

    GridHeader *hd = (GridHeader *) malloc(sizeof(GridHeader));
    assert(hd);

    int nrows, ncols;

    fscanf(fp, "%*s%d\n", &ncols);
    fscanf(fp, "%*s%d\n", &nrows);
    /*check that you dont lose precision */
    if (nrows <= maxDimension && ncols <= maxDimension) {
	hd->nrows = (dimensionType) nrows;
	hd->ncols = (dimensionType) ncols;
    }
    else {
	fprintf(stderr, "grid dimension too big for current precision\n");
	printf("change type and re-compile\n");
	exit(1);
    }
    fscanf(fp, "%*s%f\n", &(hd->xllcorner));
    fscanf(fp, "%*s%f\n", &(hd->yllcorner));
    fscanf(fp, "%*s%f\n", &(hd->cellsize));
    fscanf(fp, "%*s%f\n", &(hd->nodata_value));

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
int is_nodata(GridHeader * hd, float value)
{
    assert(hd);
    if (fabs(value - hd->nodata_value) < 0.000001) {
	return 1;
    }
    return 0;
}

/* ------------------------------------------------------------ */
/*returns 1 if value is Nodata, 0 if it is not */
int is_nodata_grid(Grid * grid, float value)
{
    assert(grid);
	return is_nodata(grid->hd, value);
}



/* ------------------------------------------------------------ */
/* create an empty grid and return it. The header and the data are set
   to NULL.  */
Grid *create_empty_grid()
{

    Grid *ptr_grid = (Grid *) malloc(sizeof(Grid));
    assert(ptr_grid);

    /*initialize structure */
    ptr_grid->hd = NULL;
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
    assert(pgrid);
    assert(pgrid->hd);

    pgrid->grid_data = (float **)malloc(pgrid->hd->nrows * sizeof(float *));
    assert(pgrid->grid_data);

    dimensionType i;

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

    assert(filename);
    FILE *fp = fopen(filename, "r");

    assert(fp);

    Grid *grid = create_empty_grid();

    grid->hd = fread_header_from_arcascii_file(fp);
    alloc_grid_data(grid);

    /*READ DATA */
    dimensionType i, j;
    int first_flag = 1;
    float value = 0;

    for (i = 0; i < grid->hd->nrows; i++) {
	for (j = 0; j < grid->hd->ncols; j++) {
	    fscanf(fp, "%f", &value);
	    grid->grid_data[i][j] = value;

	    if (first_flag) {
		if (is_nodata_grid(grid, value))
		    continue;
		grid->minvalue = grid->maxvalue = value;
		first_flag = 0;
	    }
	    else {
		  if (is_nodata_grid(grid, value))
		    continue;
		  if (value > grid->maxvalue)
		    grid->maxvalue = value;
		  if (value < grid->minvalue)
		    grid->minvalue = value;
	    }
	}
	fscanf(fp, "\n");
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
save_grid_to_arcascii_file(Grid * grid, char *filename,
						   float(*fun)(float)) {

  assert(filename && grid);
  printf("saving grid to %s\n", filename);
  fflush(stdout);
  
  FILE *outfile = fopen(filename, "r");
  if (outfile) {		/*outfile already exists */
	printf("The output file already exists. It will be overwritten\n");
	fclose(outfile);
	int ret = remove(filename);	/*delete the existing file */
	
	if (ret != 0) {
	  printf("unable to overwrite the existing output file.\n!");
	  exit(1);
	}
  }
  FILE *fp = fopen(filename, "a");
  assert(fp);
  
  /*print header */
  fprint_grid_header(fp, grid->hd);
  
  /*print data */
  dimensionType i, j;
  
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
