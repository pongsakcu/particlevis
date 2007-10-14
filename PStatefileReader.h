/*
PStatefileReader

	   The PStatefileReader is essentially a set of IO functions, one for each major file type.
	Each function takes as its parameters a ParticleSet target and a filename to load from.
	The function will then process all data from the file and load it into
	the particle set.  Supported files: state files, using LoadStateFile; color maps, using LoadColorMap;
	vector files, using LoadVectorFile; and surface maps using ParseSurfaceMap.
	   The "DoCoordinateProcessing" function translates state file coordinates to the global frame of reference
	as necessary.

	Vincent Hoon, 2007
	See copyright notice in copyright.txt for conditions of distribution and use.
*/

#pragma once
#include "PFileReader.h"
#include "ParticleSet.h"
#include "GLVectorSet.h"
#include <string>
#include <fstream>
#include <expat.h>

using std::string;

class PStatefileReader : public PFileReader
{
public:
	PStatefileReader(CSemaphore* lock = NULL);
	~PStatefileReader(void);

	bool LoadFile();	// uses default settings
	bool LoadFile(ParticleSet* target, string filename); // loads a single file into the target Particle Set.
	
	bool AddFile();
	bool AddFile(ParticleSet* target, string filename); // concatenates a single file into an existing Particle Set.

	void DoCoordinateProcessing(ParticleSet* target);	// converts object coordinates (rotation vectors, normal vectors) to global coordinates	
	virtual bool LoadStateFile(ParticleSet* target, string filename); // load state file onto a particle set	
	
	bool IsFinished() { return finish; }
	void CleanUp(ParticleSet* target);
	void Finish() { finish = true; }
	void Terminate() { terminate = true; }
protected:
	CSemaphore* particle_lock;	// particle data synchronization object 
	bool finish;
	char *data;
	int datapos, maxpos;
	string masterTokens; // contains tokens for tokenizer
	bool threadSpawn, terminate;

	string default_filename;
	ParticleSet* default_target;	
	//HANDLE h_file, map_file;
};