#include "PStatefileReader.h"
#include "GeometryEngine.h"
#include "RGBGradient.h"
#include "Camera.h"
#include "ChildView.h" // blah

#include <fcntl.h>
#include <io.h>

#include <vector>
#include <string>
#include <algorithm>
#include <math.h>
#include "TNT/tnt.h"
#include "quad.h"

extern char map_filename[256];

GeometryEngine* render;
PStatefileReader* instance = NULL;
ParticleSet *t_target;
vector<int> particles_to_mark;
// End XML

// Threading stuff
UINT LoadFileThread(LPVOID pParam);
UINT AddFileThread(LPVOID pParam);
char t_filename[MAX_PATH];

extern bool map_mode;

#define SQR(value) ((value)*(value))

PStatefileReader::PStatefileReader(CSemaphore* lock)
{
	particle_lock = lock;
	masterTokens = " \t\n";
	finish = false;
	threadSpawn = false;
	terminate = false;	
}

PStatefileReader::~PStatefileReader(void)
{	
}

UINT LoadFileThread(LPVOID pParam)
{
	PStatefileReader* pf = (PStatefileReader*)pParam;
	pf->LoadFile();
	pf->Finish();
	//AfxEndThread(0, true);
	return 0;
}

UINT AddFileThread(LPVOID pParam)
{
	PStatefileReader* pf = (PStatefileReader*)pParam;
	pf->AddFile();
	pf->Finish();
	//AfxEndThread(0, true);
	return 0;
}


// Primary data-parsing function
bool PStatefileReader::LoadFile()
{
	return LoadFile(default_target, default_filename);
}

bool PStatefileReader::AddFile()
{
	return AddFile(default_target, default_filename);
}

bool PStatefileReader::AddFile(ParticleSet *target, string filename)
{
	finish = false;
	if (!threadSpawn)
	{
		threadSpawn = true;
		default_target = target;
		//strcpy(default_filename, filename);
		default_filename = filename;
		AfxBeginThread(AddFileThread, this, 0, 0, 0, NULL);
		return true;
	}
	threadSpawn = false;
	std::ios_base::sync_with_stdio(false);
	LoadStateFile(target, filename);
	return true;
}

bool PStatefileReader::LoadFile(ParticleSet *target, string filename)
{
	finish = false;	
	if (!threadSpawn)
	{
		threadSpawn = true;
		default_target = target;
		//strcpy(default_filename, filename);
		default_filename = filename;
		AfxBeginThread(LoadFileThread, this, 0, 0, 0, NULL);
		return true;
	}
	threadSpawn = false;	
	std::ios_base::sync_with_stdio(false);
	
	CSingleLock master(particle_lock);
	master.Lock();
	target->ClearParticles();
	Particle::maxPosition = Particle::maxTransVelocity = Particle::maxRotVelocity = 0;
	master.Unlock();

	LoadStateFile(target, filename);
	DoCoordinateProcessing(target);

	return true;
}

// Internal routine for parsing the actual state file. 
// If the target set already has existing frames the routine
// will either add onto the existing set or fail if the count differs
bool PStatefileReader::LoadStateFile(ParticleSet* target, string filename) // load state file onto a particle set
{
	int env_vars = 0, particles = 0, baffles = 0, frames = 0;
	char* pos;	// for tokenizing, unused
	double currenttime = 0, radius = 5.591, Rband = 5.591e-3, Tband = 3.33e-3, Tcap = 1.03e-3, xlen, ylen;

	CSingleLock master(particle_lock);
	if (filename.length() == 0) return false;

	// buffer the token string into a character array
	char* token = new char[masterTokens.size()+1];
	masterTokens.copy(token, masterTokens.size());
	token[masterTokens.size()] = '\0';

	// Read in number of particles (check first frame)

	//AfxMessageBox("GO");
	
	if (!OpenFile(filename))
	{
		CloseFile();
		return false;
	}
	/*if (data==NULL)
	{
		CloseFile();
		return false;
	}*/
	
	int oldparticlecount = target->GetMaxParticles();
	
	buffer[0] = ' ';
	while ((buffer[0] != '*') && (!Eof())) // discard all data up to first frame
		LoadLine();

	particles = 0;
	int tokencount = 0;
	char *counttokens = NULL;
	bool readQuats = false;
	Particle::ResetStats();

	do // read until next frame
	{
		LoadLine();
		if (tokencount==0)
		{
			counttokens = strtok(buffer, token);
			while (counttokens != NULL)
			{
				tokencount++;
				counttokens = strtok(NULL, token);
			}
		}
		particles++;
	} while ((buffer[0] != '*') && (!Eof()));

	if (tokencount==13)
		readQuats = true;

	particles--;

	//AfxMessageBox("Party time");

	master.Lock();
	Particle ** pbuffer;	
	if (oldparticlecount <= 0)
	{
		if (particles <= 0)
		{
			master.Unlock();
			return false;
		}

		ParticleStore* dat_store = ParticleStore::GetNewStore(particles);
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
	
	LoadLine();
	while (buffer[0] != '*') // discard all data up to first frame
		LoadLine();

	// create value buffers to keep PSet access batched
	triple* pvalue = new triple[particles];
	quad* rqvalue = new quad[particles];
	triple* rvalue = new triple[particles];

	triple* pdvalue = new triple[particles];
	triple* rdvalue = new triple[particles];
	for (int p = 0; p < particles; p++) // zero data
	{
		pvalue[p].x = pvalue[p].y = pvalue[p].z = 0;
		rvalue[p].x = rvalue[p].y = rvalue[p].z = 0;
		pdvalue[p].x = pdvalue[p].y = pdvalue[p].z = 0;
		rdvalue[p].x = rdvalue[p].y = rdvalue[p].z = 0;
	}

	master.Unlock();

	int startframe = target->GetMaxFrames();
	// Read in each frame	
	while (!Eof())//(datapos < maxpos)
	{
		currenttime = strtod((buffer+2), &pos); // skip the '*' and pull current time in
		target->PushTime(currenttime);	// push time onto ParticleSet time stack

		LoadLine();
		// Read in all particle information
		//master.Lock();
		for (int j = 0; j < particles; j++)
		{
			if (Eof())//(datapos > maxpos)
			{
				CloseFile();
				terminate = false;
				return false;	// error reading input
			}

			// use strtod to strip out values from our input line
			pos = NULL;
			
			pvalue[j].x = strtod(buffer, &pos); if (pos==NULL) break;
			pvalue[j].y = strtod(pos, &pos); if (pos==NULL) break;
			pvalue[j].z = strtod(pos, &pos); if (pos==NULL) break;

			if (readQuats)
			{
				rqvalue[j].w = strtod(pos, &pos); if (pos==NULL) break;
				rqvalue[j].x = strtod(pos, &pos); if (pos==NULL) break;
				rqvalue[j].y = strtod(pos, &pos); if (pos==NULL) break;
				rqvalue[j].z = strtod(pos, &pos); if (pos==NULL) break;
			}
			else
			{
				rvalue[j].x = strtod(pos, &pos); if (pos==NULL) break;
				rvalue[j].y = strtod(pos, &pos); if (pos==NULL) break;
				rvalue[j].z = strtod(pos, &pos); if (pos==NULL) break;
			}

			pdvalue[j].x = strtod(pos, &pos); if (pos==NULL) break;
			pdvalue[j].y = strtod(pos, &pos); if (pos==NULL) break;
			pdvalue[j].z = strtod(pos, &pos); if (pos==NULL) break;

			rdvalue[j].x = strtod(pos, &pos); if (pos==NULL) break;
			rdvalue[j].y = strtod(pos, &pos); if (pos==NULL) break;
			rdvalue[j].z = strtod(pos, &pos);
			LoadLine();	// grab next line
		}

		master.Lock();
		if (readQuats)
		{
			for (int j = 0; j < particles; j++)
				if (pbuffer[j]->AddFrame(pvalue[j], rqvalue[j].Normalized(), pdvalue[j], rdvalue[j])==-1)
					return false;
		}
		else
		{
			for (int j = 0; j < particles; j++)
				if (pbuffer[j]->AddFrame(pvalue[j], rvalue[j], pdvalue[j], rdvalue[j])==-1)
					return false;
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

	delete[] pvalue;
	delete[] rvalue;
	delete[] pdvalue;
	delete[] rdvalue;
	delete[] rqvalue;

	CloseFile();
	input.clear();
	
	//AfxMessageBox("I am done. (A)");
	// Map Load

	delete[] token;
	return true;
}

void PStatefileReader::CleanUp(ParticleSet* target)
{
	//DoCoordinateProcessing(target);
	/*(rend->AssignLists(target->particles, target->GetMaxParticles());
	if ((strlen(map_filename) > 0) && (resolution > 0))
	{
		ParseSurfaceMap(target, map_filename, resolution);
	}*/
}

using namespace TNT;

// Transform all local coordinates into the world
// Since rotational vectors are in object coordinates, and normals are static,
// we can precompute and store them using this (hackish) routine
void PStatefileReader::DoCoordinateProcessing(ParticleSet* target)
{
	CSingleLock master(particle_lock);
	Array2D<float> matrix(4, 4);
	Array2D<float> vector(4, 1);
	Array2D<float> temp;
	//float matrixbuffer[16];
	//int i = 0;
	int max_particles = target->GetMaxParticles();
	int max_frames = target->GetMaxFrames();
	Particle *particle, *particle2;
	triple vectortemp;
	triple *vectortemp_rot, *vectortemp_normal;

	vectortemp_rot = new triple[max_frames];
	vectortemp_normal = new triple[max_frames];
	
	GeometryEngine matrix_help; // use to setup transformation matrices consistently
	//matrix_help.c_rot = triple(0, 0, 0);
	
	for (int p = 0; p < max_particles; p++)
	{
		particle = target->particles[p];
		for (int f = 0; f < max_frames; f++)
		{
			quad axis_angle = particle->PeekOrientation(f);
			matrix_help.quat = axis_angle; // passing the axis-angle directly is ok since we only call CalcMatrix
			matrix_help.CalcMatrix();

			// transform rotational velocity vector
			vectortemp = particle->PeekVector(f, ROTVELOCITY);
			vector[0][0] = vectortemp.x;
			vector[1][0] = vectortemp.y;
			vector[2][0] = vectortemp.z;
			vector[3][0] = 0;

			temp = matmult(matrix_help.matrix, vector);
			vectortemp_rot[f].x = temp[0][0];
			vectortemp_rot[f].y = temp[1][0];
			vectortemp_rot[f].z = temp[2][0];

			// transform normals to world coordinates
			vector[0][0] = 0;
			vector[1][0] = 1.0;
			vector[2][0] = 0;
			vector[3][0] = 0;

			temp = matmult(matrix_help.matrix, vector);
			vectortemp_normal[f].x = temp[0][0];
			vectortemp_normal[f].y = temp[1][0];
			vectortemp_normal[f].z = temp[2][0];
		}

		//master.Lock();
		for (int f = 0; f < max_frames; f++)
		{
			particle->SetVector(f, ROTVELOCITY, vectortemp_rot[f]);
			particle->SetVector(f, NORMALVEC, vectortemp_normal[f]);
		}
		//master.Unlock();

		if (terminate)
		{
			terminate = false;
			break;
		}
	}

	delete[] vectortemp_rot;
	delete[] vectortemp_normal;
}