#include "PZipReader.h"
#include "zlib.h"

PZipReader::PZipReader(CSemaphore* lock) : PStatefileReader(lock)
{
}

PZipReader::~PZipReader(void)
{
}

void PZipReader::LoadLine()
{
	do
	{
		if (gzgets(zipfile, buffer, 510)==Z_NULL)
			return;
	}
	while ((!Eof())&&(buffer[0] == '\r' || buffer[0] == '\n' || buffer[0] == '\0'));
}

bool PZipReader::OpenFile(string filename)
{
	zipfile = gzopen(filename.c_str(), "rb");
	if (zipfile == NULL)
		return false;
	else
		return true;
}

void PZipReader::CloseFile()
{
	gzclose(zipfile);
}

bool PZipReader::Eof()
{
	return gzeof(zipfile);
}