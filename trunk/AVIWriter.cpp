#include "stdafx.h"
#include <GL\gl.h>
#include <GL\glu.h>
#include "glext.h"
#include "AVIWriter.h"

//extern "C" void exit (int) throw (); // WHY I DO NOT KNOW

AVIWriter::AVIWriter(void)
{
	avifile = NULL;
	stream = NULL;
	filename = "output.avi";
	framenumber = 0;
	fileopen = false;
	width = 640;
	height = 480;
	fps = 15;
}

AVIWriter::AVIWriter(string filename)
{
	this->filename = filename;
	avifile = NULL;
	stream = NULL;
	framenumber = 0;
	fileopen = false;
	width = 640;
	height = 480;
	fps = 15;
}

AVIWriter::~AVIWriter(void)
{
}

void AVIWriter::SetFileName(string file)
{
	if (filename.compare(file)==0)
		return;
	filename = file;
	CloseFile();
}

bool AVIWriter::AddFrame()
{
	if (!fileopen)
	{
	if (!OpenFile())
		return false;
	}

	// Determine size of current buffer
	int w, h;
	int dims[4];    
	glGetIntegerv(GL_VIEWPORT, dims);
	w = dims[2];
	h = dims[3];
	unsigned char* imageData;

	// Grab image data

	if (width==w && height==h)
	{
		imageData = new unsigned char[width*height*3];
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		glFinish();
		glReadPixels(0, 0, width, height, GL_BGR, GL_UNSIGNED_BYTE, imageData);
		glFinish();

		AVIStreamWrite(com_stream, framenumber, 1, imageData, width*height*3, AVIIF_KEYFRAME, NULL, NULL); 
		framenumber++;
		delete[] imageData;
		return true;
	}
	else return false;
}

bool AVIWriter::OpenFile()
{
	if (fileopen)
	{
		CloseFile();
	}
	HRESULT result;
	int w = width, h = height;

	BITMAPINFOHEADER info;

	info.biSize = sizeof(info);
	info.biWidth = w;
	info.biHeight = h;
	info.biPlanes = 1;
	info.biBitCount = 24;
	info.biCompression = BI_RGB;
	info.biSizeImage = w*h*3;
	info.biClrUsed = 0;
	info.biClrImportant = 0;

	AVIFileInit();
	result = AVIFileOpen(&avifile, filename.c_str(), OF_WRITE | OF_CREATE, NULL); // open file
	if (result != AVIERR_OK)
		return false;

	AVISTREAMINFO header;

	memset(&header, 0, sizeof(header));
	header.fccType = streamtypeVIDEO; // stream type
	header.fccHandler = 0;
	header.dwScale = 1;
	header.dwRate = fps; // set fps
	header.dwSuggestedBufferSize = info.biSizeImage;
	SetRect(&header.rcFrame, 0, 0,
	(int) info.biWidth,
	(int) info.biHeight);

	result = AVIFileCreateStream(avifile, &stream, &header); // attach stream to file

	AVICOMPRESSOPTIONS c_option;
	AVICOMPRESSOPTIONS FAR * aopts[1] = {&c_option};
	memset(&c_option, 0, sizeof(c_option));

	if (!AVISaveOptions(NULL, 0, 1, &stream, (LPAVICOMPRESSOPTIONS FAR *) &aopts))
		return false;

	result = AVIMakeCompressedStream(&com_stream, stream, &c_option, NULL);
	if (result != AVIERR_OK)
		return false;

	fileopen = true;

	result = AVIStreamSetFormat(com_stream, 0, &info, // stream format
						info.biSize + // format size
						info.biClrUsed * sizeof(RGBQUAD));

	if (result != AVIERR_OK)
		return false;
	else
		return true;
}

bool AVIWriter::SetCompression()
{
	if (fileopen)
	{
		if (CloseFile())
			if (OpenFile())
				return true;
	}
	return false;
}

bool AVIWriter::CloseFile()
{
	if (fileopen)
	{
		AVIStreamRelease(stream);
		AVIStreamRelease(com_stream);
		AVIFileClose(avifile);
		AVIFileExit();
		fileopen = false;
		framenumber = 0;
		
		return true;
	}
	else return false;
}

void AVIWriter::SetFrameRate(int rate)
{
	if (rate == fps)
		return;
	fps = max(1, rate);
	CloseFile();
}

void AVIWriter::SetSize(int w, int h)
{
	if (width==w && height==h)
		return;
	width = w;
	height = h;
	CloseFile();
}