/*
SpherePatch

	   Spherepatch stores the points and normals of a triangle that forms
	part of a sphere.  Creating these points is easy, since every point
	is of distance "rho" from the origin.

	Vincent Hoon, 2006
	See copyright notice in copyright.txt for conditions of distribution and use.
*/

#pragma once

#define X 0
#define Y 1
#define Z 2
#define PI 3.1415926535897932384626433832795
#define TWOPI 6.283185307179586476925286766559

class SpherePatch
{
public:
	SpherePatch();
	SpherePatch(double ph1, double th1,
				double ph2, double th2,
				double ph3, double th3,
				double rh, bool top);
	SpherePatch(double inpoints1[3], double inpoints2[3], double inpoints3[3],
				double rh, bool top);
    ~SpherePatch();

	bool operator<(SpherePatch &p);
	void SetTexCoords(int point, double s, double t);
	void Draw() const;
	void DrawNormal() const;
	double points[3][3];
	double normals[3][3];
	double rho;
	bool tophalf;
	double texcoords[3][2];
private:
	double pointDistance(const double point1[3], const double point2[3]);
	double crossProduct(double point1[3], double point2[3]);
};