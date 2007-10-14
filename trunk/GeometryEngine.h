/*
GeometryEngine

		   The GeometryEngine implements a set of drawing operations that correspond to the
		primitives specified by XML descriptors.  It allows a series of geometric solids
		to be compiled into a set of OpenGL display lists under several levels of quality.
		   Support is provided for entering in various attributes that affect rendering,
		such as culling front faces and overriding colors.  See the XML specification
		for more information on rendering options.  Affine transformations can be
		loaded in each drawing command.
		   Once "NewList" is called, the "Add..." functions build up the specification until
	    "EndList" is called.  The internal "Render..." functions are then used to build display
		lists, which can be loaded into Particles using "AssignLists."

		Vincent Hoon, 2007
		See copyright notice in copyright.txt for conditions of distribution and use.
*/
#pragma once
#include "stdafx.h"
#include "Particle.h"
#include <vector>
#include "triple.h"
#include "quad.h"
#include <tnt.h>

using namespace std;

class CChildView;

struct geoRecord
{
	int type;
	double data[12];
	triple pos, rot;
	quad quat;
	string strdata;
	bool invert;
	geoRecord(void) { type = 0; for (int i = 0; i < 12; i++) data[i] = 0; invert = false; }
	bool operator==(const geoRecord& r) {	if ( type != r.type ) return false;
										for (int i = 0; i < 12; i++)
											{ if (data[i] != r.data[i]) return false; }
										//if (pos == r.pos) return false;
										//if (rot == r.rot) return false;
										return true; }
};

class PStatefileReader;

class GeometryEngine
{
	friend class PStatefileReader;
public:
	GeometryEngine(void);
	GeometryEngine(CChildView* main_view);
	~GeometryEngine(void);

	void SetView(CChildView* main_view) { view = main_view; }

	void NewList(int index, int count = 1, bool invert = false, int quality = 0, int textures = 3, bool transparent = true);
	int EndList();
	void AssignLists(Particle** p_list, int size);

	void AddSphere(double radius, triple position);
	void AddCylinder(double radius, double height, triple position, triple orientation, quad quat_orient);
	void AddCone(double small_radius, double large_radius, double half_angle, triple position, triple orientation, quad quat_orient);
	void AddPlate(triple corner1, triple corner2, triple corner3, triple corner4, triple position, triple orientation, quad quat_orient);
	void AddCap(double radius, double height, triple position, triple orientation, quad quat_orient);
	void AddFile(string fname, triple position, triple orientation, quad quat_orient);
	void AddOBJ(string fname, double scale, triple position, triple orientation, quad quat_orient);
	void AddLine(triple corner1, triple corner2, triple position, triple orientation, quad quat_orient);
	void AddColor(triple rgb);
	void AddFlip();

	void FlushLists();

	const static int STATIC = -1;
	int GetStaticList() { return static_list; }
	triple GetStaticListColor() { return static_color; }
	static void RenderDefaults();
	static int default_display_lists;

private:
	CChildView* view;
	void CalcMatrix();
	void Point(float x, float y, float z);
	void Normal(float x, float y, float z);
	triple c_pos, c_rot;
	quad quat;

	TNT::Array2D<float> matrix;
	TNT::Array2D<float> vector;
	TNT::Array2D<float> m_temp;
	TNT::Array2D<float> temp;

	void RenderSphere(double radius, int depth);
	void RenderCylinder(double radius, double height, int slices);
	void RenderCone(double small_radius, double large_radius, double half_angle, int slices, int stacks);
	void RenderPlate(triple corner1, triple corner2, triple corner3, triple corner4);
	void RenderCap(double radius, double height, int slices, int stacks);
	void RenderLine(triple p1, triple p2);

	void ReadTriangles(string filename);
	
	
	std::vector<int> display_Lists; // holds all allocated lists, each keyed to a particle reference
	std::vector<int> raw_Lists;	// holds all allocated lists
	std::vector<geoRecord*> geo_Lists;
	std::vector<geoRecord> geo_Stack;
	std::vector<bool> visible; // stores inversion data
	std::vector<bool> canTransparent; // stores "canTransparent" data
	std::vector<bool> spherical; // indicates if geometry is purely a sphere
	std::vector<float> radii; // stores default radii
	std::vector<triple> colors; // for manual color setting
	double default_radius;
	triple c_color;
	int static_list; // list for static geometry
	triple static_color; // color of static geometry
	int sphere_lists; // list for sphere geometry
	int c_Index, c_Count;
	int c_Quality;
	int textured;
	bool inverted, transparent;
};