#include <tuple>

using namespace std;

class Material
{
  public:
    Material(tuple<float, float, float> a, tuple<float, float, float> s, tuple<float, float, float> d, float number);
    tuple<float, float, float> Ka;
    tuple<float, float, float> Kd;
    tuple<float, float, float> Ks;
    int n;
};
