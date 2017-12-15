#include <tuple>

#include "material.h"
#include "point.h"

using namespace std;

Material::Material(Point *a, Point *d, Point *s, float number)
{
  ka = new Point(a);
  kd = new Point (d);
  ks = new Point (s);
  n = number;
}
