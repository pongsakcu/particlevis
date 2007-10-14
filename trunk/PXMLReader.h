/*
PXMLReader

		  This class parses XML descriptor files using the Expat XML parsing library.
		It interfaces with both ParticleSet and GeometryEngine to create and load a
		set of OpenGL display lists containing the described particle geometry.  It also
		collaborates with the PSurfaceReader to load in surface map data.

		Vincent Hoon, 2007
		See copyright notice in copyright.txt for conditions of distribution and use.
*/

#pragma once
#include "PFileReader.h"
#include "PSurfaceReader.h"
#include <expat.h>
#include "triple.h"
#include "quad.h"
#include <vector>
#include <string>
using std::vector;
using std::string;

class PXMLReader : public PFileReader
{
public:
	PXMLReader(CSemaphore* lock = NULL);
	~PXMLReader(void);

	bool LoadFile(ParticleSet *target, string filename);
	virtual void Terminate() { surface_mapper.Terminate(); }

	bool IsFinished() { return (finish && surface_mapper.IsFinished()); }

protected:
	CSemaphore* particle_lock;
	static void StartElement(void *userData, const XML_Char *name, const XML_Char **atts);
	static void EndElement(void *userData, const XML_Char *name);
	static void CharHandler(void *userData, const XML_Char *s, int len);

	void ParseStartElement(const XML_Char *name, const XML_Char **atts);
	void ParseEndElement(const XML_Char *name);
	void ParseCharHandler(const XML_Char *s, int len);

	int LookupTag(const char *name);

	vector<double> c_values;
	string c_Data;
	triple c_position, c_rotation;
	quad c_quat;
	triple c_color;
	string filename;
	double radius1, radius2, height, _length, angle, obj_scale;
	triple corners[4];
	int currentTag;
	int currentIndex;
	int particle_count;
	bool inverted, transparent, scene_options, static_geometry_loaded;
	int resolution;
	int camera_projection;
	int camera_direction;
	int axes;
	double velocity_vector_max;
	double velocity_vector_length;
	char map_filename[255];


	PSurfaceReader surface_mapper;
	GeometryEngine* render;
	PStatefileReader* instance;
	ParticleSet *t_target;
	vector<int> particles_to_mark;

	bool finish, terminate;
};
