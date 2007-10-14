/*

PSurfaceReader

		   This class reads in a surface map file.  It makes use of a semaphore for threaded loads, 
		and is designed to be used in concert with another loader, the PXMLReader.

		Vincent Hoon, 2007
		See copyright notice in copyright.txt for conditions of distribution and use.
*/

#pragma once
#include "PFileReader.h"
#include <fstream>

using std::ifstream;

class PSurfaceReader :
	public PFileReader
{
public:
	PSurfaceReader();
	~PSurfaceReader(void);
	
	bool LoadFile(ParticleSet* target, string filename);
	void LoadLock(CSemaphore* lock) { particle_lock = lock; }
	void SetResolution(int res) { default_resolution = res; }
	virtual void Terminate() { terminate = true; }

	bool IsFinished() { return finish; }

	void StartThread();
protected:
	CSemaphore* particle_lock;	// particle data synchronization object 
	bool finish, terminate, threadSpawn;
	int default_resolution;
	ParticleSet* default_target;
	string default_filename;
};
