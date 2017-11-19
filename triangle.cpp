#include <iostream>

#include "triangle.h"
#include "point.h"

using namespace std;

Triangle::Triangle(int v1, int v2, int v3)
{
  vertex1 = v1;
  vertex2 = v2;
  vertex3 = v3;

}

Triangle::Triangle(Triangle *t)
{
  vertex1 = t->vertex1;
  vertex2 = t->vertex2;
  vertex3 = t->vertex3;

}

bool Triangle::containsVertex(Point *p, vector<Point *> vertices)
{
  Point *p1 = vertices[vertex1];
  Point *p2 = vertices[vertex2];
  Point *p3 = vertices[vertex3];

  if ((p1->x == p->x) && (p1->y == p->y) && (p1->z == p->z))
    return true;
  else if ((p2->x == p->x) && (p2->y == p->y) && (p2->z == p->z))
    return true;
  else if ((p3->x == p->x) && (p3->y == p->y) && (p3->z == p->z))
    return true;
  else
    return false;
}
