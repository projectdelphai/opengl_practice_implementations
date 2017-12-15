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

int Polygon::getElement(Triangle *t, int element)
{
  if (element == 0)
    return t->vertex1;
  else if (element == 1)
    return t->vertex2;
  else if (element == 2)
    return t->vertex3;
  else
  {
    cout << "Tried to access beyond tuple range" << endl;
    return t->vertex1;
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
