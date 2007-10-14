#include "pfilereader.h"
//#include "ParticleSphere.h"
//#include "ParticleBiconvex.h"
//#include "ParticleWall.h"
//#include "ParticleDrum.h"
//#include "ParticleVMixer.h"

#include "ParticleRenderer.h"

#include <fcntl.h>
#include <io.h>

#include <vector>
#include <string>
#include <math.h>
#include "tnt.h"

extern int particle_type;
void StartElement(void *userData, const XML_Char *name, const XML_Char **atts);
void EndElement(void *userData, const XML_Char *name);
void CharHandler(void *userData, const XML_Char *s, int len);
int LookupTag(const char *name);
vector<double> c_values;
string c_Data;
triple c_position, c_rotation;
triple c_color;
double radius1, radius2, height, _length, angle;
triple corners[4];
int currentTag = 0;
int currentIndex = 0;
int count;
bool inverted;

ParticleRenderer* render;

PFileReader* instance = NULL;

PFileReader::PFileReader(void)
{
	masterTokens = " \t";
}

PFileReader::~PFileReader(void)
{
}

void PFileReader::setTokens(string newtokens)
{
	masterTokens = newtokens;
}

// Primary data-parsing function
bool PFileReader::LoadFile(ParticleSet *target, const char filename[MAX_PATH])
{
 
	std::ios_base::sync_with_stdio(false);

	// Particle Type; 0 = axes, 1 = sphere, 2 = tablet
    //int part_type = particle_type;
	int part_type = 0;

	target->ClearParticles();
	rend = target->PRend;
	//rend->FlushLists();

	int env_vars = 0, particles = 0, baffles = 0, frames = 0;
	char* pos;	// for tokenizing, unused
	double currenttime = 0, radius = 5.591, Rband = 5.591e-3, Tband = 3.33e-3, Tcap = 1.03e-3, xlen, ylen;

	if (filename[0]=='\0') return false;

	// buffer the token string into a character array
	char* token = new char[masterTokens.size()+1];
	masterTokens.copy(token, masterTokens.size());
	token[masterTokens.size()] = '\0';
		
	// Read Header Information
    
	vector<double> headerInfo;		// store header parameters in here

	OpenFile(filename);
	if (data==NULL)
	{
		CloseFile();
		return false;
	}

	//if (input.fail())
	//	return false;

	
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
	pos = strtok(buffer, token);
	if (pos != NULL)
		particles = strtol( pos, NULL, 10 );
	pos = strtok(NULL, token);
	if (pos != NULL)
		baffles = strtol( pos, NULL, 10 );

	LoadLine(); // contact parameters
	LoadLine(); // contact parameters
	LoadLine(); // contact parameters
	
	// Allocate heap memory for our particle object pointers
	GLParticle ** pbuffer;
	pbuffer = new GLParticle*[(particles + baffles + 1)];//(GLParticle**)malloc((particles + baffles + 1) * sizeof(GLParticle*));

	// Read in inital information

	int mylist = 0;

	for (int i = 0; i < particles; i++)
	{
		LoadLine();	// grab initialization string
		pos = strtok(buffer, token);
		if (pos != NULL)
			Rband = strtod(pos, NULL);
		pos = strtok(NULL, token);
		if (pos != NULL)
			Tband = strtod(pos, NULL);
		pos = strtok(NULL, token);
		if (pos != NULL)
			Tcap = strtod(pos, NULL);

		switch (part_type)
		{
			case 0:
			pbuffer[i] = new GLParticle(Rband);
			//rend->NewList(i, 1, false);
			//rend->AddSphere(Rband, triple(0,0,0));
			//mylist = rend->EndList();
			//pbuffer[i]->LoadRender(mylist);
			break;

			case 1:
			//pbuffer[i] = new ParticleSphere(Rband);
			break;

			case 2:
			if (Tband == 0) Tband = 3.33e-3; // hax
			if (Tcap == 0) Tcap = 1.03e-3;
			//pbuffer[i] = new ParticleBiconvex(Tband, Rband, Tcap);
			break;
		}
	}

	//pbuffer[0]->PreRender();	// prerender the particle object

	LoadLine();	// Drum dimensions are here: radius, drum length, cone length, cone angle
	double drumRadius = strtod(strtok(buffer, token), &pos);
	double drumThick = strtod(strtok(NULL, token), &pos);
	pos = strtok(NULL, token);
	if (pos != NULL)
	{
		double coneHeight = 0;	coneHeight = strtod(pos, &pos);
		double coneRadius = 0;	coneRadius = strtod(strtok(NULL, token), &pos);
		
		/*pbuffer[particles] = new ParticleDrum(
			drumRadius, drumThick,
			coneRadius, coneHeight
			);*/
		pbuffer[particles] = new GLParticle(drumRadius);
		//pbuffer[particles] = new ParticleVMixer(drumRadius, drumThick,
		//	coneHeight, coneRadius);
	}
	else
		//pbuffer[particles] = new ParticleDrum(drumRadius, drumThick);
		pbuffer[particles] = new GLParticle(drumRadius);
		//pbuffer[particles] = new ParticleVMixer(drumRadius, drumRadius * 3.0, drumRadius * 4.0, 90);



	pbuffer[particles]->canTransparent = false;

	for (int i = 0; i < baffles; i++) // initialize each baffle
	{
		LoadLine();	// grab initialization string 
		xlen = strtod(strtok(buffer, token), &pos);
		ylen = strtod(strtok(NULL, token), &pos);
		//pbuffer[i+particles+1] = new ParticleWall(xlen, ylen, 0);
		pbuffer[i+particles+1] = new GLParticle(ylen);
		pbuffer[i+particles+1]->canTransparent = false;
	}

	//for (int i = 0; i < particles + baffles + 1; i++)
	//	pbuffer[i]->setSize(400);

	// drum & baffles don't need to be prerendered

    LoadLine();

	// Read in each frame	
	while (!Eof())//(datapos < maxpos)
	{
		currenttime = strtod((buffer+2), &pos); // skip the '*' and pull current time in
		target->PushTime(currenttime);	// push time onto ParticleSet time stack

		LoadLine();
		// Read in all particle information

		triple pvalue(0, 0, 0);		// position
		triple rvalue(0, 0, 0);		// rotation (euler angles)
		triple pdvalue(0, 0, 0);	// translational velocity
		triple rdvalue(0, 0, 0);	// rotational velocity

		for (int j = 0; j < (particles + baffles + 1); j++)
		{
			if (Eof())//(datapos > maxpos)
			{
				CloseFile();
				return false;	// error reading input
			}

			// use strtod and strtok to strip out values
			//pos 
			pos = strtok(buffer, token); if (pos != NULL)
				pvalue.x = (float)strtod(pos, NULL);
			pos = strtok(NULL, token); if (pos != NULL)
				pvalue.y = (float)strtod(pos, NULL);
			pos = strtok(NULL, token); if (pos != NULL)
				pvalue.z = (float)strtod(pos, NULL);

			pos = strtok(NULL, token); if (pos != NULL)
				rvalue.x = (float)strtod(pos, NULL);
			pos = strtok(NULL, token); if (pos != NULL)
				rvalue.y = (float)strtod(pos, NULL);
			pos = strtok(NULL, token); if (pos != NULL)
				rvalue.z = (float)strtod(pos, NULL);

			pos = strtok(NULL, token); if (pos != NULL)
				pdvalue.x = (float)strtod(pos, NULL);
			pos = strtok(NULL, token); if (pos != NULL)
				pdvalue.y = (float)strtod(pos, NULL);
			pos = strtok(NULL, token); if (pos != NULL)
				pdvalue.z = (float)strtod(pos, NULL);

			pos = strtok(NULL, token); if (pos != NULL)
				rdvalue.x = (float)strtod(pos, NULL);
			pos = strtok(NULL, token); if (pos != NULL)
				rdvalue.y = (float)strtod(pos, NULL);
			pos = strtok(NULL, token); if (pos != NULL)
				rdvalue.z = (float)strtod(pos, NULL);

			// add information into specified particle object
            pbuffer[j]->AddFrame(pvalue, rvalue, pdvalue, rdvalue);

			LoadLine();	// grab next line
		}

		frames++;
	}

	/*for (int i = 0; i < frames; i+=20)
	{
		triple t(0, 1.0 - ((double)(i%255)/255.0), ((double)(i%255)/255.0));
		pbuffer[0]->AddColor(i, t);
	}
	pbuffer[0]->AddColor(frames-1, triple(0, 0, 1));*/

	//input.close();	// close the file
	//input.clear();	// reset all error flags
	CloseFile();


	target->LoadParticles(pbuffer, particles + baffles + 1);

	//free((void*)pbuffer); // in fact this is freed by ~ParticleSet
	delete[] token;

	DoCoordinateProcessing(target);
	rend->AssignLists(target->particles, target->GetMaxParticles());

	for (int i = 0; i < baffles+1; i++)
	{
		target->particles[i+particles]->AddColor(0, triple(1, 1, 1));
		target->particles[i+particles]->AddColor(target->GetMaxFrames(), triple(1, 1, 1));
	}
	return true;
}

// Load in Color-Map file specifying the base color of the particles over time
bool PFileReader::LoadColorMap(ParticleSet* target, const char filename[MAX_PATH])
{
	//input.open(filename, ios::in); // open input mapfile
	OpenFile(filename);

	if (Eof())//(datapos >= maxpos)
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
	while (!Eof())//(datapos < maxpos)
	{
		pIndex = atoi(strtok(buffer, token));
		time = strtod(strtok(NULL, token), &pos);
        rgb.x = (double)atoi(strtok(NULL, token)) / 255.0;
		rgb.y = (double)atoi(strtok(NULL, token)) / 255.0;
		rgb.z = (double)atoi(strtok(NULL, token)) / 255.0;

		frame = (int)time;//(int)( (time-timeStart) / timeStep );
        
		if (pIndex < target->GetMaxParticles())
			target->particles[pIndex]->AddColor(frame, rgb);
		LoadLine();
	}

	// "cap" off all the particles
	int lastframe = target->GetMaxFrames();
	for (int i = 0; i < target->GetMaxParticles(); i++)
	{
		target->particles[i]->AddColor(lastframe, triple(1, 1, 1));
	}

	CloseFile();
	return true;
}

// Open file for use in LoadFile
void PFileReader::OpenFile(const char filename[MAX_PATH])
{
	//input.open(filename, ios::in); // open input file
	///
    SECURITY_ATTRIBUTES sa;
    memset(&sa, 0, sizeof(sa));
    h_file = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    DWORD FileSize = GetFileSize(h_file, NULL);
	maxpos = (int)FileSize;
    map_file = CreateFileMapping(h_file, NULL, PAGE_READONLY, 0, 0, NULL);
    data = (char*)MapViewOfFile(map_file, FILE_MAP_READ, 0, 0, 0);
	datapos = 0;
	///
}

// Close any open file
void PFileReader::CloseFile()
{
	//input.close();
	///
	CloseHandle(h_file);
	CloseHandle(map_file);	
	UnmapViewOfFile(data);
	///
}

// Basic I/O function, reads in a single line from the memory-mapped file into "buffer"
void PFileReader::LoadLine() // read in a single line of data, skip blank lines
{
	/*do
	{
		input.getline(buffer, 512);
	} while ((!Eof())&&(buffer[0] == '\r' || buffer[0] == '\n' || buffer[0] == '\0'));*/
	static int b;
	b = 0;
	do
	{
		do
		{
			buffer[b++] = data[datapos++];
		} while ((buffer[b-1] != '\n') && (datapos < maxpos) && (b < 511));
		buffer[b] = '\0';
		b = 0;
	} while ((datapos < maxpos) && (buffer[0] == '\r' || buffer[0] == '\n' || buffer[0] == '\0'));
}


using namespace TNT;

// Transform all local coordinates into the world
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
			// steal the GL matrix to use for our own transform
			glLoadIdentity();
			particle->SetupMatrix(f);
			glGetFloatv(GL_MODELVIEW_MATRIX, matrixbuffer);

			i = 0;
			for (int n = 0; n < 4; n++)
			{
				for (int m = 0; m < 4; m++)
					matrix[m][n] = matrixbuffer[i++];
			}

			// transform rotational velocity vector
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

			// transform normals to world coordinates
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

// Primary XML parsing routine
void PFileReader::ParseConfig(ParticleSet* target, const char filename[MAX_PATH])
{
	// Acquire rendering object, clear it of old lists
	rend = target->PRend;
	render = rend;
	rend->FlushLists();
	currentIndex = 0;
	count = 1;
	inverted = false;

	// Create XML parsing object, setup handlers
	XML_Parser XP = XML_ParserCreate(NULL);
	XML_SetElementHandler(XP, StartElement, EndElement);
	XML_SetCharacterDataHandler(XP, CharHandler);
	
	
	// open the XML file
	int xmlfile = _open(filename, O_RDONLY);
	int bytesread = -1;

	// shovel the data into the parser, invoking handlers
	while (bytesread!=0)
	{
		void *buff = XML_GetBuffer(XP, 10384);
		if (buff == NULL) return;
		bytesread = read(xmlfile, buff, 10384);
		XML_ParseBuffer(XP, bytesread, bytesread==0);
	}

	_close(xmlfile);

	// assign the generated display lists to the particle objects
	rend->AssignLists(target->particles, target->GetMaxParticles());
}

// Handler for all XML start tags
void StartElement(void *userData, const XML_Char *name, const XML_Char **atts)
{
	c_Data.clear();
	int tex = 3;
	double att_val;
	int index = LookupTag((char*)name);
	int quality = 0;
	currentTag = index; // lookup tag type
	if (index==100) // for all <particle> tags, initialize default values
	{
		count = 1;
		inverted = false;
		c_color = triple(1, 1, 1);
		c_position = triple(0, 0, 0);
		c_rotation = triple(0, 0, 0);
	}
	if (index <= 4)
	{
		c_position = triple(0, 0, 0);
		c_rotation = triple(0, 0, 0);
	}

	int i = 0;
	for (i = 0; atts[i]; i += 2) // read in all attributes of tag
	{
		index = LookupTag(atts[i]);
		att_val = strtod(atts[i + 1], NULL);
		switch (index)
		{
		case 500:
			count = (int)att_val;
			if (count <= 0) count = 1;
			break;
		case 501:
			{
			string val(atts[i + 1]);
			if (val=="true")
				inverted = true;
			}
			break;
		case 502:
			c_color.x = att_val;
			break;
		case 503:
			c_color.y = att_val;
			break;
		case 504:
			c_color.z = att_val;
			break;
		case 505:
			quality  = (int)att_val;
			break;
		case 506:
			{
			string val(atts[i + 1]);
			if (val=="true")
				tex = TRUE;
			if (val=="false")
				tex = FALSE;
			}
			break;
		}
	}

	if (currentTag==100) // create new list for the <particle> tag
	{
		render->NewList(currentIndex, count, inverted, quality, tex);
		currentIndex+=count;
	}
}

// Handler for all XML ending tags
void EndElement(void *userData, const XML_Char *name)
{
	double c_val = 0;

	int index = LookupTag(name); // lookup tag type
	currentTag = index;

	if (index >= 200) // for any tags with input, convert the accumulated character data into a double
	{
		c_val = strtod(c_Data.c_str(), NULL);
		//c_values.push_back(c_val);
	}

	// now perform the appropriate action for the completed tag
	// either we must parse some value for a higher-level tag, draw a geometry, or finish off a list
	switch (index) 
	{
	case 0: // </sphere>
		render->AddSphere(radius1, c_position);
		break;
	case 1: // </cylinder>
		render->AddCylinder(radius1, _length, c_position, c_rotation);
		break;
	case 2: // </cone>
		render->AddCone(radius1, radius2, angle, c_position, c_rotation);
		break;
	case 3: // </plate>
		render->AddPlate(corners[0], corners[1], corners[2], corners[3], triple(0, 0, 0), triple(0, 0, 0));
		break;
	case 4: // </cap>
		render->AddCap(radius1, height, c_position, c_rotation);
		break;
	case 100: // </particle>
		render->EndList();
		break;
	case 200: // </x>
		c_position.x = c_val;
		break;
	case 201: // </y>
		c_position.y = c_val;
		break;
	case 202: // </z>
		c_position.z = c_val;
		break;
	case 203: // </eta>
		c_rotation.x = c_val;
		break;
	case 204: // </xi>
		c_rotation.y = c_val;
		break;
	case 205: // </lambda>
		c_rotation.z = c_val;
		break;
	case 206: // </radius>
		radius1 = c_val;
		break;
	case 207: // </length>
		_length = c_val;
		break;
	case 208: // </small_radius>
		radius1 = c_val;
		break;
	case 209: // </large_radius>
		radius2 = c_val;
		break;
	case 210: // </half_angle>
		angle = c_val;
		break;
	case 211: // </corner1>
		corners[0] = c_position;
		break;
	case 212: // </corner2>
		corners[1] = c_position;
		break;
	case 213: // </corner3>
		corners[2] = c_position;
		break;
	case 214: // </corner4>
		corners[3] = c_position;
		break;
	case 215: // </height>
		height = c_val;
		break;
	}
}

// convert tag text to an integer index for easy switching
int LookupTag(const char *name)
{
	int index = -1;
	string tagname(name);
	if (tagname == "sphere") index = 0;
	if (tagname == "cylinder") index = 1;
	if (tagname == "cone") index = 2;
	if (tagname == "plate") index = 3;
	if (tagname == "cap") index = 4;

	if (tagname == "particle") index = 100;

	if (tagname == "position") index = 101;
	if (tagname == "rotation") index = 102;

	if (tagname == "x") index = 200;
	if (tagname == "y") index = 201;
	if (tagname == "z") index = 202;
	if (tagname == "eta") index = 203;
	if (tagname == "xi") index = 204;
	if (tagname == "lambda") index = 205;
	if (tagname == "radius") index = 206;
	if (tagname == "length") index = 207;
	if (tagname == "small_radius") index = 208;
	if (tagname == "large_radius") index = 209;
	if (tagname == "half_angle") index = 210;
	if (tagname == "corner1") index = 211;
	if (tagname == "corner2") index = 212;
	if (tagname == "corner3") index = 213;
	if (tagname == "corner4") index = 214;
	if (tagname == "height") index = 215;

	if (tagname == "count") index = 500;
	if (tagname == "inverted") index = 501;
	if (tagname == "r") index = 502;
	if (tagname == "g") index = 503;
	if (tagname == "b") index = 504;
	if (tagname == "quality") index = 505;
	if (tagname == "textures") index = 506;
	return index;
}

// all character data encountered between tags is
// concatenated into a string to be parsed by end-tag handler
void CharHandler(void *userData, const XML_Char *s, int len)
{
	c_Data.append(s, len);
}