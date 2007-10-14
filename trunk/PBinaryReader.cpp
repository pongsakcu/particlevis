#include "PBinaryReader.h"

#define SQR(value) ((value)*(value))

PBinaryReader::PBinaryReader(CSemaphore* lock) :
PStatefileReader(lock)
{
}

PBinaryReader::~PBinaryReader(void)
{
}

bool PBinaryReader::OpenFile(string filename)
{
	if (filename.length() == 0) return false;
	input.open(filename.c_str(), ios::binary | ios::in);
	if (input.fail())
		return false;
	else
		return true;
}

void PBinaryReader::CloseFile()
{
	input.close();
	input.clear();
}

bool PBinaryReader::Eof()
{
	return input.eof() || input.fail();
}

bool PBinaryReader::LoadStateFile(ParticleSet* target, string filename) // load state file onto a particle set
{
	int env_vars = 0, particles = 0, baffles = 0, frames = 0;
	char* pos;	// for tokenizing, unused
	float currenttime = 0;

	CSingleLock master(particle_lock);
	if (filename[0]=='\0') return false;

	// buffer the token string into a character array
	char* token = new char[masterTokens.size()+1];
	masterTokens.copy(token, masterTokens.size());
	token[masterTokens.size()] = '\0';

	// Read in number of particles (check first frame)
	if (!OpenFile(filename))
	{
		CloseFile();
		return false;
	}

	int oldparticlecount = target->GetMaxParticles();
	unsigned int headerflag = 0, newparticlecount = 0;
	input.read((char*)&headerflag, 4); // Should be "DEM ", which is 541934916 as an unsigned int.
	input.read((char*)&newparticlecount, 4);
	if (headerflag != 541934916 || newparticlecount <= 0)
	{
		CloseFile();
		return false;
	}	
	particles = newparticlecount;
	master.Lock();

	Particle ** pbuffer;
	ParticleStore* dat_store = NULL;

	if (oldparticlecount <= 0)
	{
		if (particles <= 0)
		{
			master.Unlock();
			return false;
		}

		dat_store = ParticleStore::GetNewStore(particles);
		pbuffer = new Particle*[particles];

		for (int i = 0; i < particles; i++)
		{
			pbuffer[i] = new Particle(*dat_store, 5.591e-3); // 5.591e-3: magic number!
			pbuffer[i]->SetPID(i);
		}
		target->LoadParticles(pbuffer, dat_store, particles);
	}
	else
	{
		if (particles != oldparticlecount)
		{
			master.Unlock();
			return false;
		}
		pbuffer = target->particles;
	}

	CloseFile();
	if (!OpenFile(filename))
	{
		master.Unlock();
		return false;
	}

	input.read(buffer, 8);

	// create value buffers to keep PSet access batched
	triple* pvalue = new triple[particles];
	quad* rqvalue = new quad[particles];
	triple* pdvalue = new triple[particles];
	triple* rdvalue = new triple[particles];

	std::fill(pvalue, pvalue+particles, triple());
	std::fill(rqvalue, rqvalue+particles, quad());
	std::fill(pdvalue, pdvalue+particles, triple());
	std::fill(rdvalue, rdvalue+particles, triple());

	Particle::ResetStats();
	master.Unlock();
	
	int startframe = target->GetMaxFrames();
	// Read in each frame	
	while (!Eof())//(datapos < maxpos)
	{
		input.read((char*)&currenttime, 4);
		if (!Eof())
			target->PushTime(currenttime);	// push time onto ParticleSet time stack
		else
			break;
				

		// Read in all particle information

		master.Lock();
		dat_store->AllocateFrame();

		Frame f = dat_store->GetFrame(frames);
		input.read((char*)(f.pos), sizeof(triple) * particles);
		input.read((char*)(f.axis), sizeof(quad) * particles);
		input.read((char*)(f.d_pos), sizeof(triple) * particles);
		input.read((char*)(f.d_angle), sizeof(triple) * particles);	

		triple *pos = f.pos, *pos_dot = f.d_pos, *theta_dot = f.d_angle;
		quad *axis = f.axis;
		// transform quaternions to axis-angle (normally done in Particle), check maximums
		for (int p = 0; p < particles; p++)
		{			
			if (length(*pos_dot) > Particle::maxTransVelocity) Particle::maxTransVelocity = length(*pos_dot);
			if (length(*pos_dot) > Particle::maxFieldVelocity) Particle::maxFieldVelocity = length(*pos_dot);
			if (length(*theta_dot) > Particle::maxRotVelocity) Particle::maxRotVelocity = length(*theta_dot);
			if (length(*pos) > Particle::maxPosition) Particle::maxPosition = length(*pos);

			static float a, l;
			a = (2.0*acos(axis->w))*180.0/PI;
			if (a == 0)			
				*axis = quad(0, 0, 1, 0);			
			else
			{
				l = sqrt(SQR(axis->x) + SQR(axis->y) + SQR(axis->z));
				*axis = quad(a, axis->x / l, axis->y / l, axis->z / l );
			}

			pos++; axis++; pos_dot++; theta_dot++;
		}

		frames++;
		target->SetFrameCount(frames+startframe);

		master.Unlock();

		if (terminate)
		{
			terminate = false;
			break;
		}
	}

	for (int p = 0; p < particles; p++)	
		pbuffer[p]->normal.resize(frames, triple(0, 1, 0));			

	delete[] pvalue;
	delete[] rqvalue;
	delete[] pdvalue;
	delete[] rdvalue;

	CloseFile();
	input.clear();

	delete[] token;
	return true;
}