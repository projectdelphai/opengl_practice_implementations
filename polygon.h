#include <iostream>
#include <vector>
#include <set>

#include "point.h"

using namespace std;

class Polygon
{
  public:
    void display();
    void displayEdges();
    void calculateEdges(bool useDDA);
    void drawHorizontalLine(int x1, int y1, int y2);
    void drawVerticalLine(int x1, int x2, int y1);
    void drawDiagonalViaDDA(int x1, int x2, int y1, int y2);
    void drawDiagonalViaBresenham(int x1, int x2, int y1, int y2);
    Polygon();


    string description;
    int numVertexes;
    vector<Point *> vertexes;
    set<Point, PointComparator> edges;

};

