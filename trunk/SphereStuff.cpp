// Make a sphere.  This is done by iteratively tesselating
// an initial unit octahedron "depth" times.  Uses triangle strips to draw.

//extern "C" void exit (int) throw (); // WHY I DO NOT KNOW

#include <windows.h>
#include "SpherePatch.h"
#include <cstdlib>
#include <list>
#include <vector>
#include <algorithm>
#include <functional>
#include <math.h>
#include <GL\gl.h>
#include <GL\glu.h>

using namespace std;

struct triplet // heh.  NOT triple.
{
	double x;
	double y;
	double z;

	triplet(double _X, double _Y, double _Z) { x = _X; y = _Y; z = _Z; }
	triplet() { x = 0; y = 0; z = 0; }
	~triplet() { }
	double length() { return pow((x*x + y*y + z*z), 0.5); }
	void normalize() { double l = length(); x = x/l; y = y/l; z = z/l; }
	void operator=(const triplet &i) { x = i.x; y = i.y; z = i.z; }
};

int totalVertices = 0;

void sphere (double o[3], double r, int depth);
double pointDistance(const double point1[3], const double point2[3]);
void midpoint(const triplet point1, triplet &point2result);
void tesselate(triplet triangle[3], vector<triplet>* bottom, bool mode = false);
void copyPoint(const double src[3], double dst[3]);
void drawStrip(vector<triplet>& in, double r, double o[3], bool mode = false);

// Function to render a tesselated sphere, starting from an octahedron
void sphere (double o[3], double r, int depth)
{
    glMatrixMode(GL_MODELVIEW);
	//glPushMatrix();
	//glTranslatef((float)o[X], (float)o[Y], (float)o[Z]);

	totalVertices = 0;

	// Make initial octahedron
	vector<SpherePatch> patches;
	double nudge = 1e-6; // make sure we never hit domain boundaries
	double rot = 0;	 // theta
	for (int ct = 0; ct < 4; ct++)
	{
		// call spherepatch spherical constructor to create each quarter-sphere
		SpherePatch T(0 + nudge, rot+PI/4, PI/2-nudge, rot+PI/2 - nudge, PI/2-nudge, rot+nudge, 1.0, true);
		SpherePatch B(PI - nudge, rot+PI/4, PI/2-nudge, rot + nudge, PI/2-nudge, rot+PI/2 - nudge, 1.0, false);

		T.SetTexCoords(0, (double)ct / 4.0, 0);
		B.SetTexCoords(0, (double)ct / 4.0, 1.0);
		T.SetTexCoords(0, (double)ct / 4.0, 0);
		B.SetTexCoords(0, (double)ct / 4.0, 1.0);
		patches.push_back( T );
		patches.push_back( B );
		rot += (PI/2.0);
	}
	
	//list<SpherePatch>::iterator I;
	//I = patches->front();

	double distance = 0;

	typedef vector<triplet> tripvec;

	//tripvec strips;
	int trip_size = (int)pow(2.0, depth+1);
	vector<triplet>** strips = new vector<triplet>*[trip_size];

	int vCount = 0;
	int maxS = 0;
	
	glBegin(GL_TRIANGLE_STRIP);	
	for (int i = 0; i < 8; i++)
	{
		int S = 0;
		triplet tris[3];
		tris[0].x = patches[i].points[0][X];
		tris[0].y = patches[i].points[0][Y];
		tris[0].z = patches[i].points[0][Z];
		tris[1].x = patches[i].points[1][X];
		tris[1].y = patches[i].points[1][Y];
		tris[1].z = patches[i].points[1][Z];
		tris[2].x = patches[i].points[2][X];
		tris[2].y = patches[i].points[2][Y];
		tris[2].z = patches[i].points[2][Z];

		for (int j = 0; j < depth; j++)
		{
			int T = S;
			for (int k = 0; k < T; k++)
			{
				vector<triplet>* one = new vector<triplet>;
				vector<triplet>* two = new vector<triplet>;
				
				for (int l = 0; l < 2; l++)
				{
					triplet tri[3];
					tri[0] = (*strips[k])[l];
					tri[1] = (*strips[k])[l+1];
					tri[2] = (*strips[k])[l+2];
					tesselate(tri, (l%2==0) ? one : two);
                    if (l%2==0)
						two->push_back(tri[0]);
					else
						one->push_back(tri[2]);
				}
				for (int l = 2; l+2 < strips[k]->size(); l++)
				{
					triplet tri[3];
					tri[0] = (*strips[k])[l];
					tri[1] = (*strips[k])[l+1];
					tri[2] = (*strips[k])[l+2];
					tesselate(tri, (l%2==0) ? one : two, true);
                    if (l%2==0)
						two->push_back(tri[2]);
					else
						one->push_back(tri[2]);
				}
				strips[S++] = one;
				strips[S++] = two;				
			}

			strips[S++] = new vector<triplet>();
			tesselate(tris, strips[S-1]);

			for (int l = 0; l < (S - T); l++)
			{
				strips[l] = strips[l+T];
			}
			if (S > maxS) maxS = S;
			S = S - T;
		}

		//glBegin(GL_TRIANGLE_STRIP);

		for (int j = 0; j < S; j++)
		{
			//glColor3d((double)i/7.0, (double)j/(S-1.0), 1.0-(double)i/7.0);
			drawStrip(*strips[j], r, o, j%2);
			//delete strips[j];
		}

		// carefully hacked, er, I mean tuned
		vector<triplet> triangle;
		triangle.push_back(tris[0]);
		triangle.push_back(tris[1]);
		triangle.push_back(tris[2]);	
		drawStrip(triangle, r, o);

		// code for drawing the individual triangle outlines:
		
		/*
		glEnd();
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glPolygonOffset(-1, -2);
		glPointSize(2.0);
        glEnable(GL_POLYGON_OFFSET_LINE);
		for (int j = 0; j < S; j++)
		{
			//glColor3d((double)(j%32)/32.0, (double)(j%64)/64.0, 1.0-(double)i/7.0);
			glColor3f(0, 0, 0);
			glBegin(GL_TRIANGLE_STRIP);
			drawStrip(*strips[j], r, o, j%2);
			glEnd();
			delete(strips[j]);
		}
		glPolygonOffset(0, 0);
		glDisable(GL_POLYGON_OFFSET_LINE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glColor3f(1, 1, 1);
		glBegin(GL_TRIANGLE_STRIP);
		*/
		
	}
	glEnd();
	delete [] strips;

	/*
	glColor4d(1.0, 1.0, 1.0, 0.2);

	glBegin(GL_LINES);
	for (I=patches->begin(); I != patches->end(); I++)
	{
		(*I).DrawNormal();
	}
	glEnd();
	*/
	//glPopMatrix();
	int temp = totalVertices;
}

void drawStrip(vector<triplet>& in, double r, double o[3], bool mode)
{
	int i, j;
	double theta, phi;
	bool first = true;
	//if (!mode)
	{
		//for (i = (in.size() - 1); i >= 0; i--)
		for (i = 0; i < in.size(); i++)
		{
			theta = atan2(in[i].y, in[i].x);
			theta = (theta+PI) / TWOPI;

			phi = atan2( sqrt( pow(in[i].x, 2.0) + pow(in[i].y, 2.0)), in[i].z);
			phi = (phi+PI) / (2.0*PI);

			totalVertices++;
			glTexCoord2f((float)theta,(float)((2.0*phi) - 1.0));
			glNormal3f((in[i].x), (in[i].y), (in[i].z));
			glVertex3f((r*in[i].x)+o[X], (r*in[i].y)+o[Y], (r*in[i].z)+o[Z]);

			if (i == 0)
			{
				glVertex3f((r*in[i].x)+o[X], (r*in[i].y)+o[Y], (r*in[i].z)+o[Z]);
				//glVertex3f((r*in[i].x)+o[X], (r*in[i].y)+o[Y], (r*in[i].z)+o[Z]);
			}

			if (i == (in.size() - 1))
			{
				glVertex3f((r*in[i].x)+o[X], (r*in[i].y)+o[Y], (r*in[i].z)+o[Z]);
				glVertex3f((r*in[i].x)+o[X], (r*in[i].y)+o[Y], (r*in[i].z)+o[Z]);
			}
		}
		i = 0;

		/*glTexCoord2f((float)theta,(float)((2.0*phi) - 1.0));
		glNormal3f((in[i].x), (in[i].y), (in[i].z));
		glVertex3f((r*in[i].x)+o[X], (r*in[i].y)+o[Y], (r*in[i].z)+o[Z]);*/
	}
	/*else
	{
		int j = 0;
		for (i = 0; i < in.size(); i++)
		{
			j = i;
		
			theta = atan2(in[j].y, in[j].x);
			theta = (theta+PI) / TWOPI;

			phi = atan2( sqrt( pow(in[j].x, 2.0) + pow(in[j].y, 2.0)), in[j].z);
			phi = (phi+PI) / (2.0*PI);

			totalVertices++;
			glTexCoord2f((float)theta,(float)((2.0*phi) - 1.0));
			glNormal3f((in[j].x), (in[j].y), (in[j].z));
			glVertex3f((in[j].x), (in[j].y), (in[j].z));

		}
		j = in.size() - 1;
		glTexCoord2f((float)theta,(float)((2.0*phi) - 1.0));
		glNormal3f((in[j].x), (in[j].y), (in[j].z));
		glVertex3f((in[j].x), (in[j].y), (in[j].z));
	}*/
}

// 1 tri -> 4 tris
void tesselate(triplet triangle[3], vector<triplet>* bottom, bool mode)
{
	//double newpoints[6][3];
	//for (int i = 0; i < 6; i++)
	//	newpoints[i][X] = newpoints[i][Y] = newpoints[i][Z] = 0;
	triplet newpoints[6];

	//triangle[0].normalize();
	//triangle[1].normalize();
	//triangle[2].normalize();

	newpoints[0] = triangle[0];
	newpoints[1] = triangle[1];
	newpoints[2] = triangle[1];
	newpoints[3] = triangle[2];
	newpoints[4] = triangle[2];
	newpoints[5] = triangle[0];

	midpoint(newpoints[0], newpoints[1]);
	midpoint(newpoints[2], newpoints[3]);
	midpoint(newpoints[4], newpoints[5]);

	for (int i = 0; i < 6; i++)
		newpoints[i].normalize();

	/*
	      2
	     / \
	    / B \
	   1_____3
	  / \ D / \
	 / A \ / C \
	0_____5_____4

	*/

	//top.push_back(newpoints[1]);	top.push_back(newpoints[2]);	top.push_back(newpoints[3]);
	triangle[0] = newpoints[1];
	triangle[1] = newpoints[2];
	triangle[2] = newpoints[3];

	if (mode)
	{
		bottom->push_back(newpoints[5]);
		bottom->push_back(newpoints[3]);
		bottom->push_back(newpoints[4]);
	}
	else
	{
		bottom->push_back(newpoints[0]);	bottom->push_back(newpoints[1]);	bottom->push_back(newpoints[5]);
		bottom->push_back(newpoints[3]);
		bottom->push_back(newpoints[4]);
	}

	/*SpherePatch A(	newpoints[0], newpoints[1], newpoints[5],
					triangle.rho, triangle.tophalf);
	SpherePatch B(	newpoints[1], newpoints[2], newpoints[3],
					triangle.rho, triangle.tophalf);
	SpherePatch C(	newpoints[5], newpoints[3], newpoints[4],
					triangle.rho, triangle.tophalf);
	SpherePatch D(	newpoints[3], newpoints[5], newpoints[1],
					triangle.rho, triangle.tophalf);*/
}

// return distance between points
double pointDistance(const double point1[3], const double point2[3])
{
	return pow( pow(point1[X]-point2[X], 2)+ pow(point1[Y]-point2[Y], 2) + pow(point1[Z]-point2[Z], 2), 0.5);
}

// copy a point
void copyPoint(const double src[3], double dst[3])
{
	dst[X] = src[X];
	dst[Y] = src[Y];
	dst[Z] = src[Z];
}

// calculate the midpoint of two points, store in result array
void midpoint(const triplet point1, triplet &point2result)
{
	point2result.x = (point1.x + point2result.x) / 2.0;
	point2result.y = (point1.y + point2result.y) / 2.0;
	point2result.z = (point1.z + point2result.z) / 2.0;	
}