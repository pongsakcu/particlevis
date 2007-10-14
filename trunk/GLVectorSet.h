/*
GLVectorSet

		   This class provides a convenient interface for drawing standalone
		vector data.  A collection of arbitrary points and vectors can
		be loaded (AddVectors) and drawn into the scene (DrawInstance).  The
		"vector file" filetype is implemented using this class.

		Vincent Hoon, 2007
		See copyright notice in copyright.txt for conditions of distribution and use.
*/
#pragma once
#include "Particle.h"
#include "GLLineBuffer.h"
#include "ParticleStore.h"
#include "triple.h"
#include "VectorRenderer.h"
#include <vector>

using std::vector;
class ParticleSet;

class GLVectorSet
{
public:
	GLVectorSet();
	~GLVectorSet(void);
	void DrawInstance(int frame);
	bool AddVectors(int frame, int number, vector<triple>& points, vector<triple>& vectors);
	void ClearAllFrames();

	int GetFrameCount() { return vector_count.size(); }

private:
	vector<int> vector_count;
	vector<triple*> vector_data;
	vector<triple*> point_data;

	VectorRenderer rend;

	float* vertices;
	float* color;
	int vIndex, maxIndex;
	int last_frame_drawn;
};