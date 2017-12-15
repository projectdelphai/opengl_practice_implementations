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
map<Point, PointInfo, PointComparator> edges;
map<Point, int, PointComparator> rasterizedPoints;

bool useDDA = false;
bool useBresenham = true;
bool scaleTurnedOn = false;
bool rotateTurnedOn = false;
bool leftButtonUsed = false;
bool rightButtonUsed = false;

int method = 1;
int numPolygons;
int currentPolygonIndex = 0;

// viewport
int viewport[4];
tuple<int, int, int> blue(.2, .2, 1.0);
tuple<int, int, int> red(1.0, .2, .2);
tuple<int, int, int> green(.2, 1.0, .2);
tuple<int, int, int> black(0.2, 0.2, 0.2);
char *filename;


void processDatFile();
void init();
void idle();
void display();
void rasterize();
void save();
void draw_pix(int x, int y, bool drawingViewport, tuple<int, int, int> color);
void reshape(int width, int height);
void key(unsigned char ch, int x, int y);
void specialKey(int key, int x, int y);
void mouse(int button, int state, int x, int y);
void motion(int x, int y);
void check();



int main(int argc, char **argv)
{
  filename = argv[1];
  method = atoi(argv[2]);

  if (method == 1)
  {
    useDDA = true;
    useBresenham = false;
  }
  else if (method == 2)
  {
    useDDA = false;
    useBresenham = true;
  }

  //the number of pixels in the grid
  grid_width = 100;
  grid_height = 100;

  // viewport[xmin, ymin, xmax, ymax]
  viewport[0] = -1;
  viewport[1] = -1;
  viewport[2] = grid_width;
  viewport[3] = grid_height;


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
      getline(rawDatFile, line);
      int pos = line.find_first_of(" ");
      string rawX = line.substr(0, pos);
      string rawY = line.substr(pos, string::npos);

      float x = atof(rawX.c_str());
      float y = atof(rawY.c_str());

      // add vertex to polygon
      Point *p = new Point(x, y);
      polygon->vertices.push_back(p);


    }

    polygon->calculateCentroid();
    if (useDDA)
      polygon->calculateEdges(true);
    else
      polygon->calculateEdges(false);

    polygons.push_back(polygon);

    // pretty sure this isn't necessary, remove after confirming
    map<Point, PointInfo>::iterator it;
    for (it = polygon->edges.begin(); it != polygon->edges.end(); it++)
    {
      //edges.insert(pair<Point, PointInfo>(it->first, it->second));
    }

  }

}

/*void recalculateRasterizedEdges()
{
  for (int i = 0; i < numPolygons; i++)
  {
    Polygon *polygon = polygons[i];
    edges.clear();

    map<Point, PointInfo>::iterator it;
    for (it = polygon->edges.begin(); it != polygon->edges.end(); it++)
    {
      edges.insert(pair<Point, PointInfo>(it->first, it->second));
    }


  }
}*/

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
    int numVertices = polygon->numVertices;
    for (int i = 0; i < numVertices; i++)
    {
      float x = polygon->vertices[i]->x;
      float y = polygon->vertices[i]->y;

      draw_pix(x, y, false, black);
    }

    map<Point, PointInfo>::iterator it2;
    for (it2 = polygon->edges.begin(); it2 != polygon->edges.end(); it2++)
    {
      draw_pix(it2->first.x, it2->first.y, false, black);
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
  rasterizedPoints.clear();
  // draw viewport
  for (int i = viewport[0]; i < (viewport[2] + 1); i++)
  {
    draw_pix(i, viewport[1], true, green);
    draw_pix(i, viewport[3], true, green);
  }
  for (int i = viewport[1]; i < viewport[3]; i++)
  {
    draw_pix(viewport[0], i, true, green);
    draw_pix(viewport[2], i, true, green);
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

        if (findIt != (*it)->edges.end())
        {
          if (i == (previousX + 1))
          {
            bool nextPointExists = false;
            map<Point, PointInfo>::iterator it2;
            for (it2 = (*it)->edges.begin(); it2 != (*it)->edges.end(); it2++)
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
              draw_pix(i, j, false, green);
            else if (rotateTurnedOn)
            {
              draw_pix(i,j, false, blue);
            }
            else
              draw_pix(i, j, false, red);
          }
          else
            draw_pix(i, j, false, black);
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
      file << polygons[i]->vertices[j]->x << " " << polygons[i]->vertices[j]->y << '\n';
    }
  }

  file.close();

  glutDestroyWindow(glutGetWindow());
  exit(0);

}



//Draws a single "pixel" given the current grid size
//don't change anything in this for project 1
void draw_pix(int x, int y, bool drawingViewPort, tuple<int, int, int> color){
  bool pixInside=((viewport[0] < x && x < viewport[2]) && (viewport[1] < y && y < viewport[3]));

  if (pixInside || drawingViewPort)
  {
    glBegin(GL_POINTS);
    glColor3f(get<0>(color),get<1>(color),get<2>(color));
    glVertex3f(x+.5,y+.5,0);
    glEnd();
  }
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
    rotateTurnedOn = !rotateTurnedOn;
    if (rotateTurnedOn)
      scaleTurnedOn = false;
  }
  else if (ch == 'q')
  {
    save();
  }
  
  else if (ch == '+' || ch == '=')
  {
    if (scaleTurnedOn)
    {
      polygons[currentPolygonIndex]->scale(1.05, 1.05);
      rasterizedPoints.clear();
    }
    else if (rotateTurnedOn)
    {
      polygons[currentPolygonIndex]->rotate(1.0);
      rasterizedPoints.clear();
    }
  }
  else if (ch == '-')
  {
    if (scaleTurnedOn)
    {
      polygons[currentPolygonIndex]->scale(.9, .9);
      rasterizedPoints.clear();
    }
    else if (rotateTurnedOn)
    {
      polygons[currentPolygonIndex]->rotate(-1.0);
      rasterizedPoints.clear();
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
  if (key == GLUT_KEY_RIGHT)
  {
    polygons[currentPolygonIndex]->translate(Point(centerx + 1, centery));
  }
  else if (key == GLUT_KEY_LEFT)
  {
    polygons[currentPolygonIndex]->translate(Point(centerx - 1, centery));
  }
  else if (key == GLUT_KEY_UP)
  {
    polygons[currentPolygonIndex]->translate(Point(centerx, centery + 1));
  }
  else if (key == GLUT_KEY_DOWN)
  {
    polygons[currentPolygonIndex]->translate(Point(centerx, centery - 1));
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
      }
    }
    else if (rightButtonUsed)
    {
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
