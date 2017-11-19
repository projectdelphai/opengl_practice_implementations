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
    void display();
    void updateYMin(int y);
    void updateYMax(int y);
    void insertEdgePoint(Point p, PointInfo pi, int plane);
    void displayEdges();
    void calculateCentroid();
    int getElement(Triangle *t, int element);
    void calculateEdges(bool useDDA);
    void recalculateEdges(bool useDDA);
    void markVerticesAndExtrema();
    void drawHorizontalLine(float x1, float x2, float y1, int plane);
    void drawVerticalLine(float x1, float x2, float y1, int plane);
    void drawDiagonalViaDDA(float x1, float x2, float y1, float y2, int plane);
    void drawDiagonalViaBresenham(float x1, float x2, float y1, float y2, int plane);

    // transformations
    void translate(Point p);
    void scale(float dx, float dy, float dz);
    void rotate(float angle, int axis);
    void specialRotate(Point p1, Point p2, float angle);
    
    float toRadians(float degrees);
    float toDegrees(float radians);


    map<Point, PointInfo>::iterator findApproxPoint(int x, int y);
    Polygon(int grid_height);


    string description;
    vector<Point *> vertices;
    vector<pair<int, int>> edgeOrder;
    vector<Triangle *> triangles;
    vector<Triangle *> tXY;
    vector<Triangle *> tXZ;
    vector<Triangle *> tYZ;

    map<Point, PointInfo, PointComparator> edgesXY;
    map<Point, PointInfo, PointComparator> edgesXZ;
    map<Point, PointInfo, PointComparator> edgesYZ;
    map<Point, PointInfo, PointComparatorY> edgesSortedByY;
    Point centroid;

    
    int numVertices;
    int numEdges;
    int numTriangles;
    int materialID;

    float yMin;
    float yMax;

};
