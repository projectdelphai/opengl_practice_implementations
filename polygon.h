#include <iostream>
#include <vector>
#include <set>

#include "point.h"

using namespace std;

class Polygon
{
  public:
    void display();
    void calculateEdges();
    Polygon();

    string description;
    int numVertexes;
    vector<Point *> vertexes;
    set<Point, PointComparator> edges;

};

