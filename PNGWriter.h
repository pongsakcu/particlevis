/*
PNGWriter

		   Essentially a C++ wrapper for PNGLIB/ZLIB routines.  Uses glReadPixels to grab
		the OpenGL framebuffer and dumps it to a PNG file of the user's choosing.


		Vincent Hoon, 2007
		See copyright notice in copyright.txt for conditions of distribution and use.
*/
#pragma once
#include <string>
#include <fstream>
using namespace std;

class PNGWriter
{
public:
	PNGWriter(string filename);
	PNGWriter(void);
	~PNGWriter(void);

	void SetFileName(string file);
	void SetFileNumber(int number) { filenumber = number; }
	bool WriteImage(void * ptr=NULL);

	string GetFileName() { return filename; }
	int GetFileNumber() { return filenumber; }
	bool GetAscii() { return writeascii; }
	void SetAscii(bool mode) { writeascii = mode; }

private:
	int filenumber;
	string filename;
	FILE *fp;
	bool fileopen;
	bool writeascii;
	bool OpenFile(bool png);
};
