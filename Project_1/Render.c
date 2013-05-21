#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <string.h>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <unistd.h>

#include "B_Map.h"
#include "rtimer.h"

#define READ_DEBUG if(0)
#define DISPLAY_DEBUG if(0)
#define PRINT_TIMINGS if(1)

GLfloat red[3] = {1.0, 0.0, 0.0};
GLfloat green[3] = {0.0, 1.0, 0.0};
GLfloat blue[3] = {0.0, 0.0, 1.0};
GLfloat black[3] = {0.0, 0.0, 0.0};
GLfloat white[3] = {1.0, 1.0, 1.0};
GLfloat gray[3] = {0.5, 0.5, 0.5};
GLfloat yellow[3] = {1.0, 1.0, 0.0};
GLfloat magenta[3] = {1.0, 0.0, 1.0};
GLfloat cyan[3] = {0.0, 1.0, 1.0};

//the center of the viewpoint center[0]->x, center[1]->y
GLdouble center[2];

//how much of the map you can see dimension[0]->width, dimension[1]->height
GLdouble dimension[2];

//the angle at which viewpoint is angled
GLfloat theta[3] = {0,0,0};

//the map struct to store the data
B_Map* map;

//keep track of the resolution
GLint resolution = 1;

//keep track of the scale of the elevation
GLfloat elev_scale = 1.0;

//keep track of if it should be drawn as wireframe or filled triangles
GLint fill_shapes = 1;
//color to paint the map
GLint color_mode = 0;
/* //draw flow or don't draw flow */
/* GLint draw_flow = 1; */
/* //decide how big the flow lines have to be before you draw them */
/* GLfloat flow_limit = 50; */

//timers
Rtimer rtRead, rtDisplay;

//forward declaration of statements
void display(void);
void keypress(unsigned char key, int x, int y);
void main_menu(int value);

int main(int argc, char** argv, char** envp) {
  //make sure arguments were passed correctly
  if(argc != 1) {
    exit(0);
  }

  printf("INSTRUCTIONS:\nx: - angle in x direction\nX: + angle in x direction\ny: - angle in y direction\nY: + angle in y direction\nz: - angle in z direction\nZ: + angle in z direction\nl: move right\nj: move up\nk: move down\ni: move left\nu: zoom out\no: zoom in\n+: decrease resolution\n-: increase resolution\n1: decrease elevation scale\n2: increase elevation scale\nq: quit\n");

  rt_start(rtRead);//start the reading timer

  //read in the data
  map = B_Map_createFromFile(argv[0]);

  rt_stop(rtRead);//stop the reading timer
  //print the read timer
  char buf[1000];
  rt_sprint(buf, rtRead);
  PRINT_TIMINGS{printf("time to read data: %s\n", buf);}

  //set center and dimension
  center[0] = B_Map_getNCols(*map) / 2.0;
  center[1] = -B_Map_getNRows(*map) / 2.0;

  dimension[0] = B_Map_getNCols(*map) / 2.0;
  dimension[1] = B_Map_getNRows(*map) / 2.0;

  //open a window
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);

  //initialize the window size so that dimentions will stay scaled correctly
  if(B_Map_getNCols(*map) >= B_Map_getNRows(*map)) {
    glutInitWindowSize(800, 800*((float)B_Map_getNRows(*map)/(float)B_Map_getNCols(*map)));
  }
  else {
    glutInitWindowSize(800*((float)B_Map_getNCols(*map)/(float)B_Map_getNRows(*map)), 800);
  }
  glutInitWindowPosition(50, 50);
  glutCreateWindow(argv[0]);

  glEnable(GL_DEPTH_TEST);

  //camera is at (0,0,0) looking at negative y axis
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(center[0] - dimension[0], center[0] + dimension[0],
	  center[1] - dimension[1], center[1] + dimension[1],
	  0.0, 2.0);
/*   gluPerspective(60, 1, -1.0, 200.0); */

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  //OpenGL init
  glClearColor(1.0, 1.0, 1.0, 0); //set background to white

  //callback functions
  glutDisplayFunc(display);
  glutKeyboardFunc(keypress);

  //set up the menu
  glutCreateMenu(main_menu);
  glutAddMenuEntry("Quit", 1);
  glutAddMenuEntry("Toggle Fill Mode", 2);
  glutAddMenuEntry("Greyscale", 3);
  glutAddMenuEntry("Rainbow", 4);
  glutAddMenuEntry("RGB", 5);
  glutAttachMenu(GLUT_LEFT_BUTTON);

  //event handler
  glutMainLoop();
  return 0;
}

//sets the passed GLfloat to the color that the passed value should be rendered
void getColor(GLfloat* color, elev_type value, B_Map lMap) {
  //if nodata, set to background color, in this case, white
  if(value == B_Map_getNoDataValue(*map)) {
    color[0] = white[0];
    color[1] = white[1];
    color[2] = white[2];
/*     DISPLAY_DEBUG{printf("getting color for nodata value\n"); fflush(stdout);} */
    return;
  }
  //color_value is the percent of the total elevation difference that the passed value is at
  GLfloat color_value;
  color_value = ((float) (value - B_Map_getMinElev(*map)))/((float)(B_Map_getMaxElev(*map) - B_Map_getMinElev(*map)));
  //greyscale
  if(color_mode == 0) {
    color[0] = color_value;
    color[1] = color_value;
    color[2] = color_value;
    return;
  }
  //rainbow
  if(color_mode == 1) {
    //we need to split the value of 0.0 to 1.0 into 6 parts, which is where 1.0/6 comes from
    GLfloat color_bucket_value;
    //if we don't do this in term of 100, mod doesn't work, and we can't figure out what part of the given bucket our value is in.
    color_bucket_value = (fmod(color_value,1.0/6)) / (1.0/6);
    if(color_value < 1.0/6) {
      //start at .75 for this one so that we don't repeat any colors.  All others will either be out of 1.0 or 1.0-value
      color[0] = .75-color_bucket_value;
      color[1] = 0.0;
      color[2] = 1.0;
    }
    else if(color_value < 2*1.0/6) {
      color[0] = 0.0;
      color[1] = color_bucket_value;
      color[2] = 1.0;
    }
    else if(color_value < 3*1.0/6) {
      color[0] = 0.0;
      color[1] = 1.0;
      color[2] = color_bucket_value;
    }
    else if(color_value < 4*1.0/6) {
      color[0] = color_bucket_value;
      color[1] = 1.0;
      color[2] = 0.0;
    }
    else if(color_value < 5*1.0/6) {
      color[0] = 1.0;
      color[1] = 1.0 - color_bucket_value;
      color[2] = 0.0;
    }
    else {
      color[0] = 1.0;
      color[1] = 0.0;
      color[2] = color_bucket_value;
    }
    return;
  }

  //RGB
  if(color_mode == 2) {
    GLfloat color_bucket_value;
    //this one wil have 3 buckets, and treat them much like they were treated in the rainbow setting
    //if we don't do this in term of 100, mod doesn't work, and we can't figure out what part of the given bucket our value is in.
    color_bucket_value = ((int)(color_value*100) % (int)(100.0/3)) / (100.0/3);
    if(color_value < 1.0/3) {
      color[0] = 0.0;
      color[1] = 0.0;
      color[2] = color_bucket_value;
    }
    else if(color_value < 2*1.0/3) {
      color[0] = 0.0;
      color[1] = color_bucket_value;
      color[2] = 0.0;
    }
    else {
      color[0] = color_bucket_value;
      color[1] = 0.0;
      color[2] = 0.0;
    }
  }

}

//takes the elev_type value from the map and converts it to the correct value to be rendered.
GLfloat convert_elev_value(elev_type value) {
  return elev_scale * value;
}

void display_grid(B_Map* lmap) {
  //counters to keep track of position
  unsigned short c,r;
  //color of current point
  GLfloat curColor[3];

  //the current value
  elev_type curValue;

  //start drawing the triangle strip

  // we don't have to render the last row - it is rendered by the row above it
  for(r = 0; r < B_Map_getNRows(*lmap) - 1; r+=resolution) {
    glBegin(GL_TRIANGLE_STRIP);
    //render the columns from 0 to ncols
    for(c = 0; c < B_Map_getNCols(*lmap); c+=resolution) {
      //draw the current point
      curValue = B_Map_getValue(*lmap, c, r);
      if(curValue != B_Map_getNoDataValue(*lmap)) {
	getColor(curColor, curValue, *lmap);
	glColor3fv(curColor);
	glVertex3f(c, -r,  elev_scale * curValue);
      }
      
      //now, draw the point below it
      curValue = B_Map_getValue(*lmap, c, r+resolution);
      if(curValue != B_Map_getNoDataValue(*lmap)) {
	getColor(curColor, curValue, *lmap);
	glColor3fv(curColor);
	glVertex3f(c, -(r+resolution),  elev_scale * curValue);
      }
    }
    glEnd();
  }
}

//* /have a max width of maxWidth. */
/* GLfloat getLineWidth(B_Map lmap, unsigned short flow) { */
/*   GLfloat maxWidth = 50.0; */

/*   return ((GLfloat)flow/(GLfloat)B_Map_getMaxFlow(lmap)) * maxWidth; */
/* } */

//draws the flow lines, proportional to the amount of flow
/* void display_flow(B_Map* lmap) { */

/*   glColor3fv(blue); */
/*   int i; */
/*   Point* curP; */
/*   Point* pasP; */
/*   for(i = 0; i < B_Map_getNCols(*lmap)*B_Map_getNRows(*lmap); i++) { */
/*     curP = B_Map_getPoint_index(*lmap, i); */
/*     assert(curP); */
/*     //skip nodata points */
/*     if(Point_getElev(*curP) == B_Map_getNoDataValue(*lmap)) { */
/*       continue; */
/*     } */
/*     //only draw lines of a certain width */
/*     if((float)Point_getFA(*curP) < (float)B_Map_getMaxFlow(*lmap)/flow_limit) { */
/*       continue; */
/*     } */
/*     pasP = Point_getFD(*curP); */
/*     assert(pasP); */
/*     //set the line width based on the FA */
/*     glLineWidth(getLineWidth(*lmap, Point_getFA(*curP))); */
/*     //draw the line segment */
/*     glBegin(GL_LINES); */
/*     glVertex3f(Point_getCol(*curP), -Point_getRow(*curP), Point_getElev(*curP)+1); */
/*     glVertex3f(Point_getCol(*pasP), -Point_getRow(*pasP), Point_getElev(*pasP)+1); */
/*     glEnd(); */
/*   } */

/* } */

//Display the map
void display(void) {
  DISPLAY_DEBUG{printf("starting a redisplay\n");fflush(stdout);}

  rt_start(rtDisplay); // start the display timer

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  //use glOrtho to set the projection in the correct place to view the right part of the scene
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(center[0] - dimension[0], center[0] + dimension[0],
	  center[1] - dimension[1], center[1] + dimension[1],
	  -elev_scale*B_Map_getMaxElev(*map), elev_scale*B_Map_getMaxElev(*map));

  //switch back into MODELVIEW mode
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  //angle the scene based on the angle of the viewer
  //translate to the middle of the map, then rotate, then translate back, so we are rotating around the middle of the map
  glTranslatef(B_Map_getNCols(*map) / 2.0, -B_Map_getNRows(*map) /2.0, 0.0);
  glRotatef(theta[0], 1.0, 0.0, 0.0);
  glRotatef(theta[1], 0.0, 1.0, 0.0);
  glRotatef(theta[2], 0.0, 0.0, 1.0);
  glTranslatef(-B_Map_getNCols(*map) / 2.0, B_Map_getNRows(*map) /2.0, 0.0);


  //----------------------------------------------------
  if(fill_shapes) 
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  else
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  //draw the map itself
  display_grid(map);

  //now draw flow stuff
 /*  if(draw_flow) { */
/*     display_flow(map); */
/*   } */

  //flush the display so we can see everything
  glFlush();

  rt_stop(rtDisplay);//stop the display timer
  //print the display timer
  char buf[1000];
  rt_sprint(buf, rtDisplay);
  PRINT_TIMINGS{printf("time to display data: %s\n", buf);}

}

//handle keypresses
void keypress(unsigned char key, int x, int y) {
  switch(key) {
    //object movement
  case 'x':
    theta[0] -= 2.0;
    glutPostRedisplay();
    break;
  case 'X':
    theta[0] += 2.0;    
    glutPostRedisplay();
    break;
  case 'y':
    theta[1] -= 2.0;
    glutPostRedisplay();
    break;
  case 'Y':
    theta[1] += 2.0;
    glutPostRedisplay();
    break;
  case 'z':
    theta[2] -= 2.0;
    glutPostRedisplay();
    break;
  case 'Z':
    theta[2] += 2.0;
    glutPostRedisplay();
    break;
    
  case 'l':
    center[0] += dimension[0]/10;
    glutPostRedisplay();
    break;
  case 'j':
    center[0] -= dimension[0]/10;
    glutPostRedisplay();
    break;
  case 'k':
    center[1] -= dimension[1]/10;  
    glutPostRedisplay();
    break;
  case 'i':
    center[1] += dimension[1]/10;
    glutPostRedisplay();
    break;
  case 'u':
    dimension[0] *= 1.25;
    dimension[1] *= 1.25;
    glutPostRedisplay();
    break;
  case 'o':
    dimension[0] *= 0.8;
    dimension[1] *= 0.8;
    glutPostRedisplay();
    break;


    //increment and decrement the resolution
  case '+':
    resolution++;
    DISPLAY_DEBUG{printf("resolution now set at %d\n", resolution); fflush(stdout);}
    glutPostRedisplay();
    break;
  case '-':
    if(resolution != 1) {
      resolution--;
      DISPLAY_DEBUG{printf("resolution now set at %d\n", resolution); fflush(stdout);}
      glutPostRedisplay();
    }
    break;

    //scale the elevation
  case '1':
    if(elev_scale * 0.8 > 0.0){
      elev_scale *= 0.8;
/*       printf("elev_scale = %f\n", elev_scale); fflush(stdout); */
      glutPostRedisplay();
    }
    break;
  case '2':
    elev_scale *= 1.25;
/*     printf("elev_scale = %f\n", elev_scale); fflush(stdout); */
    glutPostRedisplay();
    break;

/*     //adjust the flow limit, to show more or less flow */
/*   case '3': */
/*     flow_limit *= (4.0/6.0); */
/*     glutPostRedisplay(); */
/*     break; */
/*   case '4': */
/*     flow_limit *= (6.0/4.0); */
/*     glutPostRedisplay(); */
/*     break; */



    //decide to draw flow stuff
/*   case 'f': */
/*     draw_flow = !draw_flow; */
/*     glutPostRedisplay(); */
/*     break; */

    //quit, and by quit I mean restart gis
  case 'q':
    exit(0);
    break;
  }
}

void main_menu(int value) {
  switch(value) {
  case 1:
    exit(0);
    break;
 case 2:
   fill_shapes = !fill_shapes;
   break;
  case 3:
   //greyscale
    color_mode = 0;
    break;
  case 4:
    //rainbow
    color_mode = 1;
    break;
  case 5:
    //RGB
    color_mode = 2;
  }
  glutPostRedisplay();
}
