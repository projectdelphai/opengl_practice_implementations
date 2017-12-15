#include <utility>
#include <iostream>
#include <vector>
#include <map>

#include "point.h"
#include "triangle.h"

using namespace std;

class Polygon
{
  public:
    Polygon(int grid_height);
    string description;
    Point centroid;
    
    int materialID;
    int numVertices;
    int numTriangles;

    vector<Point *> vertices;
    vector<Triangle *> triangles;
    vector<Triangle *> tXY;
    vector<Triangle *> tXZ;
    vector<Triangle *> tYZ;


    // transformations
    void translate(Point p);
    void scale(float dx, float dy, float dz);
    void rotate(float angle, int axis);
    void specialRotate(Point p1, Point p2, float angle);

    // helper functions
    void calculateCentroid();
    int getElement(Triangle *t, int element);
    float toRadians(float degrees);
    float toDegrees(float radians);

};
