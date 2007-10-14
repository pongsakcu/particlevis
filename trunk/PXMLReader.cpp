#include "PXMLReader.h"

#include <fcntl.h>
#include <io.h>
#include <algorithm>

#include "ChildView.h"

using std::transform;

PXMLReader::PXMLReader(CSemaphore* lock) :
PFileReader(),
particle_lock(lock),
finish(false),
terminate(false)
{
	
}

PXMLReader::~PXMLReader(void)
{
}

bool PXMLReader::LoadFile(ParticleSet* target, string filename)
{
	Camera* target_camera = target->cam;
	// Acquire rendering object, clear it of old lists
	render = target->PRend;
	render->FlushLists();
	currentIndex = 0;
	particle_count = 1;
	inverted = false;	
	map_filename[0] = '\0';
	resolution = 0;
	scene_options = false;
	static_geometry_loaded = false;

	
	camera_projection = -1;
	camera_direction = -1;
	particles_to_mark.clear();
	axes = -1;
	velocity_vector_max = -1;
	velocity_vector_length = -1;
	obj_scale = 1;

	// Create XML parsing object, setup handlers
	XML_Parser XP = XML_ParserCreate(NULL);
	if (XP == NULL) { finish = true;  return false; }
	XML_SetElementHandler(XP, StartElement, EndElement);
	XML_SetCharacterDataHandler(XP, CharHandler);
	XML_SetUserData(XP, this);

	// open the XML file
	int xmlfile = _open(filename.c_str(), O_RDONLY);
	int bytesread = -1;

	// shovel the data into the parser, invoking handlers
	while (bytesread!=0)
	{
		void *buff = XML_GetBuffer(XP, 10384);
		if (buff == NULL) { finish = true; return false; }
		bytesread = read(xmlfile, buff, 10384);
		if (!(XML_ParseBuffer(XP, bytesread, bytesread==0)))
		{ finish = true; return false; }
	}	

	_close(xmlfile);

	// assign the generated display lists to the particle objects
	if (target->particles == NULL || target->GetMaxParticles() < 0)
	{ finish = true; return false; }

	CSingleLock master(particle_lock);
	//master.Lock();
	render->AssignLists(target->particles, target->GetMaxParticles()); // load display lists into pset
	if (static_geometry_loaded) // load static geometry into designated particle
	{
		target->StaticParticle()->LoadRender(render->GetStaticList()); // set static geometry (if present)
		if (render->GetStaticListColor().x != -1)
			//target->StaticParticle()->AddColor(0, render->GetStaticListColor(), true);
			target->StaticParticle()->SetColor(render->GetStaticListColor());
		target->StaticParticle()->MarkParticle(MARK_UNMARKED);
	}
	else
	{
		target->StaticParticle()->MarkParticle(MARK_HIDE_MODE);
	}

	
	// set scene options (if they are specified)
	if (scene_options)
	{
		if (camera_direction != -1);
			target_camera->setViewDirection(camera_direction);
		if (camera_projection != -1);
			target_camera->setProjection(camera_projection);
		for (int i = 0; i < particles_to_mark.size(); i++)
		{
			target->MarkParticle(particles_to_mark[i]);
			CChildView::watch_list.push_back(particles_to_mark[i]);
		}
		if (axes == 1)
			CChildView::axes = true;
		if (axes == 0)
			CChildView::axes = false;

		if (velocity_vector_max != -1)
			Particle::maxVelocityMap = velocity_vector_max;
		if (velocity_vector_length != -1)
			Particle::transVelocityLength = velocity_vector_length;
	}

	// parse surface map if specified
	for (int i = 0; i < target->GetMaxParticles(); i++)
		target->particles[i]->ClearSurfaceMap();
	//master.Unlock();
	Particle::setup = false;
	if ((strlen(map_filename) > 0) && (resolution > 0))
	{		
		surface_mapper.LoadLock(particle_lock);
		surface_mapper.SetResolution(resolution);
		surface_mapper.LoadFile(target, map_filename);
	}
	finish = true;

	return true;
}

// Static handler for all XML start tags
void PXMLReader::StartElement(void *userData, const XML_Char *name, const XML_Char **atts)
{
	PXMLReader * xml = (PXMLReader*)userData;
	xml->ParseStartElement(name, atts);	// call instance method
}

// Instance method for XML start tags
void PXMLReader::ParseStartElement(const XML_Char *name, const XML_Char **atts)
{
	c_Data.clear();
	int tex = 3;
	double att_val;
	int index = LookupTag((char*)name);
	int quality = 0;
	currentTag = index; // lookup tag type
	if (index==100 || index==97) // for all <particle>/<static> tags, initialize default values
	{
		obj_scale = 1;
		particle_count = 1;
		inverted = false;
		transparent = true;
		c_color = triple(-1, -1, -1);
		c_position = triple(0, 0, 0);
		c_rotation = triple(0, 0, 0);
		c_quat = quad(0, 0, 0, 0);
	}
	if (index>=0 && index<=9)
	{
		c_position = triple(0, 0, 0);
		c_rotation = triple(0, 0, 0);
		c_quat = quad(0, 0, 0, 0);
	}
	if (index==6) // swap normals
		render->AddFlip();

	int i = 0;
	for (i = 0; atts[i]; i += 2) // read in all attributes of tag
	{
		index = LookupTag(atts[i]);
		att_val = strtod(atts[i + 1], NULL);
		switch (index)
		{
		case 500: // count
			particle_count = (int)att_val;
			if (particle_count <= 0) particle_count = 1;
			break;
		case 501: // inversion
			{
			string val(atts[i + 1]);
			if (val=="true")
				inverted = true;
			}
			break;
		case 502: // color R
			c_color.x = att_val;
			break;
		case 503: // color G
			c_color.y = att_val;
			break;
		case 504: // color B
			c_color.z = att_val;
			break;
		case 505: // quality
			quality  = (int)att_val;
			break;
		case 506: // textures
			{
			string val(atts[i + 1]);
			if (val=="true")
				tex = TRUE;
			if (val=="false")
				tex = FALSE;
			}
			break;
		case 507: // filename
			{
			string mapfile(atts[i+1]);
			strcpy(map_filename, mapfile.c_str());
			}
			break;
		case 508: // resolution
			resolution = (int)att_val;
			break;
		case 509:
			string val(atts[i + 1]);
			if (val=="true")
				transparent = true;
			if (val=="false")
				transparent = false;
			break;
		}
	}

	if (currentTag==97) // create list for the <static> tag
	{
		static_geometry_loaded = true;
		render->NewList(GeometryEngine::STATIC,
						particle_count,
						inverted,
						quality,
						tex,
						transparent);
		if (c_color.x != -1)
			render->AddColor(c_color);
	}

	if (currentTag==100) // create new list for the <particle> tag
	{
		render->NewList(currentIndex,
						particle_count,
						inverted,
						quality,
						tex,
						transparent);
		if (c_color.x != -1)
			render->AddColor(c_color);
		currentIndex+=particle_count;
	}
}

// Handler for all XML ending tags
void PXMLReader::EndElement(void *userData, const XML_Char *name)
{
	PXMLReader * xml = (PXMLReader*)userData;
	xml->ParseEndElement(name);
}

void PXMLReader::ParseEndElement(const XML_Char *name)
{
	double c_val = 0;

	int index = LookupTag(name); // lookup tag type
	currentTag = index;

	if (index >= 200) // for any tags with input, convert the accumulated character data into a double
	{
		c_val = strtod(c_Data.c_str(), NULL);
		//c_values.push_back(c_val);
	}

	// now perform the appropriate action for the completed tag
	// either we must parse some value for a higher-level tag, draw a geometry, or finish off a list
	switch (index) 
	{
	case 0: // </sphere>
		render->AddSphere(radius1, c_position);
		break;
	case 1: // </cylinder>
		render->AddCylinder(radius1, _length, c_position, c_rotation, c_quat);
		break;
	case 2: // </cone>
		render->AddCone(radius1, radius2, angle, c_position, c_rotation, c_quat);
		break;
	case 3: // </plate>
		render->AddPlate(corners[0], corners[1], corners[2], corners[3], c_position, c_rotation, c_quat);
		break;
	case 4: // </cap>
		render->AddCap(radius1, height, c_position, c_rotation, c_quat);
		break;
	case 5:
        // deprecated!
		break;
	case 6: // </flipsurface>
		render->AddFlip();
		break;
	case 7: // </file>
		render->AddFile(filename, c_position, c_rotation, c_quat);
		break;
	case 8: // </line>
		render->AddLine(corners[0], corners[1], c_position, c_rotation, c_quat);
		break;
	case 9: // </objfile>
		render->AddOBJ(filename, obj_scale, c_position, c_rotation, c_quat);
		break;
	case 97: // </static>
		render->EndList();
		break; 
	case 98: // </scene>
		scene_options = true;
		break;
	case 100: // </particle>
		render->EndList();
		break;
	case 104: // </mark>
		particles_to_mark.push_back(atoi(c_Data.c_str()));
		break;
	case 200: // </x>
		c_position.x = c_val;
		break;
	case 201: // </y>
		c_position.y = c_val;
		break;
	case 202: // </z>
		c_position.z = c_val;
		break;
	case 203: // </eta>
		c_rotation.x = c_val;
		break;
	case 204: // </xi>
		c_rotation.y = c_val;
		break;
	case 205: // </lambda>
		c_rotation.z = c_val;
		break;
	case 206: // </radius>
		radius1 = c_val;
		break;
	case 207: // </length>
		_length = c_val;
		break;
	case 208: // </small_radius>
		radius1 = c_val;
		break;
	case 209: // </large_radius>
		radius2 = c_val;
		break;
	case 210: // </half_angle>
		angle = c_val;
		break;
	case 211: // </corner1>
		corners[0] = c_position;
		c_position = triple(0, 0, 0);
		break;
	case 212: // </corner2>
		corners[1] = c_position;
		c_position = triple(0, 0, 0);
		break;
	case 213: // </corner3>
		corners[2] = c_position;
		c_position = triple(0, 0, 0);
		break;
	case 214: // </corner4>
		corners[3] = c_position;
		c_position = triple(0, 0, 0);
		break;
	case 215: // </height>
		height = c_val;
		break;
	case 218: // </filename>
		filename = c_Data;
		break;
	case 219: // </q0>
		c_quat.w = c_val;
		break;
	case 220: // </q1>
		c_quat.x = c_val;
		break;
	case 221: // </q2>
		c_quat.y = c_val;
		break;
	case 222: // </q3>
		c_quat.z = c_val;
		break;
	case 223: // </direction> (camera)
		transform(c_Data.begin(), c_Data.end(), c_Data.begin(), toupper);
		if (c_Data == "X")
			camera_direction = X;
		if (c_Data == "Y")
			camera_direction = Y;
		if (c_Data == "Z")
			camera_direction = Z;
		break;
	case 224: // </ortho>
		transform(c_Data.begin(), c_Data.end(), c_Data.begin(), toupper);
		if (c_Data == "TRUE")
			camera_projection = CAM_ORTHO;
		if (c_Data == "FALSE")
			camera_projection = CAM_PERSPECTIVE;
		break;
	case 225: // </axes>
		transform(c_Data.begin(), c_Data.end(), c_Data.begin(), toupper);
		if (c_Data == "TRUE")
			axes = 1;
		if (c_Data == "FALSE")
			axes = 0;
		break;
	case 228: // </scale>
		obj_scale = c_val;
		break;
	}
}

// all character data encountered between tags is
// concatenated into a string to be parsed by end-tag handler
void PXMLReader::CharHandler(void *userData, const XML_Char *s, int len)
{
	((PXMLReader*)userData)->c_Data.append(s, len); // inefficient: should probably use stringstreams
}

// Convert tag text to an integer index for easier switching
// TODO: this is an ugly method, make it OOP somehow!
int PXMLReader::LookupTag(const char *name)
{
	int index = -1;
	string tagname(name);

	// GEOMETRY TAGS
	if (tagname == "sphere") index = 0;
	if (tagname == "cylinder") index = 1;
	if (tagname == "cone") index = 2;
	if (tagname == "plate") index = 3;
	if (tagname == "cap") index = 4;
	if (tagname == "mapped_sphere") index = 5;
	if (tagname == "flipsurface") index = 6;
	if (tagname == "file") index = 7;
	if (tagname == "line") index = 8;
	if (tagname == "objfile") index = 9;

	// HIGH-LEVEL TAGS
	if (tagname == "static") index = 97;
	if (tagname == "scene") index = 98;
	if (tagname == "particleset") index = 99;
	if (tagname == "particle") index = 100;	

	// SCENE-LEVEL TAGS
	//if (tagname == "position") index = 101;
	//if (tagname == "rotation") index = 102;
	if (tagname == "camera") index = 103;
	if (tagname == "mark") index = 104;

	// DATA TAGS
	if (tagname == "x") index = 200;
	if (tagname == "y") index = 201;
	if (tagname == "z") index = 202;
	if (tagname == "eta") index = 203;
	if (tagname == "xi") index = 204;
	if (tagname == "lambda") index = 205;
	if (tagname == "radius") index = 206;
	if (tagname == "length") index = 207;
	if (tagname == "small_radius") index = 208;
	if (tagname == "large_radius") index = 209;
	if (tagname == "half_angle") index = 210;
	if (tagname == "corner1") index = 211;
	if (tagname == "corner2") index = 212;
	if (tagname == "corner3") index = 213;
	if (tagname == "corner4") index = 214;
	if (tagname == "height") index = 215;
	if (tagname == "position") index = 216;
	if (tagname == "orientation") index = 217;
	if (tagname == "filename") index = 218;
	if (tagname == "q0") index = 219;
	if (tagname == "q1") index = 220;
	if (tagname == "q2") index = 221;
	if (tagname == "q3") index = 222;
	if (tagname == "direction") index = 223;
	if (tagname == "ortho") index = 224;
	if (tagname == "axes") index = 225;
	if (tagname == "color_scheme") index = 226;
	if (tagname == "geometry_quality") index = 227;
	if (tagname == "scale") index = 228;

	// ATTRIBUTES
	if (tagname == "count") index = 500;
	if (tagname == "inverted") index = 501;
	if (tagname == "r") index = 502;
	if (tagname == "g") index = 503;
	if (tagname == "b") index = 504;
	if (tagname == "quality") index = 505;
	if (tagname == "textures") index = 506;
	if (tagname == "map_filename") index = 507;
	if (tagname == "map_resolution") index = 508;
	if (tagname == "transparent") index = 509;
	return index;
}