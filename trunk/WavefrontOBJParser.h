#pragma once

// Class for parsing Wavefront Object files into a form suitable for
// OpenGL rendering.
//
// Vincent Hoon, 2007
// See copyright notice in copyright.txt for conditions of distribution and use.

#include "triple.h" // vector/point type
#include <vector>

using namespace std;

class GeometryEngine;

class WavefrontOBJParser
{	
friend class GeometryEngine;
public:
	WavefrontOBJParser();
	~WavefrontOBJParser(void);

	bool ParseData(string filename); // read in file and load into internal data structures
	void DrawData(); // issue data using OpenGL
private:
	vector<triple> points;
	vector<triple> normals;
	vector<triple> texcoords;
	vector<int> polygons;

	void GenNormals();
};