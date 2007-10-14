// File reader for the experimental binary file format.
// Vincent Hoon, 2007
// See copyright notice in copyright.txt for conditions of distribution and use.
#pragma once
#include "PStatefileReader.h"
#include <fstream>

class PBinaryReader :
	public PStatefileReader
{
public:
	PBinaryReader(CSemaphore* lock = NULL);
	~PBinaryReader(void);
	bool LoadStateFile(ParticleSet* target, string filename);

protected:
	bool OpenFile(string filename);
	void CloseFile();
	bool Eof();	
};
