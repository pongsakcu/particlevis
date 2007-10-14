/*

The GraphScale objects stores information and implements functionality
related to the creation of grids in 3 dimensions to be used for
plotting 3 dimensional data.  Using OpenGL, it will draw the correct set
of planes based on camera quadrant position.  It will also set up
transformations to allow data to be plotted directly from dataspace
coordinates into worldspace corresponding to the plotted grids.

*/

#pragma once
#include "Stdafx.h"

#define XYFront 1 XZFront 2 YZFront 3 XYBack 4 XZBack 5 YZBack 6
#define X 0
#define Y 1
#define Z 2

/*
The GraphScale object contains all the necessary information for defining
and drawing the scales within the scene.  It will also translate data coordinates
into world coordinates.  It can Draw() 3 grid-planes with major and minor divisions,
and will label each major division with the proper numerals.
*/

class WGLFont;

class GraphScale
{
public:
	GraphScale(void);
	~GraphScale(void);
	void SetXRange(double lower, double upper);
	void SetYRange(double lower, double upper);
	void SetZRange(double lower, double upper);
	void SetGraphOffset(double off);
	void SetCamera(double _X, double _Y, double _Z);
	void SetCamera(double location[3]);
	void SetXGrid(double increment, int subdivisions, double weight, bool enabled = true);
	void SetYGrid(double increment, int subdivisions, double weight, bool enabled = true);
	void SetZGrid(double increment, int subdivisions, double weight, bool enabled = true);
	void SetWorldBox(double* vector);
	void SetWorldBox(double _X, double _Y, double _Z);

	double getXMin() const { return lowerVals[X]; }
	double getYMin() const { return lowerVals[Y]; }
	double getZMin() const { return lowerVals[Z]; }

	double getXMax() const { return upperVals[X]; }
	double getYMax() const { return upperVals[Y]; }
	double getZMax() const { return upperVals[Z]; }

	double getXRange() const { return (upperVals[X] - lowerVals[X]); }
	double getYRange() const { return (upperVals[Y] - lowerVals[Y]); }
	double getZRange() const { return (upperVals[Z] - lowerVals[Z]); }

	double getXInc() const { return gridIncrement[X]; }
	double getYInc() const { return gridIncrement[Y]; }
	double getZInc() const { return gridIncrement[Z]; }

	int getXSubs() const { return subs[X]; }
	int getYSubs() const { return subs[Y]; }
	int getZSubs() const { return subs[Z]; }

	bool Transform();
	bool Draw(int drawmode);  // renders grids

	void BuildFont(HDC hdc);

	void SetFontSize(double size, double aspect) { fontsize = size; this->aspect = aspect; }
	
	WGLFont* font;

private:
	void DrawPlanes();
	void DrawSubPlanes();
	void DrawScales();
	void drawChars(char* string, double _X, double _Y, double _Z, bool rightaligned = false);
	void UpdateAll(); // this recalculates the transformation and active grid planes
	// Vals are datapoint coordinates, upperCoord is world coordinate "box"
	double lowerVals[3], upperVals[3], upperCoord[3];
	// Camera is world-loc of cam, worldInc is used to store world grid increments
	double camera[3], worldInc[3];
	double gridOffset[3], gridIncrement[3], gridWeight[3]; // increment for major gridlines
	// false = draw back faces (aligned on axes), false = draw elevated (on upper coordinates)
	bool drawXY, drawXZ, drawYZ;
	bool drawXScale, drawYScale, drawZScale;
	int subs[3];
	double offset;
	GLuint base;
	GLYPHMETRICSFLOAT agmf[96];

	double fontsize, aspect;
};