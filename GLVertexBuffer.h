/*
GLVertexBuffer

		   An OpenGL utlity class that encapsulates a buffer of vertices, which can
		be drawn using vertex buffer objects or vertex arrays.  This base class issues
		point primitives.  Various subclasses can be used for other specific tasks, such
		as drawing lines or triangles.

		Vincent Hoon, 2007
		See copyright notice in copyright.txt for conditions of distribution and use.
*/

#pragma once
#include "triple.h"
#include "quad.h"
#include <gl/glew.h>

#define VB_VERTICES 0
#define VB_COLORS 1
#define VB_ATTRIBUTES 2
#define VB_TEXCOORDS 3
#define VB_INDICES 4

class GLVertexBuffer
{
public:	
	GLVertexBuffer(const int texture_dimensions = 3);	
	~GLVertexBuffer(void);

	GLVertexBuffer(const GLVertexBuffer& copy);
	GLVertexBuffer& operator=(const GLVertexBuffer& rhs);

	void UseVBOs(bool vbo) { use_vbo = vbo; }

	void EnableAttribute(unsigned int attribute_id);
	void DisableAttribute();
	void EnableTexCoords();
	void DisableTexCoords();
	bool SetCapacity(int size);
	int GetCapacity(); // current maximum capacity
	int GetSize(); // current size
	void SortByDist(const triple origin, const triple direction);

	void AddVertexSet(const triple* P, const quad* C, int length);
	// standard vertex addition
	inline void AddVertex(const triple& P, const triple& C, const float alpha = 1.0) 
	{	
		if (index>=max_index || latched)
			return;
		vertices[(index*3)+0] = P.x;
		vertices[(index*3)+1] = P.y;
		vertices[(index*3)+2] = P.z;
		color[(index*4)+0] = (unsigned char)(C.x*255);
		color[(index*4)+1] = (unsigned char)(C.y*255);
		color[(index*4)+2] = (unsigned char)(C.z*255);
		color[(index*4)+3] = (unsigned char)(alpha*255);
		index++;
	}
	// vertex + attribute value
	inline void AddVertex(const triple& P, const triple& C, const float attribute, const float alpha) 
	{
		if (index>=max_index || latched)
			return;
		GLfloat * v = &vertices[(index*3)];
		*v++ = P.x;
		*v++ = P.y;
		*v++ = P.z;
		GLubyte * c = &color[(index*4)];
		*c++ = (unsigned char)(C.x*255);
		*c++ = (unsigned char)(C.y*255);
		*c++ = (unsigned char)(C.z*255);
		*c++ = (unsigned char)(alpha*255);
		att[index] = attribute;
		index++;
	}
	void AttachAttribute(float att);
	void AttachTexCoord(float s, float t = 0, float u = 0);
	void ResetColor(int index, triple new_color, double alpha);
	
	void Bind();
	void BindAtt();
	void BindTex();
	bool Draw(); // draws points

	void Clear() { last_index = index = 0; latched = false; permuted_indices = false; }
	
protected:
	bool use_vbo, use_att, use_tex;
	const int tex_dim;

	bool permuted_indices; // indicates if the drawing order has changed (sort)

	GLint max_index, index, last_index;	
	GLuint buffer_id[5];
	GLuint att_id;
	GLfloat * vertices, * distances;
	GLubyte * color;
	GLfloat * att;
	GLfloat * tex;
	GLuint * indices, * sorted_indices;
	bool latched;
public:
	void Latch() { if (index > 0) latched = true; }
	void Unlatch() { latched = false; index = last_index = 0; }	
	bool IsLatched() { return latched; }
	bool IsUsingAtt() { return use_att; }
	bool IsUsingTex() { return use_tex; }
};
