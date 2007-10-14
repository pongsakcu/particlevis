#include ".\prandomgenerator.h"
#include "Particle.h"
//#include "ParticleMap.h"
#include "ParticleSet.h"
#include "triple.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

PRandomGenerator::PRandomGenerator(void)
{
}

PRandomGenerator::~PRandomGenerator(void)
{
}

void PRandomGenerator::GenerateRandomFrame(ParticleSet* primary, int particles, bool normal)
{
	primary->ClearParticles();
	Particle** partset = new Particle*[particles];
	ParticleStore * rand_store = ParticleStore::GetNewStore(particles);
	for (int i = 0; i < particles; i++)
		partset[i] = new Particle(*rand_store, 1e-3, i);

	triple rvec, theta;
	triple nullvec(0, 0, 0);

	double w = 0;
	for (int i = 0; i < particles; i++)
	{
		if (!normal)
		{
			rvec.x = Random() - 0.5;
			rvec.y = Random() - 0.5;
			rvec.z = Random() - 0.5;
		}
		else
		{
			do
			{
				rvec.x = 2.0*Random() - 1.0;
				rvec.y = 2.0*Random() - 1.0;
				rvec.z = 2.0*Random() - 1.0;
				w = dotproduct(rvec, rvec);
			} while (w >= 1.0);
			w = sqrt((-2.0 * log(w)) / w);
			rvec.x = rvec.x * w * 0.25;
			rvec.y = rvec.y * w * 0.25;
			rvec.z = rvec.z * w * 0.25;
		}
		/*theta.x = Random() * TWOPI;
		theta.y = Random() * TWOPI;
		theta.z = Random() * TWOPI;		
		partset[i]->AddFrame(rvec, theta, rvec, nullvec);*/
		partset[i]->AddFrame(rvec, triple(0, 0, 0), rvec, nullvec);		
	}

	primary->LoadParticles(partset, rand_store, particles);
}

void PRandomGenerator::RandomSeed()
{
	srand( (unsigned)time( NULL ) );
}

double PRandomGenerator::Random()
{
	return ((double)rand() / (double)RAND_MAX);
}