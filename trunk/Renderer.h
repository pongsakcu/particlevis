/*
Renderer

		   The Renderer is an abstract class to be used for deriving renderer classes.
		The responsibility of these classes is to translate particle data from the Particle
		and ParticleStore objects into an appropriate OpenGL visualization.  Color classification
		is handled 	through collusion with the functor-style RGBClassifiers, allowing a degree
		of independence from the specific renderer logic.
		   The general pattern of use is to Clear() out the renderer objects, set the (max) size of the
		data to be passed, and then iterate through the Particle frame data, calling Load() for
		each row of data that applies to each renderer.  At this point the Render() method can be
		called any number of times before the renderers are re-cleared and new data is loaded.  Each class
		will probably use some sort of internal buffer to insure that this cycle is efficient.		   

		Vincent Hoon, 2007
		See copyright notice in copyright.txt for conditions of distribution and use.
*/
#pragma once
#include "RGBClassifier.h"

class Renderer
{
public:
	Renderer(void);
	~Renderer(void);
	virtual void SetColorClass(RGBClassifier* classifier); // load a color classifier functor
	virtual void SetSize(int size) = 0; // possible resizing logic
	virtual void Clear() = 0; // call before new frame
	// Load call: input is center position of object, axis-angle orientation, and two 3-D vectors.
	virtual void Load(triple* position, quad* orientation, triple* d_pos, triple* d_theta, Particle* p) = 0;
	virtual void Render() = 0; // call to render
protected:
	RGBClassifier* coloring;
};