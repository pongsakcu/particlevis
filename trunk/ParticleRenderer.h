/* 
ParticleRenderer

		   Basic class for display-list based rendering.  An internal buffer is used to store the 
		drawing commands and reorder them if necessary.  In this (simple) case the buffer holds a
		ParticleStore row, a Particle ID, and the display list.
		   Enable/Disable queries allows hardware occlusion queries to be applied to the set of particles.

	   By Vincent Hoon, 2007
	   See copyright notice in copyright.txt for conditions of distribution and use.
*/

#pragma once
#include <vector>
#include "Renderer.h"
#include "RGBClassifier.h"

struct ParticleRenderCommand {
	triple pos;
	quad axis;
	triple rgb;
	unsigned int display_list;
	unsigned int PID;
	float distance;
};
typedef struct ParticleRenderCommand PRenderCmd;

using std::vector;

class ParticleRenderer : public Renderer
{
public:
	ParticleRenderer(void);
	~ParticleRenderer(void);
	void SetSize(int size); // possible resizing logic her	
	void Clear() { render_buffer.resize(0); } // call before new frame
	void Load(triple* position, quad* orientation, triple* d_pos, triple* d_theta, Particle* p); // load in particle data
	void Render(); // call to draw buffer
	
	void EnableQueries();
	void DisableQueries();
protected:
	GLuint* queries;
	bool use_queries; // enable to turn on queries

	WhiteClassifier default_color;
	vector<PRenderCmd> render_buffer;
};

class SolidRender // core functor for rendering: draw the particles by setting matrix state and invoking the list
{
public:
	SolidRender() {}
	void operator()(PRenderCmd& cmd) const
	{
		glPushMatrix();
		glTranslatef(cmd.pos.x, cmd.pos.y, cmd.pos.z);
		glRotatef(cmd.axis.w, cmd.axis.x, cmd.axis.y, cmd.axis.z);
		glColor3f(cmd.rgb.x, cmd.rgb.y, cmd.rgb.z);
		glCallList(cmd.display_list+Particle::quality);
		glPopMatrix();
	}
};

class QuerySolidRender // use occlusion queries in addition to rendering.
{
public:
	QuerySolidRender(GLuint* q) : queries(q) {}
	GLuint* queries;
	void operator()(PRenderCmd& cmd) const
	{
		glBeginQueryARB(GL_SAMPLES_PASSED_ARB, queries[cmd.PID]);
		glPushMatrix();
		glTranslatef(cmd.pos.x, cmd.pos.y, cmd.pos.z);
		glRotatef(cmd.axis.w, cmd.axis.x, cmd.axis.y, cmd.axis.z);
		glColor3f(cmd.rgb.x, cmd.rgb.y, cmd.rgb.z);
		glCallList(cmd.display_list+Particle::quality);
		glPopMatrix();
		glEndQueryARB(GL_SAMPLES_PASSED_ARB);
	}
};