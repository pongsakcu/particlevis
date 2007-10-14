#include "stdafx.h"
#include "SpherePatch.h"
#include <stdlib.h>
#include <math.h>

//extern int subs;
#define subs 1.0

SpherePatch::SpherePatch(void)
{
}

// spherical coordinates (phi, theta, rho) constructor
SpherePatch::SpherePatch(	double ph1, double th1,
							double ph2, double th2,
							double ph3, double th3,
							double rh, bool top)
{

	tophalf = top;

	double theta[3], phi[3];
	theta[0] = th1;
	phi[0] = ph1;

	theta[1] = th2;
	phi[1] = ph2;

	theta[2] = th3;
	phi[2] = ph3;

	rho = rh;
	for (int i = 0; i < 3; i++)
	{
		points[i][X] = rho*sin(phi[i]) * cos(theta[i]);
		//points[i][Y] = rho*cos(phi[i]);
		//points[i][Z] = rho*sin(phi[i]) * sin(theta[i]);
		points[i][Y] = rho*sin(phi[i]) * sin(theta[i]);
		points[i][Z] = rho*cos(phi[i]);

		double O[] = {0, 0, 0};
		double length = pointDistance(points[i], O);
		normals[i][X] = points[i][X]/length;
		normals[i][Y] = points[i][Y]/length;
		normals[i][Z] = points[i][Z]/length;

		texcoords[i][0] = -1;
		texcoords[i][1] = -1;
	}
}

// rectangular coordinates constructors
SpherePatch::SpherePatch(
					double inpoints1[3], double inpoints2[3], double inpoints3[3],
					double rh, bool top)
{
	tophalf = top;
	rho = rh;
	double O[] = {0, 0, 0};
	double length;

	length = pointDistance(inpoints1, O);
	for (int i = 0; i < 3; i++)
	{
		points[0][i] = (inpoints1[i] / length) * rho;
		normals[0][i] =(inpoints1[i] / length);
	}

	length = pointDistance(inpoints2, O);
	for (int i = 0; i < 3; i++)
	{
		points[1][i] = (inpoints2[i] / length) * rho;
		normals[1][i] =(inpoints2[i] / length);
	}

	length = pointDistance(inpoints3, O);
	for (int i = 0; i < 3; i++)
	{
		points[2][i] = (inpoints3[i] / length) * rho;
		normals[2][i] =(inpoints3[i] / length);
	}
}

SpherePatch::~SpherePatch(void)
{
}

// output normals/points of the patch
void SpherePatch::Draw() const
{
	float buffer[6];
	for (int i = 2; i >= 0; i--)
	{
		double theta = atan2(points[i][Y], points[i][X]);
		//double blue = 0;

		theta = (theta+PI) / TWOPI;

		if (theta == 1 && !tophalf)
		{
			theta = 0;
		}

		double phi = atan2( sqrt( pow(points[i][X], 2.0) + pow(points[i][Y], 2.0)), points[i][Z]);

		phi = (phi+PI) / (2.0*PI);

		buffer[0] = (float)points[i][X];
		buffer[1] = (float)points[i][Y];
		buffer[2] = (float)points[i][Z];
		buffer[3] = (float)normals[i][X];
		buffer[4] = (float)normals[i][Y];
		buffer[5] = (float)normals[i][Z];

		glTexCoord2d(theta, (2.0*phi) - 1.0);
		//glColor3d(theta, 0, 2*phi-1.0);
		//glColor3d(1, 1, 1);
		glNormal3f(buffer[3], buffer[4], buffer[5]);
		glVertex3f(buffer[0], buffer[1], buffer[2]);
	}
}

// draw normal using lines (debugging)
void SpherePatch::DrawNormal() const
{
	for (int i = X; i <= Z; i++)
	{
		glVertex3dv(points[i]);
		glVertex3d(normals[i][X] + points[i][X], normals[i][Y] + points[i][Y], normals[i][Z] + points[i][Z]);
	}
}

// return distance between points
double SpherePatch::pointDistance(const double point1[3], const double point2[3])
{
	return pow( pow(point1[X]-point2[X], 2)+ pow(point1[Y]-point2[Y], 2) + pow(point1[Z]-point2[Z], 2), 0.5);
}

// calculate cross product
double SpherePatch::crossProduct(double point1[3], double point2[3])
{
	return (point1[X] * point2[X] + point1[Y] * point2[Y] + point1[Z] * point2[Z]);
}

void SpherePatch::SetTexCoords(int point, double s, double t)
{
	texcoords[point][0] = s;
	texcoords[point][1] = t;
}

bool SpherePatch::operator<(SpherePatch &p)
{
	double centerpoint[3], othercenter[3];
	for (int i = 0; i < 3; i++)
	{
		centerpoint[i] = (points[0][i] + points[1][i] + points[2][i]) / 3.0;
		othercenter[i] = (p.points[0][i] + p.points[1][i] + p.points[2][i]) / 3.0;
	}
	double index1 = centerpoint[X] * 1.0 + centerpoint[Y] * 0.0 + 0.0*centerpoint[Z];
	double index2 = othercenter[X] * 1.0 + othercenter[Y] * 0.0 + 0.0*othercenter[Z];
	return (index1 < index2);
}
