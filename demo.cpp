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
#include <fstream>
#include <set>

#include "polygon.h"


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

// vector of polygons to display from file
vector<Polygon *> polygons;
map<Point, PointInfo, PointComparator> edges;
bool useDDA = true;

void processDatFile();
void init();
void idle();
void display();
void rasterize();
void draw_pix(int x, int y);
void reshape(int width, int height);
void key(unsigned char ch, int x, int y);
void mouse(int button, int state, int x, int y);
void motion(int x, int y);
void check();



int main(int argc, char **argv)
{
  //the number of pixels in the grid
  grid_width = 100;
  grid_height = 100;


  //the size of pixels sets the inital window height and width
  //don't make the pixels too large or the screen size will be larger than
  //your display size
  pixel_size = 5;

  /*Window information*/
  win_height = grid_height*pixel_size;
  win_width = grid_width*pixel_size;

  /*Set up glut functions*/
  /** See https://www.opengl.org/resources/libraries/glut/spec3/spec3.html ***/

  processDatFile();

  glutInit(&argc,argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  /*initialize variables, allocate memory, create buffers, etc. */
  //create window of size (win_width x win_height
  glutInitWindowSize(win_width,win_height);
  //windown title is "glut demo"
  glutCreateWindow("Project 1");

  /*defined glut callback functions*/
  glutDisplayFunc(display); //rendering calls here
  glutReshapeFunc(reshape); //update GL on window size change
  glutMouseFunc(mouse);     //mouse button events
  glutMotionFunc(motion);   //mouse movement events
  glutKeyboardFunc(key);    //Keyboard events
  glutIdleFunc(idle);       //Function called while program is sitting "idle"

  //initialize opengl variables
  init();
  //start glut event loop
  glutMainLoop();
  return 0;
}

/* read data file for information about polygons */
void processDatFile()
{
  ifstream rawDatFile("test_1.dat");
  string line;

  // get number of polygons 
  getline(rawDatFile, line);

  // convert to int
  // note: convert to strtol for better security
  int numPolygons = atoi(line.c_str());

  // loop through polygons
  for (int i = 0; i < numPolygons; i++)
  {
    // create polygon
    Polygon *polygon = new Polygon(grid_height);

    // populate with description
    getline(rawDatFile, line);
    polygon->description = line;

    // get number of vertexes in polygon
    getline(rawDatFile, line);
    int numVertexes = atoi(line.c_str());
    polygon->numVertexes = numVertexes;

    // loop through every vertex
    for (int j = 0; j < numVertexes; j++)
    {
      getline(rawDatFile, line);
      int pos = line.find_first_of(" ");
      string rawX = line.substr(0, pos);
      string rawY = line.substr(pos, string::npos);

      float x = atof(rawX.c_str());
      float y = atof(rawY.c_str());

      // add vertex to polygon
      Point *p = new Point(x, y);
      polygon->vertexes.push_back(p);


    }

    polygon->calculateEdges(useDDA);
    polygons.push_back(polygon);

    map<Point, PointInfo>::iterator it;
    for (it = polygon->edges.begin(); it != polygon->edges.end(); it++)
    {
      edges.insert(pair<Point, PointInfo>(it->first, it->second));
    }

  }

}

/*initialize gl stufff*/
void init()
{
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

  // for every polygon, draw it out
  for (vector<Polygon *>::iterator it = polygons.begin(); it != polygons.end(); it++)
  {
    Polygon *polygon = *it;
    int numVertexes = polygon->numVertexes;
    for (int i = 0; i < numVertexes; i++)
    {
      float x = polygon->vertexes[i]->x;
      float y = polygon->vertexes[i]->y;

      draw_pix(x, y);
    }

    map<Point, PointInfo>::iterator it2;
    for (it2 = polygon->edges.begin(); it2 != polygon->edges.end(); it2++)
    {
      draw_pix(it2->first.x, it2->first.y);
    }

  }

  rasterize();

  //blits the current opengl framebuffer on the screen
  glutSwapBuffers();
  //checks for opengl errors
  check();
}

void rasterize()
{
  vector<Polygon *>::iterator it;
  for (it = polygons.begin(); it != polygons.end(); it++)
  {
    map<Point, PointInfo, PointComparator> currentEdges = (*it)->edges;
    map<Point, PointInfo>::iterator findIt;

    float yMin = (*it)->yMin;
    float yMax = (*it)->yMax;

    //cout << "ymin: " << yMin << ", ymax: " << yMax << endl;

    bool on = false;

    for (float j = yMin; j < yMax + 1; j++)
    {
      for (float i = 0; i < grid_width; i++)
      {
        findIt = (*it)->findApproxPoint(i, j);



        if (findIt != (*it)->edges.end())
        {
          if (j == 6)
          {
            //cout << findIt->first.x << ", " << findIt->first.y << ", extrema: " << findIt->second.extrema << ", horiz: " << findIt->second.partOfHorizontal << endl;
            //cout << findIt->first.x << ", " << findIt->first.y << ", on: " << on << endl;
          }

          if (findIt->second.partOfHorizontal == true)
          {
            on = false;
            continue;
          }
          if (findIt->second.extrema != true)
          {
            on = !on;
          }
        }

        if ((on == true))
        {
          draw_pix(i, j);
        }

      }
      on = false;

    }
  }

}



//Draws a single "pixel" given the current grid size
//don't change anything in this for project 1
void draw_pix(int x, int y){
  glBegin(GL_POINTS);
  glColor3f(.2,.2,1.0);
  glVertex3f(x+.5,y+.5,0);
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
  switch(ch)
  {
    default:
      //prints out which key the user hit
      printf("User hit the \"%c\" key\n",ch);
      break;
  }
  //redraw the scene after keyboard input
  glutPostRedisplay();
}


//gets called when a mouse button is pressed
void mouse(int button, int state, int x, int y)
{
  //print the pixel location, and the grid location
  printf ("MOUSE AT PIXEL: %d %d, GRID: %d %d\n",x,y,(int)(x/pixel_size),(int)((win_height-y)/pixel_size));
  switch(button)
  {
    case GLUT_LEFT_BUTTON: //left button
      printf("LEFT ");
      break;
    case GLUT_RIGHT_BUTTON: //right button
      printf("RIGHT ");
    default:
      printf("UNKNOWN "); //any other mouse button
      break;
  }
  if(state !=GLUT_DOWN)  //button released
    printf("BUTTON UP\n");
  else
    printf("BUTTON DOWN\n");  //button clicked

  //redraw the scene after mouse click
  glutPostRedisplay();
}

//gets called when the curser moves accross the scene
void motion(int x, int y)
{
  //redraw the scene after mouse movement
  glutPostRedisplay();
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
