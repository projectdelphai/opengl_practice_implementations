#include <iostream>
#include <map>
#include <cmath>

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

  displayEdges();
}

void Polygon::calculateEdges(bool useDDA)
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

    // fill in every pixel (x1, y1->y2)
    if (x1 == x2)
      drawHorizontalLine(x1, y1, y2);
    // fill in every pixel (x1 -> x2, y1)
    else if (y1 == y2)
      drawVerticalLine(x1, x2, y1);
    else
    {
      if (useDDA == true)
        drawDiagonalViaDDA(x1, x2, y1, y2);
      else
        drawDiagonalViaBresenham(x1, x2, y1, y2);
    }

  }
    
}

void Polygon::drawHorizontalLine(int x1, int y1, int y2)
{
  // draw horizontal line to the right
  if (y1 < y2)
  {
    for (int y = y1 + 1; y < y2; y++)
      edges.insert(Point(x1, y));
  }
  // draw horizontal line to the left
  else
  {
    for (int y = y1 - 1; y > y2; y--)
      edges.insert(Point(x1, y));
  }
}

void Polygon::drawVerticalLine(int x1, int x2, int y1)
{
  // draw vertical line upwards
  if (x1 < x2)
  {
    for (int x = x1 + 1; x < x2; x++)
      edges.insert(Point(x, y1));
  }
  // draw vertical line downwards
  else
  {
    for (int x = x1 - 1; x > x2; x--)
      edges.insert(Point(x, y1));
  }
}

void Polygon::drawDiagonalViaDDA(int x1, int x2, int y1, int y2)
{
  int dx = x2 - x1;
  int dy = y2 - y1;

  int increment;

  if (abs(dx) > abs(dy))
    increment = abs(dx);
  else
    increment = abs(dy);

  float xIncrement = dx / (float)increment;
  float yIncrement = dy / (float)increment;

  float x = x1;
  float y = y1;

  for (int i = 0; i < increment; i++)
  {
    x = x + xIncrement;
    y = y + yIncrement;
    edges.insert(Point(x, y));

  }
}

void Polygon::drawDiagonalViaBresenham(int x1, int x2, int y1, int y2)
{
}

void Polygon::displayEdges()
{
  cout << "edges: " << endl;
  set<Point, PointComparator>::iterator it;
  for (it = edges.begin(); it != edges.end(); it++)
  {
    cout << it->x << ", " << it->y << endl;
  }
}
