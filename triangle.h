#ifndef TRIANGLE_H
#define TRIANGLE_H

#include <iostream>
#include <vector>

#include "point.h"

using namespace std;

class Triangle
{
  public:
    int polygonID;

    Triangle(int v1, int v2, int v3);
    Triangle(Triangle *t);
    Triangle(Triangle *t, vector<Point *> vertices);
    bool containsVertex(Point *P, vector<Point *> vertices);
    float minVertex(vector<Point *> vertices, int plane);
    float maxVertex(vector<Point *> vertices, int plane);
    float minVertex(int plane);
    float maxVertex(int plane);

    int vertex1;
    int vertex2;
    int vertex3;

    Point p1;
    Point p2;
    Point p3;

    Point *faceNormal;
};

#endif
