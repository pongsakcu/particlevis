/*
Quad

	The quad is a 4-tuple of real scalar values (floats) that
	supports various vector operations.

	Vincent Hoon, 2007
	See copyright notice in copyright.txt for conditions of distribution and use.
*/
#pragma once
#ifndef QUADRA
#define QUADRA

#include <math.h>

struct quad
{
	float w;
	float x;
	float y;
	float z;

	bool operator==(const quad &r) { if ((x==r.x)&&(y==r.y)&&(z==r.z)&&(w==r.w)) return true; else return false; }
	quad operator+(const quad &r) { return quad(w + r.w, x + r.x, y + r.y, z + r.z); }
	quad operator-(const quad &r) { return quad(w - r.w, x - r.x, y - r.y, z - r.z); }
	quad operator*(const double &m) { return quad(w * m, x * m, y * m, z * m); }
	quad operator/(const double &m) { return quad(w / m, x / m, y / m, z / m); }
	quad(double _W, double _X, double _Y, double _Z) { w = (float)_W; x = (float)_X; y = (float)_Y; z = (float)_Z; }
	quad() { w = 1; x = 0; y = 0; z = 0; }

	quad Normalized() { return (*this / Length()); }
	float Length() { return sqrt(w*w + x*x + y*y + z*z); }
};

//double length(quad t); // return vector length
//void normalize(quad &t); // normalize vector to unit length

/*triple crossProduct(triple vec1, triple vec2); // take the cross-product of two vectors
triple linePlaneSect(triple point, triple line, triple plane); // intersect a line (point + line vector) with a plane at the origin
triple midpoint(const triple point1, triple point2); // midpoint between two points
double dotproduct(triple one, triple two); // dot product of two vectors*/

#endif