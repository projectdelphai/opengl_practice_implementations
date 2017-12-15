#include <tuple>

using namespace std;

#include "point.h"

class Material
{
  public:
    Material(Point *a, Point *d, Point *s, float number);
    Point *ka;
    Point *kd;
    Point *ks;
    int n;
};
