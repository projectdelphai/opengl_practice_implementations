#ifndef POINT_H
#define POINT_H

#include <vector>

using namespace std;

class Point
{
  public:
    Point(float x, float y, float z);
    Point(float x, float y);
    Point();
    Point(Point *p);
    float x;
    float y;
    float z;
    float intensity;
    vector<int> adjacentTris;
};

class PointInfo
{
  public:
    PointInfo(bool horiz, bool vert, bool extr);
    bool partOfHorizontal;
    bool vertex;
    bool extrema;
};

struct PointComparator
{
  bool operator()(const Point& a, const Point& b)
  {
    if (a.x == b.x)
    {
      if (a.y < b.y)
        return true;
      else
        return false;
    }
    else if (a.x < b.x)
      return true;
    else
      return false;
  }
};

struct PointComparatorY
{
  bool operator()(const Point& a, const Point& b)
  {
    if (a.y == b.y)
    {
      if (a.x < b.x)
        return true;
      else
        return false;
    }
    else if (a.y < b.y)
      return true;
    else
      return false;
  }
};

#endif
