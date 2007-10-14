#include "PFileReader.h"

PFileReader::PFileReader() {}
PFileReader::~PFileReader() {}
bool PFileReader::IsFinished() { return true; }
void PFileReader::CleanUp() {}
void PFileReader::Finish() {}
void PFileReader::Terminate() {}

// Open file for use in LoadFile
bool PFileReader::OpenFile(string filename)
{
	if (filename.length() == 0)
		return false;
	input.open(filename.c_str(), ios::in); // open input file
	if (input.fail())
		return false;
	else
		return true;
	///
    /*SECURITY_ATTRIBUTES sa;
    memset(&sa, 0, sizeof(sa));
    h_file = CreateFile(filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    DWORD FileSize = GetFileSize(h_file, NULL);
	maxpos = (int)FileSize;
    map_file = CreateFileMapping(h_file, NULL, PAGE_READONLY|SEC_COMMIT, 0, 0, NULL);
    data = (char*)MapViewOfFile(map_file, FILE_MAP_READ, 0, 0, 0);
	datapos = 0;*/
	///
}

// Close any open file
void PFileReader::CloseFile()
{
	input.close();
	input.clear();
	///
	/*CloseHandle(h_file);
	CloseHandle(map_file);	
	UnmapViewOfFile(data);*/
	///
}

// Returns EOF flag
bool PFileReader::Eof()
{
	//return datapos > maxpos;
	return input.eof() || input.fail(); 
}


// Basic I/O function, reads in a single line from an ASCII file into "buffer"
void PFileReader::LoadLine() // read in a single line of data, skip blank lines
{
	do
	{
		input.getline(buffer, LINESIZE);
	} while ((!Eof())&&(buffer[0] == '\r' || buffer[0] == '\n' || buffer[0] == '\0'));
	/*static int b; 
	b = 0;
	do
	{
		do
		{
			buffer[b++] = data[datapos++];
		} while ((buffer[b-1] != '\n') && (datapos < maxpos) && (b < 510));
		buffer[b] = '\0';
		b = 0;
	} while ((datapos < maxpos) && (buffer[0] == '\r' || buffer[0] == '\n' || buffer[0] == '\0'));*/
}