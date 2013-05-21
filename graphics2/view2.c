/* view2.c */

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
void renderScene(void);
void keypress(unsigned char key, int x, int y);
void main_menu(int value);


void draw_xy_rect(GLfloat z, GLfloat r, GLfloat g, GLfloat b) {

  glBegin(GL_POLYGON);
  glColor3f(r,g,b);
  glVertex3f(-1,-1, z);
  glVertex3f(-1,1, z);
  glVertex3f(1,1, z);
  glVertex3f(1,-1, z);
  glEnd();
}

void draw_cube(GLfloat z1, GLfloat z2) {
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); 
  glColor3f(1.0, 1.0, 1.0);

  draw_xy_rect(z1, 1,0,0);
  draw_xy_rect(z2, 0,0,1);
  
  glBegin(GL_POLYGON);
  glVertex3f(-1,-1, z1);
  glVertex3f(-1,1, z1);
  glVertex3f(-1,1, z2);
  glVertex3f(-1,-1, z2);
  glEnd();

  glBegin(GL_POLYGON);
  glVertex3f(1,-1, z1);
  glVertex3f(1,1, z1);
  glVertex3f(1,1, z2);
  glVertex3f(1,-1, z2);
  glEnd();

}


void keypress(unsigned char key, int x, int y) {
  switch(key) {
  case 'q':
    exit(0);
    break;
  } 
}



void display(void) {
  draw_cube(-0.5, -0.7);
}


void renderScene(void) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if(1)glRotatef(45.0, 0.0,0.0,1.0);
  if(1)glTranslatef(0.5, -0.4, 0); 
 
  /* local coordinate system: translate object and its coordinate
     system 5 units in the negative z direction, away from the
     viewpoint; then rotate the object 45 deg  */
    
  if(1)glScalef(0.5,0.5,0.5);
  display();

  /* execute the drawing commands */
  glFlush();
}




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
  glutDisplayFunc(renderScene); 
  glutKeyboardFunc(keypress);

  /* event handler */
  glutMainLoop();
  return 0;
}

