#include <iostream>
#include <vector>

using namespace std;

class Point;

class Triangle
{
  public:
    Triangle(int v1, int v2, int v3);
    Triangle(Triangle *t);
    bool containsVertex(Point *P, vector<Point *> vertices);

    int vertex1;
    int vertex2;
    int vertex3;

    Point *faceNormal;
};
