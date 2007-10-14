#include "QuerySphereRenderer.h"
#include "GLOcclusionQuery.h"
#include <assert.h>

QuerySphereRenderer::QuerySphereRenderer(void) :
SphereRenderer(),
queries(0)
{
}

QuerySphereRenderer::~QuerySphereRenderer(void)
{
}

void QuerySphereRenderer::SetSize(int count)
{
	vertex_cache.reserve(count);
	radius_cache.reserve(count);
	ID_cache.reserve(count);
}

void QuerySphereRenderer::Load(triple *pos, quad *axis, triple *d_pos, triple *d_angle, Particle *p)
{
	vertex_cache.push_back(*pos);
	radius_cache.push_back(p->GetRadius());
	ID_cache.push_back(p->GetPID());
}

void QuerySphereRenderer::Clear()
{
	vertex_cache.clear();
	radius_cache.clear();
	ID_cache.clear();
}

void QuerySphereRenderer::EnableQueries()
{
	queries = GLOcclusionQuery::queries;
}

void QuerySphereRenderer::Render()
{
	if (vertex_cache.size() == 0 || queries == 0) return; // nothing to draw

	if (GLEW_ARB_point_sprite)
	{
		glDisable(GL_POINT_SMOOTH);
		glEnable(GL_POINT_SPRITE);
		glEnable(GL_TEXTURE_2D);
		glTexEnvf(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
	}
	if (GLEW_VERSION_2_0 && GLEW_ARB_shader_objects)
	{
		int count = vertex_cache.size();
		float psize;
		glGetFloatv(GL_POINT_SIZE, &psize);
		unsigned int sizevar = glGetAttribLocationARB(sphere_shader.GetProgramID(), "psize");
		glEnable(GL_VERTEX_PROGRAM_POINT_SIZE_NV); // allow point size changes in vertex program

		sphere_shader.Bind(); // bind shader.
		int view[4];
		glGetIntegerv(GL_VIEWPORT, view);
		float pixel_scale_factor_w = (float)view[2]; // viewport width, px
		float pixel_scale_factor_h = (float)view[3]; // viewport height, px

		// Pass viewport sizes, for pixel-based scales:
		glUniform1fARB(glGetUniformLocationARB(sphere_shader.GetProgramID(), "viewport_mapping"), pixel_scale_factor_h); // redundant!
		glUniform1fARB(glGetUniformLocationARB(sphere_shader.GetProgramID(), "viewport_mapping_h"), pixel_scale_factor_h);
		glUniform1fARB(glGetUniformLocationARB(sphere_shader.GetProgramID(), "viewport_mapping_w"), pixel_scale_factor_w);
	
		// Actual draw call		
		vector<triple>::iterator vertices = vertex_cache.begin();
		vector<float>::iterator radii = radius_cache.begin();
		vector<int>::iterator ID = ID_cache.begin();
		assert(vertex_cache.size() == radius_cache.size());
		assert(vertex_cache.size() == ID_cache.size());
		for (int i = 0; i < count; i++)
		{
			triple &v(*vertices);

			glBeginQueryARB(GL_SAMPLES_PASSED_ARB, queries[*ID]);
			glBegin(GL_POINTS);
			glVertexAttrib1f(sizevar, *radii);
			glVertex3f(v.x, v.y, v.z);			
			glEnd();
			glEndQueryARB(GL_SAMPLES_PASSED_ARB);

			vertices++;
			radii++;
			ID++;
		}
	}
	// Reset GL state
	if (GLEW_ARB_point_sprite && !use_points)
	{
		glDisable(GL_POINT_SPRITE);
		glTexEnvf(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_FALSE);
		glDisable(GL_TEXTURE_2D);
	}	
	if (GLEW_VERSION_2_0 && GLEW_ARB_shader_objects)
		sphere_shader.UnBind();
}