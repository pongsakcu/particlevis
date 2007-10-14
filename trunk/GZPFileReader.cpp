#include "pfilereader.h"
#include "ParticleSphere.h"
#include "ParticleBiconvex.h"
#include "ParticleWall.h"
#include "ParticleDrum.h"
#include <vector>
#include <string>
#include <math.h>
#include "tnt.h"

PFileReader::PFileReader(void)
{
	useGZip = false;
	masterTokens = " \t";
}

PFileReader::~PFileReader(void)
{
}

void PFileReader::setTokens(string newtokens)
{
	masterTokens = newtokens;
}

bool PFileReader::LoadFile(ParticleSet *target, const char filename[MAX_PATH])
{
	// Particle Type; 0 = axes, 1 = sphere, 2 = tablet
    int part_type = 1;

	int env_vars, particles, baffles, frames;
	char* pos;	// for tokenizing, unused
	double currenttime = 0, radius = 5.591, Rband = 5.591e-3, Tband = 3.33e-3, Tcap = 1.03e-3, xlen, ylen;

	if (filename[0]=='\0') return false;

	// buffer the token string into a character array
	char* token = new char[masterTokens.size()+1];
	masterTokens.copy(token, masterTokens.size());
	token[masterTokens.size()] = '\0';
		
	// Read Header Information
    
	vector<double> headerInfo;		// store header parameters in here

	if (!useGZip)
	{
		input.open(filename, ios::in); // open input file
		if (input.fail())
			return false;
	}
	else
	{
		gFile = gzopen(filename, "rb");
	}

	LoadLine();
	env_vars = atoi(buffer);
	while (env_vars > 0)
	{
       LoadLine();
	   char *t;
	   t = strtok(buffer, token);

		if (t!=NULL) // get first token
		{
			env_vars--;
			headerInfo.push_back( strtod(t, &pos) );
		}

		while ( (t = strtok(NULL, token)) != NULL)
		{
			env_vars--; // count all tokens in the line
			headerInfo.push_back( strtod(t, &pos) );
		}
	}

	// save notable header parameters
	if (headerInfo.size() >= 4)
	{
		double startTime = headerInfo[0];
		double endTime = headerInfo[1];
		double timeStep = headerInfo[3];
		target->SetTimes(timeStep, startTime, endTime);
	}

	frames = 0; //= (int)((endTime - startTime) / timeStep);

	LoadLine();
	particles = atoi( strtok(buffer, token) );
	baffles = atoi( strtok(NULL, token) );

	LoadLine(); // contact parameters
	LoadLine(); // contact parameters
	LoadLine(); // contact parameters
	
	// Allocate heap memory for our particle object pointers
	GLParticle ** pbuffer;
	pbuffer = (GLParticle**)malloc((particles + baffles + 1) * sizeof(GLParticle*));

	// Read in inital information

	for (int i = 0; i < particles; i++)
	{
		LoadLine();	// grab initialization string
		Rband = strtod(strtok(buffer, token), &pos);
		Tband = strtod(strtok(NULL, token), &pos);
		Tcap = strtod(strtok(NULL, token), &pos);

		switch (part_type)
		{
			case 0:
			pbuffer[i] = new GLParticle(radius);
			break;

			case 1:
			pbuffer[i] = new ParticleSphere(Rband);
			break;

			case 2:
			if (Tband == 0) Tband = 3.33e-3; // hack
			if (Tcap == 0) Tcap = 1.03e-3;
			pbuffer[i] = new ParticleBiconvex(Tband, Rband, Tcap);
			break;
		}
	}
	pbuffer[0]->PreRender();	// prerender the particle object

	LoadLine();	// I believe the drum dimensions are here: radius/thickness
	double drumRadius = strtod(strtok(buffer, token), &pos);
	double drumThick = strtod(strtok(NULL, token), &pos);
	pos = strtok(NULL, token);
	if (pos != NULL)
	{
		double drumSubRadius = strtod(pos, &pos);
		double drumSubThick = 0;
		drumSubThick = strtod(strtok(NULL, token), &pos);
		pbuffer[particles] = new ParticleDrum(drumRadius, drumThick, drumSubRadius, drumSubThick);
	}
	else
		pbuffer[particles] = new ParticleDrum(drumRadius, drumThick);

	for (int i = 0; i < baffles; i++) // initialize each baffle
	{
		LoadLine();	// grab initialization string 
		xlen = strtod(strtok(buffer, token), &pos);
		ylen = strtod(strtok(NULL, token), &pos);
		pbuffer[i+particles+1] = new ParticleWall(xlen, ylen, 0);
	}

	// drum & baffles don't need to be prerendered

    LoadLine();

	bool eof = (useGZip ? gzeof(gFile) : input.eof());

	// Read in each frame	
	while (!eof)
	{
		currenttime = strtod((buffer+2), &pos); // skip the '*' and pull current time in
		target->PushTime(currenttime);	// push time onto ParticleSet time stack

		LoadLine();
		// Read in all particle information
		for (int j = 0; j < (particles + baffles + 1); j++)
		{
			if (eof)
			{
				return false;	// error reading input
			}
			triple pvalue(0, 0, 0);		// position
			triple rvalue(0, 0, 0);		// rotation (euler angles)
			triple pdvalue(0, 0, 0);	// positional velocity
			triple rdvalue(0, 0, 0);	// rotational velocity

			// use strtod and strtok to strip out values
			pvalue.x = (float)strtod(strtok(buffer, token), &pos);
			pvalue.y = (float)strtod(strtok(NULL, token), &pos);
			pvalue.z = (float)strtod(strtok(NULL, token), &pos);

            rvalue.x = (float)strtod(strtok(NULL, token), &pos);
			rvalue.y = (float)strtod(strtok(NULL, token), &pos);
			rvalue.z = (float)strtod(strtok(NULL, token), &pos);

			pdvalue.x = (float)strtod(strtok(NULL, token), &pos);
			pdvalue.y = (float)strtod(strtok(NULL, token), &pos);
			pdvalue.z = (float)strtod(strtok(NULL, token), &pos);

			rdvalue.x = (float)strtod(strtok(NULL, token), &pos);
			rdvalue.y = (float)strtod(strtok(NULL, token), &pos);
			rdvalue.z = (float)strtod(strtok(NULL, token), &pos);

			// add information into specified particle object
            pbuffer[j]->AddFrame(pvalue, rvalue, pdvalue, rdvalue);

			eof = (useGZip ? gzeof(gFile) : input.eof());
			LoadLine();	// grab next line
		}

		frames++;
		eof = (useGZip ? gzeof(gFile) : input.eof());
	}

	/*for (int i = 0; i < frames; i+=20)
	{
		triple t(0, 1.0 - ((double)(i%255)/255.0), ((double)(i%255)/255.0));
		pbuffer[0]->AddColor(i, t);
	}
	pbuffer[0]->AddColor(frames-1, triple(0, 0, 1));*/

	target->LoadParticles(pbuffer,particles + baffles + 1);

	if (useGZip)
	{
		gzclose(gFile);
	}
	input.close();	// close the file
	input.clear();	// reset all error flags

	//free((void*)pbuffer); // in fact this is freed by ~ParticleSet
	delete[] token;

	DoCoordinateProcessing(target);
	return true;
}

bool PFileReader::LoadColorMap(ParticleSet* target, const char filename[MAX_PATH])
{
	input.open(filename, ios::in); // open input mapfile

	if (input.eof())
		return false;

	// buffer the token string into a character array
	char* token = new char[masterTokens.size()+1];
	masterTokens.copy(token, masterTokens.size());
	token[masterTokens.size()] = '\0';

	char* pos;	// for tokenizing, unused
	double time = 0, timeStart = target->GetStartTime(), timeEnd = target->GetEndTime();
	double timeStep = (timeEnd - timeStart) / (double)(target->GetMaxFrames()-1);
	int pIndex = 0, frame = 0;
	triple rgb;	

	LoadLine();
	while (!input.eof())
	{
		time = strtod(strtok(buffer, token), &pos);
		pIndex = atoi(strtok(NULL, token));
        rgb.x = (double)atoi(strtok(NULL, token)) / 255.0;
		rgb.y = (double)atoi(strtok(NULL, token)) / 255.0;
		rgb.z = (double)atoi(strtok(NULL, token)) / 255.0;

		frame = (int)( (time-timeStart) / timeStep );
        
		if (pIndex < target->GetMaxFrames())
			target->particles[pIndex]->AddColor(frame, rgb);
		LoadLine();
	}
	return true;
}

void PFileReader::LoadLine() // read in a single line of data, skip blank lines
{
	bool eof = false;
	if (!useGZip)
	{
		do
		{
			input.getline(buffer, 512);
			eof = input.eof();
		} while ((buffer[0] == '\r' || buffer[0] == '\n' || buffer[0] == '\0') && (!eof) );
	}
	else
	{
		do
		{
			gzgets(gFile, buffer, 512);
			eof = (bool)gzeof(gFile);
		} while ((buffer[0] == '\r' || buffer[0] == '\n' || buffer[0] == '\0') && (!eof) );
	}
}


using namespace TNT;

void PFileReader::DoCoordinateProcessing(ParticleSet* target)
{
	Array2D<float> matrix(4, 4);
	Array2D<float> vector(4, 1);
	Array2D<float> temp;
	GLParticle* particle;
	float matrixbuffer[16];
	int i = 0;
	triple vectortemp;

	glMatrixMode(GL_MODELVIEW);
	
	for (int p = 0; p < target->GetMaxParticles(); p++)
	{
		particle = target->particles[p];
		for (int f = 0; f < target->GetMaxFrames(); f++)
		{
			glLoadIdentity();
			particle->SetupMatrix(f);
			glGetFloatv(GL_MODELVIEW_MATRIX, matrixbuffer);

			i = 0;
			for (int n = 0; n < 4; n++)
			{
				for (int m = 0; m < 4; m++)
					matrix[m][n] = matrixbuffer[i++];
			}

			vectortemp = particle->peekVector(f, ROTVELOCITY);
			vector[0][0] = vectortemp.x;
			vector[1][0] = vectortemp.y;
			vector[2][0] = vectortemp.z;
			vector[3][0] = 0;

			temp = matmult(matrix, vector);
			vectortemp.x = temp[0][0];
			vectortemp.y = temp[1][0];
			vectortemp.z = temp[2][0];
			particle->setVector(f, ROTVELOCITY, vectortemp);

			vector[0][0] = 0;
			vector[1][0] = particle->getRadius() * 2.0;
			vector[2][0] = 0;
			vector[3][0] = 0;

			temp = matmult(matrix, vector);
			vectortemp.x = temp[0][0];
			vectortemp.y = temp[1][0];
			vectortemp.z = temp[2][0];
			particle->setVector(f, NORMALVEC, vectortemp);
		}
	}
}