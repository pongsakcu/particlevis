#include "GLVertexBuffer.h"
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <algorithm>

#define SQR(x) ((x) * (x))
#define min(a,b)   (((a) < (b)) ? (a) : (b))

GLVertexBuffer::GLVertexBuffer(const int texture_dimensions) :
tex_dim(texture_dimensions),
latched(false),
max_index(0),
index(0),
permuted_indices(false)
{
	buffer_id[0] = buffer_id[1] = buffer_id[2] = buffer_id[3] = -1;
	vertices = att = tex = NULL;
	distances = NULL;
	color = NULL;
	indices = sorted_indices = NULL;
	use_vbo = true;
	use_att = use_tex = false;	
}

GLVertexBuffer::~GLVertexBuffer(void)
{
	delete[] vertices;
	delete[] color;
	delete[] att;
	delete[] tex;
	delete[] indices;
	vertices = att = tex = NULL;
	color = NULL;
	indices = NULL;
}

GLVertexBuffer::GLVertexBuffer(const GLVertexBuffer& rhs) :
tex_dim(rhs.tex_dim)
{
	memcpy(this, (const void*)&rhs, sizeof(GLVertexBuffer));
	vertices = att = tex = NULL;
	color = NULL;
	indices = NULL;
	SetCapacity(rhs.max_index);
	memcpy(vertices, rhs.vertices, sizeof(float)*3*max_index);
	memcpy(color, rhs.color, sizeof(unsigned char)*4*max_index);
	memcpy(att, rhs.att, sizeof(float)*max_index);
	memcpy(tex, rhs.tex, sizeof(float)*tex_dim*max_index);
	memcpy(indices, rhs.indices, sizeof(unsigned int)*max_index);
}

GLVertexBuffer& GLVertexBuffer::operator=(const GLVertexBuffer& rhs)
{
	memcpy(this, (const void*)&rhs, sizeof(GLVertexBuffer));
	vertices = att = tex = NULL;
	color = NULL;
	indices = NULL;
	SetCapacity(rhs.max_index);
	memcpy(vertices, rhs.vertices, sizeof(float)*3*max_index);
	memcpy(color, rhs.color, sizeof(unsigned char)*4*max_index);
	memcpy(att, rhs.att, sizeof(float)*max_index);
	memcpy(tex, rhs.tex, sizeof(float)*tex_dim*max_index);
	memcpy(indices, rhs.indices, sizeof(unsigned int)*max_index);
	return *this;
}

void GLVertexBuffer::AddVertexSet(const triple* P, const quad* C, int length)
{
	memcpy(vertices, P, sizeof(float)*3*length);
	memcpy(color, C, sizeof(float)*4*length);
	index += length;
}

void GLVertexBuffer::EnableAttribute(unsigned int attribute_id)
{
	if (GLEW_VERSION_2_0)
	{
		use_att = true;
		att_id = attribute_id;
	}
}
void GLVertexBuffer::DisableAttribute()
{
	use_att = false;
}

void GLVertexBuffer::EnableTexCoords()
{
	use_tex = true;
}
void GLVertexBuffer::DisableTexCoords()
{
	use_tex = false;
}

bool GLVertexBuffer::SetCapacity(int size)
{
	if (max_index != size)
	{
		delete[] vertices; vertices = NULL;
		delete[] distances; distances = NULL;
		delete[] color; color = NULL;
		delete[] att; att = NULL;
		delete[] tex; tex = NULL;
		delete[] indices; indices = NULL;
		delete[] sorted_indices; sorted_indices = NULL;

		try
		{
			vertices = new GLfloat[3 * size];
			distances = new GLfloat[size];
			color = new GLubyte[4 * size];
			att = new GLfloat[size];
			tex = new GLfloat[tex_dim * size];
			indices = new GLuint[size];
			sorted_indices = new GLuint[size];
		}
		catch (...)
		{	
			delete[] vertices;
			delete[] distances;
			delete[] color;
			delete[] att;
			delete[] tex;
			delete[] indices;
			delete[] sorted_indices;
			max_index = 0;
			return false;
		}

		for (int i = 0; i < size; i++)
		{
			vertices[(i*3)+0] = 0;
			vertices[(i*3)+1] = 0;
			vertices[(i*3)+2] = 0;
			color[(i*4)+0] = 0;
			color[(i*4)+1] = 0;
			color[(i*4)+2] = 0;
			color[(i*4)+3] = 0;
			att[i] = 0;
			indices[i] = i;
			sorted_indices[i] = i;
			memset(&tex[i*tex_dim], 0, tex_dim*sizeof(GLfloat));
		}
		
		max_index = size;
		
		if (use_vbo && GLEW_VERSION_1_5)
		{
			glDeleteBuffers(5, buffer_id);
			glGenBuffers(5, buffer_id);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer_id[VB_INDICES]);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, size*sizeof(GLuint), indices, GL_STATIC_DRAW);

			if (glGetError() == GL_OUT_OF_MEMORY)	
			{
				max_index = size = 0;
				return false;		
			}
		}
	}
	index = 0;
	return true;
}

int GLVertexBuffer::GetCapacity()
{
	return max_index;
}

int GLVertexBuffer::GetSize()
{
	return index;
}

void GLVertexBuffer::AttachAttribute(float attribute)
{
	//if (i < max_index)
	att[index-1] = attribute;
}

void GLVertexBuffer::AttachTexCoord(float s, float t, float u)
{
	//if (i < max_index)
	int i = index - 1;	
	{
		tex[i*tex_dim + 0] = s;		
		if (tex_dim > 1)
			tex[i*tex_dim + 1] = t;		
		if (tex_dim > 2)
			tex[i*tex_dim + 2] = u;
	}
}

void GLVertexBuffer::ResetColor(int index, triple C, double alpha)
{
	if (index < max_index)
	{
		color[(index*4)+0] = (unsigned char)(C.x*255);
		color[(index*4)+1] = (unsigned char)(C.y*255);
		color[(index*4)+2] = (unsigned char)(C.z*255);
		color[(index*4)+3] = (unsigned char)(alpha*255);
	}
}

void GLVertexBuffer::BindAtt()
{
	//if (index == 0) return;
	if (GLEW_VERSION_2_0)
	{
		if (use_vbo)
		{
			glBindBuffer(GL_ARRAY_BUFFER, buffer_id[2]);
			glBufferData(GL_ARRAY_BUFFER, max_index*sizeof(GLfloat), att, GL_STREAM_DRAW);		
		}
		else
			glVertexAttribPointer(att_id, 1, GL_FLOAT, GL_FALSE, 0, att);
	}
}

void GLVertexBuffer::BindTex()
{
	if (use_vbo && GLEW_VERSION_1_5)
	{
		glBindBuffer(GL_ARRAY_BUFFER, buffer_id[3]);
		glTexCoordPointer(tex_dim, GL_FLOAT, 0, 0);
		glBufferData(GL_ARRAY_BUFFER, tex_dim*max_index*sizeof(GLfloat), tex, GL_STREAM_DRAW);
	}
	else
		glTexCoordPointer(tex_dim, GL_FLOAT, 0, tex);
}

void GLVertexBuffer::Bind()
{
	if (index == 0) return;
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	if (use_vbo && GLEW_VERSION_1_5)
	{
		if (!permuted_indices)
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer_id[VB_INDICES]);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, index*sizeof(GLuint), indices, GL_STATIC_DRAW);
		}

		glBindBuffer(GL_ARRAY_BUFFER, buffer_id[0]);
		glBufferData(GL_ARRAY_BUFFER, 3*index*sizeof(GLfloat), vertices, GL_STREAM_DRAW);
		glVertexPointer(3, GL_FLOAT, 0, 0);

		glBindBuffer(GL_ARRAY_BUFFER, buffer_id[1]);
		glBufferData(GL_ARRAY_BUFFER, 4*index*sizeof(GLbyte), color, GL_STREAM_DRAW);
		glColorPointer(4, GL_FLOAT, 0, 0);
	}
	if (use_att)
	{	glEnableVertexAttribArray(att_id);	
		BindAtt();
	}
	if (use_tex)
	{
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		BindTex();
	}
}

bool GLVertexBuffer::Draw()
{
	if (index <= 0)
		return false;

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	if (use_tex) glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	else glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	if (use_vbo && GLEW_VERSION_1_5)
	{
		if (GLEW_VERSION_2_0)
		{
			if (use_att) glEnableVertexAttribArray(att_id);	
			else glDisableVertexAttribArray(att_id);
		}

		glBindBuffer(GL_ARRAY_BUFFER, buffer_id[VB_VERTICES]);
		glVertexPointer(3, GL_FLOAT, 0, 0);

		glBindBuffer(GL_ARRAY_BUFFER, buffer_id[VB_COLORS]);
		glColorPointer(4, GL_UNSIGNED_BYTE, 0, 0);

		if (use_tex)
		{
			glBindBuffer(GL_ARRAY_BUFFER, buffer_id[VB_TEXCOORDS]);
			glTexCoordPointer(tex_dim, GL_FLOAT, 0, 0);
		}

		if (GLEW_VERSION_2_0 && use_att)
		{
			glBindBuffer(GL_ARRAY_BUFFER, buffer_id[VB_ATTRIBUTES]);
			glVertexAttribPointer(att_id, 1, GL_FLOAT, GL_FALSE, 0, 0);
		}

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer_id[VB_INDICES]);
		glDrawRangeElements(GL_POINTS, 0, max_index, index, GL_UNSIGNED_INT, 0);
	}
	else
	{
		glVertexPointer(3, GL_FLOAT, 0, vertices);
		glColorPointer(4, GL_UNSIGNED_BYTE, 0, color);
		
		if (use_att) glVertexAttribPointer(att_id, 1, GL_FLOAT, GL_FALSE, 0, att);
		if (use_tex) glTexCoordPointer(tex_dim, GL_FLOAT, 0, tex);
		
		if (!permuted_indices)
			glDrawArrays(GL_POINTS, 0, max_index);		
		else
		{
			glDrawArrays(GL_POINTS, 0, max_index);
			permuted_indices = false;
		}
	}

	if (use_att) glDisableVertexAttribArray(att_id);
	if (use_tex) glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_INDEX_ARRAY);
	return true;
}

float * dist;
inline bool comparator(const unsigned int a, const unsigned int b)
{
	// sort in reverse order.
	if (dist[a] > dist[b])
		return true;
	else
		return false;
}

void GLVertexBuffer::SortByDist(const triple origin, const triple direction)
{	
	for (int i = 0; i < index; i++)
	{
		float square_dist =
			(vertices[i*3 + 0] - origin.x)*direction.x + 
			(vertices[i*3 + 1] - origin.y)*direction.y +
			(vertices[i*3 + 2] - origin.z)*direction.z;

		distances[i] = square_dist;
		sorted_indices[i] = i;
	}
	dist = distances;
	std::sort(sorted_indices, sorted_indices+index, comparator);

	if (GLEW_VERSION_1_5 && use_vbo)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer_id[VB_INDICES]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, index*sizeof(GLuint), sorted_indices, GL_STREAM_DRAW);
	}
	permuted_indices = true;
}