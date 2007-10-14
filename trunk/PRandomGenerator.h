#pragma once

// This utility class allow the generation of a distribution of randomly placed
// particles.  No concern is given to overlapped particles.
//
// Vincent Hoon, 2007

class ParticleSet;

class PRandomGenerator
{
public:
	PRandomGenerator(void);
	~PRandomGenerator(void);
	void GenerateRandomFrame(ParticleSet* primary, int particles, bool normal = false);
	void AddRandomFrame(ParticleSet* primary, bool normal = false);
private:
	void RandomSeed();
	double Random();
};
