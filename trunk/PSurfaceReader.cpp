#include "PSurfaceReader.h"

UINT ParseSurfaceThread(LPVOID pParam)
{
	PSurfaceReader* pf = (PSurfaceReader*)pParam;
	pf->StartThread();
	pf->Finish();	
	return 0;
}

PSurfaceReader::PSurfaceReader() :
PFileReader(),
default_resolution(10),
finish(true),
terminate(false),
threadSpawn(false),
particle_lock(NULL)
{
}

PSurfaceReader::~PSurfaceReader(void)
{
	CloseFile();
}

void PSurfaceReader::StartThread()
{
	this->LoadFile(default_target, default_filename);
}

bool PSurfaceReader::LoadFile(ParticleSet* target, string filename)
{
	finish = false;
	int res = default_resolution;	
	if (!threadSpawn)
	{
		threadSpawn = true;
		default_target = target;
		default_filename = filename;
		//AfxBeginThread(ParseSurfaceThread, this, THREAD_PRIORITY_BELOW_NORMAL, 0, 0, NULL);
		AfxBeginThread(ParseSurfaceThread, this, 0, 0, 0, NULL);
		return true;
	}
	threadSpawn = false;

	CSingleLock master(particle_lock);

	char* token = " \t\n";
	//masterTokens.copy(token, masterTokens.size());
	//token[masterTokens.size()] = '\0';	

	// Load Surface Map

	master.Lock();
	target->EnableSurfaceMapping();
	Particle::SetSurfaceMapSize(res);
	Particle* cur;
	char* toke;
	double max = 0, spray = 0, time = 0;
	int fram = 0, object = 0, panel = 0, particles = target->GetMaxParticles();
	int frames = target->GetMaxFrames();
	int first = particles, last = 0;

	if (!OpenFile(filename))
	{
		//finish = true;
		CloseFile();
		return false;
	}

	LoadLine();
	
	strtok(buffer, token);
	double start_time = strtod(strtok(NULL, token), NULL);
			
	if (start_time > 0)
	{
		if (target->ValidTimes())
		{
			vector<double>::const_iterator times = target->GetTime();		
			for (int i = 0; i < frames; i++)
			{
				if (start_time >= *times++)
					fram = i;
			}
		}
	}
	
	Particle** pbuffer = target->particles;
	int* objects = new int[res *res * particles];
	int* panels = new int[res *res * particles];
	double* sprays = new double[res *res * particles];
	int count = 0;
	master.Unlock();

	do
	{
		LoadLine(); // get first line

		count = 0;
		while (buffer[0]!='*' && (!Eof()))
		{
			if (strlen(buffer) <= 1) break;
			object = atoi(strtok(buffer, token));
			panel = atoi(strtok(NULL, token));
			spray = strtod(strtok(NULL, token), NULL);
			if (object < particles)
			{
				//pbuffer[object]->AddMapPoint(fram, panel, spray);
				objects[count] = object;
				panels[count] = panel;
				sprays[count] = spray;
				count++;
				//pbuffer[object]->LoadRender(-1);
				if (object < first) first = object;
				if (object > last) last = object;
			}
			if (spray > max) max = spray;

			LoadLine();
		}

		master.Lock();
		for (int i = 0; i < count; i++)
		{
			pbuffer[objects[i]]->AddMapPoint(fram, panels[i], sprays[i]);
			//pbuffer[objects[i]]->LoadRender(-1);
		}
		if (max > 0) Particle::map_scale = max;
		master.Unlock();
		fram++;

		if (terminate)
			break;

	} while (!Eof());

	delete[] objects;
	delete[] panels;
	delete[] sprays;

	if (max == 0) max = 1.0;
	master.Lock();	
	for (int i = 0; i < target->GetMaxParticles(); i++)
	{
		cur = pbuffer[i];
		for (int j = 0; j < res*res; j++)
			if (cur->GetPureSphere()) // only spheres should be given the mapped appearance
				cur->AddMapPoint(fram, j, 0);
		//cur->LoadRender(-1);
	}
	Particle::map_scale = max;
	CloseFile();
	master.Unlock();	
	finish = true;
	return true;
}
