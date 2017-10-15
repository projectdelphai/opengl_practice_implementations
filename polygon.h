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
    void calculateEdges(bool useDDA);
    void markVertexesAndExtrema();
    void drawHorizontalLine(float x1, float x2, float y1);
    void drawVerticalLine(float x1, float x2, float y1);
    void drawDiagonalViaDDA(float x1, float x2, float y1, float y2);
    void drawDiagonalViaBresenham(float x1, float x2, float y1, float y2);
    map<Point, PointInfo>::iterator findApproxPoint(int x, int y);
    Polygon(int grid_height);


    string description;
    vector<Point *> vertexes;
    map<Point, PointInfo, PointComparator> edges;

    int numVertexes;
    float yMin;
    float yMax;

};

