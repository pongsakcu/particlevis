#include "triple.h"
#include <math.h>

double length(triple t) { return sqrt(t.x*t.x + t.y*t.y + t.z*t.z); }
void normalize(triple &t)  { double l = length(t); t.x = t.x/l; t.y = t.y/l; t.z = t.z/l; }
triple operator*(const float l, const triple &r) { return triple(r.x * l, r.y * l, r.z * l); }

// take the cross-product of two vectors
triple crossProduct(triple vec1, triple vec2)
{
	triple newvec(0, 0, 0);
	newvec.x = (vec1.y * vec2.z) - (vec2.y * vec1.z);
	newvec.y = (vec1.z * vec2.x) - (vec2.z * vec1.x);
	newvec.z = (vec1.x * vec2.y) - (vec2.x * vec1.y);
	return newvec;
}

// intersect a line (point + line vector) with a plane at the origin
triple linePlaneSect(triple point, triple line, triple plane) 
{
	triple result;
	double prod1, prod2, x;
	prod1 = dotproduct(plane, point);
	prod2 = dotproduct(plane, line);
	x = -prod1 / prod2;

	result = point + (line * x);
	return result;
}

// midpoint
triple midpoint(const triple point1, triple point2)
{
	triple pointresult;
	pointresult = (point2 + point1) / 2.0;
	return pointresult;
}

// the dot product
double dotproduct(triple one, triple two)
{
	return ((one.x * two.x) + (one.y * two.y) + (one.z * two.z));
}

triple triple::Normalized()
{
	float l = Length();
	return triple(x/l, y/l, z/l);
}

float triple::Length()
{
	return sqrt(x*x + y*y + z*z);
}