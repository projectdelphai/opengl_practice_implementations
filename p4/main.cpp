/*
 * Program p1 for ECS 175 at UC Davis Fall 2017
 * based off the template code provided by
 * TA Garrett Aldrich
 */


#ifdef WIN32
#include <windows.h>
#endif

#if defined (__APPLE__) || defined(MACOSX)
#include <OpenGL/gl.h>
//#include <OpenGL/glu.h>
#include <GLUT/glut.h>

#else //linux
#include <GL/gl.h>
#include <GL/glut.h>
#endif

//other includes
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <fstream>
#include <sstream>
#include <set>
#include <tuple>
#include <iostream>

#include "point.h"
#include "curve.h"

using namespace std;

/****set in main()****/
//the number of pixels in the grid
int grid_width;
int grid_height;

//the size of pixels sets the inital window height and width
//don't make the pixels too large or the screen size will be larger than
//your display size
float pixel_size;

/*Window information*/
int win_height;
int win_width;

void init();
void idle();
void display();
void save();
int findClosestPoint(Point p);
void draw_lines(Curve curve, bool currentCurve);
Point find_point(vector<Point> points, float t);
Point find_bspline_point(vector<Point> points, float u);
Point find_bezier_point(vector<Point> points, float t);
void draw_bezier_curve(Curve curve);
void draw_bspline_curve(Curve curve);
void draw_pix(int x, int y, int z, tuple<int, int, int> color);
void draw_lines(Point p1, Point p2, tuple<int, int, int> color);
void reshape(int width, int height);
void key(unsigned char ch, int x, int y);
void specialKey(int key, int x, int y);
void mouse(int button, int state, int x, int y);
void motion(int x, int y);
void check();

tuple<float, float, float> blue(.2, .2, 1.0);
tuple<float, float, float> red(1.0, .2, .2);
tuple<float, float, float> green(.2, 1.0, .2);
tuple<float, float, float> black(.2, .2, .2);
tuple<float, float, float> currentColor = blue;

bool leftButtonUsed;
bool rightButtonUsed;
bool editMode = false;
bool renderBezier = true;
bool renderBSpline = false;

vector<Curve> curves;
int currentCurve = 0;
bool firstPointSelected = false;
Point firstPoint;
int currentPointIndex;
int degree = 2;

int main(int argc, char **argv)
{
  //the number of pixels in the grid
  grid_width = 500;
  grid_height = 500;

  //the size of pixels sets the inital window height and width
  //don't make the pixels too large or the screen size will be larger than
  //your display size
  pixel_size = 2;

  /*Window information*/
  win_height = grid_height*pixel_size;
  win_width = grid_width*pixel_size;

  /*Set up glut functions*/
  /** See https://www.opengl.org/resources/libraries/glut/spec3/spec3.html ***/

  glutInit(&argc,argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  /*initialize variables, allocate memory, create buffers, etc. */
  //create window of size (win_width x win_height
  glutInitWindowSize(win_width,win_height);
  //windown title is "glut demo"
  glutCreateWindow("Project 4");

  /*defined glut callback functions*/
  glutDisplayFunc(display); //rendering calls here
  glutReshapeFunc(reshape); //update GL on window size change
  glutMouseFunc(mouse);     //mouse button events
  glutMotionFunc(motion);   //mouse movement events
  glutKeyboardFunc(key);    //Keyboard events
  glutIdleFunc(idle);       //Function called while program is sitting "idle"
  glutSpecialFunc(specialKey);

  glutSetWindow(1);

  //initialize opengl variables
  init();
  //start glut event loop
  glutMainLoop();
  return 0;
}

/*initialize gl stufff*/
void init()
{
  Curve curve;
  curves.push_back(curve);
    
  cout << "Degree is now: " << degree << endl;

  //set clear color (Default background to white)
  glClearColor(1.0,1.0,1.0,1.0);
  //checks for OpenGL errors
  check();
}

//called repeatedly when glut isn't doing anything else
void idle()
{
  //redraw the scene over and over again
  glutPostRedisplay();	
}

//this is where we render the screen
void display()
{
  //clears the screen
  glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
  //clears the opengl Modelview transformation matrix
  glLoadIdentity();

  for (size_t i = 0; i < curves.size(); i++)
  {
    if (renderBezier)
      draw_bezier_curve(curves[i]);
    else
      draw_bspline_curve(curves[i]);

    if (currentCurve == (int)i)
      draw_lines(curves[i], true);
    else
      draw_lines(curves[i], false);
  }

  //blits the current opengl framebuffer on the screen
  glutSwapBuffers();
  //checks for opengl errors
  check();
}

void save()
{
  glutDestroyWindow(glutGetWindow());
  exit(0);

}

int findClosestPoint(Point p)
{
  for (size_t i = 0; i < curves[currentCurve].controlPoints.size(); i++)
  {
    Point s = curves[currentCurve].controlPoints[i];

    float distance = sqrt((p.x-s.x)*(p.x-s.x) + (p.y-s.y)*(p.y-s.y));

    if (distance < 5)
    {
      return i + 1;
    }
  }

  return 0;
}

void draw_lines(Curve curve, bool currentCurve)
{
  vector<Point> controlPoints = curve.controlPoints;

  if (controlPoints.size() == 0)
    return;

  for (size_t i = 0; i < controlPoints.size() - 1; i++)
  {
    draw_lines(controlPoints[i], controlPoints[i + 1], black);
  }

  tuple<float, float, float> color;
  for (size_t i = 0; i < controlPoints.size(); i++)
  {
    if (((int)i == currentPointIndex) && currentCurve)
      color = red;
    else
      color = black;

    draw_pix(controlPoints[i].x, controlPoints[i].y, 0, color);
    draw_pix(controlPoints[i].x + 1, controlPoints[i].y, 0, color);
    draw_pix(controlPoints[i].x - 1, controlPoints[i].y, 0, color);
    draw_pix(controlPoints[i].x, controlPoints[i].y + 1, 0, color);
    draw_pix(controlPoints[i].x, controlPoints[i].y - 1, 0, color);
    draw_pix(controlPoints[i].x + 1, controlPoints[i].y + 1, 0, color);
    draw_pix(controlPoints[i].x - 1, controlPoints[i].y - 1, 0, color);
    draw_pix(controlPoints[i].x - 1, controlPoints[i].y + 1, 0, color);
    draw_pix(controlPoints[i].x + 1, controlPoints[i].y - 1, 0, color);
  }

}

void draw_bspline_curve(Curve curve)
{
  vector<Point> points = curve.controlPoints;

  if (points.size() == 0)
    return;

  if ((int)points.size() <= degree)
    return;

  for (float i = 0; i < 1; i += 0.3)
  {
    Point p = find_bspline_point(points, i);
    draw_pix(p.x, p.y, 0, blue);
  }

}

void draw_bezier_curve(Curve curve)
{
  vector<Point> controlPoints = curve.controlPoints;

  if (controlPoints.size() == 0)
    return;

  for (float t = 0.001; t < 1; t += 0.0001)
  {
    Point p = find_bezier_point(controlPoints, t);
    draw_pix(p.x, p.y, 0, blue);
  }
}

Point find_bspline_point(vector<Point> points, float u)
{
  if (points.size() == 1)
    return points[0];

  int n = points.size() - 1;
  int p = degree;
  int m = n + p + 1;
  int numKnots = m + 1;

  // create the knots vector;
  vector<float> knots;

  for (int i = 0; i < p + 1; i++)
    knots.push_back(0);

  float remainingKnots = numKnots - (2 * (p + 1));
  float division = 1 / (remainingKnots + 1);

  for (int i = 0; i < remainingKnots; i++)
    knots.push_back((i + 1) * division);

  for (int i = 0; i < p + 1; i++)
    knots.push_back(1);

  int k;

  // check if u is a knot
  for (int i = 0; i < numKnots - 1; i++)
  {
    if ((knots[i] <= u) && (knots[i+1] > u))
    {
        k = i;
        break;
    }
  }
  
  cout << "k: " << k << endl;
  // grab affected points
  vector<Point> affectedPoints;

  for (int i = 0; i < p+1; i++)
  {
    affectedPoints.push_back(points[i + k - p]);
  }
  
  // calculate curve point
  for (int r = 1; r < (p + 1); r++)
  {
    //for (int i = k - p + r; i < k - s; i++)
    for (int i = p; i > (r - 1); i--)
    {
      float alpha = (u - knots[i + k - p]) / (knots[i+1+k-r] - knots[i+k-p]);
      affectedPoints[i].x = (1.0-alpha)*affectedPoints[i-1].x + alpha*affectedPoints[i].x;
      affectedPoints[i].y = (1.0-alpha)*affectedPoints[i-1].y + alpha*affectedPoints[i].y;
    }
  }


  for (int i = 0; i < numKnots; i++)
  {
    cout << knots[i] << " ";
  }
  cout << endl;

  cout << "u: " << u << endl;
  cout << "k: " << k << endl;
  cout << "(" << affectedPoints[p].x << ", " << affectedPoints[p].y << ")" << endl;

  return Point(affectedPoints[p].x, affectedPoints[p].y);
}

Point find_bezier_point(vector<Point> points, float t)
{
  if (points.size() == 1)
    return points[0];
  else
  {
    vector<Point> newPoints;
    for (size_t i = 0; i < points.size() - 1; i++)
    {
      Point a = points[i];
      Point b = points[i + 1];

      float x = ((1 - t) * a.x) + (t * b.x);
      float y = ((1 - t) * a.y) + (t * b.y);

      newPoints.push_back(Point(x, y));
    }

    return find_bezier_point(newPoints, t);
  }

}


//Draws a single "pixel" given the current grid size
//don't change anything in this for project 1
void draw_pix(int x, int y, int z, tuple<int, int, int> color){
  glBegin(GL_POINTS);
  glColor3f(get<0>(color),get<1>(color),get<2>(color));

  glVertex3f(x + .5, y + .5, z + .5);
  glEnd();
}

void draw_lines(Point p1, Point p2, tuple<int, int, int> color)
{
  // Cohen-Sutherland code derived from 
  // https://en.wikipedia.org/wiki/Cohen%E2%80%93Sutherland_algorithm
  glLineWidth(2.0); //sets the "width" of each line we are rendering

  //tells opengl to interperate every two calls to glVertex as a line
  glBegin(GL_LINES);
  //first line will be blue    
  glColor3f(get<0>(color),get<1>(color),get<2>(color));
  glVertex2f(p1.x, p1.y);
  glVertex2f(p2.x,p2.y);

  //this will be a red line
  //notice we can use 3d points too
  //how will this change if we project to the XZ or YZ plane?
  //glColor3f(1.0,0.0,0.0);
  //glVertex3f(0.1,0.9,0.5);
  //glVertex3f(0.9,0.1,0.3);
  glEnd();
}

/*Gets called when display size changes, including initial craetion of the display*/
void reshape(int width, int height)
{
  /*set up projection matrix to define the view port*/
  //update the ne window width and height
  win_width = width;
  win_height = height;

  //creates a rendering area across the window
  glViewport(0,0,width,height);
  // up an orthogonal projection matrix so that
  // the pixel space is mapped to the grid space
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0,grid_width,0,grid_height,-10,10);

  //clear the modelview matrix
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  //set pixel size based on width, if the aspect ratio
  //changes this hack won't work as well
  pixel_size = width/(float)grid_width;

  //set pixel size relative to the grid cell size
  glPointSize(pixel_size);
  //check for opengl errors
  check();

}

//gets called when a key is pressed on the keyboard
void key(unsigned char ch, int x, int y)
{
  if (ch == 'e')
  {
    editMode = true;
  }
  else if (ch == 'n')
  {
    Curve curve;
    curves.push_back(curve);
    currentCurve = curves.size() - 1;
    firstPointSelected = false;
  }
  else if (ch == 'q')
  {
    save();
  }
  else if (ch == 'd')
  {
    curves[currentCurve].controlPoints.erase(curves[currentCurve].controlPoints.begin() + currentPointIndex);
    currentPointIndex = 0;
  }
  else if (ch == 'D')
  {
    curves.erase(curves.begin() + currentCurve);

    Curve curve;
    curves.push_back(curve);
  }
  else if (ch == 'c')
  {
    renderBezier = true;
    renderBSpline = false;
  }
  else if (ch == 'b')
  {
    renderBezier = false;
    renderBSpline = true;
  }
  else if (ch == '=' || ch == '+')
  {
    degree++;
    cout << "Degree is now: " << degree << endl;
  }
  else if (ch == '-')
  {
    degree--;
    cout << "Degree is now: " << degree << endl;
  }
  else if ((int)ch > 0)
  {
    currentCurve = (int)ch - '0' - 1;
    currentPointIndex = 0;
  }
  //redraw the scene after keyboard input
  glutPostRedisplay();
}

void specialKey(int key, int x, int y)
{
  if (key == GLUT_KEY_RIGHT)
  {
  }
  else if (key == GLUT_KEY_LEFT)
  {
  }
  else if (key == GLUT_KEY_UP)
  {
  }
  else if (key == GLUT_KEY_DOWN)
  {
  }
}


//gets called when a mouse button is pressed
void mouse(int button, int state, int x, int y)
{
  //print the pixel location, and the grid location
  //printf ("MOUSE AT PIXEL: %d %d, GRID: %d %d\n",x,y,(int)(x/pixel_size),(int)((win_height-y)/pixel_size));

  leftButtonUsed = false;
  rightButtonUsed = false;
  switch(button)
  {
    case GLUT_LEFT_BUTTON: //left button
      leftButtonUsed = true;
      break;
    case GLUT_RIGHT_BUTTON: //right button
      rightButtonUsed = true;
      break;
    default:
      printf("UNKNOWN "); //any other mouse button
      break;
  }
  if(state !=GLUT_DOWN)  //button released
  {
  }
  else
  {
    if (leftButtonUsed)
    {
      int selectedX = (float)(x/pixel_size);
      int selectedY = (float)((win_height-y)/pixel_size);

      Point p = Point(selectedX, selectedY);
      curves[currentCurve].controlPoints.push_back(p);

    }
    else if (rightButtonUsed)
    {
      int selectedX = (float)(x/pixel_size);
      int selectedY = (float)((win_height-y)/pixel_size);

      int index = findClosestPoint(Point(selectedX, selectedY));

      if (index == 0)
      {
        //curves[currentCurve].controlPoints[currentPointIndex] = Point(selectedX, selectedY);
      }
      else
        currentPointIndex = index - 1;
    }
  }

  //redraw the scene after mouse click
  glutPostRedisplay();
}

//gets called when the curser moves accross the scene
void motion(int x, int y)
{
  if (rightButtonUsed)
  {
    int selectedX = (float)(x/pixel_size);
    int selectedY = (float)((win_height-y)/pixel_size);

    curves[currentCurve].controlPoints[currentPointIndex] = Point(selectedX, selectedY);

    glutPostRedisplay();
  }
}

//checks for any opengl errors in the previous calls and
//outputs if they are present
void check()
{
  GLenum err = glGetError();
  if(err != GL_NO_ERROR)
  {
    printf("GLERROR: There was an error %s\n",gluErrorString(err) );
    exit(1);
  }
}
