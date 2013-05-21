/* 
   simple1.c 
   draws a white filled rectangle on a black background 
 
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


/* forward declarations of functions */
void display(void);
void keypress(unsigned char key, int x, int y);


int main(int argc, char** argv) {

  /* open a window */
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(500, 500);
  glutInitWindowPosition(100,100);
  glutCreateWindow(argv[0]);

  /* OpenGL init */
  glClearColor(0, 0, 0, 0);   /* set background color black*/
  glColor3f(1.0,1.0,1.0);   /* set draw color white */

  /* callback functions */
  glutDisplayFunc(display); 
  glutKeyboardFunc(keypress);

  /* event handler */
  glutMainLoop();
  return 0;
}

void display(void) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  /* draw a polygon */
  glBegin(GL_POLYGON);
  glVertex2f(-0.5,-0.5);
  glVertex2f(-0.5,0.5);
  glVertex2f(0.5,0.5);
  glVertex2f(0.5,-0.5);
  glEnd();

  /* execute the drawing commands */
  glFlush();
}

void keypress(unsigned char key, int x, int y) {
  switch(key) {
  case 'q':
    exit(0);
    break;
  } 
}
