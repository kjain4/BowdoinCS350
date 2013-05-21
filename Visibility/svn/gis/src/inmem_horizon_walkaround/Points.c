/* Will Richard
 * Walkaround algroithm
 * Points.c
 */

#include "Points.h"

//CONSTURCT AND DISTROY --------------------------------------------------------
//Create a new point, filling its elev
Point* Point_new(short elev) {
  //allocate the point and assert it
  Point* new = (Point*) malloc(sizeof(Point));
  assert(new);

  //set the elevation
  new->elev = elev;
  //we don't really care what the visibility is set at currently - it'll be set as we walkaround the layer in which the point is located

  //all done - return the new point
  return new;
}

//Create a new viewpoint
Viewpoint* Viewpoint_new(short col, short row, short elev) {
  //allocate the vp and assert it
  Viewpoint* new = (Viewpoint*) malloc(sizeof(Viewpoint));
  assert(new);

  //set the values
  new->col = col;
  new->row = row;
  new->elev = elev;

  //done - return the new viewpoint
  return new;
}

//fills the passed viewpoint
void Viewpoint_fill(Viewpoint* vp, short col, short row, short elev) {
  assert(vp);

  vp->col = col;
  vp->row = row;
  vp->elev = elev;
}


//free the passed Point
void Point_kill(Point* point) {
  //assert the point
  assert(point);

  //free it
  free(point);
}

//free the passed viewpoint
void Viewpoint_kill(Viewpoint* vp) {
  //assert the viewpoint
  assert(vp);

  //free it
  free(vp);
}

//GETTERS ----------------------------------------------------------------------
short Point_getElev(Point p) {return p.elev;}
short Point_getVis(Point p) {return p.vis;}
/* float Point_getSlope(Point p) {return p.slope;} */
/* double Point_getCenterAngle(Point p) {return p.centerAngle;} */
/* double Point_getStartAngle(Point p) {return p.startAngle;} */
/* double Point_getEndAngle(Point p) {return p.endAngle;} */


short Viewpoint_getElev(Viewpoint vp) {return vp.elev;}
short Viewpoint_getCol(Viewpoint vp) {return vp.col;}
short Viewpoint_getRow(Viewpoint vp) {return vp.row;}

//SETTERS ----------------------------------------------------------------------
void Point_setElev(Point* p, short elev) {
  assert(p);
  p->elev = elev;
}

void Point_setVis(Point* p, short vis) {
  assert(p);
  p->vis = vis;
}

/* void Point_setSlopeAndCenterAngle(Point* p, short pCol, short pRow, Viewpoint vp) { */
/*   assert(p); */
/*   p->slope = Point_calcSlope(vp, *p, pCol, pRow); */
/*   p->centerAngle = Point_calcCenterAngle(vp, pCol, pRow); */
/* } */


/* void Point_setStartAndEndAngle(Point* p, short pCol, short pRow, Viewpoint vp) { */
/*   assert(p); */
/*   p->startAngle = Point_calcStartAngle(vp, pCol, pRow); */
/*   p->endAngle = Point_calcEndAngle(vp, pCol, pRow); */
/* } */


//HELPERS ----------------------------------------------------------------------
//these are functions that calculate relationships between the Viewpoints and Points

//calculates the slope on the z plane form vp to p.  Acutally computes the square of the slope, but as long as all slopes are computed in this way, it won't matter.
float Point_calcSlope(Viewpoint vp, Point p, short pCol, short pRow) {
  float deltaZ, deltaX;
  //change in the z values is the change in elevation between vp and p
  deltaZ = (Point_getElev(p) - Viewpoint_getElev(vp));
  //change in x values is the square root of the difference in rows squared plus the difference in columns squared.
  deltaX = sqrt(((pCol - Viewpoint_getCol(vp)) * (pCol - Viewpoint_getCol(vp))) + ((pRow - Viewpoint_getRow(vp)) * (pRow - Viewpoint_getRow(vp))));
  //the slope is deltaZ / deldaX.
  return deltaZ / deltaX;
}

//Calculates the angle from the viewpoint to the point on the x-y plane.  This angle is equal to 0 if the point is directly to the right of the viewpoint, PI/2 if the point is directly above the viewpoint, etc.
double Point_calcCenterAngle(Viewpoint vp, short pCol, short pRow) {
  //in the case where the point is directly to the right of the viewpoint, make sure its angle is exactly 0.0
  if(pRow == Viewpoint_getRow(vp) && pCol > Viewpoint_getCol(vp)) {
    return 0.0;
  }
  //otherwise, return the angle using atan2
  //atan2 returns in the interval [-PI, PI], so add PI to adjust correctly.
  return atan2(pRow - Viewpoint_getRow(vp), Viewpoint_getCol(vp) - pCol) + PI;
}

//Calculates the angle of the corner of the point that is reached first if you move in counterclockwise order around the viewpoint in the x-y plane
double Point_calcStartAngle(Viewpoint vp, short pCol, short pRow) {
  //figure out what the row and column would be of the corner in question, assuming you can have half angles
  double startCol, startRow;
  //the difference in column and row between the vp and p
  short colDif = abs(pCol - Viewpoint_getCol(vp));
  short rowDif = abs(pRow - Viewpoint_getRow(vp));
  if(colDif == rowDif) {
    //we are on one of the corners
    if(pCol > Viewpoint_getCol(vp)) {
      //to the right of the viewpoint
      if(pRow > Viewpoint_getRow(vp)) {
	//below the viewpoint
	startCol = pCol - .5;
	startRow = pRow + .5;
      }
      else {
	//above the viewpoint
	startCol = pCol + .5;
	startRow = pRow + .5;
      }
    }
    else {
      //to the left of the viewpoint
      if(pRow > Viewpoint_getRow(vp)) {
	//below the viewpoint
	startCol = pCol - .5;
	startRow = pRow - .5;
      }
      else {
	//above the viewpoint
	startCol = pCol + .5;
	startRow = pRow - .5;
      }
    }
  }
  else if(colDif > rowDif) {
    //we know we are in a vertical section
    if(pCol > Viewpoint_getCol(vp)) {
      //we are to the right of the viewpoint
      startCol = pCol + .5;
      startRow = pRow + .5;
    }
    else {
      //left of the viewpoint
      startCol = pCol - .5;
      startRow = pRow - .5;
    }
  }
  else {
    //horezontal section
    if(pRow > Viewpoint_getRow(vp)) {
      //below viewpoint
      startCol = pCol - .5;
      startRow = pRow + .5;
    }
    else {
      //above viewpoint
      startCol = pCol + .5;
      startRow = pRow - .5;
    }
  }
  
  return atan2(startRow - Viewpoint_getRow(vp), Viewpoint_getCol(vp) - startCol) + PI;


/*   if(pCol < Viewpoint_getCol(vp)) { startRow = pRow - .5; } */
/*   else if (pCol == Viewpoint_getCol(vp)) { */
/*     if(pRow > Viewpoint_getRow(vp)) { startRow = pRow + .5; } */
/*     else {startRow = pRow - .5; } */
/*   } */
/*   else { startRow = pRow + .5; } */

/*   if(pRow < Viewpoint_getRow(vp)) {startCol = pCol + .5; } */
/*   else if (pRow == Viewpoint_getRow(vp)) { */
/*     if(pCol > Viewpoint_getCol(vp)) { startCol =  pCol + .5; } */
/*     else {startCol = pCol - .5; } */
/*   } */
/*   else { startCol = pCol - .5; } */
}

//Calculates the angle of the last corner of the point that is reached if you move around the viewpoint in counter clockwise order in the x-y plane
double Point_calcEndAngle(Viewpoint vp, short pCol, short pRow) {
  //figure out what the row and column would be of the corner in question, assuming you can have half angles
  double endCol, endRow;
  //the difference in column and row between the vp and p
  short colDif = abs(pCol - Viewpoint_getCol(vp));
  short rowDif = abs(pRow - Viewpoint_getRow(vp));
  if(colDif == rowDif) {
    //we are on one of the corners
    if(pCol > Viewpoint_getCol(vp)) {
      //to the right of the viewpoint
      if(pRow > Viewpoint_getRow(vp)) {
	//below the viewpoint
	endCol = pCol + .5;
	endRow = pRow - .5;
      }
      else {
	//above the viewpoint
	endCol = pCol - .5;
	endRow = pRow - .5;
      }
    }
    else {
      //to the left of the viewpoint
      if(pRow > Viewpoint_getRow(vp)) {
	//below the viewpoint
	endCol = pCol + .5;
	endRow = pRow + .5;
      }
      else {
	//above the viewpoint
	endCol = pCol - .5;
	endRow = pRow + .5;
      }
    }
  }
  else if(colDif > rowDif) {
    //we know we are in a vertical section
    if(pCol > Viewpoint_getCol(vp)) {
      //we are to the right of the viewpoint
      endCol = pCol + .5;
      endRow = pRow - .5;
    }
    else {
      //left of the viewpoint
      endCol = pCol - .5;
      endRow = pRow + .5;
    }
  }
  else {
    //horezontal section
    if(pRow > Viewpoint_getRow(vp)) {
      //below viewpoint
      endCol = pCol + .5;
      endRow = pRow + .5;
    }
    else {
      //above viewpoint
      endCol = pCol - .5;
      endRow = pRow - .5;
    }
  }
  
  return atan2(endRow - Viewpoint_getRow(vp), Viewpoint_getCol(vp) - endCol) + PI;



/*   if(pCol < Viewpoint_getCol(vp)) { endRow = pRow + .5; } */
/*   else if (pCol == Viewpoint_getCol(vp)) { */
/*     if(pRow > Viewpoint_getRow(vp)) { endRow = pRow + .5; } */
/*     else {endRow = pRow - .5; } */
/*   } */
/*   else { endRow = pRow - .5; } */

/*   if(pRow < Viewpoint_getRow(vp)) {endCol = pCol - .5; } */
/*   else if (pRow == Viewpoint_getRow(vp)) { */
/*     if(pCol > Viewpoint_getCol(vp)) { endCol =  pCol + .5; } */
/*     else {endCol = pCol - .5; } */
/*   } */
/*   else { endCol = pCol + .5; } */

/*   return atan2(endRow - Viewpoint_getRow(vp), Viewpoint_getCol(vp) - endCol) + PI; */
}
