/*

PFileReader

The PFileReader is essentially a single IO function, LoadFile, that
takes as its parameters a ParticleSet target and a filename to load from.
The function will then process all data from the file and load it into
the particle set.

*/

#pragma once
#include "ParticleSet.h"
#include <string>
#include <fstream>
#include "zlib.h"

class PFileReader
{
public:
	PFileReader(void);
	~PFileReader(void);

	bool LoadFile(ParticleSet* target, const char filename[MAX_PATH]); // loads a single file into the target Particle Set.
	void DoCoordinateProcessing(ParticleSet* target);	// converts object coordinates (rotation vectors, normal vectors) to global coordinates
	bool LoadColorMap(ParticleSet* target, const char filename[MAX_PATH]); // loads a colormap file onto the particle collection
	void LoadLine();				// pulls in one line from the line, discarding blank lines
	void setTokens(string newtokens); // sets the token function to use different tokens
	void UseGZip(bool flag) { useGZip = flag; }
private:
	ifstream input;		// primary stream object to be used
	gzFile gFile;		// gzip file object
	char buffer[512];	// contains current line of data
	string masterTokens;// contains tokens for tokenizer
	bool useGZip;
};
