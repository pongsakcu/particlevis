/*
GLPlanarSlices

		   This class supports rendering of a 3D texture using view-aligned
		texture slices.  GL_TEXTURE_3D is used for resampling the slices, and
		back to front blending of the slices is used.

		Vincent Hoon, 2007
		See copyright notice in copyright.txt for conditions of distribution and use.
*/

#pragma once
#include <vector>
#include "triple.h"
#include "TNT/tnt.h"
#include "Camera.h"
#include "RGBGradient.h"
#include "ParticleStore.h"

using std::string;
using std::vector;

using namespace TNT;

#define BLEND_OVER 0
#define BLEND_ADD 1

class GLPlanarSlices
{
public:
	GLPlanarSlices(int x = 128, int y = 128, int z = 128);
	~GLPlanarSlices(void);
	virtual bool RenderTextures();
	void Draw(Camera * cam);		

	void SetLocal(bool in) { use_local = in; }
	bool GetLocal() { return use_local; }

	void SetLocalAlpha(bool in) { use_local_alpha = in; rendered = false; }
	bool GetLocalAlpha() { return use_local_alpha; }

	void SetDummyData(bool in) { use_dummy = in; rendered = false; }
	bool GetDummyData() { return use_dummy; }

	void SetAlpha(double a) { max_alpha = a; rendered = false; }	
	double GetAlpha() { return max_alpha; }

	void SetBlending(int bmode);
	int GetBlending() { return blend_mode; }

	double GetRange() { return range; }
	void SetRange(double r) { range = r; rendered = false; }

	int GetNumSlices() { return num_slices; }
	void SetNumSlices(int nslice);

	triple GetOrigin() { return origin; }
	triple GetRotation() { return rotation; }
	triple GetScale() { return scale; }

	void SetOrigin(triple o) { origin = o; }
	void SetRotation(triple r) { rotation = r; }
	void SetScale(triple s) { scale = s; }
	void SetSize(int x, int y, int z);

	void Enable(bool e) { enable = e; }
	bool Enabled() { return enable; }

	bool LoadVOLFile(string filename);
	void ComputeDensityMap(ParticleStore * store, int size);
	void ComputeGranularTemp(ParticleStore * store, int size);

protected:
	void SlicePoint(float x, float y, float z);
	virtual void SetTextureMatrix();	

	vector<triple> pointcache;
	float * valuecache;
	triple origin, rotation, scale;
	double range;
	bool enable;

	int XMAX, YMAX, ZMAX;

	RGBGradient * slicegrad;

	unsigned char* tex_array;
	triple view_x, view_y, view_z;

	double max_alpha;
	double max_value;

	double height;
	unsigned int tex3D;
	bool rendered;
	bool use_local, use_local_alpha, use_dummy;
	int blend_mode;
	int num_slices;

	void swap(unsigned char* bytes, int length);
};
