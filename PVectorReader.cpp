#include "PVectorReader.h"

PVectorReader::PVectorReader(void) :
PFileReader()
{
}

PVectorReader::~PVectorReader(void)
{
}

bool PVectorReader::LoadFile(ParticleSet* target, string filename) // load state file onto a particle set
{
	GLVectorSet* vector_target = target->vectorSet;
	char* pos;	// for tokenizing
	int current_frame = 0;
	double currenttime;
	double max_len = 0;
	Particle::maxFieldVelocity = 0;
	vector<triple> point_cache;
	vector<triple> vector_cache;

	if (!OpenFile(filename))
	{
		CloseFile();
		return false;
	} 

	vector_target->ClearAllFrames();
	LoadLine(); // prime the pump
	while (!Eof()) //(datapos < maxpos)
	{
		currenttime = strtod((buffer+2), &pos); // skip the '*' and pull current time in
		LoadLine();
		// Read in all vector information
		while (buffer[0] != '*')
		{
			if (Eof()) // (datapos > maxpos)
			{
				//CloseFile();
				break;	// error reading input
			}

			// use strtod to strip out values from our input line
			pos = NULL;
			triple point, vector;
			point.x = strtod(buffer, &pos); if (pos==NULL) break;
			point.y = strtod(pos, &pos); if (pos==NULL) break;
			point.z = strtod(pos, &pos); if (pos==NULL) break;

			vector.x = strtod(pos, &pos); if (pos==NULL) break;
			vector.y = strtod(pos, &pos); if (pos==NULL) break;
			vector.z = strtod(pos, &pos); if (pos==NULL) break;

			if (vector.Length() > max_len)
				max_len = vector.Length();

			point_cache.push_back(point);
			vector_cache.push_back(vector);
					
			LoadLine();
		}		
		assert(point_cache.size() == vector_cache.size());
		vector_target->AddVectors(current_frame, point_cache.size(), point_cache, vector_cache);
		point_cache.clear();
		vector_cache.clear();
		current_frame++;
	}
	CloseFile();
	Particle::maxFieldVelocity = max_len;
	return true;
}

