#include <iostream>
#include <map>

#include "polygon.h"

using namespace std;

Polygon::Polygon()
{
}

void Polygon::display()
{
  cout << "Description: " << description << endl;
  cout << "Number of vertexes: " << numVertexes << endl;
  for (int i = 0; i < numVertexes; i++)
  {
    cout << vertexes[i]->x << " " << vertexes[i]->y << endl;
  }
}

void Polygon::calculateEdges()
{
  for (int vertexIndex = 0; vertexIndex < numVertexes; vertexIndex++)
  {
    Point *p1 = vertexes[vertexIndex];

    // pick next vertex or loop back to the first vertex
    Point *p2;

    if (vertexIndex == (numVertexes - 1))
      p2 = vertexes[0];
    else
      p2 = vertexes[vertexIndex + 1];

    float x1 = p1->x;
    float y1 = p1->y;
    float x2 = p2->x;
    float y2 = p2->y;


    if (x1 == x2)
    {
      // fill in every pixel (x1, y1->y2)
      cout << "a1" << endl;
      for (int y = y1 + 1; y < y2; y++)
      {
        cout << x1 << ", " << y << endl;
        edges.insert(Point(x1, y));
      }
    }
    else if (y1 == y2)
    {
      // fill in every pixel (x1 -> x2, y1)
      cout << "a2" << endl;
      for (int x = x1 + 1; x < x2; x++)
      {
        cout << x << ", " << y1 << endl;
        edges.insert(Point(x, y1));
      }
    }
    else
    {
      
    }

  }
    
  cout << "b" << endl;
 
  set<Point, PointComparator>::iterator it;
  for (it = edges.begin(); it != edges.end(); it++)
  {
    cout << it->x << ", " << it->y << endl;
  }


}
