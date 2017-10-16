#include <iostream>
#include <vector>
#include <map>

#include "point.h"

using namespace std;

class Polygon
{
  public:
    void display();
    void updateYMin(int y);
    void updateYMax(int y);
    void insertEdgePoint(Point p, PointInfo pi);
    void displayEdges();
    void calculateCentroid();
    void calculateEdges(bool useDDA);
    void markVerticesAndExtrema();
    void drawHorizontalLine(float x1, float x2, float y1);
    void drawVerticalLine(float x1, float x2, float y1);
    void drawDiagonalViaDDA(float x1, float x2, float y1, float y2);
    void drawDiagonalViaBresenham(float x1, float x2, float y1, float y2);

    // transformations
    void translate(Point p);
    void scale(float dx, float dy);
    void rotate(float angle);
    
    float toRadians(float degrees);
    float toDegrees(float radians);


    map<Point, PointInfo>::iterator findApproxPoint(int x, int y);
    Polygon(int grid_height);


    string description;
    vector<Point *> vertices;
    map<Point, PointInfo, PointComparator> edges;
    Point centroid;

    
    int numVertices;
    float yMin;
    float yMax;

};

