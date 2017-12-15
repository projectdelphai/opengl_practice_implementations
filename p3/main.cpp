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
#include <algorithm>

#include "polygon.h"
#include "material.h"
#include "edge.h"


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
int nextPane = 1;

// vector of polygons to display from file
vector<Polygon *> polygons;
map<Point, int, PointComparator> rasterizedPoints;

bool useDDA = false;
bool useBresenham = false;
bool useglLine = true;
bool scaleTurnedOn = false;
bool rotateTurnedOn = false;
bool moveLight = false;
bool leftButtonUsed = false;
bool rightButtonUsed = false;
bool illumination = true;
bool XY = true;
bool XZ = false;
bool YZ = false;

int view = 0;
int method = 1;
int numPolygons;
int currentPolygonIndex = 0;

Point rotatep1 = Point(0.0, 0.0, 0.0);
Point rotatep2 = Point(0.0, 0.0, 0.0);

// viewport
tuple<float, float, float> blue(.2, .2, 1.0);
tuple<float, float, float> red(1.0, .2, .2);
tuple<float, float, float> green(.2, 1.0, .2);
tuple<float, float, float> black(0.2, 0.2, 0.2);
tuple<float, float, float> currentColor = blue;
char *filename;

vector<Material *> materials;
Point *li;
Point *la;
float k;
Point *light_position;
Point *from_position;

Point *lineToPoint(string line);
void processInputFile();
void processLightFile();
void init();
void idle();
void display();
void rasterize();
void save();
void draw_pix(float x, float y, tuple<int, int, int> color, int intensity);
void draw_lines(Point p1, Point p2, int viewPort, tuple<int, int, int> color);
void draw_triangles(int indexA, int indexB, int indexC, Polygon *polygon, tuple<int, int, int> color, int plane);
void draw_vertices(Polygon *polygon);
void displayPoint(string line, Point *p);
float findVertexIntensity(Polygon *polygon, Point *p);
void reshape(int width, int height);
void key(unsigned char ch, int x, int y);
void specialKey(int key, int x, int y);
void mouse(int button, int state, int x, int y);
void motion(int x, int y);
void check();
void scanline_edges(Edge e1, Edge e2, int plane, tuple<int, int, int> color);
bool intersect_edge(float scany, const Edge& e, float &x_int);
void render_scanline(float y, float x1, float x2, int plane, float intensity1, float intensity2, tuple<int, int, int> color);
void recalculateNI();
void calculateNormalsIntensities(Polygon *polygon);
void displayVector(string s, Point *p);

void displayVector(string s, Point *p)
{
  cout << s << ": (" << p->x << ", " << p->y << ", " << p->z << ")" << endl;
}

struct ordX
{
  bool operator() (Triangle *a, Triangle *b)
  {
    //float minT1 = a->minVertex(2);
    float maxT1 = a->minVertex(2);
    float maxT2 = b->minVertex(2);

    //return (int)(minT1 * 10) < (int)(maxT2 * 10);
    return (int)(maxT1 * 10) < (int)(maxT2 * 10);
  }
} orderByX;

struct ordY
{
  bool operator() (Triangle *a, Triangle *b)
  {
    //float minT1 = a->minVertex(1);
    float maxT1 = a->minVertex(1);
    float maxT2 = b->minVertex(1);

    //return (int)(minT1 * 10) < (int)(maxT2 * 10);
    return (int)(maxT1 * 10) < (int)(maxT2 * 10);
  }
} orderByY;

struct ordZ
{
  bool operator() (Triangle *a, Triangle *b)
  {
    //float minT1 = a->minVertex(0);
    float maxT1 = a->minVertex(0);
    float maxT2 = b->minVertex(0);

    //return (int)(minT1 * 10) < (int)(maxT2 * 10);
    return (int)(maxT1 * 10) < (int)(maxT2 * 10);
  }
} orderByZ;

void recalculateNI()
{
    for (int i = 0; i < numPolygons; i++)
    {
      calculateNormalsIntensities(polygons[i]);
    }
}

int main(int argc, char **argv)
{
  filename = argv[1];

  //the number of pixels in the grid
  grid_width = 500;
  grid_height = 500;

  nextPane = 3;

  //the size of pixels sets the inital window height and width
  //don't make the pixels too large or the screen size will be larger than
  //your display size
  pixel_size = 2;

  /*Window information*/
  win_height = grid_height*pixel_size;
  win_width = grid_width*pixel_size;

  /*Set up glut functions*/
  /** See https://www.opengl.org/resources/libraries/glut/spec3/spec3.html ***/

  processLightFile();
  processInputFile();

  glutInit(&argc,argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  /*initialize variables, allocate memory, create buffers, etc. */
  //create window of size (win_width x win_height
  glutInitWindowSize(win_width,win_height);
  //windown title is "glut demo"
  glutCreateWindow("Project 3");

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

Point *stringToPoint(string line)
{
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

  Point *s = new Point(x, y, z);

  return s;
}

// read light and material files
void processLightFile()
{
  ifstream rawDatFile("materials.txt");
  string line;

  // number
  getline(rawDatFile, line);

  int numMaterials = atoi(line.c_str());

  for (int i = 0; i < numMaterials; i++)
  {
    // ka
    getline(rawDatFile, line);
    Point *ka = stringToPoint(line);
    // kd
    getline(rawDatFile, line);
    Point *kd = stringToPoint(line);
    // ks
    getline(rawDatFile, line);
    Point *ks = stringToPoint(line);

    // n
    getline(rawDatFile, line);
    int n = atoi(line.c_str());

    Material *material = new Material(ka, kd, ks, n);
    materials.push_back(material);

  }

  ifstream rawLightFile("light.txt");
  string line2;

  // light position
  getline(rawLightFile, line2);
  light_position = stringToPoint(line2);

  // La
  getline(rawLightFile, line2);
  la = stringToPoint(line2);

  // Li
  getline(rawLightFile, line2);
  li = stringToPoint(line2);

  // from position
  getline(rawLightFile, line2);
  float f = atof(line2.c_str());

  // K
  getline(rawLightFile, line2);
  k = atof(line2.c_str());
  
  from_position = new Point(k - f, k - f, f);
  //from_position = new Point(.5, 1 * f, .5);
}

void populatePlaneTriangles(Polygon *polygon)
{
  for (int i = 0; i < polygon->numTriangles; i++)
  {
    polygon->tXY.push_back(new Triangle(polygon->triangles[i], polygon->vertices));
    polygon->tXZ.push_back(new Triangle(polygon->triangles[i], polygon->vertices));
    polygon->tYZ.push_back(new Triangle(polygon->triangles[i], polygon->vertices));
  }

  sort(polygon->tXY.begin(), polygon->tXY.end(), orderByZ);
  sort(polygon->tXZ.begin(), polygon->tXZ.end(), orderByY);
  sort(polygon->tYZ.begin(), polygon->tYZ.end(), orderByX);

}

void calculateNormalsIntensities(Polygon* polygon)
{
    for (int j = 0; j < polygon->numTriangles; j++)
    {
      Triangle *tri = polygon->triangles[j];
      Point *p0 = polygon->vertices[tri->vertex1];
      Point *p1 = polygon->vertices[tri->vertex2];
      Point *p2 = polygon->vertices[tri->vertex3];

      Point *V = new Point(p1->x - p0->x, p1->y - p0->y, p1->z - p0->z);
      Point *W = new Point(p2->x - p0->x, p2->y - p0->y, p2->z - p0->z);

      Point *normal = new Point(V->y*W->z - V->z*W->y, V->z*W->x - V->x*W->z, V->x*W->y - V->y*W->x);
      float mag = sqrt(normal->x*normal->x + normal->y*normal->y + normal->z*normal->z);
      Point *nn = new Point(normal->x / mag, normal->y / mag, normal->z / mag);

      polygon->triangles[j]->faceNormal = new Point(nn->x, nn->y, nn->z);
    }

    float imin = 0;
    float imax = 0;
    vector<float> rawIntensities;

    for (int x = 0; x < polygon->numVertices; x++)
    {
      Point *p = polygon->vertices[x];
      float intensity = findVertexIntensity(polygon, p);

      if (intensity < imin)
        imin = intensity;
      if (intensity > imax)
        imax = intensity;

      polygon->vertices[x]->intensity = intensity;

      //rawIntensities.push_back(intensity);
    }

    for (int x = 0; x < polygon->numVertices; x++)
    {
      float rawI = polygon->vertices[x]->intensity;
      float intensity = (rawI - imin) / (imax - imin);
      
      intensity = round(intensity * 10);

      polygon->vertices[x]->intensity = intensity;

    }

}

/* read data file for information about polygons */
void processInputFile()
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
    int materialID = atoi(line.c_str());
    polygon->materialID = materialID;

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

    // get triangle vertices
    getline(rawDatFile, line);
    int numTriangles = atoi(line.c_str());
    polygon->numTriangles = numTriangles;

    for (int j = 0; j < numTriangles; j++)
    {
      getline(rawDatFile, line);

      size_t pos = line.find_first_of(" ");
      size_t pos2;

      string rawP1 = line.substr(0, pos);

      pos++;
      pos2 = line.find_first_of(" ", pos);

      string rawP2 = line.substr(pos, pos2);

      string rawP3 = line.substr(pos2, string::npos);

      int p1 = atoi(rawP1.c_str());
      int p2 = atoi(rawP2.c_str());
      int p3 = atoi(rawP3.c_str());

      Triangle *t = new Triangle(p1 - 1, p2 - 1, p3 - 1);
      t->polygonID = i;

      polygon->triangles.push_back(t);

    }
    populatePlaneTriangles(polygon);

    for (int j = 0; j < numVertices; j++)
    {
      Point *p = polygon->vertices[j];
      for (int x = 0; x < polygon->numTriangles; x++)
      {
        Triangle *t = polygon->triangles[x];

        if (t->containsVertex(p, polygon->vertices))
        {
          polygon->vertices[j]->adjacentTris.push_back(x);
        }
      }


    }


    polygon->calculateCentroid();

    calculateNormalsIntensities(polygon);

    polygons.push_back(polygon);

  }

}

/*initialize gl stufff*/
void init()
{
  //set clear color (Default background to white)
  //glClearColor(0.0,0.0,0.0,0.0);
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
    tuple<int, int, int> color = black;

    if (i == currentPolygonIndex)
    {
      if (scaleTurnedOn)
        color = red;
      else
        color = currentColor;
    }

    for (int triangleIndex = 0; triangleIndex < polygon->numTriangles; triangleIndex++)
    {
      int XYindexA = polygon->tXY[triangleIndex]->vertex1;
      int XYindexB = polygon->tXY[triangleIndex]->vertex2;
      int XYindexC = polygon->tXY[triangleIndex]->vertex3;

      int XZindexA = polygon->tXZ[triangleIndex]->vertex1;
      int XZindexB = polygon->tXZ[triangleIndex]->vertex2;
      int XZindexC = polygon->tXZ[triangleIndex]->vertex3;

      int YZindexA = polygon->tYZ[triangleIndex]->vertex1;
      int YZindexB = polygon->tYZ[triangleIndex]->vertex2;
      int YZindexC = polygon->tYZ[triangleIndex]->vertex3;

      draw_triangles(XYindexA, XYindexB, XYindexC, polygon, color, 0);
      draw_triangles(XZindexA, XZindexB, XZindexC, polygon, color, 1);
      draw_triangles(YZindexA, YZindexB, YZindexC, polygon, color, 2);
    }

    //draw_vertices(polygon);

    i++;
  }

  // XY
  draw_pix(light_position->x * 3, light_position->y * 3 + 3, red, 0);
  // XZ
  draw_pix(light_position->x * 3 + 3, light_position->z * 3 + 3, red, 0);
  // YZ
  draw_pix(light_position->y * 3, light_position->z * 3, red, 0);



  //blits the current opengl framebuffer on the screen
  glutSwapBuffers();
  //checks for opengl errors
  check();
}

void draw_vertices(Polygon *polygon)
{
  float factor = 3;
  for (int i = 0; i < polygon->numVertices; i++)
  {
    Point *p = polygon->vertices[i];

    int intensity = polygon->vertices[i]->intensity;

    // XY
    draw_pix(p->x * factor, p->y * factor + nextPane, red, intensity);
    // XZ
    draw_pix(p->x * factor + nextPane, p->z * factor + nextPane, red, intensity);
    // YZ
    draw_pix(p->y * factor, p->z * factor, red, intensity);
  }
}

void displayPoint(string line, Point *f)
{
  cout << line << " = (" << f->x << ", " << f->y << ", " << f->z << ")" << endl;

}

float findVertexIntensity(Polygon *polygon, Point *p)
{
  /*
     vector<Material *> materials;
     tuple<float, float, float> Li; = Il
     tuple<float, float, float> La; = Ia
     float k;
     tuple<float, float, float> light_position;
     tuple<float, float, float> from_position;
     */

  // set up
  Material *material = materials[polygon->materialID - 1];

  Point *ka = new Point(material->ka);
  Point *kd = new Point(material->kd);
  Point *ks = new Point(material->ks);

  Point *f = from_position;
  Point *x = light_position;

  Point *XminusP = new Point(x->x - p->x, x->y - p->y, x->z - p->z);
  Point *FminusP = new Point(f->x - p->x, f->y - p->y, f->z - p->z);

  float magXminusP = sqrt(XminusP->x*XminusP->x + XminusP->y*XminusP->y + XminusP->z*XminusP->z);
  float magFminusP = sqrt(FminusP->x*FminusP->x + FminusP->y*FminusP->y + FminusP->z*FminusP->z);

  // vectors
  Point *l = new Point(XminusP->x / magXminusP, XminusP->y / magXminusP, XminusP->z / magXminusP);

  Point *v = new Point(FminusP->x / magFminusP, FminusP->y / magFminusP, FminusP->z / magFminusP);

  // n vector is the average of all the face vectors of triangles containing the vertex
  Point *rawN = new Point(0, 0, 0);
  int count = 0;

    for (int j = 0; j < p->adjacentTris.size(); j++)
    {
      count++;
      rawN->x += polygon->triangles[p->adjacentTris[j]]->faceNormal->x;
      rawN->y += polygon->triangles[p->adjacentTris[j]]->faceNormal->y;
      rawN->z += polygon->triangles[p->adjacentTris[j]]->faceNormal->z;
    }

  float nmag = sqrt(rawN->x*rawN->x + rawN->y*rawN->y + rawN->z*rawN->z);
  // normalizing n
  Point *n = new Point(rawN->x / nmag, rawN->y / nmag, rawN->z / nmag);

  // r vector is 2(n . l)n - l
  float ldotn = l->x*n->x + l->y*n->y + l->z*n->z;
  float m = 2 * ldotn;
  Point *r1 = new Point(m * n->x, m * n->y, m * n->z);
  Point *rawR = new Point(r1->x - l->x, r1->y - l->y, r1->z - l->z);
  float rmag = sqrt(rawR->x*rawR->x + rawR->y*rawR->y + rawR->z*rawR->z);
  Point *r = new Point(rawR->x / rmag, rawR->y / rmag, rawR->z / rmag);

  float rdotv = r->x*v->x + r->y*v->y + r->z*v->z;

  float intensity = (ka->x*la->x) + ((li->x)/(magFminusP + k))*((kd->x*ldotn) + (ks->x*(pow(rdotv, material->n))));


    /*cout << "r.v: " << rdotv << endl;
    cout << "l.n: " << ldotn << endl;
    cout << "magFminusP: " << magFminusP << endl;
    cout << "materialn: " << material->n << endl;
    cout << "k: " << k << endl;
    displayVector("vertex", p);
    displayVector("ka", ka);
    displayVector("kd", kd);
    displayVector("ks", ks);
    displayVector("li", li);
    displayVector("la", la);
    displayVector("from position", f);
    displayVector("light position", x);
    displayVector("l", l);
    displayVector("v", v);
    displayVector("n", n);
    displayVector("r", r);
    cout << endl;*/
    /*displayVector("vertex", p);
    displayVector("n", n);
    cout << "ambient: " << ka->x*la->x << endl;
    cout << "diffuse: " << kd->x*ldotn << endl;
    cout << "specular: " << ks->x*(pow(rdotv, material->n)) << endl;
    cout << "intensity: " << intensity << endl;
    cout << endl << endl;*/

  return intensity;
}

void scanline_edges(Edge e1, Edge e2, int plane, tuple<int, int, int> color)
{
  if (e1.p1->y == e1.p2->y)
  {
    render_scanline(e1.p1->y, e1.p1->x, e1.p2->x, plane, e1.p1->intensity, e1.p2->intensity, color);
    return;
  }
  if (e2.p1->y == e2.p2->y)
  {
    render_scanline(e2.p2->y, e2.p1->x, e2.p2->x, plane, e2.p1->intensity, e2.p2->intensity, color);
    return;
  }

  float ymin = max(e1.p1->y, e2.p1->y);
  float ymax = min(e1.p2->y, e2.p2->y);

  for (float y = ymin; y <= ymax; y+=1.0)
  {
    float x1, x2;
    intersect_edge(y, e1, x1);
    intersect_edge(y, e2, x2);

    float intensity1 = (((y-e1.p1->y)/(e1.p2->y-e1.p1->y))*(e1.p2->intensity)) + (((e1.p2->y-y)/(e1.p2->y-e1.p1->y))*(e1.p1->intensity));
    float intensity2 = (((y-e2.p1->y)/(e2.p2->y-e2.p1->y))*(e2.p2->intensity)) + (((e2.p2->y-y)/(e2.p2->y-e2.p1->y))*(e2.p1->intensity));

    if (x1 < x2)
      render_scanline(y, x1, x2, plane, intensity1, intensity2, color);
    else
      render_scanline(y, x2, x1, plane, intensity2, intensity1, color);
  }

}

bool intersect_edge(float scany, const Edge& e, float &x_int){

  // parallel lines
  if(e.p2->y==e.p1->y) 
    return false;

  x_int = (scany-e.p1->y)*(e.p2->x-e.p1->x)/(e.p2->y-e.p1->y)+e.p1->x;
  return true;
}

//could include intensity values and use those to interpolate here
void render_scanline(float y, float x1, float x2, int plane, float intensity1, float intensity2, tuple<int,int,int> color){
  int py=(int)floor(y);

  for(int px = (int)floor(x1); px < x2; px++)
  {
    float px2 = px / 100.0;
    float py2 = py / 100.0;

    float rawIntensity = (((x2-px)/(x2-x1))*intensity1)+(((px-x1)/(x2-x1))*intensity2);

    int intensity = round(rawIntensity);

    if (plane == 0)
      draw_pix(px2 * 3, py2 * 3 + nextPane, color, intensity);
    else if (plane == 1)
      draw_pix(px2 * 3 + nextPane, py2 * 3 + nextPane, color, intensity);
    else if (plane == 2)
      draw_pix(px2 * 3, py2 * 3, color, intensity);

  }
}

void draw_triangles(int indexA, int indexB, int indexC, Polygon *polygon, tuple<int, int, int> color, int plane)
{
  Point *p1 = polygon->vertices[indexA];
  Point *p2 = polygon->vertices[indexB];
  Point *p3 = polygon->vertices[indexC];

  Edge e1, e2, e3;

  if (plane == 0)
  {
    // XY
    if (p1->y < p2->y)
    {
      e1.p1 = new Point(p1->x, p1->y);
      e1.p2 = new Point(p2->x, p2->y);
      e1.p1->intensity = p1->intensity;
      e1.p2->intensity = p2->intensity;
    }
    else
    {
      e1.p1 = new Point(p2->x, p2->y);
      e1.p2 = new Point(p1->x, p1->y);
      e1.p1->intensity = p2->intensity;
      e1.p2->intensity = p1->intensity;
    }
    if (p1->y < p3->y)
    {
      e2.p1 = new Point(p1->x, p1->y);
      e2.p2 = new Point(p3->x, p3->y);
      e2.p1->intensity = p1->intensity;
      e2.p2->intensity = p3->intensity;
    }
    else
    {
      e2.p1 = new Point(p3->x, p3->y);
      e2.p2 = new Point(p1->x, p1->y);
      e2.p1->intensity = p3->intensity;
      e2.p2->intensity = p1->intensity;
    }
    if (p2->y < p3->y)
    {
      e3.p1 = new Point(p2->x, p2->y);
      e3.p2 = new Point(p3->x, p3->y);
      e3.p1->intensity = p2->intensity;
      e3.p2->intensity = p3->intensity;
    }
    else
    {
      e3.p1 = new Point(p3->x, p3->y);
      e3.p2 = new Point(p2->x, p2->y);
      e3.p1->intensity = p3->intensity;
      e3.p2->intensity = p2->intensity;
    }

    e1.scaleUp();
    e2.scaleUp();
    e3.scaleUp();
    scanline_edges(e1, e2, plane, color);
    scanline_edges(e1, e3, plane, color);
    scanline_edges(e2, e3, plane, color);
    e1.scaleDown();
    e2.scaleDown();
    e3.scaleDown();
  }

  else if (plane == 1)
  {

    // XZ
    if (p1->z < p2->z)
    {
      e1.p1 = new Point(p1->x, p1->z);
      e1.p2 = new Point(p2->x, p2->z);
      e1.p1->intensity = p1->intensity;
      e1.p2->intensity = p2->intensity;
    }
    else
    {
      e1.p1 = new Point(p2->x, p2->z);
      e1.p2 = new Point(p1->x, p1->z);
      e1.p1->intensity = p2->intensity;
      e1.p2->intensity = p1->intensity;
    }
    if (p1->z < p3->z)
    {
      e2.p1 = new Point(p1->x, p1->z);
      e2.p2 = new Point(p3->x, p3->z);
      e2.p1->intensity = p1->intensity;
      e2.p2->intensity = p3->intensity;
    }
    else
    {
      e2.p1 = new Point(p3->x, p3->z);
      e2.p2 = new Point(p1->x, p1->z);
      e2.p1->intensity = p3->intensity;
      e2.p2->intensity = p1->intensity;
    }
    if (p2->z < p3->z)
    {
      e3.p1 = new Point(p2->x, p2->z);
      e3.p2 = new Point(p3->x, p3->z);
      e3.p1->intensity = p2->intensity;
      e3.p2->intensity = p3->intensity;
    }
    else
    {
      e3.p1 = new Point(p3->x, p3->z);
      e3.p2 = new Point(p2->x, p2->z);
      e3.p1->intensity = p3->intensity;
      e3.p2->intensity = p2->intensity;
    }

    e1.scaleUp();
    e2.scaleUp();
    e3.scaleUp();
    scanline_edges(e1, e2, plane, color);
    scanline_edges(e1, e3, plane, color);
    scanline_edges(e2, e3, plane, color);
    e1.scaleDown();
    e2.scaleDown();
    e3.scaleDown();
  }

  else if (plane == 2)
  {

    // YZ
    if (p1->z < p2->z)
    {
      e1.p1 = new Point(p1->y, p1->z);
      e1.p2 = new Point(p2->y, p2->z);
      e1.p1->intensity = p1->intensity;
      e1.p2->intensity = p2->intensity;
    }
    else
    {
      e1.p1 = new Point(p2->y, p2->z);
      e1.p2 = new Point(p1->y, p1->z);
      e1.p1->intensity = p2->intensity;
      e1.p2->intensity = p1->intensity;
    }
    if (p1->z < p3->z)
    {
      e2.p1 = new Point(p1->y, p1->z);
      e2.p2 = new Point(p3->y, p3->z);
      e2.p1->intensity = p1->intensity;
      e2.p2->intensity = p3->intensity;
    }
    else
    {
      e2.p1 = new Point(p3->y, p3->z);
      e2.p2 = new Point(p1->y, p1->z);
      e2.p1->intensity = p3->intensity;
      e2.p2->intensity = p1->intensity;
    }
    if (p2->z < p3->z)
    {
      e3.p1 = new Point(p2->y, p2->z);
      e3.p2 = new Point(p3->y, p3->z);
      e3.p1->intensity = p2->intensity;
      e3.p2->intensity = p3->intensity;
    }
    else
    {
      e3.p1 = new Point(p3->y, p3->z);
      e3.p2 = new Point(p2->y, p2->z);
      e3.p1->intensity = p3->intensity;
      e3.p2->intensity = p2->intensity;
    }

    e1.scaleUp();
    e2.scaleUp();
    e3.scaleUp();
    scanline_edges(e1, e2, plane, color);
    scanline_edges(e1, e3, plane, color);
    scanline_edges(e2, e3, plane, color);
    e1.scaleDown();
    e2.scaleDown();
    e3.scaleDown();
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
    file << polygons[i]->materialID << "\n";
    file << polygons[i]->numVertices << "\n";
    for (int j = 0; j < polygons[i]->numVertices; j++)
    {
      file << (polygons[i]->vertices[j]->x) << " " << (polygons[i]->vertices[j]->y) << " " << (polygons[i]->vertices[j]->z) << "\n";
    }
    file << polygons[i]->numTriangles << "\n";
    for (int j = 0; j < polygons[i]->numTriangles; j++)
    {
      file << polygons[i]->triangles[j]->vertex1 + 1 << " " << polygons[i]->triangles[j]->vertex2 + 1 << " " << polygons[i]->triangles[j]->vertex3 + 1 << "\n";
    }
  }

  file.close();

  glutDestroyWindow(glutGetWindow());
}



//Draws a single "pixel" given the current grid size
//don't change anything in this for project 1
void draw_pix(float x, float y, tuple<int, int, int> color, int intensity)
{

  // dark points
  glBegin(GL_POINTS);
  glColor3f(get<0>(color),get<1>(color),get<2>(color));
  if (intensity < 1)
    glVertex3f(x+.01, y+.01, 0); // 1
  if (intensity < 2)
    glVertex3f(x+.02, y+.01, 0); // 2
  if (intensity < 3)
    glVertex3f(x+.01, y+.02, 0); // 3
  if (intensity < 4)
    glVertex3f(x+.00, y+.00, 0); // 4
  if (intensity < 5)
    glVertex3f(x+.00, y+.01, 0); // 5
  if (intensity < 6)
    glVertex3f(x+.02, y+.00, 0); // 6
  if (intensity < 7)
    glVertex3f(x+.02, y+.02, 0); // 7
  if (intensity < 8)
    glVertex3f(x+.00, y+.02, 0); // 8
  if (intensity < 9)
    glVertex3f(x+.01, y+.00, 0); // 9
  glEnd();
  // white points
  glBegin(GL_POINTS);
  glColor3f(1, 1, 1);
  if (intensity > 0)
    glVertex3f(x+.01, y+.01, 0); // 1
  if (intensity > 1)
    glVertex3f(x+.02, y+.01, 0); // 2
  if (intensity > 2)
    glVertex3f(x+.01, y+.02, 0); // 3
  if (intensity > 3)
    glVertex3f(x+.00, y+.00, 0); // 4
  if (intensity > 4)
    glVertex3f(x+.00, y+.01, 0); // 5
  if (intensity > 5)
    glVertex3f(x+.02, y+.00, 0); // 6
  if (intensity > 6)
    glVertex3f(x+.02, y+.02, 0); // 7
  if (intensity > 7)
    glVertex3f(x+.00, y+.02, 0); // 8
  if (intensity > 8)
    glVertex3f(x+.01, y+.00, 0); // 9
  glEnd();
}

int computeCode(float x, float y, int xmin, int xmax, int ymin, int ymax)
{
  int code = 0;

  if (x < xmin) // too far left
    code |= 1;
  else if (x > xmax) // too far right
    code |= 2;
  if (y < ymin) // too far below
    code |= 4;
  else if (y > ymax) // too far above
    code |= 8;

  return code;

}
void draw_lines(Point p1, Point p2, int viewPort, tuple<int, int, int> color)
{
  // Cohen-Sutherland code derived from 
  // https://en.wikipedia.org/wiki/Cohen%E2%80%93Sutherland_algorithm
  float xmin, xmax, ymin, ymax;

  if (viewPort == 1)
  {
    xmin = 0;
    xmax = 1;
    ymin = 1;
    ymax = 2;
  }
  else if (viewPort == 2)
  {
    xmin = 1;
    xmax = 2;
    ymin = 1;
    ymax = 2;
  }
  else if (viewPort == 3)
  {
    xmin = 0;
    xmax = 1;
    ymin = 0;
    ymax = 1;
  }
  else if (viewPort == 4)
  {
    xmin = 1;
    xmax = 2;
    ymin = 0;
    ymax = 1;
  }

  int p1Code = computeCode(p1.x, p1.y, xmin, xmax, ymin, ymax);
  int p2Code = computeCode(p2.x, p2.y, xmin, xmax, ymin, ymax);

  float x1 = p1.x;
  float x2 = p2.x;
  float y1 = p1.y;
  float y2 = p2.y;

  bool accept = false;
  while (true)
  {
    if (!(p1Code | p2Code))
    {
      accept = true;
      break;
    }
    else if (p1Code & p2Code)
    {
      break;
    }
    else
    {
      float x, y;
      int error = p1Code ? p1Code : p2Code;

      if (error & 8) // top fix
      {
        x = x1 + (x2 - x1) * (ymax - x1) / (x2 - x1);
        y = ymax;
      }
      else if (error & 4) // bottom fix
      {
        x = x1 + (x2 - x1) * (ymin - x1) / (x2 - x1);
        y = ymin;
      }
      else if (error & 2) // right fix
      {
        x = xmax;
        y = x1 + (x2 - x1) * (xmax - x1) / (x2 - x1);
      }
      else if (error & 1) // left fix
      {
        x = xmin;
        y = x1 + (x2 - x1) * (xmin - x1) / (x2 - x1);
      }

      if (error == p1Code)
      {
        x1 = x;
        y1 = y;
        p1Code = computeCode(x1, y1, xmin, xmax, ymin, ymax);
      }
      else
      {
        x2 = x;
        y2 = y;
        p2Code = computeCode(x2, y2, xmin, xmax, ymin, ymax);
      }
    }
  }

  if (accept)
  {

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
  //glOrtho(0,grid_width * 2,0,grid_height * 2,-10,10);
  glOrtho(0,6,0,6,-10,10);

  //clear the modelview matrix
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  //set pixel size based on width, if the aspect ratio
  //changes this hack won't work as well
  pixel_size = width/(float)grid_width;
  cout << pixel_size << endl;

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
    cout << "Please enter the light position:" << endl;
    cout << "x y z" << endl;

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

    light_position = new Point(p1x, p1y, p1z);

    recalculateNI();

  }
  else if (ch == 'f')
  {
    string x;
    cout << "Please enter the from point:" << endl;
    cout << "x y z" << endl;

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

    from_position = new Point(p1x, p1y, p1z);

    recalculateNI();
  }
  else if (ch == 'l')
  {
    moveLight = !moveLight;
    if (moveLight == true)
      cout << "Ability to move light turned on" << endl;
    else
      cout << "Ability to move light turned off" << endl;

  }
  else if (ch == 'b')
  {
    recalculateNI();
  }
  else if (ch == 'z')
  {
    polygons[currentPolygonIndex]->rotate(1.0, 0);
    recalculateNI();
  }
  else if (ch == 'Z')
  {
    polygons[currentPolygonIndex]->rotate(-1.0, 0);
    recalculateNI();
  }
  else if (ch == 'x')
  {
    polygons[currentPolygonIndex]->rotate(1.0, 1);
    recalculateNI();
  }
  else if (ch == 'X')
  {
    polygons[currentPolygonIndex]->rotate(-1.0, 1);
    recalculateNI();
  }
  else if (ch == 'y')
  {
    polygons[currentPolygonIndex]->rotate(1.0, 2);
    recalculateNI();
  }
  else if (ch == 'Y')
  {
    polygons[currentPolygonIndex]->rotate(-1.0, 2);
    recalculateNI();
  }
  else if (ch == 'q')
  {
    //save();
    exit(0);
  }
  else if (ch == '+' || ch == '=')
  {
    if (scaleTurnedOn)
    {
      polygons[currentPolygonIndex]->scale(1.05, 1.05, 1.05);
    }
    else if (!scaleTurnedOn && moveLight)
    {
      light_position = new Point(light_position->x, light_position->y, light_position->z+.1);
      displayVector("light", light_position);
      recalculateNI();
    }
    else if (!scaleTurnedOn && !rotateTurnedOn)
    {
      float centerx = polygons[currentPolygonIndex]->centroid.x;
      float centery = polygons[currentPolygonIndex]->centroid.y;
      float centerz = polygons[currentPolygonIndex]->centroid.z;
      polygons[currentPolygonIndex]->translate(Point(centerx, centery, centerz + .1));

      recalculateNI();
    }
  }
  else if (ch == '-')
  {
    if (scaleTurnedOn)
    {
      polygons[currentPolygonIndex]->scale(.9, .9, .9);
      recalculateNI();
    }
    else if (!scaleTurnedOn && moveLight)
    {
      light_position = new Point(light_position->x, light_position->y, light_position->z-.1);
      displayVector("light", light_position);
      recalculateNI();
    }

    else if (!scaleTurnedOn && !rotateTurnedOn)
    {
      float centerx = polygons[currentPolygonIndex]->centroid.x;
      float centery = polygons[currentPolygonIndex]->centroid.y;
      float centerz = polygons[currentPolygonIndex]->centroid.z;
      polygons[currentPolygonIndex]->translate(Point(centerx, centery, centerz - .1));

      recalculateNI();
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
    if (moveLight)
    {
      light_position = new Point(light_position->x+.1, light_position->y, light_position->z);
      displayVector("light", light_position);
    }
    else
    {
      polygons[currentPolygonIndex]->translate(Point(centerx + .1, centery, centerz));
    }
    recalculateNI();
  }
  else if (key == GLUT_KEY_LEFT)
  {
    if (moveLight)
    {
      light_position = new Point(light_position->x-.1, light_position->y, light_position->z);
      displayVector("light", light_position);
    }
    else
    {
      polygons[currentPolygonIndex]->translate(Point(centerx - .1, centery, centerz));
    }
    recalculateNI();
  }
  else if (key == GLUT_KEY_UP)
  {
    if (moveLight)
    {
      light_position = new Point(light_position->x, light_position->y+.1, light_position->z);
      displayVector("light", light_position);
    }
    else
    {
      polygons[currentPolygonIndex]->translate(Point(centerx, centery + .1, centerz));
    }
    recalculateNI();
  }
  else if (key == GLUT_KEY_DOWN)
  {
    if (moveLight)
    {
      light_position = new Point(light_position->x, light_position->y-.1, light_position->z);
      displayVector("light", light_position);
    }
    else
    {
      polygons[currentPolygonIndex]->translate(Point(centerx, centery - .1, centerz));
    }
    recalculateNI();
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

    }
    else if (rightButtonUsed)
    {
    }
  }

  //redraw the scene after mouse click
  glutPostRedisplay();
}

//gets called when the curser moves accross the scene
void motion(int x, int y)
{
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
