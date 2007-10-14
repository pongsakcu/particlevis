/*
PVectorReader

	   The PVectorReader supports the loading of vectorfiles into a ParticleSet.
	The GLVectorSet in the target is used to hold the loaded data.

	Vincent Hoon, 2007
	See copyright notice in copyright.txt for conditions of distribution and use.
*/
#pragma once
#include "PFileReader.h"
#include "GLVectorSet.h"

class PVectorReader :
	public PFileReader
{
public:
	PVectorReader(void);
	~PVectorReader(void);
	bool LoadFile(ParticleSet* target, string filename); // loads a single file into the target Particle Set.
};
