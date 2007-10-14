#include "MappedSphereRenderer.h"
#include <math.h>
#include <assert.h>

#ifndef clamp
#define clamp(x,a,b) ( max( (a), min((b), (x))) )
#endif

MappedSphereRenderer::MappedSphereRenderer(const int res) :
ParticleRenderer(),
texture_cache(NULL),
resolution(res),
size(0),
bound(false),
loaded(0)
{
	use_queries = false;
}

MappedSphereRenderer::~MappedSphereRenderer(void)
{
}

void MappedSphereRenderer::Clear()
{
	loaded = 0;
	tex_coords = texture_cache;
	render_buffer.clear();
}

void MappedSphereRenderer::SetResolution(int res)
{
	resolution = res;
	int tempsize = size;
	size = 0;
	SetSize(tempsize); // flush vertices, recreate buffer
}

void MappedSphereRenderer::SetSize(int newsize) {	
	if (size != newsize)
	{
		buffer.GenerateSphere(resolution);
		delete[] texture_cache;
		texture_cache = new float[newsize * resolution*(resolution+2)];
	}
	size = newsize;
	bound = false;
	render_buffer.reserve(newsize);
	render_buffer.resize(0);
	tex_coords = texture_cache;
	loaded = 0;
}

void MappedSphereRenderer::Load(triple* position, quad* orientation, triple* d_pos, triple* d_theta, Particle* p)
{	
	int index = 0;
	MapRenderCommand cmd;

	if (p->IsMarked() != MARK_UNMARKED && Particle::useMarkColor)
		cmd.rgb = Particle::markingColor;
	else if (p->ColorOverride)		
		cmd.rgb = p->rgb;		
	else
		cmd.rgb = coloring->operator()(*position, *orientation, *d_pos, *d_theta, p);

	cmd.pos = *position;
	cmd.axis = *orientation;
	cmd.PID = p->PID;
	cmd.radius = p->GetRadius();
	render_buffer.push_back(cmd);
	if (p->mapped_points.size() >= (Particle::currentFrame+1))
	{
		int current = Particle::currentFrame;
		int size = p->mapped_points.size();
		static const float logscale = log(11.0);
		float const sqrtscale = sqrt(Particle::map_scale);
		static const double R = 0.5;
		double value = 0;
		int raw_index;
		assert(p->PID <= this->size);
		for (int i = 0; i < resolution; i++)
		{
			for (int j = -1; j <= resolution; j++)
			{
				raw_index = i*resolution + j;//(res*res)*current + i*res + j;
				if (j==-1) raw_index++;
				if (j==resolution) raw_index--;
				value = p->mapped_points[current][raw_index];

				if (Particle::surface_map_scale == LINEAR)
				{
					value /= Particle::map_scale;
					*tex_coords++ = clamp(value, 1e-4, 0.9999);
				}
				else if (Particle::surface_map_scale == LOG)
				{
					value = log((10.0*(double)value/Particle::map_scale) + 1.0);
					value /= logscale;
					*tex_coords++ = clamp(value, 1e-4, 0.9999);
				}
				else if (Particle::surface_map_scale == QUAD)
				{
					value = sqrt(value) / sqrtscale;
					*tex_coords++ = clamp(value, 1e-4, 0.9999);
				}
			}
		}
		
	}
	else
	{
		for (int i = 0; i < (resolution*(resolution+2)); i++)			
			*tex_coords++ = (float)rand() / (float)RAND_MAX;		
	}
	loaded++;
}

void MappedSphereRenderer::Render()
{
	if (loaded == 0) return;
	//if (!bound)
	{
		if (GLEW_VERSION_1_5)
		{
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		}
		buffer.Bind();
		bound = true;
	}
	for (int p = 0; p < loaded; p++)
	{
		MapRenderCommand cmd = render_buffer[p];

		if (use_queries)
			glBeginQueryARB(GL_SAMPLES_PASSED_ARB, queries[cmd.PID]);
		glPushMatrix();
		glTranslatef(cmd.pos.x, cmd.pos.y, cmd.pos.z);
		glRotatef(cmd.axis.w, cmd.axis.x, cmd.axis.y, cmd.axis.z);
		glColor3f(cmd.rgb.x, cmd.rgb.y, cmd.rgb.z);
		glScalef(cmd.radius, cmd.radius, cmd.radius);
		buffer.SetMapPoints(&texture_cache[p * resolution*(resolution+2)], (resolution)*(resolution+2));
		//buffer.BindTex();
		buffer.Draw();
		glPopMatrix();
		if (use_queries)
			glEndQueryARB(GL_SAMPLES_PASSED_ARB);
	}
}