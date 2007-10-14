#include "ParticleSet.h"
#include "stdafx.h"
#include <vector>
#include <algorithm>
#include <math.h>
#include <iomanip>
#include <sstream>

#include <GL/glew.h>
#include "GLWnd.h"

using namespace std;

#define SQR(value) ((value)*(value))

double drumR = 304.8e-3;
bool ParticleSet::cullFaces = false;
bool ParticleSet::useQueries = false;
int ParticleSet::currentframe = 0;
struct QueryType ParticleSet::currentQuery;
extern GLuint* queries; // hax

ParticleSet::ParticleSet(CSemaphore* lock, Camera* camera) :
cam(camera),
use_colormaps(false),
use_surfacemaps(false),
coneangle(60),
maxframes(0),
listbase(0),
particle_count(0),
particles(NULL),
usePoints(false),
useParticles(true),
useLighting(false),
useVelocity(false),
useRotVelocity(false),
useSpecular(false),
useTextures(true),
useTransparency(false),
useTransparentBounds(false),
useBoundmarking(false),
useSphereShader(0),
alpha(0.15),
maxIndex(0),
colorMode(0),
showCone(false),
firstDraw(true),
conelist(0),
redrawCone(false),
useAlphaSort(false),
failed_resize_attempt(false),
marked_count(0)
{
	bounds[0] = bounds[1] = bounds[2] = bounds[3] = bounds[4] = bounds[5] = 0;
	useBound[0] = useBound[1] = useBound[2] = false;

	lockdown = lock;

	static_geometry.AddFrame(triple(), triple(), triple(), triple()); // origin 0 for static geometry
	static_geometry.MarkParticle(MARK_HIGHLIGHT_MODE);

	Particle::alpha = 1.0;
	currentQuery.flag_notrans = false;
	currentQuery.hide = false;
	currentQuery.mark = true;
	currentQuery.occluded_set = false;

	static_geometry.LoadRender(-1);
	static_geometry.MarkParticle(MARK_HIDE_MODE);

	vectorSet = new GLVectorSet();

	current_coloring = &white_coloring;
	map_renderer.SetColorClass(current_coloring);
}

ParticleSet::~ParticleSet(void)
{
	if (particles != NULL)
	{
		for (int i = 0; i < particle_count; i++)
			delete particles[i];
		delete [] particles;
		delete [] current_visibility;
		delete [] draw_opaque_particle;
		delete [] current_distances;
	}
	delete vectorSet;
}

int qsorter(const void* A, const void* B)
{
	Particle** L = (Particle**)(A);
	Particle** R = (Particle**)(B);
	if (**L < **R) return -1;
	else return 1;
}

void ParticleSet::DrawFrame()
{
	DrawFrame(currentframe);
}

// Draw a single frame, rendering all particles, vectors, and other data
void ParticleSet::DrawFrame(int frame)
{	
	#ifdef BENCHMARK
	stringstream msg;
	LARGE_INTEGER freq, prev, next;
	QueryPerformanceFrequency(&freq);	
	double drawtime, rendertime;
	#endif

	CSingleLock master(lockdown); // semaphore controlling access to particle array		
	ParticleSet::currentframe = frame;

	if (firstDraw) // Very first draw call.  We now have a valid context, initialize shaders
	{		
		conelist = glGenLists(1);
		sphere_renderer.SelectShader("Fixed.vert", "Sphere2.frag");
		trans_sphere_renderer.SelectShader("Fixed.vert", "Sphere2.frag");
		occ_renderer.SelectShader("OneLight.vert", "SolidDisk.frag");
		firstDraw = false;
	}
	if (redrawCone == true)
	{
		glNewList(conelist, GL_COMPILE);
		DrawCone(cone1, cone2, coneangle);
		glEndList();
	}	

	static double velocity;
	static int marked = 0;
	static float white[] = {1, 1, 1, 1};
	static float black[] = {0, 0, 0, 1};

	int Tindex = 0;
	transparent.clear();

	vIndex = 0;

	if (useTextures)
		glEnable(GL_TEXTURE_2D);
	else
		glDisable(GL_TEXTURE_2D);

	if (useLighting)
	{
		glEnable(GL_LIGHTING);
		glEnable(GL_COLOR_MATERIAL);
		glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
		glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_FALSE);
		glMaterialfv(GL_FRONT, GL_SPECULAR, black);

		if (useSpecular)
		{
			glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 40.0);
			glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, white);
			glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, black);
		}
	}
	else
	{
		glDisable(GL_LIGHTING);
	}
	glDisable(GL_BLEND);
	glDisable(GL_LINE_SMOOTH);
	glDepthMask(GL_TRUE);

	if (Particle::currentFrame != frame)
	{
		Particle::setup = false;
		Particle::currentFrame = frame;
	}

	if (frame != -1) // if we have a valid frame, issue rendering commands
	{
		static bool bounded;
		triple C, position;
		if (useParticles)
			Particle::showParticle = true;
		else
			Particle::showParticle = false;

		master.Lock();
		if (maxframes <= 0) { master.Unlock(); return; }

		// If size of set has changed, resize vertex buffers/arrays
		if (vBuf.GetCapacity() != GetMaxParticles())
		{
			failed_resize_attempt = false;
			vBuf.SetCapacity(GetMaxParticles());			
			vBuf_trans.SetCapacity(GetMaxParticles());
			int vector_size = max(32*particle_count + 256 + maxframes, 256 + 4 * maxframes);
			if (!vectorBuf.SetCapacity(vector_size))
					failed_resize_attempt = true;
			Particle::setup = false;

			sphere_renderer.SetSize(GetMaxParticles());
			trans_sphere_renderer.SetSize(GetMaxParticles());
		}

		if (vectorBuf.GetCapacity() < 2 * marked_count * Particle::trailLength && !failed_resize_attempt)
		{
			if (!vectorBuf.SetCapacity(2 * marked_count * Particle::trailLength))
			{
				failed_resize_attempt = true;
				vectorBuf.SetCapacity(max(32*particle_count + 256 + maxframes, 256 + 4 * maxframes));
			}
			Particle::setup = false;
		}

		for (int i = 0; i < particle_count; i++)					
			current_visibility[i] = true;
		CalcVisibility(); // calculate which particles are to be rendered, and their transparency state

		if (!Particle::setup)
			DrawParticles();

		if (Particle::showParticle) // Draw particle instances.
		{
			// Draw the static geometry first
			if (static_geometry.IsMarked() != MARK_HIDE_MODE)			
				static_geometry.DrawInstance(0);			

			if (usePoints) // draw point primitives
			{
				glDisable(GL_TEXTURE_2D);
				glDisable(GL_LIGHTING);
				glColor4f(1, 1, 1, alpha);

				sphere_renderer.SetPointPrimitives(true);
				trans_sphere_renderer.SetPointPrimitives(true);
			}
			else // draw regular particles
			{
				sphere_renderer.SetPointPrimitives(false);
				trans_sphere_renderer.SetPointPrimitives(false);
				glColor4f(1, 1, 1, 1);
			}

			if (useQueries)
			{
				opaque_renderer.EnableQueries();
				map_renderer.EnableQueries();
				//occ_renderer.EnableQueries(); // experimental
			}
			else
			{				
				opaque_renderer.DisableQueries();
				map_renderer.DisableQueries();
			}

			glColor4f(1, 1, 1, 1);

			#ifdef BENCHMARK
			QueryPerformanceCounter(&prev);
			#endif

			// Render using point sprites or shaded spheres:
			if (usePoints || useSphereShader)
				sphere_renderer.Render();
				//occ_renderer.Render(); // experimental
			opaque_renderer.Render();
			glEnable(GL_TEXTURE_2D);
			if (use_surfacemaps)
				map_renderer.Render();

			#ifdef BENCHMARK
			QueryPerformanceCounter(&next);
			drawtime = ((double)(next.QuadPart - prev.QuadPart)/(double)freq.QuadPart);
			msg << "Draw: " << setiosflags(ios_base::fixed) << setw(12) << setprecision(4) << drawtime;
			prev = next;
			#endif
		}
		
		if (useTransparency || showCone) // Draw transparent particle instances.
		{
			glEnable(GL_CULL_FACE);
			glDepthMask(GL_FALSE);
			glEnable(GL_BLEND);

			if (useTextures)
				glEnable(GL_TEXTURE_2D);
			else
				glDisable(GL_TEXTURE_2D);
			
			// all particles to be drawn transparently are stacked in "transparent_renderer" or "trans_sphere_renderer"
			if (!useAlphaSort)
			{
				glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			}
			else // sort particles by depth before rendering
			{
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				transparent_renderer.SortByDistance(Particle::currentCam, Particle::currentCamDirection);
				trans_sphere_renderer.SortByDistance(Particle::currentCam, Particle::currentCamDirection);
			}
			
			Particle::alpha = alpha;
			transparent_renderer.SetAlpha(alpha);
			transparent_renderer.Render();
			if (usePoints || useSphereShader)
			{
				if (usePoints && Particle::quality >= 4) // at highest quality, enable point smoothing.
				{
					glEnable(GL_POINT_SMOOTH);	
					trans_sphere_renderer.Render();
					glDisable(GL_POINT_SMOOTH);
				}
				else
					trans_sphere_renderer.Render();
			}

			if (showCone)
			{
				glEnable(GL_CULL_FACE);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glColor4f(1, 0, 0, alpha*2);
				glCullFace(GL_FRONT);
				glCallList(conelist);
				glCullFace(GL_BACK);
				glCallList(conelist);
			}
			
			glDisable(GL_CULL_FACE);
			glDepthMask(GL_TRUE);
			glDisable(GL_BLEND);
			glColor4f(1, 1, 1, 1);
		}
	}

	// Now draw metaparticle information, velocity, pathing, etc.
	DrawVectors();
	master.Unlock();
	if (!Particle::setup)	
		Particle::setup = true;

	#ifdef BENCHMARK
	glFinish();
	QueryPerformanceCounter(&next);	
	rendertime = ((double)(next.QuadPart - prev.QuadPart)/(double)freq.QuadPart);
	msg << " Render: " << setiosflags(ios_base::fixed) << setw(12) << setprecision(4) << rendertime;
	msg << " Ratio: " << setiosflags(ios_base::fixed) << setw(9) << setprecision(3) << (drawtime / rendertime);
	message = msg.str();
	#endif
}

// Draw vector informations
void ParticleSet::DrawVectors()
{
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	if (Particle::quality >= 3)
	{
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_BLEND);
		glDepthMask(GL_FALSE);
	}

	if (!Particle::setup)
	{
		if (Particle::quality >= 0)
		{
			//set up vertex/color buffers
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			glDisableClientState(GL_NORMAL_ARRAY);
			glDisable(GL_LIGHTING);
			if (Particle::quality >= 3)
			{
				glEnable(GL_LINE_SMOOTH);
				glEnable(GL_BLEND);
				glDepthMask(GL_FALSE);
			}
			glLineWidth(Particle::lineWidth);
			if (vectorBuf.GetSize() > 0)
			{
				vectorBuf.Latch();			
				vectorBuf.Bind();						
				vectorBuf.Draw(); // draws the lines
			}			
		}
	}
	else
	{
		// set state, draw vector buffer
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
		glDisable(GL_LIGHTING);
		glLineWidth(Particle::lineWidth);
		vectorBuf.Draw();
	}

	vector_renderer.Render();
	vectorSet->DrawInstance(currentframe);

	if (Particle::quality >= 3)
	{
		glDisable(GL_LINE_SMOOTH);
		glDisable(GL_BLEND);
		glDepthMask(GL_TRUE);
	}
}

void ParticleSet::NextFrame() // Advance current frame
{
	currentframe = min(maxframes, currentframe+1);
}

void ParticleSet::PreviousFrame() // Set current frame to previous
{
	currentframe = max(0, currentframe-1);
}

void ParticleSet::MarkAll()
{
	for (int p = 0; p < particle_count; p++)
	{
		particles[p]->MarkParticle(MARK_HIGHLIGHT_MODE);
	}
	Particle::setup = false;
}

void ParticleSet::UnMarkAll()
{
	for (int p = 0; p < particle_count; p++)
	{
		particles[p]->MarkParticle(MARK_UNMARKED);
	}
	Particle::setup = false;
}

// Mark particle at X/Y screen coordinate
int ParticleSet::MarkParticle(int x, int y, int frame, int mode)//, Camera* cam)
{		
	bool temp = Particle::showParticle;
	bool temp_trans = useTransparency;
	useTransparency = false;
	Particle::showParticle = true;
    int totalP = particle_count;

	if (totalP==0) return -1;
	if (maxframes==0) return -1;

	int currentP = 0;
	//GLint maxnames = 64;
	bool alreadymarked = false;
	int tomark = -1;
	int i;
	GLint viewport[4];	

	glClear(GL_DEPTH_BUFFER_BIT);
	glGetIntegerv(GL_VIEWPORT, viewport);
	GLuint selectbuf[1020];
	glSelectBuffer(1020, selectbuf);

	glRenderMode(GL_SELECT);
	glDisable(GL_LIGHTING);	
	glDisable(GL_BLEND);
	glDisable(GL_POINT_SMOOTH);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPickMatrix((double)x, (double)(viewport[3]-y), 3.0, 3.0, viewport);
	cam->Draw();

	glInitNames();	
	glPushName( (GLuint)0 );

	glColor4f(1, 1, 1, 1);
	// Load the name onto the selection stack, render a single particle, repeat
	for (int i = 0; i < particle_count; i++)					
		current_visibility[i] = true;
	CalcVisibility();

	Frame current = data_store->GetFrame(currentframe);
	triple * pos = current.pos;
	quad * axis = current.axis;

	glMatrixMode(GL_MODELVIEW);	
	for (int currentP = 0; currentP < totalP; currentP++)
	{
		if (current_visibility[currentP])
		{
			glLoadName((GLuint)currentP);
			glPushMatrix();
			glTranslatef(pos->x, pos->y, pos->z);
			glRotatef(axis->w, axis->x, axis->y, axis->z);
			particles[currentP]->DrawInstance(currentframe);
			glPopMatrix();
		}
		pos++;
		axis++;
	}

	int hitnumber = glRenderMode(GL_RENDER);
	GLuint *ptr = selectbuf;
	int names = 1, hit_name = -1, nearest = 0xffffffff;

	for(int i = 0; i < hitnumber; i++) // check each hitrecord, select the nearest one.
	{
		names = *ptr++;	// should always be 1 for us
		if (names < 1) continue;
		if (*ptr < nearest)
		{
			nearest = *ptr; // get Z-near
			ptr++; ptr++;	// discard other data
			for(int j = 0; j < names; j++)
			{
				hit_name = *ptr++;
			}
			if (hit_name < 0) continue;
			if (particles[hit_name]->IsMarked()!=MARK_UNMARKED)
			{
				alreadymarked = true;
				// maybe some sort of cycling action here would be good
				tomark = hit_name;
			}
			else
			{
				alreadymarked = false;
			}
		}
		else
		{
			ptr+=names+2;
		}
	}
	// just use nearest hitrecord name
	if (hitnumber > 0 && hit_name != -1)
	{
		if (alreadymarked)
			MarkParticle(hit_name, MARK_UNMARKED);
		else
			MarkParticle(hit_name, mode);
	}

	Particle::showParticle = temp;
	glEnable(GL_LIGHTING);

	Particle::setup = false;
	useTransparency = temp_trans;
	return hit_name;
}

// Mark particle by ID.
void ParticleSet::MarkParticle(int particle, int mode)
{
	if (particle < particle_count && particle >= 0)
		particles[particle]->MarkParticle(mode);
	Particle::setup = false;
}

// Clear all particles.  A critical section, lock before calling.
void ParticleSet::ClearParticles()
{
	int temp_count = particle_count;
	particle_count = 0;
	maxframes = 0;
	if (particles != NULL)
	{
		for (int i = 0; i < temp_count; i++)
			delete particles[i];
		delete [] particles;
		transparent.clear();
	}
	particles = NULL;
	particle_count = -1;
	maxIndex = -1;
	frameTimes.clear();	
	maxTransVelocity =  maxRotVelocity = 0;
	use_surfacemaps = use_colormaps = false;
}

// Load in a group of particle objects, with an associated store
void ParticleSet::LoadParticles(Particle** buf, ParticleStore * store, int count)
{
	data_store = store;
	if (count < 0) count = 0;
	if (particles != NULL)
	{
		for (int i = 0; i < particle_count; i++)
		{
			delete particles[i];
		}
		delete [] particles;
		delete [] current_visibility;
		delete [] current_distances;
		transparent.clear();
	}
	if (count==0)
		maxframes = 0;
	else
		maxframes = buf[0]->GetFrames();
	particles = buf;
	particle_count = count;

	maxIndex = 32 * count + 255;
	maxTransVelocity =  maxRotVelocity = 0;
	current_visibility = new bool[count];
	draw_opaque_particle = new bool[count];
	current_distances = new float[count];

	for (int i = 0; i < count; i++)
	{
		if (particles[i]->GetMaxVelocity() > maxTransVelocity)
			maxTransVelocity = particles[i]->GetMaxVelocity();
        if (particles[i]->GetMaxRVelocity() > maxRotVelocity)
			maxRotVelocity = particles[i]->GetMaxRVelocity();
		current_visibility[i] = true;
		draw_opaque_particle[i] = true;
	}
}

// Enable a bound (axis aligned clipping plane)
void ParticleSet::SetBound(int dimension, double min, double max)
{
	useBound[dimension] = true;
	bounds[dimension*2] = min;
	bounds[(dimension*2)+1] = max;
}

// Retrieve an internal option
bool ParticleSet::GetOption(int option)
{
	switch (option)
	{
	case TEXTURING:
		return useTextures;
	case LIGHTING:
		return useLighting;
	case R_SPECULAR:
		return useSpecular;
	case R_PARTICLES:
		return useParticles;
	case R_VELOCITY:
		return useVelocity;
	case R_USEPOINTS:
		return usePoints;
	case R_ROTVELOCITY:
		return useRotVelocity;
	case BOUNDCHECKX:
		return useBound[X];
	case BOUNDCHECKY:
		return useBound[Y];
	case BOUNDCHECKZ:
		return useBound[Z];
	case R_TRANSPARENT:
		return useTransparency;
	case R_ALPHASORT:
		return useAlphaSort;
	case R_SPHERESHADER:
		return useSphereShader==1;
		break;
	case R_SIMPLESHADER:
		return useSphereShader==2;
		break;
	case TRANSBOUNDS:
		return useTransparentBounds;
		break;
	case MARKBOUNDS:
		return useBoundmarking;
		break;
	}
	return false;
}

// Set an internal option
void ParticleSet::SetOption(int option, bool value)
{
	// These options affect the set's rendering state
	switch (option)
	{
	case TEXTURING:
		useTextures = value;
		break;
	case LIGHTING:
		useLighting = value;
		break;
	case R_SPECULAR:
		useSpecular = value;
		break;
	case R_PARTICLES:
		useParticles = value;
		break;
	case R_VELOCITY:
		useVelocity = value;		
		break;
	case R_USEPOINTS:
		usePoints = value;
		//Particle::setup = false;
		break;
	case R_ROTVELOCITY:
		useRotVelocity = value;
		Particle::rotVector = value;		
		break;
	case BOUNDCHECKX:
		useBound[X] = value;
		break;
	case BOUNDCHECKY:
		useBound[Y] = value;
		break;
	case BOUNDCHECKZ:
		useBound[Z] = value;
		break;
	case R_TRANSPARENT:
		useTransparency = value;
		break;
	case R_ALPHASORT:
		useAlphaSort = value;
		break;
	case R_SPHERESHADER:
		useSphereShader = value ? 1 : 0;
		break;
	case R_SIMPLESHADER:
		useSphereShader = value ? 2 : 0;
		break;
	case TRANSBOUNDS:
		useTransparentBounds = value;
		break;
	case MARKBOUNDS:
		useBoundmarking = value;
		break;
	}
	Particle::setup = false;
}

// Set a default coloring mode for the particles
void ParticleSet::SetColorMode(int mode)
{
	colorMode = mode;	
	switch (colorMode)
	{
	case COLORVEL:
		current_coloring = &trans_coloring;
		break;
	case COLORRVEL:
		current_coloring = &rot_coloring;
		break;
	case COLORPOS:
		current_coloring = &pos_coloring;
		break;
	case COLORMAP:
	case COLORWHITE:
		if (use_colormaps)
			current_coloring = &colormapping;
		else
			current_coloring = &white_coloring;
		break;
	}
}

// Draw a cone shape from "tip" to "base", where the radius at "base" is "baseradius" (rarely used)
void ParticleSet::DrawCone(triple tip, triple base, double angle)
{
	int slices = 4;
	switch (Particle::quality)
	{
	case 0:
		slices = 16;
		break;
	case 1:
		slices = 24;
		break;
	case 2:
		slices = 32;
		break;
	case 3:
		slices = 48;
		break;
	case 4:
		slices = 64;
		break;
	}

	double theta = 0, thetaD = 0, minh = 0, h = 0, hD = 0;
	double r, s, t;
	double x2, y2, z2;
	double t2, slope, stemp;
	thetaD = TWOPI / (double)slices;

	r = 0;

	theta = TWOPI;
	glBegin(GL_TRIANGLE_FAN);
	triple n, u, v; // normal, planar vectors
	triple up(0, 1, 0), current;
	n.x = (tip.x - base.x);
	n.y = (tip.y - base.y);
	n.z = (tip.z - base.z);
	double len = length(n);
	normalize(n);
	glNormal3f(n.x, n.y, n.z);
	glVertex3f(tip.x, tip.y, tip.z);

	r = tan(angle/2.0 / 360.0 * TWOPI) * len;

	// check for special case (normal = Y)
	if (n.x == 0 && n.z == 0)
	{
		u.x = 1; u.y = 0; u.z = 0;
		v.x = 0; v.y = 0; v.z = 1;
	}
	else
	{	// get U and V on the proper plane for our planar circle
		u = crossProduct(up, n);
		v = crossProduct(u, n);
		normalize(u);
		normalize(v);
	}

	for (int j = 0; j <= slices; j++)
	{
		current.x = r * cos(theta) * u.x + r * sin(theta) * v.x + base.x;
		current.y = r * cos(theta) * u.y + r * sin(theta) * v.y + base.y;
		current.z = r * cos(theta) * u.z + r * sin(theta) * v.z + base.z;

		triple L = IntersectLineCylinder(tip, current, drumR);		

		n = current;
		n.x -= base.x;
		n.y -= base.y;
		n.z -= base.z;
		normalize(n);
		glNormal3f(n.x, n.y, n.z);
		glVertex3f(L.x, L.y, L.z);

		theta-=thetaD;
	}
	glEnd();
}

// Intersection function between lines and cylinders: used to hack in spray cones
triple ParticleSet::IntersectLineCylinder(triple p1, triple p2, double radius)
{
	double a, b, c, d;
	a = SQR(p2.x - p1.x) + SQR(p2.y - p1.y);
	b = 2.0 * (((p2.x - p1.x)*p1.x) +  ((p2.y - p1.y)*p1.y));
	c = SQR(p1.x) + SQR(p1.y) - SQR(radius);
	d = SQR(radius + 0);
	
	double s = (-b + sqrt( SQR(b) - (4.0 * a * c))) / (2.0 * a);
	triple result(0,0,0);
	result.x = p1.x + (s * (p2.x - p1.x));
	result.y = p1.y + (s * (p2.y - p1.y));
	result.z = p1.z + (s * (p2.z - p1.z));
	return result;
}

// Apply a color grid to the particle set
void ParticleSet::AddGrid(double x, double y, int axis, triple color1, triple color2)
{
	triple check, color = Particle::markingColor;
	double R;
	int lastframe = 0, thisframe = 1;
	bool *ptest = new bool[particle_count];
	for(int i = 0; i < particle_count; i++)
	{
		ptest[i] = false;
		particles[i]->ClearColors();
	}

	if (x < 1e-14 || y < 1e-14) // test for less than epsilon
		return;

	double max_pos = Particle::GetMaxPosition();
	if (x > 0)
	{
		for(double A = -max_pos; A <= max_pos; A+=x)
		{
			for(int i = 0; i < particle_count; i++)
			{
				R = particles[i]->GetRadius();
				check = particles[i]->GetPosition(currentframe);
				switch (axis)
				{
				case X:
					if (fabs(check.y - A) <= R)
					{
						ptest[i] = true;
					}
					break;
				case Y:
					if (fabs(check.x - A) <= R)
					{
						ptest[i] = true;
					}
					break;
				case Z:
					if (fabs(check.x - A) <= R)
					{
						ptest[i] = true;
					}
					break;
				}
			}
		}
	}

	if (y > 0)
	{
		for(double A = -max_pos; A <= max_pos; A+=y)
		{
			for(int i = 0; i < particle_count; i++)
			{
				R = particles[i]->GetRadius();
				check = particles[i]->GetPosition(currentframe);
				switch (axis)
				{
				case X:
					if (fabs(check.z - A) <= R)
					{
						ptest[i] = true;
					}
					break;
				case Y:
					if (fabs(check.z - A) <= R)
					{
						ptest[i] = true;
					}
					break;
				case Z:
					if (fabs(check.y - A) <= R)
					{
						ptest[i] = true;
					}
					break;
				}
			}
		}
	}


	for(int i = 0; i < particle_count; i++)
	{
		if (ptest[i]) 
		{
			particles[i]->SetColor(color1);
			particles[i]->canTransparent = false;
		}
		else
		{
			particles[i]->SetColor(color2);
			particles[i]->canTransparent = true;
		}
	}
	delete [] ptest;
	EnableColorMapping();
}

// Apply a color split to the particle set
void ParticleSet::AddSplit(int axis, triple origin, triple color1, triple color2)
{
	triple check;	
	for(int i = 0; i < particle_count; i++)
	{
		particles[i]->ClearColors();
		check = particles[i]->GetPosition(currentframe) - origin;
		float check_dim;
		switch (axis)
		{
		case X:
			check_dim = check.x;
			break;
		case Y:
			check_dim = check.y;
			break;
		case Z:
			check_dim = check.z;
			break;
		}

		if (check_dim > 0)	
			particles[i]->SetColor(color1);		
		else		
			particles[i]->SetColor(color2);		
	}
	EnableColorMapping();
}

// Clear all particle colors
void ParticleSet::ClearColor(void)
{
	for(int i = 0; i < particle_count; i++)
	{
		particles[i]->ClearColors();
		particles[i]->canTransparent = true;
	}
	use_colormaps = false;
	Particle::setup = false;
}

// Return the maximum translational velocity
double ParticleSet::GetMaxVelocity()
{
	for (int i = 0; i < particle_count; i++)
	{
		if (particles[i]->GetMaxVelocity() > maxTransVelocity)
			maxTransVelocity = particles[i]->GetMaxVelocity();
	}
	return maxTransVelocity;
}

// Return the maximum angular velocity
double ParticleSet::GetMaxRVelocity()
{
	for (int i = 0; i < particle_count; i++)
	{
        if (particles[i]->GetMaxRVelocity() > maxRotVelocity)
			maxRotVelocity = particles[i]->GetMaxRVelocity();
	}
	return maxRotVelocity;
}

// Copy a statefile frame to the Windows clipboard
void ParticleSet::CopyFrameToString(int frame, stringstream& output)
{
	if (frame < 0 || frame > maxframes)
		return;

	triple joe;
	quad frank;
	double c_theta, s_theta;
	output << "*\t" << GetTime(frame) << endl;
	for (int i = 0; i < particle_count; i++)
	{
		joe = particles[i]->PeekVector(frame, POSITION);
		output << joe.x << " " << joe.y << " " << joe.z << " ";
		// convert from axis angle to quaternion
		{
			frank = particles[i]->PeekOrientation(frame); // axis-angle
			c_theta = cos((frank.w/180.0*PI) / 2.0);
			s_theta = sin((frank.w/180.0*PI) / 2.0);
			output << c_theta << " " << frank.x * s_theta << " " << frank.y  * s_theta << " " << frank.z  * s_theta << " ";
		}

		joe = particles[i]->PeekVector(frame, POSVELOCITY);
		output << joe.x << " " << joe.y << " " << joe.z << " ";

		joe = particles[i]->PeekVector(frame, ROTVELOCITY);
		output << joe.x << " " << joe.y << " " << joe.z << endl;
	}
}

// Draw a set of axes in the scene, given size D and alpha transparency A
void ParticleSet::DrawAxes(double D, double A)
{
	glBegin(GL_LINES);
		glColor4d(0, 0, 1, A); // blue = X
			glVertex3d(0, 0, 0);	glVertex3d(1.0*D, 0, 0);
			glVertex3d(1*D, 0, 0);	glVertex3d(0.95*D, 0.05*D, 0);
			glVertex3d(1*D, 0, 0);	glVertex3d(0.95*D, -.05*D, 0);

			glVertex3d(1.02*D, 0.0*D, 0);	glVertex3d(1.04*D, 0.02*D, 0);
			glVertex3d(1.02*D, 0.02*D, 0);	glVertex3d(1.04*D, 0.0*D, 0);

		glColor4d(1, 0, 0, A);	// red = Y
			glVertex3d(0, 0, 0);	glVertex3d(0, 1.0*D, 0);
			glVertex3d(0, 1*D, 0);	glVertex3d(-.05*D, 0.95*D, 0);
			glVertex3d(0, 1*D, 0);	glVertex3d(0.05*D, 0.95*D, 0);

			glVertex3d(0, 1.04*D, 0);	glVertex3d(-0.01*D, 1.05*D, 0);
			glVertex3d(0, 1.04*D, 0);	glVertex3d(0.01*D, 1.05*D, 0);
			glVertex3d(0, 1.02*D, 0);	glVertex3d(0, 1.04*D, 0);

		glColor4d(0, 1, 0, A); // green = Z
			glVertex3d(0, 0, 0);	glVertex3d(0, 0, 1.0*D);
			glVertex3d(0, 0, 1*D);	glVertex3d(0, 0.05*D, 0.95*D);
			glVertex3d(0, 0, 1*D);	glVertex3d(0, -.05*D, 0.95*D);

			glVertex3d(0, 0.0*D, 1.03*D);	glVertex3d(0, 0.0*D, 1.05*D);
			glVertex3d(0, 0.0*D, 1.05*D);	glVertex3d(0, 0.02*D, 1.03*D);
			glVertex3d(0, 0.02*D, 1.03*D);	glVertex3d(0, 0.02*D, 1.05*D);
	glEnd();
}

// Invert the marked set of particles to unmarked, and vice versa
void ParticleSet::InvertSelection(void)
{
	for (int i = 0; i < particle_count; i++)
	{
		if (particles[i]->IsMarked() == MARK_UNMARKED)
		{
			particles[i]->MarkParticle(MARK_HIGHLIGHT_MODE);
		}
		else
		{
			particles[i]->MarkParticle(MARK_UNMARKED);
		}
	}
	Particle::setup = false;
}

/* View frustum culling.  Has a bug, not currently used.
void ParticleSet::ViewFrustumClipping()
{
	triple centers[4], normals[4];
	if (cam->getFrustumPlanes(centers, normals))
	{
	for (int i = 0; i < particle_count; i++)
	{
		triple pos = particles[i]->GetPosition(currentframe);
		float radius = particles[i]->GetRadius();
		current_visibility[i] = true;
		for (int j = 0; j < 4; j++)
		{
			if (dotproduct(normals[j], (pos - centers[j])) < -radius)
			{
				current_visibility[i] = false;
				break;
			}
		}
	}
	}
}
*/

// Writes the default color of a particle to C, given velocities.
void ParticleSet::GetColor(const triple &deltapos, const triple &deltatheta, triple& C)
{
	double speed;
	// Get Color
	if (current_particle->marked != MARK_UNMARKED)
	{
		C = triple(1, 0, 0);
	}	
	else if (current_particle->ColorOverride)
	{
		C = current_particle->rgb;
	}
	else
	{
		switch (colorMode)
			{
			case COLORVEL:
				speed = length(deltapos) / Particle::maxVelocityMap;
				Particle::grad.ConvertToColor(speed, C); // load gradient color into C
				break;
			case COLORRVEL:
				speed = length(deltatheta) / Particle::maxRVelocityMap;
				Particle::grad.ConvertToColor(speed, C); // load gradient color into C
				break;
			case COLORWHITE:
			case COLORMAP:
				if (current_particle->colorMap.size() > currentframe)
					C = current_particle->colorMap[currentframe];
				else
					C = triple(1, 1, 1);
				break;
			}
	}
}

// Recalculate the distance from camera origin for each particle
void ParticleSet::ComputeDistances() 
{	
	triple * positions = data_store->GetFramePos(currentframe);
	float * distances = current_distances;

	for (int i = 0; i < particle_count; i++)
	{
		static triple L;
		L = *positions;
		L -= Particle::currentCam;
		//*distances = dotproduct(L, Particle::currentCamDirection);

		particles[i]->camera_distance = dotproduct(L, Particle::currentCamDirection);

		positions++; // move pointers to next members
		distances++;
	}

}

// Compute visibility information for the entire particle set
void ParticleSet::CalcVisibility()
{
	// Iterate over all the particles in the frame, calculating visibility/transparency information
	Frame current = data_store->GetFrame(currentframe);
	triple * pos = current.pos;
	bool bounding_enabled = useBound[X] || useBound[Y] || useBound[Z];
	for (int p = 0; p < particle_count; p++)
	{		
		current_particle = particles[p];
		bool bounded = false;
		int markmode = current_particle->IsMarked();
		if (bounding_enabled)
		{					
			if (useBound[X])
				if (pos->x < bounds[0] || pos->x > bounds[1])
					bounded = true;
			if (useBound[Y])
				if (pos->y < bounds[2] || pos->y > bounds[3])
					bounded = true;
			if (useBound[Z])
				if (pos->z < bounds[4] || pos->z > bounds[5])
					bounded = true;
			current_visibility[p] = !bounded || (useTransparency && useTransparentBounds);
			// If the particle is outside bounds, turn visibility off, or draw as transparent
		}

		if (useBoundmarking && !bounded)
			current_particle->MarkParticle(MARK_HIGHLIGHT_MODE);
			
		if (markmode==MARK_HIGHLIGHT_MODE) // marked particles are always visible
			current_visibility[p] = true;
		else if (markmode==MARK_HIDE_MODE) // hidden particle are always... hidden
			current_visibility[p] = false;

		// "useTransparency" - transparency rendering is on
		// "bounded" - if the current particle has failed occlusion checks, this is true
		draw_opaque_particle[p] =	(!useTransparency) || // if transparency is enabled, draw as transparent particle
									(markmode!=MARK_UNMARKED) ||  // marked particles always opaque
									(useTransparentBounds && !bounded) || // particles inside XYZ bounds are opaque when "transbound" is on
									!(current_particle->canTransparent); // "canTransparent" is marked false, always opaque
		pos++;
	}

}

// Primary particle draw function: load all relevant data into renderers.
void ParticleSet::DrawParticles()
{
	Frame current = data_store->GetFrame(currentframe);
	triple * pos = current.pos;
	quad * axis = current.axis;
	triple * d_pos = current.d_pos;
	triple * d_angle = current.d_angle;

	PrepRenderer(opaque_renderer);
	PrepRenderer(transparent_renderer);
	PrepRenderer(sphere_renderer);
	PrepRenderer(trans_sphere_renderer);
	vector_renderer.Clear();
	vector_renderer.SetSize(min(18*particle_count, 6*1024*1024)); // can run out of memory here!
	vector_renderer.SetColorClass(current_coloring);

	if (use_surfacemaps)
		PrepRenderer(map_renderer);
	vectorBuf.Clear();
	trans_sphere_renderer.SetAlpha(alpha);
	marked_count = 0;

	for (int p = 0; p < particle_count; p++)
	{
		double current_alpha = 1.0;
		current_particle = particles[p];
		int markmode = current_particle->IsMarked();
		bool pointmode = usePoints || useSphereShader && current_particle->GetPureSphere();
		triple C;

		if (current_visibility[p]) 
		{
			if (Particle::showParticle)
			{
				if (draw_opaque_particle[p]) // the particle is to be drawn as an opaque object
				{
					if (pointmode) // copy data to buffer for point rendering
						sphere_renderer.Load(pos, axis, d_pos, d_angle, current_particle);
						//occ_renderer.Load(pos, axis, d_pos, d_angle, current_particle); // experimental
					else if (current_particle->IsMapped())
						map_renderer.Load(pos, axis, d_pos, d_angle, current_particle);
					else
						opaque_renderer.Load(pos, axis, d_pos, d_angle, current_particle);			
				}
				else if (useTransparency) // if transparency is on then draw particles transparently (except specially marked particles)
				{					
					if (pointmode)									
						trans_sphere_renderer.Load(pos, axis, d_pos, d_angle, current_particle);
					else
						transparent_renderer.Load(pos, axis, d_pos, d_angle, current_particle);
				}
			}

			// Draw all enabled vectors into the vector_renderer
			if (useVelocity)
			{
				vector_renderer.SetType(POSVELOCITY);
				vector_renderer.Load(pos, axis, d_pos, d_angle, current_particle);
			}
			if (useRotVelocity)		
			{
				vector_renderer.SetType(ROTVELOCITY);
				vector_renderer.Load(pos, axis, d_pos, d_angle, current_particle);
			}
			if (Particle::normalVector)
			{
				vector_renderer.SetType(NORMALVEC);
				vector_renderer.Load(pos, axis, d_pos, d_angle, current_particle);
			}
			if (markmode == MARK_HIGHLIGHT_MODE)
			{
				current_particle->DrawPath(currentframe, vectorBuf);
				marked_count++;
			}
		}
		// increment particle data pointers
		pos++;
		axis++;
		d_pos++;
		d_angle++;
	}
}

void ParticleSet::PrepRenderer(Renderer& rend) // Prepare a renderer for use this frame
{
	rend.Clear();
	rend.SetSize(particle_count);
	rend.SetColorClass(current_coloring);
}

void ParticleSet::SetShader(string vert, string frag, bool velocity) // Load GLSL shaders
{
	sphere_renderer.SelectShader(vert, frag);
	trans_sphere_renderer.SelectShader(vert, frag);
	if (velocity) // pass velocity
	{
		sphere_renderer.EnableVelocity();
		trans_sphere_renderer.EnableVelocity();
	}
	else
	{
		sphere_renderer.DisableVelocity();
		trans_sphere_renderer.DisableVelocity();
	}
}

bool ParticleSet::IsVisible(int PID) // Accessor for current visibility information
{
	if (PID >= 0 && PID < particle_count)
		return current_visibility[PID];
	else
		return false;
}