/*
PFileReader

		   The PFileReader is the superclass from which basic file I/O classes are derived.
		Each class is designed to take file input and load it into a target ParticleSet, via the LoadFile
		function.  

		Vincent Hoon, 2007
		See copyright notice in copyright.txt for conditions of distribution and use.
*/
#pragma once
#include "ParticleSet.h"
#include <string>
#include <fstream>
#define LINESIZE 512

using std::string;

class PFileReader
{
public:
	PFileReader(void);
	~PFileReader(void);

	virtual bool LoadFile(ParticleSet* target, string filename) = 0; // loads a single file into the target Particle Set.

	// Check or modify loading state, mainly for threaded methods
	virtual bool IsFinished();
	virtual void CleanUp();
	virtual void Finish();
	virtual void Terminate();
protected:
	ifstream input;
	char buffer[LINESIZE]; // buffer for a single single of ASCII data
	virtual void LoadLine();
	virtual bool OpenFile(string filename);
	virtual void CloseFile();
	virtual bool Eof();
};