/*
ParticleSet

		   The ParticleSet class is designed to manage large collections of particles and provides
		simple methods for rendering the entire collection.  Functions for marking individual
		particles via mouse clicks are also provided (MarkParticle).
		   The particles should be initialized on the heap and then passed into the ParticleSet using
		LoadParticles (count is the number particles).  DrawFrame is the primary function
		for rendering the particle set.


		Vincent Hoon, 2007
		See copyright notice in copyright.txt for conditions of distribution and use.
*/
#pragma once
#include "Particle.h"
#include "Camera.h"
#include <vector>
#include "GeometryEngine.h"
#include "GLVectorSet.h"
#include "GLShader.h"
#include "GLVertexBuffer.h"
#include "GLLineBuffer.h"
#include "ParticleRenderer.h"
#include "TransparentParticleRenderer.h"
#include "SphereRenderer.h"
#include "VectorRenderer.h"
#include "MappedSphereRenderer.h"
#include "QuerySphereRenderer.h"
#include <afxmt.h>
#include <sstream>

#define BOUNDCHECKX 0
#define BOUNDCHECKY 1
#define BOUNDCHECKZ 2
#define TEXTURING 3
#define LIGHTING 4
#define R_PARTICLES 5
#define R_VELOCITY 6
#define R_USEPOINTS 7
#define R_TRANSPARENT 8
#define R_ROTVELOCITY 9
#define R_SPECULAR 10
#define R_ALPHASORT 11
#define R_HIDDEN 12
#define R_SPHERESHADER 13
#define R_SIMPLESHADER 14
#define TRANSBOUNDS 15
#define MARKBOUNDS 16

using namespace std;

struct QueryType
{
	bool mark;
	bool hide;
	bool flag_notrans;
	bool occluded_set;
};

class ParticleSet
{
public:
	ParticleSet(CSemaphore *lock, Camera* cam);
	~ParticleSet(void);

	void DrawFrame(int frame);
	void DrawFrame();
	void NextFrame();
	void PreviousFrame();

	int MarkParticle(int x, int y, int frame, int mode);
	void MarkParticle(int particle, int mode = MARK_UNMARKED);
	void MarkAll();
	void UnMarkAll();
	void LoadParticles(Particle** buf, ParticleStore* store, int count);
	ParticleStore* GetDataStore() { return data_store; }

	int GetMaxFrames() { return maxframes; }
	int GetMaxParticles() { return particle_count; }
	bool IsVisible(int PID);

	void SetBound(int dimension, double min, double max);
	double* GetBounds() { return bounds; }
	void SetTimes(double step, double start, double stop) { timeStep = step; startTime = start; endTime = stop; }
	double GetTimeStep() { return timeStep; }
	double GetStartTime() { return startTime; }
	double GetEndTime() { return endTime; }
	void PushTime(const double time) { frameTimes.push_back(time); }
	double GetTime(int frame) { if (frameTimes.size() > 0) return frameTimes[frame]; else return 0; }
	bool ValidTimes() { return frameTimes.size() > 0; }
	vector<double>::const_iterator GetTime() { return frameTimes.begin(); }

	bool GetOption(int option);
	void SetOption(int option, bool value);
	void SetAlpha(double a) { alpha = a; }
	void SetColorMode(int mode);
	int GetColorMode() { return colorMode; }
	double GetAlpha() { return alpha; }
	Particle* StaticParticle() { return &static_geometry; }	
	void EnableColorMapping() { use_colormaps = true; SetColorMode(colorMode); }
	void EnableSurfaceMapping() { use_surfacemaps = true; }

	bool * current_visibility;
	bool * draw_opaque_particle;
	float * current_distances;
	triple cone1, cone2;
	double coneangle;
	int conelist;
	bool firstDraw, redrawCone, showCone;

	void ClearParticles();	

	Particle ** particles;
	GeometryEngine * PRend;
	GLVectorSet * vectorSet;

	void SetFrameCount(int frame) { maxframes = frame; }
	void CopyFrameToString(int frame, stringstream& output);

	static bool cullFaces;
	static bool useQueries;
	static struct QueryType currentQuery;

	GLVertexBuffer vBuf, vBuf_trans;
	GLLineBuffer vectorBuf;

	Camera * cam;

	static int currentframe;
private:
	int maxframes, listbase, particle_count, marked_count;
	int colorMode;	

	int vIndex, maxIndex;
	bool failed_resize_attempt;
	double bounds[6], alpha;
	bool useBound[3];
	bool useParticles, usePoints, useLighting, useVelocity;
	bool useTextures, useRotVelocity, useSpecular, useAlphaSort;
	bool useTransparency, useTransparentBounds, useBoundmarking;
	bool use_colormaps, use_surfacemaps;
	int useSphereShader;
	vector<double> frameTimes;
	Particle* current_particle;

	double maxTransVelocity, maxRotVelocity;

	Particle static_geometry; // the single "static" particle	
	vector<Particle*> transparent;
	vector<Particle*> spheres;

	double timeStep, startTime, endTime;
	
	CSemaphore *lockdown; // Access semaphore
	ParticleStore *data_store; // The data store

	// Renderer objects
	ParticleRenderer opaque_renderer;
	MappedSphereRenderer map_renderer;
	TransparentParticleRenderer transparent_renderer;
	SphereRenderer sphere_renderer, trans_sphere_renderer;
	QuerySphereRenderer occ_renderer;
	VectorRenderer vector_renderer;	

	// Color classification objects
	RGBClassifier * current_coloring;
	DistClassifier pos_coloring;
	TransVelocityClassifier trans_coloring;
	RotVelocityClassifier rot_coloring;
	WhiteClassifier white_coloring;
	ParticleColorClassifier std_coloring;
	ParticleColorClassifier colormapping;	
	void PrepRenderer(Renderer& rend);

	void DrawVectors();
	void DrawParticles();

	void GetColor(const triple &deltapos, const triple &deltatheta, triple& C);
	void CalcVisibility();	
	void ComputeDistances();
public:
	string message;
	void RenderVectors(const int type, const int count, triple * pos, triple * d_pos, GLVertexBuffer& vBuf);
	triple IntersectLineCylinder(triple p1, triple p2, double radius);
	void DrawAxes(double D, double A);
	void DrawCone(triple tip, triple base, double baseradius);
	void AddGrid(double x, double y, int axis = Z, triple color1 = triple(0, 0, 1), triple color2 = triple(1, 1, 1));
	void AddSplit(int axis = X, triple origin = triple(0, 0, 0), triple color1 = triple(1, 0, 0), triple color2 = triple(0, 0, 1));
	void SetShader(string vert, string frag, bool velocity = false);
	void ClearColor(void);
	double GetMaxVelocity();
	double GetMaxRVelocity();
	Particle* GetParticle(int p) { if (p < particle_count) return particles[p]; else return NULL; }
	void InvertSelection(void);
};