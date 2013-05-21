
/*
   A grid in ArcInfo Ascii Grid Format
 */


#ifndef __GRID_H
#define __GRID_H

#include <stdio.h>
#include <limits.h>



/* this should accomodate grid sizes up to 2^16-1=65,535
   If this is not enough, change type and recompile */
typedef unsigned short int dimensionType;
static const dimensionType maxDimension = USHRT_MAX - 1;


typedef struct grid_header {
    dimensionType ncols;  /*number of columns in the grid */
    dimensionType nrows;  /*number of rows in the grid */
    float xllcorner;	  /*xllcorner refers to the western edge of grid */
    float yllcorner;	  /*yllcorner refers to the southern edge of grid */
    float cellsize;		  /*the resolution of the grid */
    float nodata_value;   /*the value that represents missing data */
} GridHeader;



typedef struct grid_ {
    GridHeader *hd;

    /*two dimensional array holding all the values in the grid */
    float **grid_data;

    float minvalue;		/*the minimum value in the grid */
    float maxvalue;		/*the maximum value in the grid */
} Grid;




/* read the header of the grid stored in this file;*/
void read_header_from_arcascii_filename(GridHeader *hd, char* fname);

/*  read header of the grid stored in this file;*/
void read_header_from_arcascii_file(GridHeader *hd, FILE* fp);

/* create a new empty header */
GridHeader* create_empty_header(void);

/*copy from b to a */
void copy_header(GridHeader * a, GridHeader b);


/*print header */
void print_grid_header(GridHeader * hd);
void fprint_grid_header(FILE * fp, GridHeader * hd);


/*returns 1 if value is Nodata, 0 if it is not */
int is_header_nodata(GridHeader * hd, float value);

int is_nodata(Grid * grid, float value);
/* return 1 iff grid(i,j) is Nodata; 0 otherwise */
int is_nodata_at(Grid* grid, dimensionType i, dimensionType j);

/* return the value at (i,j) */
float get(Grid* grid, dimensionType i, dimensionType j);

/* set data[i][j] in teh grid to this value */
void set(Grid* g, dimensionType i, dimensionType j, float value);

/* set data[i][j] in teh grid to NODATA */
void set_nodata(Grid* g, dimensionType i, dimensionType j);


/* create and return an empty grid */
Grid *create_empty_grid(void);

/*allocate memory for grid data, grid must have a header */
void alloc_grid_data(Grid * grid);

/*scan an arcascii file and fill the information in the given structure */
Grid *read_grid_from_arcascii_file(char *filename);

/*destroy the structure and reclaim all memory allocated */
void destroy_grid(Grid * grid);

/*save grid into an arcascii file; Loops through all elements x in
  row-column order and writes fun(x) to file */
void save_grid_to_arcascii_file_fun(Grid * grid, char *filename,
				    float (*fun) (float));

/*save grid into an arcascii file; Loops through all elements x in
  row-column order and writes x to file */
void save_grid_to_arcascii_file(Grid * grid, char *filename);



#endif
