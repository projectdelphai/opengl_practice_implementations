#include <tuple>

#include "material.h"

using namespace std;

Material::Material(tuple<float, float, float> a, tuple<float, float, float> s, tuple<float, float, float> d, float number)
{
  Ka = a;
  Ks = s;
  Kd = d;
  n = number;
}
