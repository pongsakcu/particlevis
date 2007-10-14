/*

Particle

		   The Particle class contains all the information relating to the state of a particle
		inside the visualization.  Information such as the appearance of the particle(including
		geometry in the form of display lists, whether it is a sphere, and default color),
		the marking state of the particle, and the presence of additional data such as surface
		or color maps.
		   An interface to the ParticleStore class is provided: each particle is able to access
		and modify its own physical state.  The position, orientation (axis-angle representation)
		and two vectors (translational velocity and rotational velocity) are stored for the particle
		at each timestep.

		Vincent Hoon, 2007
		See copyright notice in copyright.txt for conditions of distribution and use.
*/

#pragma once
#include "triple.h"
#include <malloc.h>
#include <memory.h>
#include "stdafx.h"
#include "RGBGradient.h"
#include "GLVertexBuffer.h"
#include <vector>
#include "ParticleStore.h"
#include "GLVertexBuffer.h"
#include "GLMappedSphereBuf.h"
#include "quad.h"

// Constants for different marking modes
#define MARK_UNMARKED -1
#define MARK_HIGHLIGHT_MODE 0
//#define MARK_TRACE_MODE 1
#define MARK_HIDE_MODE 2

// numerical constants
#define PI 3.1415926535897932384626433832795
#define TWOPI 6.283185307179586476925286766559

#define POSVELOCITY 0
#define ROTVELOCITY 1
#define NORMALVEC 2
#define POSITION 3
#define ROTATION 4
#define SURFACEMAP 5
#define FIELDVEC 6

#define LINEAR 0
#define LOG 1
#define QUAD 2

#define COLORWHITE 0
#define COLORMAP 1
#define COLORVEL 2
#define COLORRVEL 3
#define COLORPOS 4

using namespace std;

class Particle
{
	friend class ParticleSet;
	friend class ParticleRenderer;	
	friend class MappedSphereRenderer;
	friend class PBinaryReader;
public:
	Particle(ParticleStore &s = ParticleStore::null_store, double r = 0, int ID = 0);
	~Particle(void);
	bool operator<(Particle &right);
	void SetPID(int id) { PID = id; }

	void DrawInstance(int frame);	// Draws particle, employing display list if loaded
	void DrawGeometry();			// Basic particle geometry
	void DrawPath(const int frame, GLVertexBuffer& vBuf);	// Draws particle path information
	//void DrawVector(const int frame, const int type, GLVertexBuffer& vBuf); // Draws particle vector information
	inline void DrawPoint(int frame);		// Draws particle as point only
	int PreRender();				// Loads geometry into a display list and stores it
	void LoadRender(int displaylist);// Loads an external display list into object
	bool TestBounds(int dimension, float min, float max, int frame);

	int AddFrame(triple position, quad theta, triple pos_dot, triple theta_dot, bool field = false);
	int AddFrame(triple position, triple theta, triple pos_dot, triple theta_dot, bool field = false);
	quad PeekOrientation(int frame);

	void AddColor(int frame, triple rgb, bool persistent = true); // Add color mapping information (random access)
	void ClearColors() { colorMap.clear(); rgb = triple(-1, -1, 1); ColorOverride = false; }
	void MarkParticle(int mode);	// Mark particle for specialized display (ie pathline)

	triple GetPosition(int frame); // peek at particle position in passed frame
	//inline void GetColor(int frame, int colormode, triple& C); // return color at specified frame
	int GetSize() { return maxsize; }
	int GetFrames() { return frames; }
	int GetList() { return displaylist; }
	int GetPID() { return PID; }
	float GetRadius() { return radius; }
	void SetRadius(float r) { radius = r; }
	int IsMarked() { return marked; }
	bool IsMapped() { return mapped_points.size() >= (currentFrame+1); }

	void SetupMatrix(int frame);	// multiplies the current matrix by the proper rotation/translation matrices
	float GetVelocity(int frame);
	float GetRVelocity(int frame);
	triple GetNormal() { return normal[currentFrame]; }
	static float GetMaxPosition() { return maxPosition; }
	static float GetMaxVelocity() { return maxTransVelocity; }
	static float GetMaxRVelocity() { return maxRotVelocity; }
	static float GetMaxFieldVelocity() { return maxFieldVelocity; }	
	static void ResetStats() { maxPosition = maxTransVelocity = maxRotVelocity = maxFieldVelocity = 0; }
	triple PeekVector(int frame, int type);
	void SetVector(int frame, int type, triple input);
	bool GetPureSphere() const { return pureSphere; }

	void SetAsSphere(bool pure) { pureSphere = pure; }

	static int quality;	// level of quality to use in rendering, 0-4
	static int trailLength;	// length of trail, in frames

	static float maxVelocityMap; // maximum velocity, for scaling
	static float maxRVelocityMap; // same as above for rotation	
	static float maxFieldVectorMap; // same as above for rotation	
	
	static float transVelocityLength; // maximum length of translational vectors
	static float rotVelocityLength; // maximum length of rotational vectors
	static float fieldVelocityLength; // maximum length of field vectors

	static float lineWidth, pointSize;

	static bool normalVector; // whether to draw a fixed up vector on each particle
	static bool rotVector; // whether to draw angular velocity on each particle
	static bool showParticle; // whether to draw particle geometry
	static bool setup; // flag for whether particles are ready for rendering	
	static int currentFrame;
	static bool useMarkColor; // indicates if marked particles should be colored differently
	static triple markingColor; // color to use for marking (default red)
	static triple pathColor1, pathColor2; // colors for creating pathline (1 blends to 2)

	static float *map_vertices;
	static float *colors;
	static unsigned int *vertex_order;
	static unsigned int *texture_order;
	static float *current_tex_coords;	

	bool canTransparent;	// flag to allow transparent drawing of the particle (true) or prohibit (false)
	bool canDraw;			// flag to allow drawing of the particle.  switch off to disable the particle

	void SetColor(triple rgb_color);
	//static triple SetColor(int saturation, int brightness, int hue);
	static RGBGradient grad;
	static float alpha;
	static float map_scale; // scaling value for surface map

	static void SetCamera(triple cam, triple target);
	static void SetSurfaceMapSize(int size);
	static void SetSurfaceMapScale(int scale) { surface_map_scale = scale; }
	static int GetSurfaceMapScale() { return surface_map_scale; }
	static int GetSurfaceMapSize() { return surface_map_size; }
	void AddMapPoint(int frame, int panel, double mass); // adds a point to the surface map
	void ClearSurfaceMap();
	//void ComputeDistance();		// function for computing distance from "currentCam"

	triple rgb;	
	bool ColorOverride;	

	float camera_distance;		// current distance from camera origin
	static float maxPosition, maxTransVelocity, maxRotVelocity, maxFieldVelocity;

	static GLMappedSphereBuf sphereMapBuffer;
	// Color mapping
	vector<triple> colorMap;
protected:
	int frames;				// number of currently stored frames
	int maxsize;			// current storage available (increase with increaseSize)
	int marked;	// if particle is marked for tracking
	static int surface_map_size;
	static int surface_map_scale;
	float radius;			// radius of particle
	bool pureSphere;		// pure sphere

	static triple currentCam, currentTarget, currentCamDirection;	

	// define four arrays of three-dimensional values:
	
	/*vector<triple> pos;			// position
	vector<triple> deltapos;	// translational velocity
	vector<triple> deltatheta;	// rotational velocity
	vector<quad> theta;			// quaternion orientation*/

	ParticleStore& data_store;

	vector<triple> normal;		// normal vector
	vector<float*> mapped_points;// surface map values	

	int displaylist;	// buffered OpenGL command-list (PreRender)
	unsigned int PID;
};