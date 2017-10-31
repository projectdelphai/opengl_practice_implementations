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
map<Point, int, PointComparator> rasterizedPoints;

bool useDDA = false;
bool useBresenham = false;
bool useglLine = true;
bool scaleTurnedOn = false;
bool rotateTurnedOn = false;
bool leftButtonUsed = false;
bool rightButtonUsed = false;
bool XY = true;
bool XZ = false;
bool YZ = false;

int view = 0;
int method = 1;
int numPolygons;
int currentPolygonIndex = 0;

// viewport
int viewport[4];
tuple<float, float, float> blue(.2, .2, 1.0);
tuple<float, float, float> red(1.0, .2, .2);
tuple<float, float, float> green(.2, 1.0, .2);
tuple<float, float, float> black(0.2, 0.2, 0.2);
tuple<float, float, float> currentColor = blue;
char *filename;


void processDatFile();
void init();
void idle();
void display();
void rasterize();
void save();
void draw_pix(int x, int y, int z, bool drawingViewport, tuple<int, int, int> color);
void draw_lines(Point p1, Point p2, tuple<int, int, int> color);
void reshape(int width, int height);
void key(unsigned char ch, int x, int y);
void specialKey(int key, int x, int y);
void mouse(int button, int state, int x, int y);
void motion(int x, int y);
void check();



int main(int argc, char **argv)
{
  filename = argv[1];

  if (argc == 3)
  {
    method = atoi(argv[2]);

    if (method == 1)
    {
      useDDA = true;
    }
    else if (method == 2)
    {
      useBresenham = true;
    }
  }

  //the number of pixels in the grid
  grid_width = 500;
  grid_height = 500;

  // viewport[xmin, ymin, xmax, ymax]
  viewport[0] = -1;
  viewport[1] = -1;
  viewport[2] = grid_width;
  viewport[3] = grid_height;


  //the size of pixels sets the inital window height and width
  //don't make the pixels too large or the screen size will be larger than
  //your display size
  pixel_size = 2;

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
  glutCreateWindow("Project 2");

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

/* read data file for information about polygons */
void processDatFile()
{
  ifstream rawDatFile(filename);
  string line;

  // get number of polygons 
  getline(rawDatFile, line);

  // convert to int
  // note: convert to strtol for better security
  numPolygons = atoi(line.c_str());

  // loop through polygons
  for (int i = 0; i < numPolygons; i++)
  {
    // create polygon
    Polygon *polygon = new Polygon(grid_height);

    // populate with description
    getline(rawDatFile, line);
    polygon->description = line;

    // get number of vertices in polygon
    getline(rawDatFile, line);
    int numVertices = atoi(line.c_str());
    polygon->numVertices = numVertices;

    // loop through every vertex
    for (int j = 0; j < numVertices; j++)
    {
      // separate each line into three points
      getline(rawDatFile, line);
      size_t pos = line.find_first_of(" ");
      size_t pos2;

      string rawX = line.substr(0, pos);

      pos++;
      pos2 = line.find_first_of(" ", pos);

      string rawY = line.substr(pos, pos2);

      string rawZ = line.substr(pos2, string::npos);

      float x = atof(rawX.c_str());
      float y = atof(rawY.c_str());
      float z = atof(rawZ.c_str());

      // add vertex to polygon
      Point *p = new Point(x, y, z);
      polygon->vertices.push_back(p);


    }

    // get number of edges
    getline(rawDatFile, line);
    int numEdges = atoi(line.c_str());
    polygon->numEdges = numEdges;

    for (int j = 0; j < numEdges; j++)
    {
      getline(rawDatFile, line);
      size_t pos = line.find_first_of(" ");

      string rawEdge1 = line.substr(0, pos);
      string rawEdge2 = line.substr(pos, string::npos);

      int edge1 = atoi(rawEdge1.c_str());
      int edge2 = atoi(rawEdge2.c_str());

      polygon->edgeOrder.push_back(pair<int, int>(edge1 - 1, edge2 - 1));

    }


    polygon->calculateCentroid();
   polygons.push_back(polygon);

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

  int i = 0;
  // for every polygon, draw it out
  for (vector<Polygon *>::iterator it = polygons.begin(); it != polygons.end(); it++)
  {
    Polygon *polygon = *it;
    
    bool insideViewPort = true;
    for (int j = 0; j < polygon[i].numVertices; j++)
    {
      float x = polygon[i].vertices[j]->x;
      float y = polygon[i].vertices[j]->y;

      if ((x < 0 || x > 1) || (y < 0 || y > 1))
      {
        insideViewPort = false;
      }
    }

    if (!insideViewPort)
      continue;

    map<Point, PointInfo, PointComparator> edges;

    tuple<int, int, int> color = black;

    if (i == currentPolygonIndex)
    {
      if (scaleTurnedOn)
        color = red;
      else
        color = currentColor;
    }

    for (int edgeOrderIndex = 0; edgeOrderIndex < polygon->numEdges; edgeOrderIndex++)
    {
      int indexA = polygon->edgeOrder[edgeOrderIndex].first;
      int indexB = polygon->edgeOrder[edgeOrderIndex].second;

      Point *p1 = polygon->vertices[indexA];
      Point *p2 = polygon->vertices[indexB];

      // XY
      draw_lines(Point(p1->x, p1->y+1), Point(p2->x, p2->y+1), color);
      // XZ
      draw_lines(Point(p1->x+1, p1->z+1), Point(p2->x+1, p2->z+1), color);
      // YZ
      draw_lines(Point(p1->y, p1->z), Point(p2->y, p2->z), color);


    }

    i++;
  }


  //rasterize();

  //blits the current opengl framebuffer on the screen
  glutSwapBuffers();
  //checks for opengl errors
  check();
}

void rasterize()
{
  rasterizedPoints.clear();
  // draw viewport
  for (int i = viewport[0]; i < (viewport[2] + 1); i++)
  {
    draw_pix(i, viewport[1], 0, true, green);
    draw_pix(i, viewport[3], 0, true, green);
  }
  for (int i = viewport[1]; i < viewport[3]; i++)
  {
    draw_pix(viewport[0], 0, i, true, green);
    draw_pix(viewport[2], 0, i, true, green);
  }

  // for every polygon, fill it in via horizontal scan lines
  int polygonIndex = 0;
  vector<Polygon *>::iterator it;
  for (it = polygons.begin(); it != polygons.end(); it++)
  {
    map<Point, PointInfo>::iterator findIt;

    float yMin = (*it)->yMin;
    float yMax = (*it)->yMax;

    bool on = false;

    float previousX = 0;
    for (float j = yMin; j < yMax + 1; j++)
    {
      for (float i = 0; i < grid_width; i++)
      {
        // find the floating point edge given an integer point
        findIt = (*it)->findApproxPoint(i, j);

        if (findIt != (*it)->edgesXY.end())
        {
          if (i == (previousX + 1))
          {
            bool nextPointExists = false;
            map<Point, PointInfo>::iterator it2;
            for (it2 = (*it)->edgesXY.begin(); it2 != (*it)->edgesXY.end(); it2++)
            {
              if (((int)it2->first.y == 79 && j == 79))
              {
              }
              if (((int)it2->first.y) == j)
              {
                if ((int)it2->first.x > i)
                {
                  nextPointExists = true;
                }
              }
            }
            if (!nextPointExists)
            {
              on = false;
            }
            continue;

          }

          if (findIt->second.partOfHorizontal == true)
          {
            on = false;
            continue;
          }

          previousX = i;
          // as long as it's not an extrema, flip the on bit
          // to start drawing or stop drawing
          if (findIt->second.extrema != true)
          {
            on = !on;
            continue;
          }
        }

        if ((on == true))
        {
          if (polygonIndex == currentPolygonIndex)
          {
            if (scaleTurnedOn)
              draw_pix(i, j, 0, false, green);
            else if (rotateTurnedOn)
            {
              draw_pix(i,j, 0, false, blue);
            }
            else
              draw_pix(i, j, 0, false, red);
          }
          else
            draw_pix(i, j, 0, false, black);
          // keep track of rasterized points for mouse interactions
          rasterizedPoints.insert(pair<Point, int>(Point(i, j), polygonIndex));
        }

      }
      on = false;

      //draw_pix((*it)->centroid.x, (*it)->centroid.y, false, black);

    }
    polygonIndex++;
  }

}

void save()
{

  ofstream file;
  file.open(filename);
  file << numPolygons << "\n";

  for (int i = 0; i < numPolygons; i++)
  {
    file << polygons[i]->description << "\n";
    file << polygons[i]->numVertices << "\n";
    for (int j = 0; j < polygons[i]->numVertices; j++)
    {
      file << (polygons[i]->vertices[j]->x) << " " << (polygons[i]->vertices[j]->y) << " " << (polygons[i]->vertices[j]->z) << "\n";
    }
    file << polygons[i]->numEdges << "\n";
    for (int j = 0; j < polygons[i]->numEdges; j++)
    {
      file << (polygons[i]->edgeOrder[j].first+1) << " " << (polygons[i]->edgeOrder[j].second+1) << "\n";
    }
  }

  file.close();

  glutDestroyWindow(glutGetWindow());
  exit(0);

}



//Draws a single "pixel" given the current grid size
//don't change anything in this for project 1
void draw_pix(int x, int y, int z, bool drawingViewPort, tuple<int, int, int> color){
  bool pixInside=((viewport[0] < x && x < viewport[2]) && (viewport[1] < y && y < viewport[3]));

  if (pixInside || drawingViewPort)
  {
    glBegin(GL_POINTS);
    glColor3f(get<0>(color),get<1>(color),get<2>(color));

    glVertex3f(x + .5, y + .5, z + .5);



    glEnd();
  }
}

void draw_lines(Point p1, Point p2, tuple<int, int, int> color){
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
  glOrtho(0,2,0,2,-10,10);

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

  rasterizedPoints.clear();
}

//gets called when a key is pressed on the keyboard
void key(unsigned char ch, int x, int y)
{
  if (ch == 's')
  {
    scaleTurnedOn = !scaleTurnedOn;
    if (scaleTurnedOn)
      rotateTurnedOn = false;
  }
  else if (ch == 'r')
  {
    string x;
    cout << "Please enter point 1, point 2, and the rotation angle in the following format:" << endl;
    cout << "x1 y1 z1 x2 y2 z2 angle" << endl;
    
    getline(cin, x);
    istringstream s(x);
    vector<string> components;

    for (string key; s >> key;)
    {
      components.push_back(key);
    }

    float p1x = atof(components[0].c_str());
    float p1y = atof(components[1].c_str());
    float p1z = atof(components[2].c_str());
    float p2x = atof(components[3].c_str());
    float p2y = atof(components[4].c_str());
    float p2z = atof(components[5].c_str());
    float angle = atof(components[6].c_str());

    Point p1 = Point(p1x, p1y, p1z);
    Point p2 = Point(p2x, p2y, p2z);

    polygons[currentPolygonIndex]->specialRotate(p1, p2, angle);

  }
  else if (ch == 'p')
  {
    // cycle through colors
    tuple<float, float, float> c = currentColor;
    float first_color = get<0>(c);
    float second_color = get<1>(c);
    float third_color = get<2>(c);

    if ((int)first_color == 1)
    {
      first_color = 0.0;
      second_color += 0.1;
   }
    else if ((int)second_color == 1)
    {
      second_color = 0;
      third_color += 0.1;
    }
    else if ((int)third_color == 1)
    {
      third_color = 0;
      first_color += 0.1;
    }
    else
      first_color += 0.1;

    cout << first_color << ", " << second_color << ", " << third_color << endl;
    currentColor = tuple<float, float, float>(first_color, second_color, third_color);

  }
  else if (ch == 'z')
    polygons[currentPolygonIndex]->rotate(1.0, 0);
  else if (ch == 'Z')
    polygons[currentPolygonIndex]->rotate(-1.0, 0);
  else if (ch == 'x')
    polygons[currentPolygonIndex]->rotate(1.0, 1);
  else if (ch == 'X')
    polygons[currentPolygonIndex]->rotate(-1.0, 1);
  else if (ch == 'y')
    polygons[currentPolygonIndex]->rotate(1.0, 2);
  else if (ch == 'Y')
    polygons[currentPolygonIndex]->rotate(-1.0, 2);
  else if (ch == 'q')
  {
    save();
  }
  else if (ch == '+' || ch == '=')
  {
    if (scaleTurnedOn)
    {
      polygons[currentPolygonIndex]->scale(1.05, 1.05, 1.05);
      rasterizedPoints.clear();
    }
    else if (!scaleTurnedOn && !rotateTurnedOn)
    {
      float centerx = polygons[currentPolygonIndex]->centroid.x;
      float centery = polygons[currentPolygonIndex]->centroid.y;
      float centerz = polygons[currentPolygonIndex]->centroid.z;
      polygons[currentPolygonIndex]->translate(Point(centerx, centery, centerz + .1));
    }
  }
  else if (ch == '-')
  {
    if (scaleTurnedOn)
    {
      polygons[currentPolygonIndex]->scale(.9, .9, .9);
      rasterizedPoints.clear();
    }
    else if (!scaleTurnedOn && !rotateTurnedOn)
    {
      float centerx = polygons[currentPolygonIndex]->centroid.x;
      float centery = polygons[currentPolygonIndex]->centroid.y;
      float centerz = polygons[currentPolygonIndex]->centroid.z;
      polygons[currentPolygonIndex]->translate(Point(centerx, centery, centerz - .1));
    }

  }
  else if ((int)ch > 0)
  {
    currentPolygonIndex = (int)ch - '0' - 1;
  }
  //redraw the scene after keyboard input
  glutPostRedisplay();
}

void specialKey(int key, int x, int y)
{
  float centerx = polygons[currentPolygonIndex]->centroid.x;
  float centery = polygons[currentPolygonIndex]->centroid.y;
  float centerz = polygons[currentPolygonIndex]->centroid.z;
  if (key == GLUT_KEY_RIGHT)
  {
    polygons[currentPolygonIndex]->translate(Point(centerx + .1, centery, centerz));
  }
  else if (key == GLUT_KEY_LEFT)
  {
    polygons[currentPolygonIndex]->translate(Point(centerx - .1, centery, centerz));
  }
  else if (key == GLUT_KEY_UP)
  {
    polygons[currentPolygonIndex]->translate(Point(centerx, centery + .1, centerz));
  }
  else if (key == GLUT_KEY_DOWN)
  {
    polygons[currentPolygonIndex]->translate(Point(centerx, centery - .1, centerz));
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
      /*
      int selectedX = (int)(x/pixel_size);
      int selectedY = (int)((win_height-y)/pixel_size);

      for (int i = 0; i < numPolygons; i++)
      {
        map<Point, int>::iterator ret;
        ret = rasterizedPoints.find(Point(selectedX, selectedY));

        if (ret != rasterizedPoints.end())
        {
          currentPolygonIndex = ret->second;
          break;
        }
      }*/
    }
    else if (rightButtonUsed)
    {
      /*
      int x2 = (int)(x/pixel_size);
      int y2 = (int)((win_height-y)/pixel_size);

      // for the min point
      int x1 = viewport[0];
      int y1 = viewport[1];
      int dx = x2 - x1;
      int dy = y2 - y1;
      float minDistance = sqrt((dx*dx) + (dy*dy));

      x1 = viewport[2];
      y1 = viewport[3];
      dx = x2 - x1;
      dy = y2 - y1;
      float maxDistance = sqrt((dx*dx) + (dy*dy));

      if (minDistance < maxDistance)
      {
        viewport[0] = x2;
        viewport[1] = y2;
      }
      else
      {
        viewport[2] = x2;
        viewport[3] = y2;
      }

      if (viewport[0] > viewport[2])
      {
        int temp = viewport[0];
        viewport[0] = viewport[2];
        viewport[2] = temp;
      }
      if (viewport[1] > viewport[3])
      {
        int temp = viewport[1];
        viewport[1] = viewport[3];
        viewport[3] = temp;
      }

      cout << viewport[0] << ", " << viewport[1] << ", " << viewport[2] << ", " << viewport[3] << endl;
      */

    }
  }

  //redraw the scene after mouse click
  glutPostRedisplay();
}

//gets called when the curser moves accross the scene
void motion(int x, int y)
{
  if (leftButtonUsed)
  {
    int selectedX = (int)(x/pixel_size);
    int selectedY = (int)((win_height-y)/pixel_size);
    polygons[currentPolygonIndex]->translate(Point(selectedX, selectedY));
    rasterizedPoints.clear();
    //redraw the scene after mouse movement
  }

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
