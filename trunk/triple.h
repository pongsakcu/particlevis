/*
Triple

	   A simple representation of a 3 dimension spatial vector.  Floats are used
	to store each parameter, and various vector operations are supported.

	Vincent Hoon, 2007
	See copyright notice in copyright.txt for conditions of distribution and use.
*/
#pragma once
#ifndef TRIPLE
#define TRIPLE 0x00001

struct triple
{
	float x;
	float y;
	float z;
	//float padding;

	bool operator==(const triple &r) { if ((x==r.x)&&(y==r.y)&&(z==r.z)) return true; else return false; }
	bool operator!=(const triple &r) { if ((x!=r.x)||(y!=r.y)||(z!=r.z)) return true; else return false; }
	void operator-=(const triple &r) { x -= r.x;  y -= r.y; z -= r.z; }
	void operator+=(const triple &r) { x += r.x;  y += r.y; z += r.z; }
	triple operator+(const triple &r) { return triple(x + r.x, y + r.y, z + r.z); }
	triple operator-(const triple &r) { return triple(x - r.x, y - r.y, z - r.z); }
	triple operator*(const float &m) { return triple(x * m, y * m, z * m); }
	triple operator/(const float &m) { return triple(x / m, y / m, z / m); }
	//triple(double _X, double _Y, double _Z) { x = (float)_X; y = (float)_Y; z = (float)_Z; }
	triple(double _X, double _Y, double _Z) : x(_X), y(_Y), z(_Z)
	{}
	triple() : x(0), y(0), z(0)
	{}
	triple Normalized();
	float Length();
};

double length(triple t); // return vector length
void normalize(triple &t); // normalize vector to unit length

triple crossProduct(triple vec1, triple vec2); // take the cross-product of two vectors
triple linePlaneSect(triple point, triple line, triple plane); // intersect a line (point + line vector) with a plane at the origin
triple midpoint(const triple point1, triple point2); // midpoint between two points
double dotproduct(triple one, triple two); // dot product of two vectors

triple operator*(const float l, const triple &r);

#endif