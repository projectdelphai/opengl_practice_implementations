#include "point.h"

Point::Point(float rx, float ry)
{
  x = rx;
  y = ry;
}

Point::Point()
{
  x = 0;
  y = 0;
}

PointInfo::PointInfo(bool horiz, bool vert, bool extr)
{
  partOfHorizontal = horiz;
  vertex = vert;
  extrema = extr;
}
