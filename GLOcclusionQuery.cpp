#include "GLOcclusionQuery.h"
#include "ParticleSet.h"
#include <algorithm>

GLuint* GLOcclusionQuery::queries = NULL;
int GLOcclusionQuery::querysize = 0;

GLOcclusionQuery::GLOcclusionQuery(void)
{
}

GLOcclusionQuery::~GLOcclusionQuery(void)
{
}

void GLOcclusionQuery::InitQueries(int newquerysize)
{
	if (querysize != newquerysize)
	{
		delete[] queries;
		queries = new GLuint[newquerysize];
		std::fill(queries, queries+newquerysize, 0);
		glDeleteQueriesARB(newquerysize, queries);	
		glGenQueriesARB(newquerysize, queries);
		querysize = newquerysize;
	}
}

bool GLOcclusionQuery::QueryParticle(int PID)
{
	GLuint sampleCount;
	glGetQueryObjectuivARB(queries[PID], GL_QUERY_RESULT_ARB, &sampleCount);
	return (ParticleSet::currentQuery.occluded_set ?
		(sampleCount == 0) : (sampleCount > 0));
}