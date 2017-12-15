#include "point.h"

Point::Point(float rx, float ry, float rz)
{
  x = rx;
  y = ry;
  z = rz;
}

Point::Point(float rx, float ry)
{
  x = rx;
  y = ry;
  z = 0;
}

Point::Point()
{
  x = 0;
  y = 0;
  z = 0;
}

bool Point::operator==(const Point &p) const 
{
  return ((x == p.x) && (y == p.y));
}

PointInfo::PointInfo(bool horiz, bool vert, bool extr)
{
  partOfHorizontal = horiz;
  vertex = vert;
  extrema = extr;
}

