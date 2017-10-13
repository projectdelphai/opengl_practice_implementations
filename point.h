class Point
{
  public:
    int x;
    int y;
    Point(int x, int y);
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
