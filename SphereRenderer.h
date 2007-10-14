/*
SphereRenderer

		   Given input, the SphereRenderer is responsible for drawing either a collection of point
	primitives or point_sprite primitives using the passed positions.  In the case of GLSL support,
	the point_sprites primitives can be used to implement one of the sphere shading programs passed
	in "SelectShader."

	By Vincent Hoon, 2007
	See copyright notice in copyright.txt for conditions of distribution and use.
*/
#pragma once
#include "ParticleRenderer.h"
#include "GLVertexBuffer.h"
#include "GLShader.h"

class SphereRenderer : public ParticleRenderer
{
public:
	SphereRenderer(void);
	~SphereRenderer(void);
	//void SetColorClass(RGBClassifier* classifier);
	void SetSize(int size); // possible resizing logic
	void Clear(); // call before new frame
	void Load(triple* position, quad* orientation, triple* d_pos, triple* d_theta, Particle* p);
	void Render(); // call to render

	void EnableVelocity() { use_velocity = true; vBuf.EnableTexCoords(); }
	void DisableVelocity() { use_velocity = false; vBuf.DisableTexCoords(); }

	void SortByDistance(triple camera, triple direction);
	void SelectShader(string vertname, string pixelname);
	void SetAlpha(float a) { alpha = a; }

	void EnableQueries() {} // occlusion queries unimplemented
	void DisableQueries() {}

	void SetPointPrimitives(bool usepoints) { use_points = usepoints; } // Pass as pure points instead of using shaders.
protected:
	float alpha;
	float scalar;
	bool bound, use_velocity, use_points;
	GLVertexBuffer vBuf;
	GLShader sphere_shader;
};
