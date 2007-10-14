#include "VectorRenderer.h"
#include "Particle.h"

VectorRenderer::VectorRenderer(void) : 
bound(false),
type(POSVELOCITY)
{
}

VectorRenderer::~VectorRenderer(void)
{
}

void VectorRenderer::SetSize(int size) // possible resizing logic	
{
	vBuf.SetCapacity(size);
	bound = false;
}

void VectorRenderer::Clear() // call before new frame
{
	vBuf.Clear();
	bound = false;
}

void VectorRenderer::Load(triple* pos, quad* axis, triple* d_pos, triple* d_angle, Particle* p) // load in particle data
{
	triple delta(0, 0, 0);
	triple origin(0, 0, 0);
	triple color(0, 0, 0);
	double headlength = 0.925; // length of arrowhead
	double headwidth = 0.05; // width of arrowhead
	double vscale = Particle::transVelocityLength; // scaling factor, to enlarge or shrink vector size arbitrarily
	origin = *pos;		

	switch (type)
	{
		case POSVELOCITY:
			vscale = Particle::transVelocityLength;// / maxTransVelocity;
			delta = *d_pos;
			break;
		case ROTVELOCITY:
			vscale = Particle::rotVelocityLength;// / maxRotVelocity;
			headlength = 0.9;
			headwidth = 0.05;
			delta = *d_angle;
			break;
		case NORMALVEC:
			vscale = Particle::transVelocityLength;
			headlength = 1.0;
			headwidth = 0.085;
			delta = p->GetNormal();
			break;
		case FIELDVEC:
			delta = *d_pos;
			vscale = Particle::fieldVelocityLength;// / maxFieldVelocity;
			break;
	}		

	switch (type)
	{		
		case ROTVELOCITY:
			color = rot_coloring(*pos, *axis, *d_pos, *d_angle, p);
			break;
		case FIELDVEC:
			color = field_coloring(*pos, *axis, *d_pos, *d_angle, p);
			break;
		case POSVELOCITY:
			color = vel_coloring(*pos, *axis, *d_pos, *d_angle, p);
			break;
		case NORMALVEC:
			color.x = 0;
			color.y = 0;
			color.z = 0;
			break;	
	}

	//if (power<=0)
	//	return;
	//if (power > 1) power = 1;
	
	triple up(0, 0, 1);
	triple cross(0, 0, 0);
	if (delta.x == 0 && delta.y == 0)
	{
		cross.x = 0;
		cross.y = sqrt((delta.x*delta.x) +
						(delta.y*delta.y) +
						(delta.z*delta.z));
		cross.z = 0;
	}
	else
	{
		cross = crossProduct(delta, up);
		double vecLength = sqrt((delta.x*delta.x) +
						(delta.y*delta.y) +
						(delta.z*delta.z));
		double crossLength = sqrt((cross.x*cross.x) +
						(cross.y*cross.y) +
						(cross.z*cross.z));
		cross.x *= (vecLength / crossLength);
		cross.y *= (vecLength / crossLength);
		cross.z *= (vecLength / crossLength);
	}
		

	// draw vector-line+arrowhead

	vBuf.AddVertex(origin, color);
	// vector-line itself
	vBuf.AddVertex(vscale*delta + origin, color);

	vBuf.AddVertex(headlength*vscale*delta + headwidth*vscale*cross + origin, color);
	// begin arrowhead 1

	if (type != NORMALVEC)
	{
		vBuf.AddVertex(vscale*delta + origin, color);
		// end arrowhead 1
		// begin arrowhead 2
		vBuf.AddVertex(vscale*delta + origin, color);
	}

	vBuf.AddVertex(headlength*vscale*delta + -headwidth*vscale*cross + origin, color);
	// end arrowhead 2

	if (type == ROTVELOCITY) // add additional arrowhead
	{
		vBuf.AddVertex(0.95*vscale*delta + origin, color);
		// arrowhead 3
		vBuf.AddVertex((headlength-0.05)*vscale*delta + headwidth*vscale*cross + origin, color);

		vBuf.AddVertex(0.95*vscale*delta + origin, color);
		// arrowhead 4
		vBuf.AddVertex((headlength-0.05)*vscale*delta + -headwidth*vscale*cross + origin, color);
	}
}

void VectorRenderer::Render() // call to render
{
	if (vBuf.GetSize() == 0) return; // nothing to draw
	if (!bound)
	{
		vBuf.Bind();
		bound = true;
	}
	vBuf.Draw();
}