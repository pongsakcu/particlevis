/*
AVIWriter class

		   Uses VFW and OpenGL to capture and save a
		video-only AVI file of the current scene.
		Frames are added to the AVI as the scene
		frames are played back.  The AVI is finalized
		whenever CloseFile() is called, generally at
		program termination.

		Vincent Hoon, 2007
*/

#pragma once
#include <string>
#include <fstream>
#include "Vfw.h"
using namespace std;

class AVIWriter
{
public:
	AVIWriter(string filename);
	AVIWriter(void);
	~AVIWriter(void);

	void SetFileName(string file);
	void SetFrameRate(int rate);
	void SetSize(int w, int h);
	bool SetCompression();
	bool AddFrame();

	string GetFileName() { return filename; }
	int GetFrameNumber() { return framenumber; }
	int GetWidth() { return width; }
	int GetHeight() { return height; }
	int GetFPS() { return fps; }

	bool FileOpen() { return fileopen; }

	bool OpenFile();
	bool CloseFile();

private:
	int fps, width, height;
	int framenumber;
	string filename;
	PAVIFILE avifile;
	PAVISTREAM stream, com_stream;
	bool fileopen;
};
