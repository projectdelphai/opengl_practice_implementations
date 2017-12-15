#include <iostream>
#include <map>
#include <cmath>

#include "polygon.h"

using namespace std;

Polygon::Polygon(int grid_height)
{
  yMin = grid_height;
  yMax = 0;
}

void Polygon::display()
{
  cout << "Description: " << description << endl;
  cout << "Number of vertices: " << numVertices << endl;
  for (int i = 0; i < numVertices; i++)
  {
    cout << vertices[i]->x << " " << vertices[i]->y << endl;
  }

  displayEdges();
}

void Polygon::updateYMin(int y)
{
  if (y < yMin)
    yMin = y;
}

void Polygon::updateYMax(int y)
{
  if (y > yMax)
    yMax = y;
}

void Polygon::insertEdgePoint(Point p, PointInfo pi)
{
  pair<map<Point, PointInfo>::iterator, bool> ret;
  ret = edges.insert(pair<Point, PointInfo>(p, pi));
  edgesSortedByY.insert(pair<Point, PointInfo>(p, pi));

  if (ret.second == false)
  {
    map<Point, PointInfo>::iterator it = edges.find(p);
    if (pi.partOfHorizontal == true && it->second.partOfHorizontal == false)
      it->second.partOfHorizontal = true;
  }
  updateYMin(p.y);
  updateYMax(p.y);

}

map<Point, PointInfo>::iterator Polygon::findApproxPoint(int x, int y)
{
  //cout << "looking for: x = " << x << ", y = " << y << endl;
  map<Point, PointInfo>::iterator it;
  for (it = edges.begin(); it != edges.end(); it++)
  {
    //cout << "element: x: " << it->x << ", y: " << it->y << endl;
    //cout << "int element: x: " << (int)it->x << ", y: " << (int)it->y << endl;
    if ((int)it->first.x > x)
    {
      //cout << "not found" << endl;
      return edges.end();
    }
    else if (((int)it->first.x == x) && ((int)it->first.y == y))
    {
      //cout << "found" << endl;
      return it;
    }
  }
  //cout << "not found" << endl;
  return edges.end();
}

void Polygon::calculateCentroid()
{
  float sumx = 0;
  float sumy = 0;
  // simple method though not as accurate
  for (int i = 0; i < numVertices; i++)
  {
    sumx = sumx + vertices[i]->x;
    sumy = sumy + vertices[i]->y;
  }

  float averagex = sumx / numVertices;
  float averagey = sumy / numVertices;

  centroid.x = averagex;
  centroid.y = averagey;

}


void Polygon::calculateEdges(bool useDDA)
{
  for (int vertexIndex = 0; vertexIndex < numVertices; vertexIndex++)
  {
    Point *p1 = vertices[vertexIndex];

    // pick next vertex or loop back to the first vertex
    Point *p2;

    if (vertexIndex == (numVertices - 1))
      p2 = vertices[0];
    else
      p2 = vertices[vertexIndex + 1];

    float x1 = p1->x;
    float y1 = p1->y;
    float x2 = p2->x;
    float y2 = p2->y;

    // check if new min/max found
    updateYMin(y1);
    updateYMin(y2);
    updateYMax(y1);
    updateYMax(y2);

    // fill in every pixel (x1, y1->y2)
    if (x1 == x2)
      drawVerticalLine(x1, y1, y2);
    // fill in every pixel (x1 -> x2, y1)
    else if (y1 == y2)
      drawHorizontalLine(x1, x2, y1);
    else
    {
      if (useDDA == true)
        drawDiagonalViaDDA(x1, x2, y1, y2);
      else
        drawDiagonalViaBresenham(x1, x2, y1, y2);
    }

  }

  markVerticesAndExtrema();

  //display();

}

void Polygon::markVerticesAndExtrema()
{
  for (int vertexIndex = 0; vertexIndex < numVertices; vertexIndex++)
  {
    // find point before, current and after for each vertex
    Point *p1 = vertices[vertexIndex];
    Point *p0, *p2;

    if (vertexIndex == 0)
      p0 = vertices[numVertices - 1];
    else
      p0 = vertices[vertexIndex - 1];
    if (vertexIndex == (numVertices - 1))
      p2 = vertices[0];
    else
      p2 = vertices[vertexIndex + 1];

    if (((p0->y < p1->y) && (p1->y > p2->y)) || ((p0->y > p1->y) && (p1->y < p2->y)))
    {
      //cout << "extrema found: x=" << p1->x << ", y=" << p1->y << endl;
      map<Point, PointInfo>::iterator it = findApproxPoint((int)p1->x, (int)p1->y);

      if (it == edges.end())
      {
        //cout << "x: " << it->first.x << ", y: " << it->first.y << endl;
        //cout << "HELP" << endl;
      }
      else
      {
        //cout << "x: " << it->first.x << ", y: " << it->first.y << endl;
        it->second.vertex = true;
        it->second.extrema = true;
      }
    }


  }
}

void Polygon::drawHorizontalLine(float x1, float x2, float y1)
{
  // draw horizontal line to the right
  if (x1 < x2)
  {
    for (float x = x1; x < (x2 + 1); x++)
      insertEdgePoint(Point(x, y1), PointInfo(true, false, false));
  }
  // draw horizontal line to the left
  else
  {
    for (float x = x1; x > (x2 - 1); x--)
    {
      insertEdgePoint(Point(x, y1), PointInfo(true, false, false));
    }
  }
}

void Polygon::drawVerticalLine(float x1, float y1, float y2)
{
  // draw vertical line upwards
  if (y1 < y2)
  {
    for (float y = y1; y < (y2 + 1); y++)
      insertEdgePoint(Point(x1, y), PointInfo(false, false, false));
  }
  // draw vertical line downwards
  else
  {
    for (float y = y1; y > (y2 - 1); y--)
      insertEdgePoint(Point(x1, y), PointInfo(false, false, false));
  }
}

void Polygon::drawDiagonalViaDDA(float x1, float x2, float y1, float y2)
{
  float dx = x2 - x1;
  float dy = y2 - y1;

  float increment;

  if (abs(dx) > abs(dy))
    increment = abs(dx);
  else
    increment = abs(dy);

  float xIncrement = dx / increment;
  float yIncrement = dy / increment;

  float x = x1;
  float y = y1;

  insertEdgePoint(Point(x, y), PointInfo(false, false, false));
  for (int i = 0; i < (int)increment; i++)
  {
    x = x + xIncrement;
    y = y + yIncrement;

    insertEdgePoint(Point(x, y), PointInfo(false, false, false));

  }
}

void Polygon::drawDiagonalViaBresenham(float x1, float x2, float y1, float y2)
{
  if (x2 < x1 && y2 < y1)
    drawDiagonalViaBresenham(x2, x1, y2, y1);

  float dx = x2 - x1;
  float dy = y2 - y1;
  float slope = dy / dx;
  float slope2 = dx / dy;

  if (slope > 0 && slope < 1)
  {
    float y = y1;
    float e = 0.0; 

    for (float x = x1; x <= x2; x++)
    {
      insertEdgePoint(Point(x, y), PointInfo(false, false, false));

      e += slope;

      if (e >= 0.5)
      {
        y++;
        e = e - 1.0;
      }
    }
  }
  else if (slope < 0 && slope >= -1)
  {
    float y = y1;
    float e = 0.0; 

    if (x1 < x2)
    {
      for (float x = x1; x <= x2; x++)
      {
        insertEdgePoint(Point(x, y), PointInfo(false, false, false));

        e += abs(slope);

        if (e >= 0.5)
        {
          y--;
          e = e - 1.0;
        }
      }
    }
    else
    {
      for (float x = x1; x >= x2; x--)
      {
        insertEdgePoint(Point(x, y), PointInfo(false, false, false));

        e += abs(slope);

        if (e >= 0.5)
        {
          y++;
          e = e - 1.0;
        }
      }
    }
  }
  else if (slope >= 1)
  {
    float x = x1;
    float e = 0.0; 

    for (float y = y1; y <= y2; y++)
    {
      insertEdgePoint(Point(x, y), PointInfo(false, false, false));

      e += slope2;

      if (e >= 0.5)
      {
        x++;
        e = e - 1.0;
      }
    }
  }
  else if (slope <= -1)
  {
    float x = x1;
    float e = 0.0; 

    if (y2 < y1)
    {
      for (float y = y1; y >= y2; y--)
      {
        insertEdgePoint(Point(x, y), PointInfo(false, false, false));

        e += abs(slope2);

        if (e >= 0.5)
        {
          x++;
          e = e - 1.0;
        }
      }
    }
    else
    {
      for (float y = y1; y <= y2; y++)
      {
        insertEdgePoint(Point(x, y), PointInfo(false, false, false));

        e += abs(slope2);

        if (e >= 0.5)
        {
          x--;
          e = e - 1.0;
        }
      }

    }
  }


}

void Polygon::translate(Point p)
{
  float x1 = centroid.x;
  float y1 = centroid.y;

  float x2 = p.x;
  float y2 = p.y;

  float dx = x2 - x1;
  float dy = y2 - y1;

  for (int i = 0; i < numVertices; i++)
  {
    vertices[i]->x += dx;
    vertices[i]->y += dy;
  }

  map<Point, PointInfo, PointComparator> newEdges;
  map<Point, PointInfo>::iterator it;
  for (it = edges.begin(); it != edges.end(); it++)
  {
    float newX = it->first.x + dx;
    float newY = it->first.y + dy;

    newEdges.insert(pair<Point, PointInfo>(Point(newX, newY), it->second));

  }

  centroid.x += dx;
  centroid.y += dy;

  edges.clear();
  edgesSortedByY.clear();
  for (it = newEdges.begin(); it != newEdges.end(); it++)
  {
    insertEdgePoint(it->first, it->second);
  }

}

void Polygon::scale(float dx, float dy)
{
  if (dx == 0 && dy == 0)
    return;

  // translate to origin
  Point oldPoint(centroid.x, centroid.y);

  translate(Point(0.0, 0.0));

  for (int i = 0; i < numVertices; i++)
  {
    vertices[i]->x *= dx;
    vertices[i]->y *= dy;
  }

  edges.clear();
  edgesSortedByY.clear();
  calculateEdges(true);

  translate(oldPoint);

}

float Polygon::toRadians(float degrees)
{
  return (degrees * M_PI) / 180;
}

float Polygon::toDegrees(float radians)
{
  return (radians * 180) / M_PI;
}

void Polygon::rotate(float angle)
{
  Point oldPoint(centroid.x, centroid.y);

  translate(Point(0.0, 0.0));

  for (int i = 0; i < numVertices; i++)
  {
    float angleInRadians = toRadians(angle);

    float x = vertices[i]->x;
    float y = vertices[i]->y;

    float costheta = cos(angleInRadians);
    float sintheta = sin(angleInRadians);

    vertices[i]->x = (x * costheta) - (y * sintheta);
    vertices[i]->y = (x * sintheta) + (y * costheta);
  }

  edges.clear();
  edgesSortedByY.clear();
  calculateEdges(true);

  translate(oldPoint);
}

void Polygon::displayEdges()
{
  cout << "edges: " << endl;
  map<Point, PointInfo>::iterator it;
  for (it = edges.begin(); it != edges.end(); it++)
  {
    cout << it->first.x << ", " << it->first.y << ", horiz: " << it->second.partOfHorizontal << ", extrema: " << it->second.extrema << endl;
  }
}
