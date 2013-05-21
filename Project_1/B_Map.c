#include "Elev_type.h"
#include "B_Map.h"

#define MAP_DEBUG if(0)

//Construct and Distroy ------------------------------------------------
//create and return a new map, with the data array allocated but not filled
B_Map* B_Map_new(int num_c, int num_r, elev_type no_data_value){

  //create the new map
  B_Map* newB_Map = (B_Map*) malloc (sizeof(B_Map));
  assert(newB_Map);

  //store the values passed
  newB_Map->ncols = num_c;
  newB_Map->nrows = num_r;
  newB_Map->NODATA = no_data_value;
  newB_Map->minElev = (elev_type) 0;
  newB_Map->maxElev = (elev_type) 0;
/*   newB_Map->maxFlow = 0; */

  //allocate the array
  newB_Map->elev_data = (elev_type*) malloc(sizeof(elev_type) * (num_c*num_r));
  assert(newB_Map->elev_data);

  return newB_Map;
}

//create a map from file
B_Map* B_Map_createFromFile(char* grid_path) {
  assert(grid_path);
  MAP_DEBUG{printf("starting to create B_Map from file\n"); fflush(stdout);}

  FILE* gridFile = fopen(grid_path, "r+");
  assert(gridFile);

  float header[6];
  B_Map_readHeader(gridFile, header);

  MAP_DEBUG{printf("header is %f, %f, %f, %f, %f, %f\n", header[0], header[1], header[2], header[3], header[4], header[5]); fflush(stdout);}

  //we have all we need to make a map, so lets make it
  B_Map* newB_Map = B_Map_new(header[0], header[1], header[5]);

  //start reading in values.  
  unsigned short c, r;
  //We'll use the float temp, and cast it to elev_type for storage
  float temp;
  elev_type value;
  //we need to set the max and min elev values the first time through, so this will make sure that is done
  int firstTime;
  firstTime = 1;

  //start reading
  for(r = 0; r < B_Map_getNRows(*newB_Map); r++) {
    for(c = 0; c < B_Map_getNCols(*newB_Map); c++) {
      //read in the value
      fscanf(gridFile, "%f", &temp);
      value = (elev_type) temp;
      //store the value
      B_Map_setValue(newB_Map, c, r, value);
      //check if min or max elev need no be updated, and do so accordingly if its a valid point
      if(value != B_Map_getNoDataValue(*newB_Map)) {
	if(value < B_Map_getMinElev(*newB_Map) || firstTime) {
	  B_Map_setMinElev(newB_Map, value);
	}
	if(value > B_Map_getMaxElev(*newB_Map) || firstTime) {
	  B_Map_setMaxElev(newB_Map, value);
	}
	if(firstTime) {
	  firstTime = 0;
	}
      }
    }
  }

  fclose(gridFile);

  return(newB_Map);
}

//free up the map
void B_Map_kill(B_Map* map) {
  assert(map);
  free(map->elev_data);

  //now free the map itself
  free(map);
}

//HELPERS --------------------------------------------------
/*fills the passed float[6] which has all the header data, where
 float[0] = ncols, float[1] = nrows, float[2] = xllcorner, float[3] = yllcorner, float[4] = cellsize and float[5] = NODATA_value
 after this is called, the data from the file is ready to be read*/
void B_Map_readHeader(FILE* gridFile, float headerData[6]) {
  assert(gridFile);
  assert(headerData);

  //going to grab one line at a time of the header, and parse it.
  char headerLine[40];
  //for non-float values, we will grab it in its form, than convert to float for storage
  unsigned short temp;
  //the 14th character (character number 13) is always where the value is
  //first line is ncols
  if(fgets(headerLine, 40, gridFile) != NULL){
    sscanf(&headerLine[13], "%hu", &temp);
    headerData[0] = (float) temp;
  }
  //2nd line is nrows
  if(fgets(headerLine, 40, gridFile) != NULL){
    sscanf(&headerLine[13], "%hu", &temp);
    headerData[1] = (float) temp;
  }
  //3rd line is xllcorner
  if(fgets(headerLine, 40, gridFile) != NULL){
    sscanf(&headerLine[13], "%f", &headerData[2]);
  }
  //4th line is yllcorner
  if(fgets(headerLine, 40, gridFile) != NULL){
    sscanf(&headerLine[13], "%f", &headerData[3]);
  }
  //5th line is cellsize
  if(fgets(headerLine, 40, gridFile) != NULL){
    sscanf(&headerLine[13], "%f", &headerData[4]);
  }
  //6th and last line of header is NODATA
  if(fgets(headerLine, 40, gridFile) != NULL){
    sscanf(&headerLine[13], "%f", &headerData[5]);
  }
}




//GETTERS --------------------------------------------------
unsigned short B_Map_getNRows(B_Map map) {
  assert(&map);
  return map.nrows;
}

unsigned short B_Map_getNCols(B_Map map) {
  assert(&map);
  return map.ncols;
}

elev_type B_Map_getNoDataValue(B_Map map) {
  assert(&map);
  return map.NODATA;
}

elev_type B_Map_getMinElev(B_Map map) {
  assert(&map);
  return map.minElev;
}

elev_type B_Map_getMaxElev(B_Map map) {
  assert(&map);
  return map.maxElev;
}

/* unsigned short B_Map_getMaxFlow(B_Map map) { */
/*   assert(&map); */
/*   return map.maxFlow; */
/* } */


//returns the value stored at i,j
elev_type B_Map_getValue(B_Map map, unsigned short i, unsigned short j) {
  assert(&map);
  return map.elev_data[i+j*B_Map_getNCols(map)];
}

//SETTERS ----------------------------------------------------------
//set the value at i, j
void B_Map_setValue(B_Map* map, int i, int j, elev_type value) {
  assert(map);
  map->elev_data[i+j*B_Map_getNCols(*map)] = value;
}

//set the min and max elev
void B_Map_setMinElev(B_Map* map, elev_type value) {
  assert(map);
  map->minElev = value;
}
void B_Map_setMaxElev(B_Map* map, elev_type value) {
  assert(map);
  map->maxElev = value;
}
