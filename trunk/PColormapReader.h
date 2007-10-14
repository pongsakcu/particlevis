// PColormapReader: A file reader for the colormap file format.
// Vincent Hoon, 2007
// See copyright notice in copyright.txt for conditions of distribution and use.
#pragma once
#include "pfilereader.h"

class PColormapReader :
	public PFileReader
{
public:
	PColormapReader(void);
	~PColormapReader(void);

	bool LoadFile(ParticleSet* target, string filename); // loads a colormap file onto the particle collection
	void SetPersistence(bool persist) { persistent = persist; } // set persistent/nonpersistent behavior for coloring
protected:
	bool persistent;
	string masterTokens;
};
