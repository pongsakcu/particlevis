/*
MappedSphereRenderer

	   This class handles the drawing of spherical particles that have scalar data
	mapped onto their surfaces.  This renderer is overall quite brittle, but should
	be able to render maps of multiple (square) sizes.  Samples are loaded from each
	Particle object and passed as texture coordiates on the generated spheres.
	   This class is tightly bound to the storage provided by the Particle->mapped_points field,
	but a separate data structure for the surface maps would be nice to have.

	Some important fields:
		resolution - The size of the mapping grid: resolution * resolution total samples are used.
		size - The maximum number of particles that can be passed via Load().
		render_buffer - contains the position/oriention/colors for each particle to be rendered
		tex_coords - contains the set of texture coordinates to be passed during rendering (copied from Particle's "mapped_points")       
		buffer - a vertex buffer/array containing the sphere object, used to load up and render spheres one at a time

	Vincent Hoon, 2007
	See copyright notice in copyright.txt for conditions of distribution and use.
*/
#pragma once
#include "ParticleRenderer.h"
#include "GLMappedSphereBuf.h"

class MappedSphereRenderer :
	public ParticleRenderer
{
public:
	MappedSphereRenderer(const int res = 10);
	~MappedSphereRenderer(void);

	void SetSize(int size); // possible resizing logic	
	void Clear(); // call before new frame
	void Load(triple* position, quad* orientation, triple* d_pos, triple* d_theta, Particle* p); // load in particle data
	void Render(); // call to render	

	void SetResolution(int res);

protected:
	int resolution;
	int size;
	int loaded;
	GLMappedSphereBuf buffer;
	float * texture_cache;
	bool bound;
	float * tex_coords;

	struct MRenderCmd { 
		triple pos;
		quad axis;
		triple rgb;
		float radius;
		unsigned int PID;
		float distance;
	};
	typedef struct MRenderCmd MapRenderCommand;

	vector<MapRenderCommand> render_buffer;
};