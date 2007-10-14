#include "PColormapReader.h"

PColormapReader::PColormapReader(void) :
PFileReader(),
persistent(true)
{
	masterTokens = " \t\n";
}

PColormapReader::~PColormapReader(void)
{
}

// Load in Color-Map file specifying the base color of the particles over time
bool PColormapReader::LoadFile(ParticleSet* target, string filename)
{
	//CSingleLock master(particle_lock);
	target->ClearColor();
	target->EnableColorMapping();
	if (!OpenFile(filename))
		return false;

	if (Eof())
		return false;

	// buffer the token string into a character array
	char* token = new char[masterTokens.size()+1];
	masterTokens.copy(token, masterTokens.size());
	token[masterTokens.size()] = '\0';

	char* pos;	// for tokenizing
	double time = 0, timeStart = target->GetStartTime(), timeEnd = target->GetEndTime();
	double timeStep = (timeEnd - timeStart) / (double)(target->GetMaxFrames()-1);
	int pIndex = 0, frame = 0;
	triple rgb;

	LoadLine();
	while (!Eof())
	{
		rgb = triple(0, 0, 0);
		pos = strtok(buffer, token);
		if (pos)
			pIndex = atoi(pos);
		pos = strtok(NULL, token);
		if (pos)
			time = strtod(pos, NULL);
		pos = strtok(NULL, token);
		if (pos)
			rgb.x = min(1.0, (double)atoi(pos) / 255.0);
		pos = strtok(NULL, token);
		if (pos)
			rgb.y = min(1.0, (double)atoi(pos) / 255.0);
		pos = strtok(NULL, token);
		if (pos)
			rgb.z = min(1.0, (double)atoi(pos) / 255.0);

		frame = (int)time;
        
		if (pIndex < target->GetMaxParticles() && pIndex >= 0)
			target->particles[pIndex]->AddColor(frame, rgb, persistent);
		else
			return false; // error parsing file!
		
		LoadLine();
	}

	// "cap" off all the particles
	if (persistent)
	{
		int lastframe = target->GetMaxFrames();
		for (int i = 0; i < target->GetMaxParticles(); i++)
		{
			target->particles[i]->AddColor(lastframe, triple(1, 1, 1), persistent);

		}
	}

	CloseFile();
	return true;
}