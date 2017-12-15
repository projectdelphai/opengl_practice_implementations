#include "edge.h"
#include "point.h"

void Edge::scaleUp()
{
  p1->x *= 100;
  p1->y *= 100;
  p2->x *= 100;
  p2->y *= 100;
}

void Edge::scaleDown()
{
  p1->x /= 100;
  p1->y /= 100;
  p2->x /= 100;
  p2->y /= 100;
}
