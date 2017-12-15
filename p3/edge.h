#ifndef EDGE_H
#define EDGE_H

#include "point.h"

class Edge
{
  public:
    Point *p1;
    Point *p2;
    void scaleUp();
    void scaleDown();
};

#endif
