#include "GLVectorSet.h"
#include "ParticleSet.h"
#include <assert.h>

#define MAX_VEC 20000

GLVectorSet::GLVectorSet() :
last_frame_drawn(-1),
maxIndex(0)
{
	rend.SetType(FIELDVEC);
}

GLVectorSet::~GLVectorSet(void)
{
	ClearAllFrames();
}

// Draw a single frame of vector data
void GLVectorSet::DrawInstance(int frame)
{
	static Particle nullparticle;
	static quad nullquad;
	static triple nulltriple;
	// draw velocity vectors	
	if (maxIndex == 0 || point_data.size() <= frame)
		return;

	if (last_frame_drawn != frame || !Particle::setup)
	{
		int count = vector_count[frame];
		triple *pos, *vec;
		pos = point_data[frame];
		vec = vector_data[frame];
		rend.Clear();
		for (int i = 0; i < count; i++)	
		{
			rend.Load(pos, &nullquad, vec, &nulltriple, &nullparticle);
			pos++;
			vec++;
		}
	}	
	glLineWidth(1.0);
	rend.Render();
	last_frame_drawn = frame;
}

// Add a frame of vectors to the set
bool GLVectorSet::AddVectors(int frame, int number, vector<triple>& points, vector<triple>& vectors)
{
	triple * point_cache;
	triple * vector_cache;

	assert(number == points.size());
	assert(number == vectors.size());

	// Allocate memory for the new frame
	point_cache = new triple[number];
	vector_cache = new triple[number];	

	if (point_data.size() <= frame)
	{
		point_data.resize(frame+1);
		vector_data.resize(frame+1);		
		vector_count.resize(frame+1);
	}
	point_data[frame] = point_cache;
	vector_data[frame] = vector_cache;
	vector_count[frame] = number;

	for (int i = 0; i < number; i++)	
	{		
		point_cache[i] = points[i];
		vector_cache[i] = vectors[i];
	}	

	// Insure our renderer is big enough to handle all frames
	if (number > maxIndex)
	{
		rend.SetSize(number*6);
		maxIndex = number;
	}
	return true;
}

// Delete all data
void GLVectorSet::ClearAllFrames()
{
	maxIndex = 0;
	for (int i = 0; i < point_data.size(); i++)
	{
		delete [] point_data[i];
		delete [] vector_data[i];
	}
	point_data.clear();
	vector_data.clear();
	rend.Clear();
}