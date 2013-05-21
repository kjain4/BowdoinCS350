#include "FA.h"

//Main function
int main(int argc, char** argv, char** envp) {

  if(argc != 3) {
    exit(0);
  }

  //rtimer stuff
  Rtimer timer;

  //start the timer
  rt_start(timer);

  //make the FA map
  FA_Map* map = FA_Map_createFromFile(argv[0], argv[1]);

  //fill up the FA map
  FA_fill(map);

  //output the FA Map
  FA_Map_writeMap(*map, argv[0], argv[2]);

  //free up the map
  FA_Map_kill(map);

  //stop the timer
  rt_stop(timer);
  //print the timer
  //print the read timer
  char buf[1000];
  rt_sprint(buf, timer);
  printf("time for flow accumulation algorithm: %s\n", buf);

  //exit
  exit(0);
}


//Construct and Distroy ------------------------------------------------
//create and return a new FA_Map, with its fa_data array allocated but not filled
FA_Map* FA_Map_new(unsigned short num_c, unsigned short num_r, elev_type no_data_value) {

  //create the new FA_Map
  FA_Map* new_FA_Map = (FA_Map*) malloc(sizeof(FA_Map));
  assert(new_FA_Map);

  //store the passed values
  new_FA_Map->ncols = num_c;
  new_FA_Map->nrows = num_r;
  new_FA_Map->NODATA = no_data_value;

  //allocate and initialze the data array
  new_FA_Map->fa_data = (FA_Point**) malloc(sizeof(FA_Point*) * num_c * num_r);
  assert(new_FA_Map->fa_data);

  int i;
  for(i = 0; i < num_c*num_r; i++) {
    new_FA_Map->fa_data[i] = (FA_Point*) malloc(sizeof(FA_Point));
    assert(new_FA_Map->fa_data[i]);
  }

  return new_FA_Map;

}

//create an FA_Map from file
FA_Map* FA_Map_createFromFile(char* grid_path, char* fd_path) {
  assert(grid_path);

   FILE* gridFile = fopen(grid_path, "r+");
  assert(gridFile);

  //read the data out of the header
  float header[6];
  B_Map_readHeader(gridFile, header);

  //we have the data to make the map, so make the FA_map
  FA_Map* newFA_Map = FA_Map_new((unsigned short) header[0], (unsigned short) header[1], (elev_type) header[5]);

  //open up the FD array, so we can traverse both at the same time
  FILE* fdFile = fopen(fd_path, "r+");
  assert(fdFile);
  //also, read the header from the fdFile, so it is lined up at the same place
  B_Map_readHeader(fdFile, header);

  //start reading in values
  unsigned short c, r;
  //We'll use the float temp, and cast it to elev_type for storage
  float temp;
  elev_type e_value;
  //also, store the fd values in an unsigned short
  int fd_value;

  //start reading
  for(r = 0; r < FA_Map_getNRows(*newFA_Map); r++) {
    for(c = 0; c < FA_Map_getNCols(*newFA_Map); c++) {
      //read in the elev value
      fscanf(gridFile, "%f", &temp);
      e_value = (elev_type) temp;
      //read in the fd value
      fscanf(fdFile, "%d", &fd_value);
      //store the value
      FA_setPoint(newFA_Map, c, r, e_value, fd_value);
    }
  }

  fclose(gridFile);

  return newFA_Map;

}

//write the FA map to file.  in_path is passed so we can copy the header from it
void FA_Map_writeMap(FA_Map map, char* in_path, char* out_path) {

  //first open the files
  FILE* inFile = fopen(in_path, "r+");
  assert(inFile);

  FILE* outFile = fopen(out_path, "w+");
  assert(outFile);

  //write the header (the first 6 lines) from inFile to outFile
  int i;
  char headerLine[40];
  for(i = 0; i < 6; i++) {
    fputs(fgets(headerLine, 40, inFile), outFile);
  }

  //all done with the input file, so close it
  fclose(inFile);

  //now, go through and write all the recorded values
  int c, r;
  for(r = 0; r < FA_Map_getNRows(map); r++) {
    for(c = 0; c < FA_Map_getNCols(map); c++) {
      //if the point is NODATA, write the NODATA value
      if(FA_Point_getElev(*FA_Map_getPoint_ij(map, c, r)) == FA_Map_getNoDataValue(map)) {
	fprintf(outFile, "%hi ", FA_Map_getNoDataValue(map));
      }
      //otherwise, write the fa value
      fprintf(outFile, "%hu ", FA_Point_getFAValue(*FA_Map_getPoint_ij(map, c, r)));
    }
    //at the end of every row, write a newline
    fprintf(outFile, "\n");
  }

  fclose(outFile);
}

//free the FA_Map
void FA_Map_kill(FA_Map* map) {
  assert(map);
  //free the array
  int c;
  for(c = 0; c < FA_Map_getNCols(*map); c++) {
    free(map->fa_data[c]);
  }
  free(map->fa_data);

  //free the rest of the map
  free(map);
}

//GETTERS --------------------------------------------------
//Map Getters
unsigned short FA_Map_getNRows(FA_Map map) {
  assert(&map);
  return map.nrows;
}
unsigned short FA_Map_getNCols(FA_Map map) {
  assert(&map);
  return map.ncols;
}
elev_type FA_Map_getNoDataValue(FA_Map map) {
  assert(&map);
  return map.NODATA;
}
//get the FA_Point stored at i,j
FA_Point* FA_Map_getPoint_ij(FA_Map map, unsigned short i, unsigned short j) {
  assert(&map);
  return map.fa_data[i+j*FA_Map_getNCols(map)];
}
//get the FA_Point storted at index i
FA_Point* FA_Map_getPoint_index(FA_Map map, unsigned short i) {
  assert(&map);
  return map.fa_data[i];
}

//FA_Point getters
unsigned short FA_Point_getCol(FA_Point p) {
  assert(&p);
  return p.col;
}
unsigned short FA_Point_getRow(FA_Point p) {
  assert(&p);
  return p.row;
}
elev_type FA_Point_getElev(FA_Point p) {
  assert(&p);
  return p.elev;
}
unsigned short FA_Point_getFAValue(FA_Point p) {
  assert(&p);
  return p.fa;
}
FA_Point* FA_Point_getFD(FA_Point p) {
  assert(&p);
  return p.fd;
}

//SETTERS ----------------------------------------------------------
//initialze the passed FA_Point* with passed values
void FA_setPoint(FA_Map* map, unsigned short c, unsigned short r, elev_type elev, int fd) {
  assert(map);

  FA_Point* p = FA_Map_getPoint_ij(*map, c, r);

  //store the values
  p->col = c;
  p->row = r;
  p->elev = elev;

  //set starting fa to 1
  p->fa = 1;

  //calculate and store the right Point* to store in fd
  if(elev != FA_Map_getNoDataValue(*map)) {
    p->fd = FA_PointInDir(*map, c, r, (unsigned short) fd);
  }
  else {
    //set fd to NULL if the point is NODATA
    p->fd = NULL;
  }

}

//increment the FA value of the passed FA_Point* by value
void FA_IncrementValue(FA_Point* p, unsigned short value) {
  assert(p);

  p->fa += value;
}

//HELPERS ----------------------------------------------------------
//given an FA_Map, a point and a direction, return the FA_Point in that direction using the stadard set in the FD files
FA_Point* FA_PointInDir(FA_Map map, unsigned short c, unsigned short r, unsigned short dir) {
  switch(dir) {
  case 0:
    return FA_Map_getPoint_ij(map, c, r);
    break;
  case 1:
    if(c != 0 && r != 0)
      return FA_Map_getPoint_ij(map, c-1, r-1);
    else 
      return NULL;
    break;
  case 2:
    if(r != 0)
      return FA_Map_getPoint_ij(map, c, r-1);
    else
      return NULL;
    break;
  case 3:
    if(c != FA_Map_getNCols(map)-1 && r != 0)
      return FA_Map_getPoint_ij(map, c+1, r-1);
    else
      return NULL;
    break;
  case 4:
    if(c != FA_Map_getNCols(map)-1)
      return FA_Map_getPoint_ij(map, c+1, r);
    else
      return NULL;
    break;
  case 5:
    if(c != FA_Map_getNCols(map)-1 && r != FA_Map_getNRows(map)-1)
      return FA_Map_getPoint_ij(map, c+1, r+1);
    else
      return NULL;
    break;
  case 6:
    if(r != FA_Map_getNRows(map)-1)
      return FA_Map_getPoint_ij(map, c, r+1);
    else
      return NULL;
    break;
  case 7:
    if(c != 0 && r != FA_Map_getNRows(map)-1)
      return FA_Map_getPoint_ij(map, c-1, r+1);    
    else
      return NULL;
    break;
  case 8:
    if(c != 0)
      return FA_Map_getPoint_ij(map, c-1, r);
    else
      return NULL;
    break;
  }
  return NULL;
}

//WORKHORSE ----------------------------------------------------------
//actually does the work for the FA algorithm - passed a FA grid, fills the FA grid
void FA_fill(FA_Map* map) {
  assert(map);

  //qsort the data array by elevation
  qsort(map->fa_data, FA_Map_getNCols(*map)*FA_Map_getNRows(*map),
	sizeof(FA_Point*), sort_by_elev);

  //go through the array starting at highest elevation and pass the accumulated water down
  int i;
  FA_Point* curP; //current point that we are looking at
  FA_Point* passP; //point to pass the flow to
  for(i = 0; i<FA_Map_getNCols(*map)*FA_Map_getNRows(*map); i++) {
    curP = FA_Map_getPoint_index(*map, i);
    assert(curP);
    if(FA_Point_getElev(*curP) == FA_Map_getNoDataValue(*map)) {
      continue;
    }
    passP = FA_Point_getFD(*curP);
    if(passP == NULL) {
      exit(0);
    }
    assert(passP);

    if(curP != passP) {
      FA_IncrementValue(passP, FA_Point_getFAValue(*curP));
    }
  }

  //resort the array by r,c
  qsort(map->fa_data, FA_Map_getNCols(*map)*FA_Map_getNRows(*map),
	sizeof(FA_Point*), sort_by_rc);

}



//SORTERS FOR QSORT----------------------------------------------------------
//sort by row column
int sort_by_rc(const void * a, const void * b) {
  assert(a);
  assert(b);
  FA_Point** p1 = (FA_Point**) a;
  FA_Point** p2 = (FA_Point**) b;

  //if rows are not equal, we can just compare if they are equal
  if((*p1)->row != (*p2)->row) {
    if((*p1)->row < (*p2)->row) return -1;
    else if((*p1)->row == (*p2)->row) return 0;
    else return 1;
  }
  //rows are equal, so compare columns
  else {
    if((*p1)->col < (*p2)->col) return -1;
    else if((*p1)->col == (*p2)->col) return 0;
    else return 1;
  }

}
//sort by elevation.  Sorts with highest elevation first.
int sort_by_elev(const void* a, const void* b) {
  assert(a);
  assert(b);
  FA_Point** p1 = (FA_Point**) a;
  FA_Point** p2 = (FA_Point**) b;

  if(((*p1))->elev > (*p2)->elev) return -1;
  else if((*p1)->elev == (*p2)->elev) {
    //if they have the same elevation, sort by rc, so that points with the same elevation will be sorted near oneanother
    return sort_by_rc(a, b);
  }
  else return 1;

}
