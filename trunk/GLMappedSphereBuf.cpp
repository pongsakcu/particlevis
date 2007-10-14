#include "GLMappedSphereBuf.h"
#include <math.h>
#include <memory.h>
#include <algorithm>

#define PI 3.1415926535897932384
#define TWOPI 6.283185307179586476925286766559

GLMappedSphereBuf::GLMappedSphereBuf(void) :
GLVertexBuffer(1)
{
	use_tex = true;
	use_vbo = false;
	//GenerateSphere(10);
	normal_buffer = 0;
}

GLMappedSphereBuf::~GLMappedSphereBuf(void)
{
}

void GLMappedSphereBuf::GenerateSphere(int res)
{
	SetCapacity(res*(res+2)*2); // allocate memory in the bufer
	std::fill(indices, indices + res*(res+2)*2, 0);
	if (use_vbo && GLEW_VERSION_1_5)
		glGenBuffers(1, &normal_buffer);
	// generate sphere vertices
	int index = 0, i = 0, j = 0, k = 0, s = 10;	
	double steps = (double)res;
	double phi = 0, theta = 0, step_phi = (PI / steps), step_theta = (TWOPI / steps);
	double offset_phi = (PI/steps) / 2.0;
	double offset_theta = (TWOPI/steps) / 2.0; 

	triple vertex;

	for (i = 0; i < res; i++)
	{		
		phi = 0;
		for (j = -1; j <= res; j++)
		{
			if (j==-1)
			{
				AddVertex( triple(0.0, 1.0, 0.0), triple(1, 1, 1));
			}
			else if (j==res)
			{
				AddVertex( triple(0.0, -1.0, 0.0), triple(1, 1, 1));
			}
			else
			{
				triple vertex;
				vertex.x = 1.0*sin(phi+offset_phi) * cos(theta+offset_theta);	// x
				vertex.y = 1.0*cos(phi+offset_phi);                         	// y
				vertex.z = 1.0*sin(phi+offset_phi) * sin(theta+offset_theta);	// z
				AddVertex(vertex, triple(1, 1, 1));
				phi += step_phi;
			}
		}
		theta += step_theta;
	}

	// fill indices
	//unsigned int* new_order = new unsigned int[res*(res+2)*2];

	index = 0;
	for (j = 0; j < res - 1; j++)
	{
		for (k = 0; k < res+2; k++)
		{
			//if (((j+1)*(res+2) + k) >= 120)
			//	int crap = 0;
			indices[ index++ ] = (j*(res+2)) + k;
			indices[ index++ ] = ((j+1)*(res+2)) + k;			
		}		
	}
	for (k = 0; k < res+2; k++)
	{
		indices[ index++ ] = ((res-1)*(res+2)) + k;
		indices[ index++ ] = k;		
	}

	this->index = index;
}

void GLMappedSphereBuf::Bind()
{
	use_tex = false;
	GLVertexBuffer::Bind();
	use_tex = true;
	if (use_vbo && GLEW_VERSION_1_5)
	{
		glBindBuffer(GL_ARRAY_BUFFER, normal_buffer);
		glBufferData(GL_ARRAY_BUFFER, 3*index*sizeof(GLfloat), vertices, GL_STREAM_DRAW);
		glTexCoordPointer(tex_dim, GL_FLOAT, 0, 0);
	}
	else
	{
		glNormalPointer(GL_FLOAT, 0, vertices);
	}

}

void GLMappedSphereBuf::SetMapPoints(float * points, int count)
{
	if (count > max_index)
		return;
	// copy in the points, bind them
	if (use_vbo && GLEW_VERSION_1_5)
	{
		glBindBuffer(GL_ARRAY_BUFFER, buffer_id[VB_TEXCOORDS]);
		glBufferData(GL_ARRAY_BUFFER, count*sizeof(GLfloat), points, GL_STREAM_DRAW);
	}
	else
		memcpy(tex, points, sizeof(float)*count);
		//tex = points;
}

void GLMappedSphereBuf::BindTex()
{
	if (use_vbo && GLEW_VERSION_1_5)
	{
		glBindBuffer(GL_ARRAY_BUFFER, buffer_id[VB_TEXCOORDS]);
		glBufferData(GL_ARRAY_BUFFER, tex_dim*max_index*sizeof(GLfloat), tex, GL_STREAM_DRAW);
	}
	else
	{
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(tex_dim, GL_FLOAT, 0, tex);
	}
}

bool GLMappedSphereBuf::Draw()
{
	//if (latched)
		//index = last_index;
	if (index <= 0)
		return false;	

	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
			
	if (use_vbo && GLEW_VERSION_1_5)
	{
		glBindBuffer(GL_ARRAY_BUFFER, buffer_id[VB_VERTICES]);
		glVertexPointer(3, GL_FLOAT, 0, 0);

		glBindBuffer(GL_ARRAY_BUFFER, normal_buffer);
		glNormalPointer(GL_FLOAT, 0, 0);

		if (use_tex)
		{
			glBindBuffer(GL_ARRAY_BUFFER, buffer_id[VB_TEXCOORDS]);
			glTexCoordPointer(tex_dim, GL_FLOAT, 0, 0);
		}

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer_id[VB_INDICES]);
		glDrawRangeElements(GL_TRIANGLE_STRIP, 0, max_index, max_index, GL_UNSIGNED_INT, 0);
	}
	else
	{
		glVertexPointer(3, GL_FLOAT, 0, vertices);
		glNormalPointer(GL_FLOAT, 0, vertices);		
		glTexCoordPointer(tex_dim, GL_FLOAT, 0, tex);
		//glIndexPointer(GL_UNSIGNED_INT, 0, indices);
		//glDrawArrays(GL_TRIANGLE_STRIP, 0, max_index);
		glDrawElements(GL_TRIANGLE_STRIP, max_index, GL_UNSIGNED_INT, indices);
	}

	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	return true;
}