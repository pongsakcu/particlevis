#include "RGBClassifier.h"
#include "ParticleSet.h"

RGBClassifier::RGBClassifier() {}
RGBClassifier::~RGBClassifier() {}

// Color map classification
triple ParticleColorClassifier::operator() (triple pos, quad axis, triple d_pos, triple d_angle, Particle * p)
{
	if (p->colorMap.size() > ParticleSet::currentframe)
		return p->colorMap[ParticleSet::currentframe];
	else
		return triple(1, 1, 1);
}
