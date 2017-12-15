#include <iostream>

#include "triangle.h"
#include "point.h"

using namespace std;

Triangle::Triangle(int v1, int v2, int v3)
{
  vertex1 = v1;
  vertex2 = v2;
  vertex3 = v3;

  p1 = Point(0, 0, 0);
  p2 = Point(0, 0, 0);
  p3 = Point(0, 0, 0);

}

Triangle::Triangle(Triangle *t)
{
  vertex1 = t->vertex1;
  vertex2 = t->vertex2;
  vertex3 = t->vertex3;

  polygonID = t->polygonID;

}

Triangle::Triangle(Triangle *t, vector<Point *> vertices)
{
  vertex1 = t->vertex1;
  vertex2 = t->vertex2;
  vertex3 = t->vertex3;

  polygonID = t->polygonID;

  p1 = Point(vertices[vertex1]->x, vertices[vertex1]->y, vertices[vertex1]->z);
  p2 = Point(vertices[vertex2]->x, vertices[vertex2]->y, vertices[vertex2]->z);
  p3 = Point(vertices[vertex3]->x, vertices[vertex3]->y, vertices[vertex3]->z);

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

float Triangle::minVertex(vector<Point *> vertices, int plane)
{
  // 0 = XY =>Z 
  // 1 = XZ => Y
  // 2 = YZ => X
  
  Point *p1 = vertices[vertex1];
  Point *p2 = vertices[vertex2];
  Point *p3 = vertices[vertex3];

  if (plane == 0)
    return min(min(p1->z, p2->z), p3->z);
  else if (plane == 1)
    return min(min(p1->y, p2->y), p3->y);
  else if (plane == 2)
    return min(min(p1->x, p2->x), p3->x);
  else 
    return 0;

}

float Triangle::maxVertex(vector<Point *> vertices, int plane)
{
  // 0 = XY =>Z 
  // 1 = XZ => Y
  // 2 = YZ => X
  
  Point *p1 = vertices[vertex1];
  Point *p2 = vertices[vertex2];
  Point *p3 = vertices[vertex3];

  if (plane == 0)
    return max(max(p1->z, p2->z), p3->z);
  else if (plane == 1)
    return max(max(p1->y, p2->y), p3->y);
  else if (plane == 2)
    return max(max(p1->x, p2->x), p3->x);
  else
    return 0;


}

float Triangle::minVertex(int plane)
{
  // 0 = XY =>Z 
  // 1 = XZ => Y
  // 2 = YZ => X
  
  if (plane == 0)
    return min(min(p1.z, p2.z), p3.z);
  else if (plane == 1)
    return min(min(p1.y, p2.y), p3.y);
  else if (plane == 2)
    return min(min(p1.x, p2.x), p3.x);
  else 
    return 0;

}

float Triangle::maxVertex(int plane)
{
  // 0 = XY =>Z 
  // 1 = XZ => Y
  // 2 = YZ => X
  
  if (plane == 0)
    return max(max(p1.z, p2.z), p3.z);
  else if (plane == 1)
    return max(max(p1.y, p2.y), p3.y);
  else if (plane == 2)
    return max(max(p1.x, p2.x), p3.x);
  else
    return 0;


}
