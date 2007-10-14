#include "stdafx.h"
#include <fstream>
#include <math.h>
#include "WGLContext.h"
#include "Particle.h"
#include "mrislices.h"

//extern PFNGLTEXIMAGE3DPROC glTexImage3D;
/*extern void readDataFromFile(double& scaleX, double& scaleY, double& scaleZ);
extern unsigned short int arrayData3D[256][256][109];
extern double maxDataValue;*/

MRISlices::MRISlices(void)
{
	max_value = 0;
	height = 0;
	tex3D = 0;
	rendered = false;
	loaded_files = false;
	use_local = false;
	use_local_alpha = false;
	glTexImage3D = NULL;

	tex_array = new unsigned char[256 * 256 * 111 * 4];
	//matrix = TNT::Array2D<float>(4, 4, 0.0);
	for (int x = 0; x < (256 * 256 * 111 * 4); x++)
				tex_array[x] = 0;

	slicegrad = &(Particle::grad);
	origin = triple(0, 0, 0); // centerpoint of object
	range = 0.2;
	max_alpha = 64.0;
	blend_mode = 0;
	num_slices = 256;
}
MRISlices::~MRISlices(void)
{
}

bool MRISlices::RenderTextures()
{
	double local_max;
	if (glTexImage3D == NULL)
		glTexImage3D = (PFNGLTEXIMAGE3DPROC)wglGetProcAddress("glTexImage3D");
	
	if (glTexImage3D == NULL)
		return false;

	double scalex = 1, scaley = 1, scalez = 1;
	if (loaded_files == false)
	{
		//GetFiles();
		readDataFromFile(scalex, scaley, scalez);
		loaded_files = true;
		max_value = maxDataValue;
	}

	for (int i = 0; i < 111; i++)
	{
		local_max = 0.0;
		if ((i != 0) && (i != 110))
			for (int x = 0; x < 256; x++)
			{
				for (int y = 0; y < 256; y++)
				{
					if (arrayData3D[x][y][i] > local_max)
						local_max = arrayData3D[x][y][i-1];
				}
			}
		else
			local_max = 1.0;

		for (int x = 0; x < 256; x++)
		{
			for (int y = 0; y < 256; y++)
			{
				double value;
				int t_offset = ((256 * 256 * 4 * i))+
							((256 * 4) * x) +
							(4 * y);

				if ((i != 0) && (i != 110))
					value = arrayData3D[x][y][i-1];
				else
					value = 0;

				//double value = (double)(x + y + i) / (255.0 + 255.0 + 18.0);
				triple rgb;
				if (use_local)
					rgb = slicegrad->ConvertToColor(value / local_max);
				else
					rgb = slicegrad->ConvertToColor(value / max_value);

				tex_array[t_offset] = (unsigned char)(rgb.x * 255.0);
				tex_array[t_offset+1] = (unsigned char)(rgb.y * 255.0);
				tex_array[t_offset+2] = (unsigned char)(rgb.z * 255.0);

				if (use_local_alpha)
					tex_array[t_offset+3] = (unsigned char)((value / local_max) * max_alpha);
				else
					tex_array[t_offset+3] = (unsigned char)((value / max_value) * max_alpha);

				// Make borders transparent.
				if (x == 0 || y == 0 || x == 255 || y == 255)
					tex_array[t_offset+3] = 0;
			}
		}
	}

	if (!rendered || tex3D <= 0)
		glGenTextures(1, &tex3D);

	glBindTexture(GL_TEXTURE_3D, tex3D);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);

	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, 256, 256, 111, 0, GL_RGBA,
			GL_UNSIGNED_BYTE, tex_array);

	return true;
}

void MRISlices::SetTextureMatrix() // Orient the head properly
{
	glTranslatef(0.5, 0.5, 0.5);
	glRotatef(90, 0, 0, 1);
	//glScalef(5, 5, 10);
	glScalef(5, 5, 8.43253f);
}