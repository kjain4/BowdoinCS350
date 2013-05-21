/* simple4.c 

 draws a rectangle with vertices of different colors; note the color
 bleding, done automatically
 
 menu: switch between filled/outline and quiut

 Laura Toma
 */



#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif


GLfloat red[3] = {1.0, 0.0, 0.0};
GLfloat green[3] = {0.0, 1.0, 0.0};
GLfloat blue[3] = {0.0, 0.0, 1.0};
GLfloat black[3] = {0.0, 0.0, 0.0};
GLfloat white[3] = {1.0, 1.0, 1.0};
GLfloat gray[3] = {0.5, 0.5, 0.5};
GLfloat yellow[3] = {1.0, 1.0, 0.0};
GLfloat magenta[3] = {1.0, 0.0, 1.0};
GLfloat cyan[3] = {0.0, 1.0, 1.0};

GLint fillmode = 0;



/* forward declarations of functions */
void display(void);
void keypress(unsigned char key, int x, int y);
void main_menu(int value);


int main(int argc, char** argv) {

    /* open a window */
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(500, 500);
  glutInitWindowPosition(100,100);
  glutCreateWindow(argv[0]);

  /* OpenGL init */
  glClearColor(0, 0, 0, 0);   /* set background color black*/

    
  /* callback functions */
  glutDisplayFunc(display); 
  glutKeyboardFunc(keypress);

  
  glutCreateMenu(main_menu);
  glutAddMenuEntry("Fill/Outline", 1);
  glutAddMenuEntry("Quit", 2);
  glutAttachMenu(GLUT_RIGHT_BUTTON);

  /* event handler */
  glutMainLoop();
  return 0;
}


void draw_polygon(){

  if (fillmode) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  } else {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  }
  
 /* draw a polygon */
  glBegin(GL_POLYGON);
  glColor3fv(blue);   
  glVertex2f(-0.5,-0.5);
  glColor3fv(white);   
  glVertex2f(-0.5,0.5);
  glColor3fv(red);   
  glVertex2f(0.5,0.5);
  glColor3fv(gray);  
  glVertex2f(0.5,-0.5);
  glEnd();
}



void display(void) {

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  draw_polygon();

  /* execute the drawing commands */
  glFlush();
}

void
keypress(unsigned char key, int x, int y) {
  switch(key) {
  case 'q':
    exit(0);
    break;
  } 
}


void main_menu(int value)
{
  switch (value){
  case 1: 
    /* toggle outline/fill */
    fillmode = !fillmode;
    break;
  case 2: 
    exit(0);
  }
  glutPostRedisplay();
}
