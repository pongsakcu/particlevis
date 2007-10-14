#include "gllinebuffer.h"
#include <stdlib.h>

GLLineBuffer::GLLineBuffer(void) :
GLVertexBuffer(0)
{
}

GLLineBuffer::~GLLineBuffer(void)
{
}

bool GLLineBuffer::SetCapacity(int size)
{
	if (max_index != size)
	{
		delete[] vertices; vertices = NULL;
		delete[] color; color = NULL;
		delete[] indices; indices = NULL;

		try
		{
			vertices = new GLfloat[3 * size];
			color = new GLubyte[4 * size];
			indices = new GLuint[size];
		}
		catch (...)
		{	
			delete[] vertices;
			delete[] color;
			delete[] indices;
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
			indices[i] = i;
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

bool GLLineBuffer::Draw()
{
	//if (latched)
		//index = last_index;

	if (index%2 != 0)
		index--;
	if (index <= 0)
		return false;
	
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	if (use_vbo && GLEW_VERSION_1_5)
	{
		glBindBuffer(GL_ARRAY_BUFFER, buffer_id[VB_VERTICES]);
		glVertexPointer(3, GL_FLOAT, 0, 0);

		glBindBuffer(GL_ARRAY_BUFFER, buffer_id[VB_COLORS]);
		glColorPointer(4, GL_UNSIGNED_BYTE, 0, 0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer_id[VB_INDICES]);
		//glIndexPointer(GL_UNSIGNED_INT, 0, 0);
		glDrawRangeElements(GL_LINES, 0, max_index-1, index, GL_UNSIGNED_INT, 0);
		//glDrawArrays(GL_LINES, 0, index);
	}
	else
	{
		//glBindBuffer(GL_ARRAY_BUFFER, 0);
		glVertexPointer(3, GL_FLOAT, 0, vertices);
		glColorPointer(4, GL_UNSIGNED_BYTE, 0, color);

		//glIndexPointer(GL_UNSIGNED_INT, 0, indices);
		//glDrawArrays(GL_LINES, 0, index);
		if (!permuted_indices)
		{
			//glIndexPointer(GL_UNSIGNED_INT, 0, indices);
			glDrawArrays(GL_LINES, 0, index);
		}
		else
		{
			//glIndexPointer(GL_UNSIGNED_INT, 0, sorted_indices);
			glDrawArrays(GL_LINES, 0, index);
			//glDrawElements(GL_LINES, index, GL_UNSIGNED_INT, indices);
			permuted_indices = false;
		}
	}

	//last_index = index;
	//if (!latched)
	//	index = 0;

	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	return true;
}