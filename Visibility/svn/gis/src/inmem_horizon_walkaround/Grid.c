/* Will Richard
 * Walkaround Visibility Algorithm
 * Grid.c
 */

#include "Grid.h"

//CONSTURCT AND DISTROY --------------------------------------------------------
//Create and recturn a new grid, with the data array alloctade but not filled
Grid* Grid_new(unsigned short num_c, unsigned short num_r, short no_data_value) {
  //create the new grid
  Grid* newGrid = (Grid*) malloc(sizeof(Grid));
  assert(newGrid);

  //store the values passed
  newGrid -> ncols = num_c;
  newGrid->nrows = num_r;
  newGrid->NODATA = no_data_value;

  //allocate the array
  newGrid->data = (Point**) malloc(sizeof(Point*) * num_c);
  assert(newGrid->data);
  int i;
  for(i = 0; i < num_c; i++) {
    newGrid->data[i] = (Point*) malloc(sizeof(Point)* num_r);
    assert(newGrid->data[i]);
  }

  return newGrid;
}

//create a grid from the file at the passed path
Grid* Grid_createFromFile(char* grid_path) {
  assert(grid_path);

  //open the file
  FILE* gridFile = fopen(grid_path, "r+");
  assert(gridFile);

  //allocate and fill the header
  int header[6];
  Grid_readHeader(gridFile, header);

  //make the grid
  Grid* newGrid = Grid_new((unsigned short)header[0], (unsigned short)header[1],
			   (short) header[5]);
  
  //start reading in values
  unsigned short c, r;
  short value;
  for(r = 0; r < Grid_getNRows(*newGrid); r++) {
    for(c = 0; c < Grid_getNCols(*newGrid); c++) {
      //read in the short value
      fscanf(gridFile, "%hd", &value);
      Grid_setPoint(newGrid, c, r, value);
    }
  }

  fclose(gridFile);

  return newGrid;
}


//free the grid
void Grid_kill(Grid* grid) {
  assert(grid);
  
  //free each row of the array
  int i;
  for(i = 0; i < Grid_getNCols(*grid); i++) {
    free(grid->data[i]);
  }

  //free the arary now
  free(grid->data);

  //free the grid
  free(grid);
}

//GETTERS ----------------------------------------------------------------------
unsigned short Grid_getNCols(Grid grid) { return grid.ncols; }
unsigned short Grid_getNRows(Grid grid) { return grid.nrows; }
short Grid_getNoDataValue(Grid grid) { return grid.NODATA; }
//return a pointer to the point at i,j
Point* Grid_getPoint(Grid* grid, int i, int j) {
  assert(i < Grid_getNCols(*grid));
  assert(j < Grid_getNRows(*grid));
  assert(i >= 0);
  assert(j >= 0);

  return &(grid->data[i][j]);
}

//SETTERS ----------------------------------------------------------------------
//set the value at the point c,r.
void Grid_setPoint(Grid* grid, int c, int r, short elev) {
  assert(grid);
  Point_setElev(Grid_getPoint(grid, c, r), elev);
}

//HELPERS ----------------------------------------------------------------------
/* returns the passed int[6] which has all the header data from <file> where:
 * int[0] = ncols
 * int[1] = nrows
 * int[2] = xllcorner
 * int[3] = yllcorner
 * int[4] = cellsize
 * int[6] = NODATA_value
 */
int* Grid_readHeader(FILE* gridFile, int* header) {
  assert(gridFile);


  //going to grab onto one line at a time of the header, and parse it
  char headerLine[40];
  //the 14th character (character number 13) is always the value
  //first line in ncols
  if(fgets(headerLine, 40, gridFile) != NULL) {
    sscanf(&headerLine[13], "%d", &header[0]);
  }
  //2nd line is nrows
  if(fgets(headerLine, 40, gridFile) != NULL) {
    sscanf(&headerLine[13], "%d", &header[1]);
  }
  //3rd line is xllcorner
  if(fgets(headerLine, 40, gridFile) != NULL) {
    sscanf(&headerLine[13], "%d", &header[2]);
  }
  //4th line is yllcorner
  if(fgets(headerLine, 40, gridFile) != NULL) {
    sscanf(&headerLine[13], "%d", &header[3]);
  }
  //5th line is cellsize
  if(fgets(headerLine, 40, gridFile) != NULL) {
    sscanf(&headerLine[13], "%d", &header[4]);
  }
  //6th line is NODATA
  if(fgets(headerLine, 40, gridFile) != NULL) {
    sscanf(&headerLine[13], "%d", &header[5]);
  }

  return header;
}

//Output the visibility information of <grid> to <outputPath>
void Grid_outputVisGrid(Grid* grid, char* outputPath, char* inputPath)  {
  assert(grid);
  assert(outputPath);
  assert(inputPath);

  //open the files
  FILE* inputFile = fopen(inputPath, "r");
  assert(inputFile);

  FILE* outputFile = fopen(outputPath, "w");
  assert(outputFile);

  //write the header, i.e. the first 6 lines of inputFile to outputFile
  int i;
  char headerLine[100];
  for(i = 0; i < 6; i++) {
    fputs(fgets(headerLine, 100, inputFile), outputFile);
  }
  //done with the input file, so close it.
  int error = fclose(inputFile);
  assert(error==0); 

  //now, go through all the recorded values and record the visibility data
  int c, r;
  for(r = 0; r < Grid_getNRows(*grid); r++) {
  for(c = 0; c < Grid_getNCols(*grid); c++) {
      
      
      //if the point is NODATA, write a NODATA value
      if(Point_getElev(*Grid_getPoint(grid, c, r))==Grid_getNoDataValue(*grid)){
      fprintf(outputFile, "%hi ", Grid_getNoDataValue(*grid));
      }
      //otherwise, print the visibility value
      else {
      fprintf(outputFile, "%hi ", Point_getVis(*Grid_getPoint(grid, c, r)));
      }
   }
    //at the end of every row, write a newline
    fprintf(outputFile, "\n");
  }
  
  assert(outputFile); 
  error = fclose(outputFile);
  assert(error ==0);
}
