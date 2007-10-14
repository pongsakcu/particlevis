#include "SphereRenderer.h"
#include "GLWnd.h"

SphereRenderer::SphereRenderer(void) :
bound(false),
use_velocity(false),
use_points(false),
alpha(1.0),
scalar(1.0),
ParticleRenderer()
{
}

SphereRenderer::~SphereRenderer(void)
{
}

void SphereRenderer::Load(triple *pos, quad *axis, triple *d_pos, triple *d_angle, Particle *p)
{	
	static triple rgb;
	if (p->IsMarked() != MARK_UNMARKED && Particle::useMarkColor)
		rgb = Particle::markingColor;
	else if (p->ColorOverride)		
		rgb = p->rgb;		
	else
		rgb = coloring->operator()(*pos, *axis, *d_pos, *d_angle, p);

	vBuf.AddVertex(*pos, rgb, p->GetRadius(), alpha);
	if (use_velocity)
		vBuf.AttachTexCoord(
			(d_pos->x) * scalar,
			(d_pos->y) * scalar,
			(d_pos->z) * scalar);
}

void SphereRenderer::Clear()
{
	vBuf.Clear();
	bound = false;
	scalar = Particle::transVelocityLength / Particle::maxTransVelocity; // normalized velocity length factor
}

void SphereRenderer::SetSize(int count)
{
	if (vBuf.GetCapacity() != count)
	{
		if (GLEW_VERSION_2_0 && GLEW_ARB_shader_objects) // initialize the shader attributes		
			vBuf.EnableAttribute(glGetAttribLocationARB(sphere_shader.GetProgramID(), "psize"));		
		vBuf.SetCapacity(count);
	}
}

void SphereRenderer::Render()
{
	if (vBuf.GetSize() == 0) return; // nothing to draw
	if (!bound)
	{
		vBuf.Bind();
		bound = true;
	}		
	if (!use_points)
	{
		if (GLEW_ARB_point_sprite)
		{
			glDisable(GL_POINT_SMOOTH);
			glEnable(GL_POINT_SPRITE);
			glEnable(GL_TEXTURE_2D);
			glTexEnvf(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
		}
		if (GLEW_VERSION_2_0 && GLEW_ARB_shader_objects)
		{		
			float psize, multisample = (float)GLWnd::useAA;
			glGetFloatv(GL_POINT_SIZE, &psize);
			unsigned int sizevar = glGetAttribLocationARB(sphere_shader.GetProgramID(), "psize");
			glEnable(GL_VERTEX_PROGRAM_POINT_SIZE_NV); // allow point size changes in vertex program

			sphere_shader.Bind(); // bind shader.
			int view[4];
			glGetIntegerv(GL_VIEWPORT, view);
			float pixel_scale_factor_w = (float)view[2]; // viewport width, px
			float pixel_scale_factor_h = (float)view[3]; // viewport height, px
			if (GLWnd::useAA == 8) // multisampling interacts with the scale strangely, might only work for NVidia
			{
				pixel_scale_factor_w *= 2.0;
				pixel_scale_factor_h *= 2.0;
			}
			// Pass viewport sizes, for pixel-based scales:
			glUniform1fARB(glGetUniformLocationARB(sphere_shader.GetProgramID(), "viewport_mapping"), pixel_scale_factor_h); // redundant!
			glUniform1fARB(glGetUniformLocationARB(sphere_shader.GetProgramID(), "viewport_mapping_h"), pixel_scale_factor_h);
			glUniform1fARB(glGetUniformLocationARB(sphere_shader.GetProgramID(), "viewport_mapping_w"), pixel_scale_factor_w);
		}
	}

	// Actual draw call
	vBuf.Draw();

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

void SphereRenderer::SelectShader(string vertname, string pixelname)
{
	Clear();
	if (GLEW_VERSION_2_0 && GLEW_ARB_vertex_shader && GLEW_ARB_fragment_shader)
	{
		sphere_shader.LoadVertFile(vertname.c_str());
		sphere_shader.LoadFragFile(pixelname.c_str());
	}
	else
		use_points = true;	
	SetSize(0);
}

// Direction isn't actually used here
void SphereRenderer::SortByDistance(triple camera, triple direction)
{
	vBuf.SortByDist(camera, direction);
}