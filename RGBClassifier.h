/*
	RGB color classifiers are functors that convert particle data into color.
	This header defines the classifiers commonly used by ParticleVis.

	Vincent Hoon, 2007
	See copyright notice in copyright.txt for conditions of distribution and use.
*/
#pragma once
#include "Particle.h"

class RGBClassifier // Abstract classifier superclass.
{
public:
	RGBClassifier();
	~RGBClassifier();
	virtual triple operator() (triple pos, quad axis, triple d_pos, triple d_angle, Particle * p) = 0;
};


// Single color
class ColorClassifier : public RGBClassifier
{
public:
	ColorClassifier(triple color) : c(color) {}
	triple c;
	triple operator() (triple pos, quad axis, triple d_pos, triple d_angle, Particle * p)
	{
		return c;
	}
};

// White (for convenience)
class WhiteClassifier : public RGBClassifier
{
public:
	triple operator() (triple pos, quad axis, triple d_pos, triple d_angle, Particle * p)
	{
		return triple(1, 1, 1);
	}
};

// Position-based classification
class PosClassifier : public RGBClassifier
{
public:
	triple operator() (triple pos, quad axis, triple d_pos, triple d_angle, Particle * p)
	{
		return triple(0.5, 0.5, 0.5) + (pos / Particle::maxPosition);
	}
};

// Distance-based classification
class DistClassifier : public RGBClassifier
{
public:
	triple operator() (triple pos, quad axis, triple d_pos, triple d_angle, Particle * p)
	{
		return Particle::grad.ConvertToColor(pos.Length() / Particle::maxPosition);
	}
};

// Velocity based classification
class TransVelocityClassifier : public RGBClassifier
{
public:
	triple operator() (triple pos, quad axis, triple d_pos, triple d_angle, Particle * p)
	{
		return Particle::grad.ConvertToColor(d_pos.Length() / Particle::maxVelocityMap);
	}
};

// Angular velocity based classification
class RotVelocityClassifier : public RGBClassifier
{
public:
	triple operator() (triple pos, quad axis, triple d_pos, triple d_angle, Particle * p)
	{
		return Particle::grad.ConvertToColor(d_angle.Length() / Particle::maxRVelocityMap);
	}
};

// Color map classification
class ParticleColorClassifier : public RGBClassifier
{
public:
	triple operator() (triple pos, quad axis, triple d_pos, triple d_angle, Particle * p);
};

// Velocity field classification
class VectorFieldClassifier : public RGBClassifier
{
public:
	triple operator() (triple pos, quad axis, triple d_pos, triple d_angle, Particle * p)
	{
		return Particle::grad.ConvertToColor(d_pos.Length() / Particle::maxFieldVectorMap);
	}
};