/*
TransparentParticleRenderer

		   This renderer is essentially identical to the ParticleRenderer, except that it
		allows for reordering of the render buffer in order to perform transparent rendering
		with a order-dependent blend method (e.g. the "over" operator and similar).  The core
		drawing routine also passes an alpha value at render time.  This class may be merged
		back into the ParticleRenderer eventually since it differs very little.

		By Vincent Hoon, 2007
		See copyright notice in copyright.txt for conditions of distribution and use.
*/
#pragma once
#include "ParticleRenderer.h"

class TransparentParticleRenderer : public ParticleRenderer
{
public:
	TransparentParticleRenderer(void);
	~TransparentParticleRenderer(void);

	void SetAlpha(float alpha);
	void SortByDistance(triple origin, triple direction); // re-order buffer by new (camera) origin

	void Render(); // overriden render function
private:
	float alpha;
};
