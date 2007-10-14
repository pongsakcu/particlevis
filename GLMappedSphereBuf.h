/*
GLMappedSphereBuf

		   An OpenGL utlity class that encapsulates a buffer of triangle strips
		that approximate a sphere.  The 1-D texture coordinates at each vertex of 
		the sphere can varied dynamically.  This allows textured interpolation
		across the surface of the sphere.  To draw many sphere instances,
		only "SetMapPoints" and "BindTex" need to be called repeatedly during rendering.
		Currently this class operates using vertex array-based drawing only.

		Vincent Hoon, 2007
		See copyright notice in copyright.txt for conditions of distribution and use.
*/
#pragma once
#include "GLVertexBuffer.h"

class GLMappedSphereBuf :
	public GLVertexBuffer
{
public:
	GLMappedSphereBuf(void);
	~GLMappedSphereBuf(void);
	bool Draw();
	void SetMapPoints(float * points, int count); // load in texture coordinates
	void GenerateSphere(int res);  // create triangulated sphere of given resolution (res * res)
	void Bind();
	void BindTex(); // bind in texture coordinates only
protected:
	unsigned int normal_buffer;
};
