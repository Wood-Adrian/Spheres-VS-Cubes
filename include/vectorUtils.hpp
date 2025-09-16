#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <limits>

#include <ogcsys.h>
#include <gccore.h>

//common vector functions that are just kinda useful


//Get the distance between 2 points
//@return The distance between vect1 and vect2
float VectorDistance(const guVector& vect1, const guVector& vect2);

//Get the unit vector for the direction to go from location->destination
//@param location First point (going from)
//@param destination Second point (going to)
//@return Vector pointing from location -> destination with a length of 1
guVector DirectionUnitVector(const guVector& location, const guVector& destination);

//Test if a point has "passed" another point based on a direction vector
//@param position First point being tested (typically the point that is moving)
//@param destination Second point being tested (typically the point the first point is moving toward)
//@param direction The direction the test is conducted with (typically the direction that `position` is moving in)
//@return true if position has surpassed destination in the direction of direction
bool VectorSurpassedPoint(const guVector& position, const guVector& destination, const guVector& direction);

//Möller–Trumbore intersection algorithm
//@param rayOrg Origin of ray (current position)
//@param rayDir Direction of ray
//@param tri1 First point of a triangle
//@param tri2 Second point of a triangle
//@param tri3 Third point of a triangle
//@param pos (out) Co-ordinate of where the intersection happened if return value is true, unchanged if return value is false
//@return true if ray has intersected inside the triangle
bool RayIntersectTriangle(const guVector& rayOrg, const guVector& rayDir, const guVector& tri1, const guVector& tri2, const guVector& tri3, guVector& pos);
