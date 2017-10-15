#include "point.h"

Point::Point(float rx, float ry)
{
  x = rx;
  y = ry;
}

PointInfo::PointInfo(bool horiz, bool vert, bool extr)
{
  partOfHorizontal = horiz;
  vertex = vert;
  extrema = extr;
}
