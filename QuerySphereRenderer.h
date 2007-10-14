/*
QuerySphereRenderer

		  This class is designed to issue large collection of vertex-buffered
		points to be used for occlusion queries.

		Vincent Hoon, 2007
		See copyright notice in copyright.txt for conditions of distribution and use.
*/
#pragma once
#include "sphererenderer.h"

class QuerySphereRenderer : public SphereRenderer
{
public:
	QuerySphereRenderer(void);
	~QuerySphereRenderer(void);

	void SetSize(int size); // possible resizing logic
	void Clear(); // call before new frame
	void Load(triple* position, quad* orientation, triple* d_pos, triple* d_theta, Particle* p);
	void Render(); // call to render

	void EnableQueries();
	void DisableQueries();

protected:
	GLuint* queries;
	vector<triple> vertex_cache;
	vector<float> radius_cache;
	vector<int> ID_cache;
};
