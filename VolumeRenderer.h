#pragma once
#include "Renderer.h"
#include "GLPlanarSlices.h"

// TODO: Implement 3-D texturing using this class.  Currently unused.

class VolumeRenderer :
	public Renderer
{
public:
	VolumeRenderer(void);
	~VolumeRenderer(void);
	void SetSize(int size); // set size of volume cube
	void Clear(); // call before new data
	void Load(triple* position, quad* orientation, triple* d_pos, triple* d_theta, Particle* p); // set orientation
	void Render(); // call to render
protected:
	GLPlanarSlices slices;
};
