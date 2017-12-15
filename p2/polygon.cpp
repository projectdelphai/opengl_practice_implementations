#include <iostream>
#include <map>
#include <cmath>
#include <glm/mat4x4.hpp>     
#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>

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

  //displayEdges();
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

void Polygon::insertEdgePoint(Point p, PointInfo pi, int plane)
{
  pair<map<Point, PointInfo>::iterator, bool> ret;
  
  if (plane == 0) // XY
    ret = edgesXY.insert(pair<Point, PointInfo>(p, pi));
  else if (plane == 1) //XZ
  {
    ret = edgesXZ.insert(pair<Point, PointInfo>(p, pi));
  }
  else if (plane == 2) // YZ
    ret = edgesYZ.insert(pair<Point, PointInfo>(p, pi));

  edgesSortedByY.insert(pair<Point, PointInfo>(p, pi));

  if (ret.second == false)
  {
    map<Point, PointInfo>::iterator it;
    if (plane == 0) // XY
      it = edgesXY.find(p);
    else if (plane == 1) //XZ
      it = edgesXZ.find(p);
    else if (plane == 2) // YZ
      it = edgesYZ.find(p);
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
  for (it = edgesXY.begin(); it != edgesXY.end(); it++)
  {
    //cout << "element: x: " << it->x << ", y: " << it->y << endl;
    //cout << "int element: x: " << (int)it->x << ", y: " << (int)it->y << endl;
    if ((int)it->first.x > x)
    {
      //cout << "not found" << endl;
      return edgesXY.end();
    }
    else if (((int)it->first.x == x) && ((int)it->first.y == y))
    {
      //cout << "found" << endl;
      return it;
    }
  }
  //cout << "not found" << endl;
  return edgesXY.end();
}

void Polygon::calculateCentroid()
{
  float sumx = 0;
  float sumy = 0;
  float sumz = 0;

  // simple method though not as accurate
  for (int i = 0; i < numVertices; i++)
  {
    sumx = sumx + vertices[i]->x;
    sumy = sumy + vertices[i]->y;
    sumz = sumz + vertices[i]->z;
  }

  float averagex = sumx / numVertices;
  float averagey = sumy / numVertices;
  float averagez = sumz / numVertices;

  centroid.x = averagex;
  centroid.y = averagey;
  centroid.z = averagez;

}


void Polygon::calculateEdges(bool useDDA)
{
  for (int i = 0; i < 3; i++)
  {
    for (int edgeOrderIndex = 0; edgeOrderIndex < numEdges; edgeOrderIndex++)
    {
      int indexA = edgeOrder[edgeOrderIndex].first;
      int indexB = edgeOrder[edgeOrderIndex].second;

      Point *p1 = vertices[indexA];
      Point *p2 = vertices[indexB];

      float x1, y1, x2, y2;
      if (i == 0) // XY
      {
        x1 = p1->x;
        y1 = p1->y;
        x2 = p2->x;
        y2 = p2->y;

      }
      else if (i == 1) // XZ
      {
        x1 = p1->x;
        y1 = p1->z;
        x2 = p2->x;
        y2 = p2->z;
        
      }
      else if (i == 2) // YZ
      {
        x1 = p1->y;
        y1 = p1->z;
        x2 = p2->y;
        y2 = p2->z;
      }

      // check if new min/max found
      updateYMin(y1);
      updateYMin(y2);
      updateYMax(y1);
      updateYMax(y2);

      // fill in every pixel (x1, y1->y2)
      if (x1 == x2)
        drawVerticalLine(x1, y1, y2, i);
      // fill in every pixel (x1 -> x2, y1)
      else if (y1 == y2)
        drawHorizontalLine(x1, x2, y1, i);
      else
      {
        if (useDDA == true)
          drawDiagonalViaDDA(x1, x2, y1, y2, i);
        else
          drawDiagonalViaBresenham(x1, x2, y1, y2, i);
      }

    }

    markVerticesAndExtrema();
  }

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

      if (it == edgesXY.end())
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

void Polygon::drawHorizontalLine(float x1, float x2, float y1, int plane)
{
  // draw horizontal line to the right
  if (x1 < x2)
  {
    for (float x = x1; x < (x2 + 1); x++)
      insertEdgePoint(Point(x, y1), PointInfo(true, false, false), plane);
  }
  // draw horizontal line to the left
  else
  {
    for (float x = x1; x > (x2 - 1); x--)
    {
      insertEdgePoint(Point(x, y1), PointInfo(true, false, false), plane);
    }
  }
}

void Polygon::drawVerticalLine(float x1, float y1, float y2, int plane)
{
  // draw vertical line upwards
  if (y1 < y2)
  {
    for (float y = y1; y < (y2 + 1); y++)
      insertEdgePoint(Point(x1, y), PointInfo(false, false, false), plane);
  }
  // draw vertical line downwards
  else
  {
    for (float y = y1; y > (y2 - 1); y--)
      insertEdgePoint(Point(x1, y), PointInfo(false, false, false), plane);
  }
}

void Polygon::drawDiagonalViaDDA(float x1, float x2, float y1, float y2, int plane)
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

  insertEdgePoint(Point(x, y), PointInfo(false, false, false), plane);
  for (int i = 0; i < (int)increment; i++)
  {
    x = x + xIncrement;
    y = y + yIncrement;

    insertEdgePoint(Point(x, y), PointInfo(false, false, false), plane);

  }
}

void Polygon::drawDiagonalViaBresenham(float x1, float x2, float y1, float y2, int plane)
{
  if (x2 < x1 && y2 < y1)
    drawDiagonalViaBresenham(x2, x1, y2, y1, plane);

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
      insertEdgePoint(Point(x, y), PointInfo(false, false, false), plane);

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
        insertEdgePoint(Point(x, y), PointInfo(false, false, false), plane);

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
        insertEdgePoint(Point(x, y), PointInfo(false, false, false), plane);

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
      insertEdgePoint(Point(x, y), PointInfo(false, false, false), plane);

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
        insertEdgePoint(Point(x, y), PointInfo(false, false, false), plane);

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
        insertEdgePoint(Point(x, y), PointInfo(false, false, false), plane);

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
  float z1 = centroid.z;

  float x2 = p.x;
  float y2 = p.y;
  float z2 = p.z;

  float dx = x2 - x1;
  float dy = y2 - y1;
  float dz = z2 - z1;

  for (int i = 0; i < numVertices; i++)
  {
    vertices[i]->x += dx;
    vertices[i]->y += dy;
    vertices[i]->z += dz;
  }

  centroid.x += dx;
  centroid.y += dy;
  centroid.z += dz;

}

void Polygon::scale(float dx, float dy, float dz)
{
  if (dx == 0 && dy == 0)
    return;

  // translate to origin
  Point oldPoint(centroid.x, centroid.y, centroid.z);

  translate(Point(0.0, 0.0, 0.0));

  for (int i = 0; i < numVertices; i++)
  {
    vertices[i]->x *= dx;
    vertices[i]->y *= dy;
    vertices[i]->z *= dz;
  }

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

void Polygon::rotate(float angle, int axis)
{
  Point oldPoint(centroid.x, centroid.y, centroid.z);

  translate(Point(0.0, 0.0, 0.0));

  for (int i = 0; i < numVertices; i++)
  {
    float angleInRadians = toRadians(angle);

    float x = vertices[i]->x;
    float y = vertices[i]->y;
    float z = vertices[i]->z;

    float costheta = cos(angleInRadians);
    float sintheta = sin(angleInRadians);

    if (axis == 0) // z axis rotation
    {
      vertices[i]->x = (x * costheta) - (y * sintheta);
      vertices[i]->y = (x * sintheta) + (y * costheta);
      vertices[i]->z = z;
    }
    else if (axis == 1) // x axis rotation
    {
      vertices[i]->y = (y * costheta) - (z * sintheta);
      vertices[i]->z = (y * sintheta) + (z * costheta);
      vertices[i]->x = x;
    }
    else if (axis == 2)
    {
      vertices[i]->z = (z * costheta) - (x * sintheta);
      vertices[i]->x = (z * sintheta) + (x * costheta);
      vertices[i]->y = y;
    }
  }

  translate(oldPoint);
}

void Polygon::specialRotate(Point p1, Point p2, float angle)
{
  float angleInRadians = toRadians(angle);
  float costheta = cos(angleInRadians);
  float sintheta = sin(angleInRadians);


  Point vector = Point(p2.x - p1.x, p2.y - p1.y, p2.z - p1.z);
  float vectorMagnitude = sqrt(vector.x*vector.x + vector.y*vector.y + vector.z*vector.z);

  Point unitVector = Point(vector.x / vectorMagnitude, vector.y / vectorMagnitude, vector.z / vectorMagnitude);

  float a = unitVector.x;
  float b = unitVector.y;
  float c = unitVector.z;
  float d = sqrt(b*b + c*c);

  glm::mat4 T(
      1, 0, 0, 0,
      0, 1, 0, 0,
      0, 0, 1, 0,
      -1*p1.x, -1*p1.y, -1*p1.z, 1
      );

  glm::mat4 RotationX(
      1, 0, 0, 0,
      0, c/d, b/d, 0,
      0, -1*b/d, c/d, 0,
      0, 0, 0, 1
      );

  glm::mat4 RotationY(
      d, 0, a, 0,
      0, 1, 0, 0,
      -1*a, 0, d, 0,
      0, 0, 0, 1
      );

  glm::mat4 RotationZ(
      costheta, sintheta, 0, 0,
      -1*sintheta, costheta, 0, 0,
      0, 0, 1, 0,
      0, 0, 0, 1
      );

  glm::mat4 Tinverse = inverse(T);
  glm::mat4 RotationXInverse = inverse(RotationX);
  glm::mat4 RotationYInverse = inverse(RotationY);

  glm::mat4 rotationMatrix = Tinverse * RotationXInverse * RotationYInverse * RotationZ * RotationY * RotationX * T;

  for (int i = 0; i < numVertices; i++)
  {
    float x = vertices[i]->x;
    float y = vertices[i]->y;
    float z = vertices[i]->z;

    vertices[i]->x = x*rotationMatrix[0][0] + y*rotationMatrix[1][0] + z*rotationMatrix[2][0];
    vertices[i]->y = x*rotationMatrix[0][1] + y*rotationMatrix[1][1] + z*rotationMatrix[2][1];
    vertices[i]->z = x*rotationMatrix[0][2] + y*rotationMatrix[1][2] + z*rotationMatrix[2][2];
  }
}

void Polygon::displayEdges()
{
  cout << "edges: " << endl;
  map<Point, PointInfo>::iterator it;
  for (it = edgesXY.begin(); it != edgesXY.end(); it++)
  {
    cout << it->first.x << ", " << it->first.y << ", horiz: " << it->second.partOfHorizontal << ", extrema: " << it->second.extrema << endl;
  }
}
