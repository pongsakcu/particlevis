/*
VectorRenderer

		   This class facilitates particle data vector rendering.  Various vector types, defined in
		Particle.h, are supported.  All vectors are rendered using lines and an arrowhead that is
		determined by the vector type.  Color classification is also tied to the vector type, using
		the gradient approach.

		By Vincent Hoon, 2007
		See copyright notice in copyright.txt for conditions of distribution and use.
*/
#pragma once
#include "Renderer.h"
#include "GLLineBuffer.h"

class VectorRenderer :
	public Renderer
{
public:
	VectorRenderer(void);
	~VectorRenderer(void);	
	void SetSize(int size); // possible resizing logic	
	void Clear(); // call before new frame
	void Load(triple* position, quad* orientation, triple* d_pos, triple* d_theta, Particle* p); // load in particle data
	void Render(); // call to render

	void SetType(int vectortype) { type = vectortype; }
protected:
	int type;
	bool bound;
	GLLineBuffer vBuf;
	TransVelocityClassifier vel_coloring;
	RotVelocityClassifier rot_coloring;
	VectorFieldClassifier field_coloring;
};
