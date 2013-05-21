/* cube.c */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif


GLfloat pos[3] = {0,0,0};
GLfloat theta[3] = {0,0,0};

/* forward declarations of functions */
void display(void);
void keypress(unsigned char key, int x, int y);


void draw_xy_rect(GLfloat z, GLfloat r, GLfloat g, GLfloat b) {
  glBegin(GL_POLYGON);
  glColor3f(r,g,b);
  glVertex3f(-1,-1, z);
  glVertex3f(-1,1, z);
  glVertex3f(1,1, z);
  glVertex3f(1,-1, z);
  glEnd();
}

void draw_yz_rect(GLfloat x, GLfloat r, GLfloat g, GLfloat b) {
  glBegin(GL_POLYGON);
  glColor3f(r,g,b);
  glVertex3f(x,-1, 1);
  glVertex3f(x,1, 1);
  glVertex3f(x,1, -1);
  glVertex3f(x,-1, -1);
  glEnd();
}

void draw_xz_rect(GLfloat y, GLfloat r, GLfloat g, GLfloat b) {
  glBegin(GL_POLYGON);
  glColor3f(r,g,b);
  glVertex3f(-1,y, 1);
  glVertex3f(-1,y, -1);
  glVertex3f(1,y, -1);
  glVertex3f(1,y, 1);
  glEnd();
}


void cube() {
  GLfloat f = 1, b = -1;
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); 

  /* back face */
  draw_xy_rect(b,0,0,1);
  /* side faces */
  draw_yz_rect(-1, 0,0,1);
  draw_yz_rect(1, 0,0,1);

  /* front face */
  draw_xy_rect(f,1,0,0);

  /* middle z=0 face */
  draw_xy_rect(0,0,1,0);
  /* middle x=0 face */
  draw_yz_rect(0,0,1,0);
  /* middle y=0 face */
  draw_xz_rect(0,0,1,0);
}

void drawAxes() {
  glBegin(GL_LINES);
  glColor3f(0,1,0);
  glVertex3i(-1,0,0);
  glVertex3i(1,0,0);
  glVertex3i(0,1,0);
  glVertex3i(0,-1,0);
  glVertex3i(0,0,1);
  glVertex3i(0,0,-1);
  glEnd();
}


void display(void) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glLoadIdentity(); 

  /* modeling*/
  glTranslatef(pos[0], pos[1], pos[2]);  
  glRotatef(theta[0], 1,0,0);
  glRotatef(theta[1], 0,1,0);
  glRotatef(theta[2], 0,0,1);
  
  drawAxes();  
  cube();

  glFlush();
}


void
keypress(unsigned char key, int x, int y) {
  switch(key) {

    /* various object rotations: */
  case 'x':
    theta[0] += 5.0; 
    //glRotatef(5.0, 1, 0, 0);
    glutPostRedisplay();
    break;
  case 'y':
    theta[1] += 5.0;
    //glRotatef(5.0, 0, 1, 0);
    glutPostRedisplay();
    break;
  case 'z':
    theta[2] += 5.0;
    //glRotatef(5.0, 0, 0, 1);
    glutPostRedisplay();
    break;
  case 'X':
    theta[0] -= 5.0; 
    //glRotatef(-5.0, 1, 0, 0);
    glutPostRedisplay();
    break;
  case 'Y':
    theta[1] -= 5.0; 
    //glRotatef(-5.0, 0, 1, 0);
    glutPostRedisplay();
    break;
  case 'Z':
    theta[2] -= 5.0; 
    //glRotatef(-5.0, 0, 0, 1);
    glutPostRedisplay();
    break;

  case 'b':
    pos[2] -= 0.5; 
    //glTranslatef(0,0, -0.5);
    glutPostRedisplay();
    break;
  case 'f':
    pos[2] += 0.5; 
    //glTranslatef(0,0, 0.5);
    glutPostRedisplay();
    break;

  case 'd': 
     pos[1] -= 0.5; 
    //glTranslatef(0,0.5,0);
    glutPostRedisplay();
    break;
  case 'u': 
    pos[1] += 0.5; 
    //glTranslatef(0,-0.5,0);
    glutPostRedisplay();
    break;
  case 'l':
    pos[0] -= 0.5; 
    glutPostRedisplay();
    break;
  case 'r':
    pos[0] += 0.5; 
    glutPostRedisplay();
    break;
  case 'q':
    exit(0);
    break;
  } 
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
  
  /* camera is at (0,0,0) looking along negative y axis */
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(60, 1 /* aspect */, 1, 10.0); /* the frustrum is from z=-1 to z=-10 */ 
  glMatrixMode(GL_MODELVIEW);
  pos[2] -= 5;
  pos[1] = -1.3;


  /* callback functions */
  glutDisplayFunc(display); 
  glutKeyboardFunc(keypress);
  
   /* event handler */
  glutMainLoop();
  return 0;
}

