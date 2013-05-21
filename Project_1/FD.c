#include "FD.h"

#define FD_DEBUG if(0)

//Main function
int main(int argc, char** argv, char** envp) {

  if(argc != 2) {
    exit(0);
  }

  //rtimer stuff
  Rtimer timer;

  //start the timer
  rt_start(timer);

  //make the FD map
  FD_Map* map = FD_Map_createFromFile(argv[0]);
  FD_DEBUG{printf("done making map\n"); fflush(stdout);}

  //fill up the FD map
  FD_fill(map);
  FD_DEBUG{printf("done filling the map\n"); fflush(stdout);}

  //output the map
  FD_Map_writeMap(*map, argv[0], argv[1]);
  FD_DEBUG{printf("done writing the map"); fflush(stdout);}

  //free up the map
  FD_Map_kill(map);

  //stop the timer
  rt_stop(timer);
  //print the timer
  //print the read timer
  char buf[1000];
  rt_sprint(buf, timer);
  printf("time for flow direction algorithm: %s\n", buf);

  //exit
  FD_DEBUG{printf("freed everything - exiting\n"); fflush(stdout);}
  exit(0);
}


//Construct, Output and Distroy
//create FD_Map from file
FD_Map* FD_Map_createFromFile(char* grid_path) {

  //create the new FD_Map
  FD_Map* new_FD_Map = (FD_Map*) malloc (sizeof(FD_Map));
  assert(new_FD_Map);

  //make the elev_map from the file
  new_FD_Map->b_map = B_Map_createFromFile(grid_path);

  //allocate the array
  new_FD_Map->fd_data = (short*) malloc(sizeof(short) * B_Map_getNRows(*new_FD_Map->b_map) * B_Map_getNCols(*new_FD_Map->b_map));
  assert(new_FD_Map->fd_data);

  return new_FD_Map;
}


//write the FD map to file.  in_path is passed so that we can copy the header from it
void FD_Map_writeMap(FD_Map map, char* in_path, char* out_path) {

  FD_DEBUG{printf("starting to write the FD map\n"); fflush(stdout);}
  //first open the files
  FILE* inFile = fopen(in_path, "r+");
  assert(inFile);

  FD_DEBUG{printf("opened the infile\n"); fflush(stdout);}

  FILE* outFile = fopen(out_path, "w+");
  assert(outFile);

  FD_DEBUG{printf("opened the outfile\n"); fflush(stdout);}

  //write the header (the first 6 lines) from inFile to outFile
  int i;
  char headerLine[40];
  for(i = 0; i < 6; i++) {
    fputs(fgets(headerLine, 40, inFile), outFile);
  }

  FD_DEBUG{printf("wrote the header\n"); fflush(stdout);}

  //now, go through and write all the recorded values
  int c, r;
  for(r = 0; r < B_Map_getNRows(*FD_Map_getBMap(map)); r++) {
    for(c = 0; c < B_Map_getNCols(*FD_Map_getBMap(map)); c++) {
      fprintf(outFile, "%hi ", FD_get_value(map, c, r));
    }
    fprintf(outFile, "\n");
  }
  FD_DEBUG{printf("done writing all the data\n"); fflush(stdout);}

}

//free up the FD_Map
void FD_Map_kill(FD_Map* map) {
  assert(map);

  //free up the elev_map
  B_Map_kill(FD_Map_getBMap(*map));

  //free up the fd_data
  free(map->fd_data);
}

//GETTERS --------------------------------------------------
B_Map* FD_Map_getBMap(FD_Map map) {
  assert(&map);

  return map.b_map;
}

short FD_get_value(FD_Map map, unsigned short i, unsigned short j) {
  assert(&map);

  return map.fd_data[i + j*B_Map_getNCols(*FD_Map_getBMap(map))];
}

//SETTERS ----------------------------------------------------------
void FD_Map_setValue(FD_Map* map, unsigned short i, unsigned short j, unsigned short value) {
  assert(map);

  map->fd_data[i + j*B_Map_getNCols(*FD_Map_getBMap(*map))] = value;
}




//WORKHORSE ----------------------------------------------------------
//actually does the work for the FD algorithm - passed a FD grid, fills the FD grid

/*fills the fd array with the following values depending on which direction is the one of steepest decent.
1  2  3
8  0  4
7  6  5
It looks at all neighbors, and compares them to the previously found steepest past of decent.  If no steepest decent is found, 0 is assigned.  If multiple points have the same elevation, each possible direction is stored in the "possibles" array.  If more than one value is found, the stored value is chosen at random.*/
void FD_fill(FD_Map* map) {
  assert(map);

  unsigned short possibles[8]; //stores all the possible steepest value;
  unsigned short numPossibles = 0; //keeps track of how many possible values there are.
  elev_type steepest; //the steepest evelation found so far.

  //set up the random stuff
  srand((unsigned int) time(NULL));

  int indexToStore; //if we need to store one of several directions, this is going to store which direction to store

  unsigned short c, r, i;
  for(c = 0; c < B_Map_getNCols(*FD_Map_getBMap(*map)); c++) {
    for(r = 0; r < B_Map_getNRows(*FD_Map_getBMap(*map)); r++) {
      //write the NODATA value if it's nodata in the map
      if(B_Map_getValue(*FD_Map_getBMap(*map), c, r) == B_Map_getNoDataValue(*FD_Map_getBMap(*map))) {
	FD_Map_setValue(map, c, r, B_Map_getNoDataValue(*FD_Map_getBMap(*map)));
	continue;
      }

      //make sure we reset the number of possibilites and steepest elevation
      numPossibles = 0;
      steepest = B_Map_getMaxElev(*FD_Map_getBMap(*map));

      //go through all the possible directions, and find the steepest decent
      for(i = 1; i < 8; i++) {
	//skip point if its a valid point
	if(FD_elevInDir(*FD_Map_getBMap(*map), c, r, i) == 
	   B_Map_getNoDataValue(*FD_Map_getBMap(*map))) {
	  continue;
	}

	//consider the value if it is lower than 
	if(FD_elevInDir(*FD_Map_getBMap(*map), c, r, i) < 
	   B_Map_getValue(*FD_Map_getBMap(*map), c, r)) {
	  //if it is lower than our current point, store it if it's lower than the lowest point so far
	  
	  if(FD_elevInDir(*FD_Map_getBMap(*map), c, r, i) < steepest) {
	    steepest = FD_elevInDir(*FD_Map_getBMap(*map), c, r, i);
	    possibles[0] = i;
	    numPossibles = 1;
	  }
	  
	  //if the value we're looking at equals the steepest value found so far, store it, and increment the number of possible steepest values
	  else if(FD_elevInDir(*FD_Map_getBMap(*map), c, r, i) == steepest) {
	    possibles[numPossibles] = i;
	    numPossibles++;
	  }
	}
      }

      //now that all possible directions have been considered, store values depending on how many possible answers there are
      if(numPossibles == 0) {
	//there are now points lower than this point, so we should store a 0 in fd to say stay in this point
	FD_Map_setValue(map, c, r, 0);
      }
      else if(numPossibles == 1) {
	//there is only one possible steepest direction, so store it
	FD_Map_setValue(map, c, r, possibles[0]);
      }
      else{
	//there is more than one possible steepest direction, so choose one randomly
	indexToStore = rand() % numPossibles;
	FD_Map_setValue(map, c, r, possibles[indexToStore]);
      }
    }
  }
}


//HELPERS ----------------------------------------------------------

//given a map, point and direction, return the elev at the point in that directino
elev_type FD_elevInDir(B_Map map, unsigned short c, unsigned short r, unsigned short dir) {
  switch(dir) {
  case 1:
    if(c != 0 && r != 0)
      return B_Map_getValue(map, c-1, r-1);
    else 
      return B_Map_getNoDataValue(map);
    break;
  case 2:
    if(r != 0)
      return B_Map_getValue(map, c, r-1);
    else
      return B_Map_getNoDataValue(map);
    break;
  case 3:
    if(c != B_Map_getNCols(map)-1 && r != 0)
      return B_Map_getValue(map, c+1, r-1);
    else
      return B_Map_getNoDataValue(map);
    break;
  case 4:
    if(c != B_Map_getNCols(map)-1)
      return B_Map_getValue(map, c+1, r);
    else
      return B_Map_getNoDataValue(map);
    break;
  case 5:
    if(c != B_Map_getNCols(map)-1 && r != B_Map_getNRows(map)-1)
      return B_Map_getValue(map, c+1, r+1);
    else
      return B_Map_getNoDataValue(map);
    break;
  case 6:
    if(r != B_Map_getNRows(map)-1)
      return B_Map_getValue(map, c, r+1);
    else
      return B_Map_getNoDataValue(map);
  case 7:
    if(c != 0 && r != B_Map_getNRows(map)-1)
      return B_Map_getValue(map, c-1, r+1);    
    else
      return B_Map_getNoDataValue(map);
    break;
  case 8:
    if(c != 0)
      return B_Map_getValue(map, c-1, r);
    else
      return B_Map_getNoDataValue(map);
    break;
  }
  return B_Map_getNoDataValue(map);
}
