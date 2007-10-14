/*
GLLineBuffer

		  A GLVertexBuffer subclass specialized in buffering and
		drawing collections of GL_LINES.

		Vincent Hoon, 2007
		See copyright notice in copyright.txt for conditions of distribution and use.
*/

#pragma once
#include "glvertexbuffer.h"

class GLLineBuffer :
	public GLVertexBuffer
{
public:
	GLLineBuffer(void);
	~GLLineBuffer(void);
	bool Draw();
	bool SetCapacity(int size);
};
