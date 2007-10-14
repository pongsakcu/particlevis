#include "ParticleStore.h"
#include <assert.h>

ParticleStore* ParticleStore::singleton = 0;
ParticleStore ParticleStore::null_store;

ParticleStore::ParticleStore(const int particlecount) :
particle_count(particlecount)
{
	data.reserve(128);
	data.push_back(NewFrame());
}

ParticleStore::~ParticleStore(void)
{
	vector<Frame>::iterator iter;
	for (iter = data.begin(); iter != data.end(); iter++)	
		delete[] (unsigned char*)((*iter).pos);	
	data.clear();	
}

int ParticleStore::GetFrameCount() const
{
	return data.size();
}

int ParticleStore::GetParticleCount() const
{
	return particle_count;
}

bool ParticleStore::SetParticleData(int frame, int particle,
		triple pos, quad axis, triple d_pos, triple d_angle)
{
	while (data.size() <= frame)
	{
		Frame new_frame = NewFrame();
		if (new_frame.pos == NULL)
			return false;
		data.push_back(new_frame);
	}

	Frame i = data[frame];
	i.pos[particle] = pos;
	i.axis[particle] = axis;
	i.d_pos[particle] = d_pos;
	i.d_angle[particle] = d_angle;
	return true;
}

bool ParticleStore::AllocateFrame()
{
	Frame new_frame = NewFrame();
	if (new_frame.pos == NULL)
		return false;
	data.push_back(new_frame);
	return true;
}

triple * ParticleStore::GetFramePos(int frame) const
{
	if (data.size() > frame)
		return data[frame].pos;
	else	
		return data[0].pos;
}

quad * ParticleStore::GetFrameAxis(int frame) const
{
	if (data.size() > frame)
		return data[frame].axis;
	else
		return data[0].axis;
}

triple * ParticleStore::GetFrameVel(int frame) const
{
	if (data.size() > frame)
		return data[frame].d_pos;
	else
		return data[0].d_pos;
}

triple * ParticleStore::GetFrameRVel(int frame) const
{
	if (data.size() > frame)
		return data[frame].d_angle;
	else
		return data[0].d_angle;
}

Frame ParticleStore::GetFrame(int frame) const
{
	if (data.size() > frame)
		return data[frame];
	else
		return data[0];
}

Frame ParticleStore::NewFrame()
{
	unsigned int size = (3*sizeof(triple) + sizeof(quad)) * particle_count;
	unsigned char * memblock = new unsigned char[size];
	if (!memblock)
		return Frame();

	Frame newframe;
	newframe.pos = (triple*)(memblock);
	newframe.axis = (quad*)(memblock + sizeof(triple)*particle_count);
	newframe.d_pos = (triple*)(memblock + (sizeof(triple)+sizeof(quad))*particle_count);
	newframe.d_angle = (triple*)(memblock + (2*sizeof(triple)+sizeof(quad))*particle_count);

	for (int i = 0; i < size; i++)
		*(memblock++) = 0;
	return newframe;
}


ParticleStore* ParticleStore::GetNewStore(int count)
{
	if (singleton != NULL)
		delete singleton;
	singleton = new ParticleStore(count);
	return singleton;
}