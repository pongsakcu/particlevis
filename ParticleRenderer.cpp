#include "stdafx.h"
#include "ParticleRenderer.h"
#include "GLOcclusionQuery.h"
#include <algorithm>
#include <functional>

using namespace std;

ParticleRenderer::ParticleRenderer(void) :
use_queries(false),
queries(NULL)
{
}

ParticleRenderer::~ParticleRenderer(void)
{
}

void ParticleRenderer::Load(triple* position, quad* orientation, triple* d_pos, triple* d_theta, Particle* p)
{
	static PRenderCmd cmd;
	cmd.pos = *position;
	cmd.axis = *orientation;
	cmd.display_list = p->displaylist;
	cmd.PID = p->PID;
	if (p->IsMarked() != MARK_UNMARKED && Particle::useMarkColor)
		cmd.rgb = Particle::markingColor;
	else if (p->ColorOverride)		
		cmd.rgb = p->rgb;		
	else
		cmd.rgb = coloring->operator()(*position, *orientation, *d_pos, *d_theta, p);
	render_buffer.push_back( cmd );
}

void ParticleRenderer::Render()
{
	if (!use_queries)
		for_each(render_buffer.begin(), render_buffer.end(), SolidRender());
	else	
		for_each(render_buffer.begin(), render_buffer.end(), QuerySolidRender(queries));	
}

void ParticleRenderer::SetSize(int size)
{
	render_buffer.reserve(size);
}

void ParticleRenderer::EnableQueries()
{
	use_queries = true;
	queries = GLOcclusionQuery::queries;
}

void ParticleRenderer::DisableQueries()
{
	use_queries = false;
}