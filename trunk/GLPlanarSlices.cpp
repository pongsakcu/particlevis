#include "stdafx.h"
#include "GLPlanarSlices.h"
#include <math.h>
#include <string>
#include <algorithm>
#include <fstream>
#include "WGLContext.h"
#include "Particle.h"

using namespace std;

// Implementation of GLPlanarSlices follows.
// TODO: Fix singularity at top/bottom

#define CUTOFF 8

GLPlanarSlices::GLPlanarSlices(int x, int y, int z) :
XMAX(x),
YMAX(y),
ZMAX(z),
tex_array(NULL),
rendered(false),
max_value(0),
height(0),
tex3D(0),
use_local(false),
use_local_alpha(false),
valuecache(NULL)
{
	valuecache = NULL;
	tex_array = NULL;
	this->SetSize(x, y, z);
	slicegrad = &(Particle::grad);

	origin = triple(0, 0, 0);
	rotation = triple(0, 0, 0);
	scale = triple(1, 1, 1);	
	range = 1.0;
	max_alpha = 128.0;
	blend_mode = 0;

	num_slices = 128;
	enable = false;
	use_dummy = true;
}

GLPlanarSlices::~GLPlanarSlices(void)
{
}

void GLPlanarSlices::SetSize(int x, int y, int z)
{
	XMAX = x;
	YMAX = y;
	ZMAX = z;
	delete[] tex_array;
	tex_array = new unsigned char[XMAX * YMAX * ZMAX * 4];
	fill(tex_array, tex_array+(XMAX * YMAX * ZMAX * 4), 0);
	delete[] valuecache;
	valuecache = new float[XMAX * YMAX * ZMAX];
	fill(valuecache, valuecache + (XMAX * YMAX * ZMAX), 0.f);
}

// Render the file data to a texture
bool GLPlanarSlices::RenderTextures()
{
	double max = 1.0;

	if (!GLEW_EXT_texture3D || !GLEW_VERSION_1_2)
		return false;

	for (int z = 0; z < ZMAX; z++)
	{
		if (use_local)
		{
		max = 0.0;
		//if (i > 0 && i < (filenames.size()+1))
		{
			for (int x = 0; x < XMAX; x++)
			{
				for (int y = 0; y < YMAX; y++)
				{
					int v_offset = (XMAX * YMAX * z) +
							(XMAX * y) +
							 x;

					if (valuecache[v_offset] > max)
						max = valuecache[v_offset];
				}
			}
		}
		//else
		//	max = 1.0;
		}
		else
			max = max_value;		

		// Step through the loaded data and classify it
		// Store to the tex_array
		for (int x = 0; x < XMAX; x++)
		{
			for (int y = 0; y < YMAX; y++)
			{
				double value;
				// calculate indices, row-major format.  t = texture array, v = value array (shift by 1)
				int t_offset = ((XMAX * YMAX * 4 * z))+
							((XMAX * 4) * y) +
							(4 * x);

				int v_offset = (XMAX * YMAX * z)+
							(XMAX * y) +
							 x;

				//if ((i > 0) && (i < (filenames.size() + 1)))
				//if ((i > 0) && (i < (ZMAX-1)))
				value = (valuecache[v_offset]);
				//else
				//	value = 0;

				triple rgb;

				if (use_dummy) // override with cube texture
				{
					float x_norm = (2.0*x / (float)XMAX) - 1.0;
					float y_norm = (2.0*y / (float)YMAX) - 1.0;
					float z_norm = (2.0*z / (float)ZMAX) - 1.0;
					value = max(0, 1.333 - sqrt(x_norm*x_norm + y_norm*y_norm + z_norm*z_norm));
					max = 1.333;
				}

				rgb = slicegrad->ConvertToColor(value / max);

				tex_array[t_offset] = (unsigned char)(rgb.x * 255.0);
				tex_array[t_offset+1] = (unsigned char)(rgb.y * 255.0);
				tex_array[t_offset+2] = (unsigned char)(rgb.z * 255.0);
				if (use_local_alpha)
					tex_array[t_offset+3] = (unsigned char)((value / max) * max_alpha);
				else
					tex_array[t_offset+3] = (unsigned char)((value / max_value) * max_alpha);

				if (use_dummy) // override with cube texture
				{
					float x_norm = (2.0*x / (float)XMAX) - 1.0;
					float y_norm = (2.0*y / (float)YMAX) - 1.0;
					float z_norm = (2.0*z / (float)ZMAX) - 1.0;
					float radius = fmod(max(0, 1.8 - sqrt(x_norm*x_norm + y_norm*y_norm + z_norm*z_norm)), 1.0);
					float con = pow(fmod(radius, 0.333f) / 0.333, 4.0);
					tex_array[t_offset+3] = (unsigned char)(z > ZMAX*0.5 && x > XMAX*0.5) ? 0 : max_alpha*con;
				}

				// Make borders transparent.
				if (x == 0 || y == 0 || z == 0 || x == XMAX-1 || y == YMAX-1 || z == ZMAX-1)
					tex_array[t_offset+3] = 0;
			}
		}
	}

	// Allocate texture identifier
	if (!rendered || tex3D <= 0)
		glGenTextures(1, &tex3D);
	
	if (GLEW_EXT_texture3D && GLEW_VERSION_1_2)
	{
		// Setup 3D texturing
		glBindTexture(GL_TEXTURE_3D, tex3D);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);

		// Load tex_array into texture memory
		glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, XMAX, YMAX, ZMAX, 0, GL_RGBA,
					GL_UNSIGNED_BYTE, tex_array);		
		return true;
	}
	else
		return false;
}

// Render the view-aligned slices
void GLPlanarSlices::Draw(Camera * cam)
{
	static double coords[6];
	static triple view;

	if (!GLEW_EXT_texture3D || !GLEW_VERSION_1_2)
		return;

	if (!enable)
		return;

	if (!rendered)
		rendered = RenderTextures();	

	cam->getCamera(coords);	
	view = triple(coords[3], coords[4], coords[5]) - triple(coords[X], coords[Y], coords[Z]); // reverse viewing vector

	triple up = cam->getUpVector();
	
	if (view != up && (view*-1 != up))
	{
		view_x = crossProduct(up, view);		
		view_y = crossProduct(view_x, view);
		view_z = view;
	}
	else
	{
		view_x = crossProduct(up + triple(0.1, 0.1, 0.1), view);
		view_x = crossProduct(view_x, view);
		view_z = view;
	}

	normalize(view_x);
	normalize(view_y);
	normalize(view_z);

	glEnable(GL_TEXTURE_3D);	
	if (blend_mode == BLEND_OVER)
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	if (blend_mode == BLEND_ADD)
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	triple p;
	glBindTexture(GL_TEXTURE_3D, tex3D);
	
	double istep = (range*2.0) / num_slices;
	glColor4d(1, 1, 1, 1);
		
	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	SetTextureMatrix();
	glBegin(GL_QUADS);
	for (int i = 0; i < num_slices; i++)
	{
		SlicePoint(-range, -range, range-i*istep);
		SlicePoint(range, -range, range-i*istep);
		SlicePoint(range, range, range-i*istep);
		SlicePoint(-range, range, range-i*istep);
	}
	glEnd();
	glPopMatrix();
	glDisable(GL_TEXTURE_3D);
}

void GLPlanarSlices::SetTextureMatrix() // rotate/scale the texture dataset
{
	glTranslatef(0.5, 0.5, 0.5);
	glScalef(1.0/scale.x, 1.0/scale.y, 1.0/scale.z);		
	glRotatef(rotation.z, 0, 0, 1);
	glRotatef(rotation.y, 0, 1, 0);	
	glRotatef(rotation.x, 1, 0, 0);	
	
	//glScalef(5.667, 5.667, 5.0373); // head?	
}

// Issue a single point/texture coordinate
void GLPlanarSlices::SlicePoint(float x, float y, float z)
{
	static triple v, t;
	t = (view_x * x) + (view_y * y) + (view_z * z);
	v = t + origin;
	glTexCoord3f(t.x, t.y, t.z);
	glVertex3f(v.x, v.y, v.z);
}

void GLPlanarSlices::SetBlending(int bmode)
{
	if (bmode == BLEND_ADD || bmode == BLEND_OVER)
		blend_mode = bmode;
}

void GLPlanarSlices::SetNumSlices(int nslice)
{
	num_slices = nslice;
}

bool GLPlanarSlices::LoadVOLFile(string filename)
{
	ifstream volfile;
	if (filename.length() == 0) return false;
	volfile.open(filename.c_str(), ios::in | ios::binary);
	if (volfile.fail())
		return false;

	string comment;
	char buffer[255];
	float size_x, size_y, size_z, pos_x, pos_y, pos_z;
	int format, endian;

	getline(volfile, comment, '\n');
	volfile >> XMAX >> YMAX >> ZMAX;
	if (XMAX <= 0 || YMAX <= 0 || ZMAX <= 0)
		return false;
	
	volfile >> size_x >> size_y >> size_z;
	volfile >> pos_x >> pos_y >> pos_z;
	volfile >> format >> endian;
	if (volfile.fail()) return false;

	SetSize(XMAX, YMAX, ZMAX);
	rendered = false;
	char bytebuffer[4];
	fill(bytebuffer, bytebuffer+4, 0);

	volfile.getline(buffer, 255, '\n');
	max_value = 0.0;
	int i = 0;
	for (int z = 0; z < ZMAX; z++)    
		for (int y = 0; y < YMAX; y++)
			for (int x = 0; x < XMAX; x++)
			{
				int offset = (XMAX * YMAX * z) +
							(XMAX * y) +
							 x;
				float data;
				if (format == 8)  // 1 byte unsigned integer
					data = (float)volfile.get(); 
				else 
				{
					if (format == 16) // 2 byte unsigned integer
					{
						volfile.read(bytebuffer, 2);
						if (endian == 0)
							swap((unsigned char*)(bytebuffer), 2);
						data = (float)(*(unsigned int*)bytebuffer);
					}
					else if (format == 32) // 4 byte unsigned integer
					{
						volfile.read(bytebuffer, 4);
						if (endian == 0)
							swap((unsigned char*)(bytebuffer), 4);
						data = (float)(*(unsigned int*)bytebuffer);
					}
					else if (format == -32) // single precision float
					{
						volfile.read(bytebuffer, 4);
						if (endian == 0)
							swap((unsigned char*)(bytebuffer), 4);
						data = *((float*)bytebuffer);
					}					
				}
				valuecache[offset] = data;
				if (data != 0) i++;
				if (data > max_value)
					max_value = data;
			}	
	scale.x = XMAX * size_x;
	scale.y = YMAX * size_y;
	scale.z = ZMAX * size_z;
	rotation = triple(0, 0, 0);
	range = max(max(scale.x, scale.y), scale.z) * 0.75;
	origin = (scale / 2.0) + triple(pos_x, pos_y, pos_z);
	if (max_value == 0.0) return false;	
	use_dummy = false;
	return true;	
}

void GLPlanarSlices::swap(unsigned char* bytes, int length)
{
	unsigned char temp;
	unsigned char* ptr1 = bytes, *ptr2 = bytes+length-1;
	while (ptr1 < ptr2)
	{
		temp = *ptr1;
		*ptr1 = *ptr2;
		*ptr2 = temp;
		ptr1++; ptr2--;
	}
}

void GLPlanarSlices::ComputeDensityMap(ParticleStore * store, int size)
{
	float scaling = Particle::maxPosition * 2;
	SetSize(size, size, size);
	rendered = false;
	use_dummy = false;
	scale = triple(scaling, scaling, scaling);
	range = scaling * 0.75;
	origin = triple(0, 0, 0);
	triple scale_offset = scale / 2.0;
	int frames = store->GetFrameCount();

	for (int f = 0; f < frames; f++)
	{
		triple * pos = store->GetFrame(f).pos;
		for (int p = 0; p < store->GetParticleCount(); p++)
		{
			triple location = *pos + scale_offset;
			int x, y, z, v_offset;
			x = (int)((float)size*(location.x / scale.x)+0.5);
			y = (int)((float)size*(location.y / scale.y)+0.5);
			z = (int)((float)size*(location.z / scale.z)+0.5);
			if (x > 0 && y > 0 && z > 0 && x < XMAX && y < YMAX && z < ZMAX)
			{
				v_offset = (XMAX * YMAX * z) +
								(XMAX * y) +
							x;
				valuecache[v_offset]++;
			}
			pos++;
		}
	}

	max_value = 0;
	float *values = valuecache;
	for (int i = 0; i < XMAX * YMAX * ZMAX; i++)
	{
		if (*values > max_value)
			max_value = *values;	
		values++;
	}
}

void GLPlanarSlices::ComputeGranularTemp(ParticleStore * store, int size)
{
	float scaling = Particle::maxPosition * 2;
	SetSize(size, size, size);
	rendered = false;
	use_dummy = false;
	scale = triple(scaling, scaling, scaling);
	range = scaling * 0.75;
	origin = triple(0, 0, 0);
	triple scale_offset = scale / 2.0;
	int frames = store->GetFrameCount();

	triple* cell_average = new triple[XMAX * YMAX * ZMAX];
	int* cell_count = new int[XMAX * YMAX * ZMAX];
	fill(cell_count, cell_count + XMAX * YMAX * ZMAX, 0);

	for (int f = 0; f < frames; f++)
	{
		int x, y, z, v_offset;
		triple *pos = store->GetFrame(f).pos, *d_pos = store->GetFrame(f).d_pos;

		for (int p = 0; p < store->GetParticleCount(); p++)
		{			
			triple location = *pos + scale_offset;
			x = (int)((float)size*(location.x / scale.x)+0.5);
			y = (int)((float)size*(location.y / scale.y)+0.5);
			z = (int)((float)size*(location.z / scale.z)+0.5);
			if (x > 0 && y > 0 && z > 0 && x < XMAX && y < YMAX && z < ZMAX)
			{
				v_offset =	(XMAX * YMAX * z) +
							(XMAX * y) +
							 x;
				cell_average[v_offset]+=*d_pos;
				cell_count[v_offset]++;
			}
			pos++; d_pos++;
		}			
	}

	max_value = 0;
	float *values = valuecache;	
	triple *avg = cell_average;
	int *count = cell_count;
	for (int i = 0; i < XMAX * YMAX * ZMAX; i++)
	{
		if (*count > CUTOFF)
		{
			float value = avg->Length() / (float)*count;
			if (value > max_value)
				max_value = value;
			*values = value;
		}
		else
			*values = 0;
		values++; avg++; count++;

	}
	delete[] cell_average;
	delete[] cell_count;
}
