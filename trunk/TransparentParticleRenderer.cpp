#include "TransparentParticleRenderer.h"
#include <algorithm>
#include <functional>

using namespace std;

class TransparentRender // use alpha values, otherwise same as ParticleRenderer
{
public:
	TransparentRender(float a) : alpha(a) {}
	float alpha;
	void operator()(PRenderCmd& cmd)
	{
		glPushMatrix();
		glTranslatef(cmd.pos.x, cmd.pos.y, cmd.pos.z);
		glRotatef(cmd.axis.w, cmd.axis.x, cmd.axis.y, cmd.axis.z);
		glColor4f(cmd.rgb.x, cmd.rgb.y, cmd.rgb.z, alpha);
		glCallList(cmd.display_list+Particle::quality);
		glPopMatrix();
	}
};

class DistanceCalc : public unary_function<PRenderCmd&, void>
{
public:
	DistanceCalc(triple origin, triple direction) : o(origin), d(direction) {}
	triple o, d;
	void operator()(PRenderCmd& cmd)
	{
		cmd.distance = dotproduct(cmd.pos - o, d);
	}
};

class ParticleSorter : public binary_function<PRenderCmd&, PRenderCmd&, bool>
{
public:
	ParticleSorter() {}
	bool operator()(PRenderCmd& one, PRenderCmd& two)
	{
		return (one.distance > two.distance);
	}
};

TransparentParticleRenderer::TransparentParticleRenderer(void) :
ParticleRenderer(),
alpha(1.0)
{
}

TransparentParticleRenderer::~TransparentParticleRenderer(void)
{
}

void TransparentParticleRenderer::SetAlpha(float a)
{
	alpha = a;
}

void TransparentParticleRenderer::SortByDistance(triple origin, triple direction)
{
	for_each(render_buffer.begin(), render_buffer.end(), DistanceCalc(origin, direction));
	sort(render_buffer.begin(), render_buffer.end(), ParticleSorter());
}

void TransparentParticleRenderer::Render()
{
	if (!use_queries)
		for_each(render_buffer.begin(), render_buffer.end(), TransparentRender(alpha));
	else	
		for_each(render_buffer.begin(), render_buffer.end(), QuerySolidRender(queries));	
}