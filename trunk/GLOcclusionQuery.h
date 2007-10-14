#pragma once
#include "stdafx.h"

// GL Utility class that manages occlusion queries.
// Vincent Hoon, 2007
// See copyright notice in copyright.txt for conditions of distribution and use.

class GLOcclusionQuery
{
public:
	GLOcclusionQuery(void);
	~GLOcclusionQuery(void);

	static GLuint* queries;
	static void InitQueries(int newquerysize);
	static bool QueryParticle(int PID);
private:
	static int querysize;
};
