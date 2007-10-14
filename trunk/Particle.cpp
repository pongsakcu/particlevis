#include "Particle.h"
#include <math.h>
#include <algorithm>
#include "TNT.h"
#include "GeometryEngine.h"

#define SQR(value) ((value)*(value))

#define new DEBUG_NEW

using namespace std;
using namespace TNT;

int Particle::quality = 1;
int Particle::trailLength = 25;
float Particle::maxVelocityMap = 0.0;
float Particle::maxRVelocityMap = 0.0;
float Particle::maxFieldVectorMap = 0.0;
float Particle::transVelocityLength = 1.0;
float Particle::rotVelocityLength = 1.0;
float Particle::fieldVelocityLength = 1.0;
bool Particle::normalVector = false;
bool Particle::showParticle = true;
bool Particle::rotVector = false;
float* Particle::map_vertices = NULL;
float* Particle::colors = NULL;
float* Particle::current_tex_coords = NULL;
unsigned int* Particle::vertex_order = NULL;
unsigned int* Particle::texture_order = NULL;
float Particle::map_scale = 1.0;
int Particle::surface_map_size = 0;
int Particle::surface_map_scale = 0;
float Particle::lineWidth = 1.0;
float Particle::pointSize = 4.0;

GLMappedSphereBuf Particle::sphereMapBuffer; // only used in deprecated calls

triple Particle::currentCam = triple(0, 0, 0);
triple Particle::currentTarget = triple(0, 0, 0);
triple Particle::currentCamDirection = triple(0, 0, 0);
triple Particle::markingColor = triple(1, 0, 0);
triple Particle::pathColor1 = triple(0, 1, 0);
triple Particle::pathColor2 = triple(0, 1, 1);
int Particle::currentFrame = 0;
RGBGradient Particle::grad;
float Particle::alpha = 1.0;
bool Particle::setup = false;
bool Particle::useMarkColor = true;

float Particle::maxPosition = 0.0;
float Particle::maxTransVelocity = 0.0;
float Particle::maxRotVelocity = 0.0;
float Particle::maxFieldVelocity = 0.0;

#ifndef clamp
#define clamp(x,a,b) ( max( (a), min((b), (x))) )
#endif

Particle::Particle(ParticleStore &s, double r, int ID) : // r = radius.  Unused in default Particle.
data_store(s),
frames(0),
maxsize(0),
marked(MARK_UNMARKED),
radius(r),
canTransparent(true),
camera_distance(0),
PID(ID),
pureSphere(true),
ColorOverride(false)
{
	rgb.x = rgb.y = rgb.z = -1.0;
	canTransparent = true;
	if (grad.Allocated()==0)
	{
		grad.AddControlPoint(0, triple(0, 0, 1));
		grad.AddControlPoint(0.33, triple(0, 1, 0));
		grad.AddControlPoint(0.66, triple(1, 1, 0));
		grad.AddControlPoint(1.0, triple(1, 0, 0));

		grad.CalculateGradient(2048);
	}
	displaylist = GeometryEngine::default_display_lists;
}

Particle::~Particle(void)
{
	ClearSurfaceMap();
	if (maxsize > 0)
	{
		maxsize = 0;
		colorMap.clear();
	}
}

bool Particle::operator<(Particle &right) // used for sorting particles in terms of the camera origin
{
	if (!canTransparent)
		return false;
	else 
	{
		if (camera_distance < right.camera_distance) { return true; }
		else  { return false; }
	}
}

void Particle::DrawGeometry()	// Draw untransformed geometry
{
	// Generally the Particle::quality class variable
	// will determine the complexity of the geometry drawn here.
	// This allows for rudimentary level-of-detail adaptation.

	if (mapped_points.size() > 0)
	{ 
		int res = surface_map_size, index = 0;
		glScalef(radius, radius, radius);
				
		if (mapped_points.size() >= (currentFrame+1))
		{			
			int current = currentFrame;
			int size = mapped_points.size();
			static float logscale = log(11.0);
			float sqrtscale = sqrt(map_scale);
			static double R = 0.5;
			double value = 0;
			int raw_index;
			float * tex_coords = current_tex_coords;

			for (int i = 0; i < res; i++)
			{
				for (int j = -1; j <= res; j++)
				{
					raw_index = i*res + j;//(res*res)*current + i*res + j;
					if (j==-1) raw_index++;
					if (j==res) raw_index--;
					value = mapped_points[current][raw_index];

					if (surface_map_scale == LINEAR)
					{
						value /= map_scale;
						*tex_coords++ = clamp(value, 1e-4, 0.9999);
					}
					else if (surface_map_scale == LOG)
					{
						value = log((10.0*(double)value/map_scale) + 1.0);
						value /= logscale;
						*tex_coords++ = clamp(value, 1e-4, 0.9999);
					}
					else if (surface_map_scale == QUAD)
					{
						value = sqrt(value) / sqrtscale;
						*tex_coords++ = clamp(value, 1e-4, 0.9999);
					}
				}
			}
			
		}
		else
		{
			float * tex_coords = current_tex_coords;
			for (int i = 0; i < (res*(res+2)); i++)
			{
				*tex_coords++ = 0.0;
			}
		}		

		sphereMapBuffer.SetMapPoints(current_tex_coords, (2*(res)*(res+2)));
		sphereMapBuffer.BindTex();
		sphereMapBuffer.Draw();
		}
	else
	{
		//glDisable(GL_POINT_SMOOTH);
		glBegin(GL_POINTS);
		glNormal3d(0, 1, 0);
		glVertex3d(0, 0, 0);
		glEnd();

		glColor3d(0, 0, 1);
		glBegin(GL_LINES);
		glNormal3d(1, 0, 0);
		glVertex3d(-radius, 0, 0);
		glVertex3d(radius, 0, 0);
		glNormal3d(0, 1, 0);
		glVertex3d(0, -radius, 0);
		glVertex3d(0, radius, 0);
		glNormal3d(0, 0, 1);
		glVertex3d(0, 0, -radius);
		glVertex3d(0, 0, radius);
		glEnd();
	}
}

// Draw particle in world coordinates.  Assume modelview is current matrix.
void Particle::DrawInstance(int frame) 
{
	if (ColorOverride)
		glColor3f(rgb.x, rgb.y, rgb.z);
	if (displaylist!=-1)
		glCallList(displaylist+Particle::quality);
	else
		DrawGeometry();
}

// Do Matrix Setup

void Particle::SetupMatrix(int frame)
{
	Frame f = data_store.GetFrame(frame);
	triple pos = f.pos[PID];
	quad theta = f.axis[PID];
	glTranslatef(pos.x, pos.y, pos.z);
	glRotatef(theta.w, theta.x, theta.y, theta.z);
}

// Draw path of this particle
void Particle::DrawPath(const int frame, GLVertexBuffer& vBuf)
{
	// this function and DrawVector both add their lines/colors to an external
	// array (mypath/mycolor) that is rendered with glDrawArrays

	if (marked != MARK_HIGHLIGHT_MODE || trailLength <= 0)
		return;

	float alpha = 2.0; //fade = -0.01 * (1.0/trailLength);
	float param1 = 1.0, param2 = 0.0;
	float doublefade = -2.0 / (float)(trailLength);
	float fade = -1.0 / (float)(trailLength);
	float output_alpha = 1.0;

	int j = 0;
	triple last_point, point, last_color, color = pathColor1;
	point = data_store.GetFramePos(frame)[PID];
	for (int i = frame-1; (i >= 0) && (j < trailLength); i--) // count backwards, do not go below 0
	{
		alpha+=doublefade;
		param1+=fade;
		param2-=fade;
		//color = triple(0.0, 1.0, max(0.0, 1.0 - alpha));
		last_color = color;
		color.x = (pathColor1.x * param1) + (pathColor2.x * param2);
		color.y = (pathColor1.y * param1) + (pathColor2.y * param2);
		color.z = (pathColor1.z * param1) + (pathColor2.z * param2);

		last_point = point;
		point = data_store.GetFramePos(i)[PID];
		
		vBuf.AddVertex(last_point, last_color, output_alpha);
		if (Particle::quality >= 3)		
			//output_alpha = min(1.0, alpha);
			output_alpha = param1;
		else
			output_alpha = 1.0;
		vBuf.AddVertex(point, color, output_alpha);
		
		j++;
	}
}

void Particle::DrawPoint(int frame) // simply draw a vertex at particle center
{
	triple pos = data_store.GetFramePos(frame)[PID];
	glVertex3d(pos.x, pos.y, pos.z);
}

void Particle::LoadRender(int displaylist) // acquire display list for rendering
{
	this->displaylist = displaylist;
}

// Inserts complete information on a single frame into the arrays of particle information
// Always places info at end of current arrays, returns number of total frames stored.

int Particle::AddFrame(triple position, triple theta, triple pos_dot, triple theta_dot, bool field)
{
	// construct a quaternion from the Euler angles
	quad temp;	double C1, C2, C3, S1, S2, S3;
	C1 = cos(theta.x/2.0);
	C2 = cos(theta.y/2.0);
	C3 = cos(theta.z/2.0);
	S1 = sin(theta.x/2.0);
	S2 = sin(theta.y/2.0);
	S3 = sin(theta.z/2.0);

	temp.w = (C1 * C2 * C3) - (S1 * S2 * S3);
	temp.x = (C1 * S2 * S3) + (S1 * C2 * C3);
	temp.y =(-S1 * C2 * S3) + (C1 * S2 * C3);
	temp.z = (C1 * C2 * S3) + (S1 * S2 * C3);

	// pass to overloaded frame-adder
	return AddFrame(position, temp, pos_dot, theta_dot, field);
}

int Particle::AddFrame(triple position, quad theta, triple pos_dot, triple theta_dot, bool field)
{
	static float a, l;
	a = (2.0*acos(theta.w))*180.0/PI;
	quad axis_angle;
	if (a == 0)
	{
		axis_angle = quad(0, 0, 1, 0);
	}
	else
	{
		l = sqrt(SQR(theta.x) + SQR(theta.y) + SQR(theta.z));
		axis_angle = quad(a, theta.x / l, theta.y / l, theta.z / l );
	}

	if (!data_store.SetParticleData(frames, PID, position, axis_angle, pos_dot, theta_dot))
		return -1;
	normal.push_back(triple(0, 0, 0));	

	if (!field)
	{
		if (length(pos_dot) > maxTransVelocity) maxTransVelocity = length(pos_dot);
	}
	else
	{
		if (length(pos_dot) > maxFieldVelocity) maxFieldVelocity = length(pos_dot);
	}
	if (length(theta_dot) > maxRotVelocity) maxRotVelocity = length(theta_dot);
	if (length(position) > maxPosition) maxPosition = length(position);
	return frames++;
}

// Allows a particle's color to change over time-- once frame "time" is reached,  the specified
// color will be loaded in.  Can be arbitrarily sequenced, so long as initial timevalues are sequential.
void Particle::AddColor(int frame, triple rgb, bool persistent)
{
	if (persistent)
	{
	while (colorMap.size() < frame)
		colorMap.push_back(this->rgb);
	}
	else
		colorMap.resize(frame, triple(1, 1, 1));

    colorMap.push_back(rgb);
	this->rgb = rgb;
}

// "marks" a particle: whenever marked != -1, the particle's path is drawn and its color is highlighted
void Particle::MarkParticle(int mode)
{
    marked = mode;
}

// returns the speed at passed frame
float Particle::GetVelocity(int frame)
{
	triple vel = data_store.GetFrameVel(frame)[PID];
	return vel.Length();
}

// returns rotational speed.
float Particle::GetRVelocity(int frame)
{

	triple rvel = data_store.GetFrameRVel(frame)[PID];
	return rvel.Length();
}

triple Particle::GetPosition(int frame)
{
	return data_store.GetFramePos(frame)[PID];
}

// tests if the particle is within the passed bounds
bool Particle::TestBounds(int dimension, float min, float max, int frame)
{
	/*switch (dimension)
	{
	case X:
		return ((pos[frame].x <= max) && (pos[frame].x >= min));
		break;
	case Y:
		return ((pos[frame].y <= max) && (pos[frame].y >= min));
		break;
	case Z:
		return ((pos[frame].z <= max) && (pos[frame].z >= min));
		break;
	}*/
	return false;
}

triple Particle::PeekVector(int frame, int type)
{
	
	if (frame >= frames) return triple(0,0,0);
	Frame f = data_store.GetFrame(frame);
	if (type == POSITION)
		return f.pos[PID];

	if (type == ROTATION)
	{
		quad theta = f.axis[PID];

		float x = atan2(2.0*(theta.w*theta.x + theta.y * theta.z),
		( 1.0 - 2.0 * (SQR(theta.x) + SQR(theta.y)) ) );
		float y = asin(2.0 * (theta.w*theta.y - theta.z*theta.x));
		float z = atan2(2.0*(theta.w*theta.z + theta.x*theta.y),
		 1.0 - 2.0 * (SQR(theta.y) + SQR(theta.z)));
		//return triple( theta.x, theta.y, theta.z );
		return triple(x, y, z);
	}

	if (type == ROTVELOCITY)
		return f.d_angle[PID];
	if (type == POSVELOCITY)
		return f.d_pos[PID];
	if (type == NORMALVEC)
		return normal[frame];
	return triple(0,0,0);
}

quad Particle::PeekOrientation(int frame)
{
	return data_store.GetFrameAxis(frame)[PID];
	//return theta[frame];
}

void Particle::SetVector(int frame, int type, triple input)
{
	//if (frame >= frames) return;
	if (type == ROTVELOCITY)
		data_store.GetFrameRVel(frame)[PID] = input;
		//deltatheta[frame] = input;
	if (type == POSVELOCITY)
		data_store.GetFrameVel(frame)[PID] = input;
		//deltapos[frame] = input;
	if (type == NORMALVEC)
		normal[frame] = input;
}

void Particle::SetColor(triple rgb_color)
{
	ColorOverride = true;
	rgb = rgb_color;
}

/*triple Particle::SetColor(int saturation, int brightness, int hue)
{
	if (hue > 256) hue = 256;
	hue+=64;
	hue=256-hue;
	if (hue > 256) hue = 256;
	if ( saturation == 0 )
	{	
		return triple(brightness/256.0, brightness/256.0, brightness/256.0);
	}
	else
	{
		double V = brightness / (double)256.0;
		double S = saturation / (double)256.0;
        int mcp6 =(256 / 6 + 1);
		int I = hue / mcp6;
		double F = hue % mcp6 / (double)mcp6;
		double M = V * (1 - S);
		double N = V * (1 - S * F);
		double K = V * (1 - S * (1 - F));
		double R, G, B;
		switch ( I )
		{
			default: R = V; G = K; B = M; break;
			case 1: R = N; G = V; B = M; break;
			case 2: R = M; G = V; B = K; break;
			case 3: R = M; G = N; B = V; break;
			case 4: R = K; G = M; B = V; break;
			case 5: R = V; G = M; B = N; break;
		}
		//glColor4f(R, G, B, A);
		return triple(R, G, B);
	}
}*/

void Particle::SetSurfaceMapSize(int size)
{
	current_tex_coords = new float[(size+2)*size*2];
	Particle::sphereMapBuffer.GenerateSphere(size);
	Particle::sphereMapBuffer.Bind();
	surface_map_size = size;
}

void Particle::AddMapPoint(int frame, int panel, double mass)
{
	// created interleaved arrays for the surface-maps
	if (mapped_points.size()==0)
	{
		mapped_points.push_back(new float[surface_map_size*surface_map_size]);
		for (int i = 0; i < surface_map_size*surface_map_size; i++)
			mapped_points[0][i] = 0;
	}

	int size = mapped_points.size();
	if (size <= frame)
	{
		while (size < frame)
		{
			mapped_points.push_back(mapped_points[size - 1]);
			size = mapped_points.size();
		}
        //float* buffer = new float[surface_map_size*surface_map_size];
		// each frame requires a [size x size] array
		mapped_points.push_back(new float[surface_map_size*surface_map_size]);

		// copy old data in
		for (int i = 0; i < surface_map_size*surface_map_size; i++)
			mapped_points[frame][i] = mapped_points[frame-1][i];
	}
	// place map data into array
	mapped_points[frame][panel] = mass;
}

void Particle::ClearSurfaceMap()
{
	int size = mapped_points.size();
	if (size==0) return;
	for (int i = size-1; i > 0; i--)
	{
		if (mapped_points[i] != mapped_points[i-1])
			delete mapped_points[i];
	}
	delete mapped_points[0];
	mapped_points.clear();
}

void Particle::SetCamera(triple cam, triple target)
{
	currentCam = cam;
	currentTarget = target;
	currentCamDirection = (target - cam);
	normalize(currentCamDirection);
}