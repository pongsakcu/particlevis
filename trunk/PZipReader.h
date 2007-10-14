/*
PZipReader

	  The PZipReader is an extension of the statefile reader that
	adds support for GZipped files.  The zlib libary is used
	to override all the file I/O functions of the statefile class,
	allowing Gzipped statefiles to be read instead of plain ASCII.

	Vincent Hoon, 2007
	See copyright notice in copyright.txt for conditions of distribution and use.
*/
#pragma once
#include "PStatefileReader.h"
#include "zlib.h"

class PZipReader :
	public PStatefileReader
{
public:
	PZipReader(CSemaphore* lock = NULL);
	~PZipReader(void);

	void LoadLine();
private:
	bool OpenFile(string filename);
	void CloseFile();
	bool Eof();

	gzFile zipfile;
};
