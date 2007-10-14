#include "GeometryEngine.h"
#include <iostream>
#include <fstream>
#include "ChildView.h"
#include <math.h>
#include <vector>
#include "WavefrontOBJParser.h"

extern void sphere(double o[3], double r, int depth);
int GeometryEngine::default_display_lists = 0;

#define SQR(value) ((value)*(value))

GeometryEngine::GeometryEngine()
{
	default_radius = 5.591e-3;
	//default_radius = 0;
	inverted = false;
	// Generate sphere lists
	sphere_lists = glGenLists(5);
	static double o[] = {0, 0, 0};
	for (int i = 0; i < 5; i++)
	{
		glNewList(sphere_lists + i, GL_COMPILE);
		sphere(o, 1.0, i);
		glEndList();
	}

	matrix = TNT::Array2D<float>(4, 4, 0.0);
	m_temp = TNT::Array2D<float>(4, 4, 0.0);
	vector = TNT::Array2D<float>(4, 1, 0.0);
	temp = TNT::Array2D<float>(4, 1, 0.0);

	c_color = triple(-1, -1, -1);
	c_rot = triple();
	c_pos = triple();
	quat = quad();
}

GeometryEngine::GeometryEngine(CChildView* main_view)
{
	default_radius = 5.591e-3;
	//default_radius = 0;
	inverted = false;
	// Generate sphere lists
	sphere_lists = glGenLists(5);
	static double o[] = {0, 0, 0};
	for (int i = 0; i < 5; i++)
	{
		glNewList(sphere_lists + i, GL_COMPILE);
		sphere(o, 1.0, i);
		glEndList();
	}

	view = main_view;

	matrix = TNT::Array2D<float>(4, 4, 0.0);
	m_temp = TNT::Array2D<float>(4, 4, 0.0);
	vector = TNT::Array2D<float>(4, 1, 0.0);
	temp = TNT::Array2D<float>(4, 1, 0.0);

	quat = quad(0, 0, 0, 0);
}

GeometryEngine::~GeometryEngine(void)
{
	for (int i = 0; i < raw_Lists.size(); i++)
	{
		glDeleteLists(raw_Lists[i], 5);
	}
	glDeleteLists(sphere_lists, 5);
}

void GeometryEngine::AssignLists(Particle** p_list, int size)
{
	if (default_radius==0)
		default_radius = 5.591e-3;
	using namespace std;
	if (display_Lists.size() == 0) return;

	int maxsize = min(display_Lists.size(), canTransparent.size()) - 1;
	for (int i = 0; i < size; i++)
	{
		int true_index = min(maxsize, i);
		p_list[i]->LoadRender(display_Lists[true_index]);
		p_list[i]->SetRadius(radii[true_index]);
		if (canTransparent[true_index])
			p_list[i]->canTransparent = true;
		else
			p_list[i]->canTransparent = false;

		if (colors[true_index].x != -1)
		{
			//p_list[i]->ColorOverride = true;
			//p_list[i]->rgb = colors[min(maxsize, i)];
			p_list[i]->SetColor(colors[min(maxsize, i)]);
		}
		else
			p_list[i]->ColorOverride = false;

		if (spherical[true_index])
			p_list[i]->SetAsSphere(true);
		else
			p_list[i]->SetAsSphere(false);
			
	}
}

void GeometryEngine::NewList(int index, int count, bool invert, int quality, int textures, bool transparent)
{
	c_Count = count;
	inverted = invert;
	geo_Stack.clear();
	c_Index = index;
	c_Quality = quality;
	textured = textures;
	c_color = triple(-1, -1, -1);
	c_rot = triple();
	c_pos = triple();
	quat = quad();
	this->transparent = transparent;
}

int GeometryEngine::EndList()
{
	if (!wglMakeCurrent(view->pDC->m_hDC, view->hRC))
		return -1;

	int currentlist = 0;
	static int slices[] = {4, 8, 16, 32, 48, 72, 96};
	static int stacks[] = {1, 2, 2, 3, 10, 16, 20};
	if (c_Quality > 2) c_Quality = 2;

	int size = geo_Stack.size();
	if (size == 0)
		return -1;
	float current_radius = default_radius;
	bool sphere_flag = true;

	// check for collisions here

	currentlist = glGenLists(5);
	if (currentlist <= 0)
	{
		return -1;
	}

	for (int detail = 0; detail < 5; detail++) // list for each detail level
	{
		glNewList(currentlist + detail, GL_COMPILE);
		if (inverted)
			glEnable(GL_CULL_FACE);
		if (textured == FALSE)
		{
			glDisable(GL_TEXTURE_2D);
			//glColor3f(1, 1, 1);
		}
		if (textured == TRUE)
		{
			glEnable(GL_TEXTURE_2D);
		}

		for (int i = 0; i < size; i++) // rendering loop
		{
			c_rot = geo_Stack[i].rot;
			c_pos = geo_Stack[i].pos;
			bool rot = true, pos = true;
			if ( c_rot == triple(0, 0, 0) && quat == quad(0, 0, 0, 0))
				rot = false;
			if ( c_pos == triple(0, 0, 0) )
				pos = false;			

			quat = geo_Stack[i].quat;
			if (!(quat == quad(0, 0, 0, 0))) // transform quaternion to axis-angle form
			{
				quat = quat.Normalized(); // normalize input quaternion
				quad temp;
				double a, l;
				a = (2.0*acos(quat.w))*180.0/PI;
				if (a == 0)
				{
					temp = quad(0, 0, 1, 0);
				}
				else
				{
					l = sqrt((quat.x)*(quat.x) + (quat.y)*(quat.y) + (quat.z)*(quat.z));
					temp = quad(a, quat.x / l, quat.y / l, quat.z / l );
				}
				quat = temp;
			}
			CalcMatrix();

			// only spheres and color-changes are allowed.
			if ((geo_Stack[i].type != 0 && geo_Stack[i].type != 5) ||
				(c_pos != triple(0, 0, 0)))
				sphere_flag = false; // not a pure sphere
			
			
			switch (geo_Stack[i].type)
			{
			case 0: // sphere	
				RenderSphere(geo_Stack[i].data[0], detail+c_Quality);
				current_radius = geo_Stack[i].data[0];

				/* // TO ADD OUTINES TO TRIANGLES, USE THIS CODE
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glPolygonOffset(-2, -4);
				glPointSize(2.0);
				glColor3f(0, 0, 0);
				glEnable(GL_POLYGON_OFFSET_LINE);		
				RenderSphere(geo_Stack[i].data[0], detail+c_Quality);
				glDisable(GL_POLYGON_OFFSET_LINE);	
				glPolygonOffset(0, 0);				
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				*/
				break;
			case 1: // cone
				RenderCone(geo_Stack[i].data[0], geo_Stack[i].data[1], geo_Stack[i].data[2], slices[detail+c_Quality], stacks[detail+c_Quality]);
				break;
			case 2: // cylinder
				RenderCylinder(geo_Stack[i].data[0], geo_Stack[i].data[1], slices[detail+c_Quality]);
				break;
			case 3: // plate
				{
				triple c1(geo_Stack[i].data[0], geo_Stack[i].data[1], geo_Stack[i].data[2]);
				triple c2(geo_Stack[i].data[3], geo_Stack[i].data[4], geo_Stack[i].data[5]);
				triple c3(geo_Stack[i].data[6], geo_Stack[i].data[7], geo_Stack[i].data[8]);
				triple c4(geo_Stack[i].data[9], geo_Stack[i].data[10], geo_Stack[i].data[11]);
				if (inverted)
					glDisable(GL_CULL_FACE);
				RenderPlate(c1, c2, c3, c4);
				if (inverted)
					glEnable(GL_CULL_FACE);
				}
				break;
			case 4: // cap
				RenderCap(geo_Stack[i].data[0], geo_Stack[i].data[1], slices[detail+c_Quality], stacks[detail+c_Quality]);
				current_radius = geo_Stack[i].data[0];
				break;
			case 5: // color change
				c_color = triple(geo_Stack[i].data[0], geo_Stack[i].data[1], geo_Stack[i].data[2]);
				break;
			case 6: // flip surfaces
				inverted = !inverted;
				break;
			case 7: // load triangle file
				ReadTriangles(geo_Stack[i].strdata);
				break;
			case 8: // line object
				{
				triple c1(geo_Stack[i].data[0], geo_Stack[i].data[1], geo_Stack[i].data[2]);
				triple c2(geo_Stack[i].data[3], geo_Stack[i].data[4], geo_Stack[i].data[5]);
				RenderLine(c1, c2);
				}
				break;
			case 9: // obj triangle file
				{
				using std::vector;
				int index = 0;
				double scale = geo_Stack[i].data[0];
				WavefrontOBJParser parse;				
				parse.ParseData(geo_Stack[i].strdata);
				//parse.DrawData();

				vector<int>::iterator iter;
				glBegin(GL_TRIANGLES);
				iter = parse.polygons.begin();
				while (iter != parse.polygons.end())
				{
					index = *(iter++);
					if (index >= 0 && index < parse.normals.size())
						Normal(parse.normals[index].x, parse.normals[index].y, parse.normals[index].z);
					
					index = *(iter++);
					if (index >= 0 && index < parse.texcoords.size())
						glTexCoord2d(parse.texcoords[index].x, parse.texcoords[index].y);

					index = *(iter++);
					if (index >= 0 && index < parse.points.size())
						Point(parse.points[index].x * scale, parse.points[index].y * scale, parse.points[index].z * scale);
				}
				glEnd();
				break;
				}
			}			
		}

		if (inverted)
			glDisable(GL_CULL_FACE);
		glEndList();
	}

	if (c_Index != STATIC)
	{
		display_Lists.resize(c_Index+c_Count, 0);
		visible.resize(c_Index+c_Count, true);
		canTransparent.resize(c_Index+c_Count, true);
		radii.resize(c_Index+c_Count, 0.0);
		colors.resize(c_Index+c_Count, triple(-1, -1, -1));
		spherical.resize(c_Index+c_Count, true);

		for (int i = 0; i < c_Count; i++) 
		{
			visible[c_Index+i] = inverted;
			canTransparent[c_Index+i] = transparent;
			display_Lists[c_Index+i] = currentlist;
			radii[c_Index+i] = current_radius;
			colors[c_Index+i] = c_color;
			spherical[c_Index+i] = sphere_flag;
		}
	}
	else
	{
		static_list = currentlist;
		static_color = c_color;
	}

	raw_Lists.push_back(currentlist);
	return currentlist;
}

void GeometryEngine::AddSphere(double radius, triple position)
{
	if (default_radius==0)
		default_radius = radius;
	geoRecord sphere;
	sphere.type = 0;
	sphere.data[0] = radius;
	sphere.pos = position;
	geo_Stack.push_back(sphere);
}

void GeometryEngine::AddCone(double small_radius, double large_radius, double half_angle, triple position, triple orientation, quad quat_orient)
{
	geoRecord cone;
	cone.type = 1;
	cone.data[0] = small_radius;
	cone.data[1] = large_radius;
	cone.data[2] = half_angle;
	cone.pos = position;
	cone.rot = orientation;
	cone.quat = quat_orient;
	geo_Stack.push_back(cone);
}

void GeometryEngine::AddCylinder(double radius, double height, triple position, triple orientation, quad quat_orient)
{
	if (default_radius==0)
		default_radius = radius;
	geoRecord cylinder;
	cylinder.type = 2;
	cylinder.data[0] = radius;
	cylinder.data[1] = height;
	cylinder.pos = position;
	cylinder.rot = orientation;
	cylinder.quat = quat_orient;
	geo_Stack.push_back(cylinder);
}

void GeometryEngine::AddPlate(triple corner1, triple corner2, triple corner3, triple corner4, triple position, triple orientation, quad quat_orient)
{
	geoRecord plate;
	plate.type = 3;
	plate.data[0] = corner1.x;
	plate.data[1] = corner1.y;
	plate.data[2] = corner1.z;
	plate.data[3] = corner2.x;
	plate.data[4] = corner2.y;
	plate.data[5] = corner2.z;
	plate.data[6] = corner3.x;
	plate.data[7] = corner3.y;
	plate.data[8] = corner3.z;
	plate.data[9] = corner4.x;
	plate.data[10] = corner4.y;
	plate.data[11] = corner4.z;

	plate.pos = position;
	plate.rot = orientation;
	plate.quat = quat_orient;
	geo_Stack.push_back(plate);
}

void GeometryEngine::AddCap(double radius, double height, triple position, triple orientation, quad quat_orient)
{
	geoRecord cap;
	cap.type = 4;
	cap.data[0] = radius;
	cap.data[1] = height;
	cap.pos = position;
	cap.rot = orientation;
	cap.quat = quat_orient;
	geo_Stack.push_back(cap);
}

void GeometryEngine::AddColor(triple rgb)
{
	geoRecord color;
	color.type = 5;
	color.data[0] = rgb.x;
	color.data[1] = rgb.y;
	color.data[2] = rgb.z;
	geo_Stack.push_back(color);
}

void GeometryEngine::AddFlip()
{
	geoRecord flip;
	flip.type = 6;
	geo_Stack.push_back(flip);
}

void GeometryEngine::AddFile(string fname, triple position, triple orientation, quad quat_orient)
{
	geoRecord file;
	file.type = 7;
	file.strdata = fname;
	file.pos = position;
	file.rot = orientation;
	file.quat = quat_orient;
	geo_Stack.push_back(file);
}

void GeometryEngine::AddOBJ(string fname, double scale, triple position, triple orientation, quad quat_orient)
{
	geoRecord objfile;
	objfile.type = 9;
	objfile.data[0] = scale;
	objfile.strdata = fname;
	objfile.pos = position;
	objfile.rot = orientation;
	objfile.quat = quat_orient;
	geo_Stack.push_back(objfile);
}

void GeometryEngine::AddLine(triple corner1, triple corner2, triple position, triple orientation, quad quat_orient)
{
	geoRecord line;
	line.type = 8;
	line.data[0] = corner1.x;
	line.data[1] = corner1.y;
	line.data[2] = corner1.z;
	line.data[3] = corner2.x;
	line.data[4] = corner2.y;
	line.data[5] = corner2.z;

	line.pos = position;
	line.rot = orientation;
	line.quat = quat_orient;

	geo_Stack.push_back(line);
}

void GeometryEngine::RenderCone(double small_radius, double large_radius, double half_angle, int slices, int stacks)
{
	//bool top = true;
	double subthickness = 0;
	double thickness = 0;

    subthickness = ((large_radius - small_radius) / tan(half_angle));

	if (subthickness == 0) return;

	double theta = 0, thetaD = 0, x, y, minh = 0, h = 0, hD = 0;
	double r, r2, rD = 0, s, t;
	double x2, y2, t2, slope, stemp;
	double cx, sy;
	thetaD = TWOPI / (double)slices;

	minh = small_radius / tan(half_angle); // origin point (apex of full cone)
	hD = subthickness / (double)stacks;
	//rD = (small_radius - large_radius) / (double)stacks;
	//r = large_radius;
	rD = (large_radius - small_radius) / (double)stacks;
	r = small_radius;

	slope = hD / (large_radius - small_radius); // invert

	for (int i = 0; i < stacks; i++)
	{
		h += hD;
		r += rD;

		theta = TWOPI;
		glBegin(GL_TRIANGLE_STRIP);
		//glBegin(GL_QUAD_STRIP);
		for (int j = slices; j >= 0; j--)
		{
			s = (double)j / (double)slices;
			if (j==slices)
				s = 0.99;
			if (j==0)
			{
				theta = 0;
				s = 0.01;
			}
			//s = 1.0 - s;

			r2 = r - rD;

			x = r * cos(theta);
			y = r * sin(theta);
			x2 = r2 * cos(theta);
			y2 = r2 * sin(theta);
			t = 1.0 - ((double)(i+1) / (double)stacks);
			t2 = 1.0 - ((double)(i) / (double)stacks);

			cx = cos(theta);
			sy = sin(theta);

			stemp = sqrt(cx*cx + sy*sy + slope*slope);

			// if "inverted" is true:
			// flip the normals and draw the points "backwards,"
			// so we only see the back-face of the cone
			if (inverted)
				Normal(-cx / stemp, -sy / stemp, slope/stemp);
			else
				Normal(cx / stemp, sy / stemp, slope/stemp);


			if (!inverted)
			{
				glTexCoord2d(s, t2);
				Point(x2, y2, minh + h - hD);

				glTexCoord2d(s, t);
				Point(x, y, minh + h);
			}
			else
			{
				glTexCoord2d(s, t);
				Point(x, y, minh + h);

				glTexCoord2d(s, t2);
				Point(x2, y2, minh + h - hD);
			}

			theta-=thetaD;
		}
		glEnd();
	}
}

void GeometryEngine::RenderSphere(double radius, int depth)
{
	double o[3];
	o[0] = c_pos.x;
	o[1] = c_pos.y;
	o[2] = c_pos.z;
	if (radius==0) // render a point in the case of r=0
	{
		glBegin(GL_POINTS);
		Point(0, 0, 0);
		glEnd();
		return;
	}
	if (!inverted && depth < 5) // use the pre-generated sphere lists: transform the list to radius r
	{
		if (length(c_pos) > 0)
			glTranslatef(c_pos.x, c_pos.y, c_pos.z);
		if (radius != 1.0)
			glScaled(radius, radius, radius);

		glCallList(sphere_lists + depth);

		// psuedo-pop
		if (radius != 1.0)	
			glScaled(1.0/radius, 1.0/radius, 1.0/radius);
		if (length(c_pos) > 0)
			glTranslatef(-c_pos.x, -c_pos.y, -c_pos.z);
		
		// (Slower but accurate method)
		//sphere(o, radius, depth);
	}
	else // we don't generate inverted spheres: instead tesselate a new sphere
	{
		//glScaled(-radius, -radius, -radius);
		//glCallList(sphere_lists + depth);
		if (inverted)
			sphere(o, -radius, depth);
		else
			sphere(o, radius, depth);
	}
}

void GeometryEngine::RenderCylinder(double radius, double height, int slices)
{
	if (radius==0)
	{
		glBegin(GL_LINES);
		Point(0, 0, 0);
		Point(0, 0, height);
		glEnd();
		return;
	}

	int stacks = 1;
	double theta = 0, thetaD = 0, x, y, minh = 0, h = 0, hD = 0, r = 0, s, t;
	thetaD = TWOPI / (double)slices;
	hD = height / (double)stacks;
	minh = -height / 2.0;


	for (int i = 0; i < stacks; i++)
	{
		h += hD;

		theta = TWOPI;
		glBegin(GL_TRIANGLE_STRIP);
		for (int j = 0; j <= slices; j++)
		{
			s = (double)j / (double)slices;
			if (j==slices)
				s = 0.99;
			if (j==0)
			{
				theta = 0;
				s = 0.01;
			}

			x = radius * cos(theta);
			y = radius * sin(theta);

			// if inverted:
			// flip the normals and draw the points "backwards,"
			// so we only see the back-face of the cylinder
			if (inverted)
			{
				Normal(-x / radius, -y / radius, 0);
				s = 1.0 - s;
			}
			else
				Normal(x / radius, y / radius, 0);

			if (inverted)
			{
				t = (((double)(i+1) / (double)stacks) / 2.0) + 0.25;
				glTexCoord2d(s, t);
				Point(x, y, minh + h);

				t = (((double)(i) / (double)stacks) / 2.0) + 0.25;
				glTexCoord2d(s, t);
				Point(x, y, minh + h - hD);
			}
			else
			{
				t = (((double)(i) / (double)stacks) / 2.0) + 0.25;
				glTexCoord2d(s, t);
				Point(x, y, minh + h - hD);

				t = (((double)(i+1) / (double)stacks) / 2.0) + 0.25;
				glTexCoord2d(s, t);
				Point(x, y, minh + h);
			}

			theta-=thetaD;
		}
		glEnd();
	}
}

void GeometryEngine::RenderPlate(triple corner1, triple corner2, triple corner3, triple corner4)
{
	triple dir1, dir2, normal;
	dir1 = corner2 - corner1;
	dir2 = corner4 - corner1;
	normal = crossProduct(dir2, dir1);
    normalize(normal);

	glBegin(GL_TRIANGLE_STRIP);
	Normal(normal.x, normal.y, normal.z);
	glTexCoord2d(1, 1);
	Point(corner1.x, corner1.y, corner1.z);
	glTexCoord2d(0, 1);
	Point(corner2.x, corner2.y, corner2.z);
	glTexCoord2d(1, 0);
	Point(corner4.x, corner4.y, corner4.z);
	glTexCoord2d(0, 0);
	Point(corner3.x, corner3.y, corner3.z);
	glEnd();
}

void GeometryEngine::RenderLine(triple p1, triple p2)
{
	glBegin(GL_LINES);
	Normal(0, 1, 0);
	Point(p1.x, p1.y, p1.z);
	Normal(0, 1, 0);
	Point(p2.x, p2.y, p2.z);
	glEnd();
}

void GeometryEngine::RenderCap(double radius, double height, int slices, int stacks)
{
	if (height == 0) // if height is 0, make a flat cap
	{
		double u, v;
		double theta = TWOPI, thetaD = TWOPI / (double)slices;
		glBegin(GL_TRIANGLE_FAN);
		glTexCoord2d(0.5, 0.5);
		if (!inverted)
		{
			theta = 0;
			Normal(0, 0, 1);
		}
		else
			Normal(0, 0, -1);

		Point(0, 0, 0);

		for (int i = 0; i <= slices; i++)
		{
			u = cos(theta);
			v = sin(theta);
			if (!inverted)
				theta+=thetaD;
			else
				theta-=thetaD;
			glTexCoord2d((u/2.0)+0.5, (v/2.0)+0.5);
			Point(radius*u, radius*v, 0);
		}
		glEnd();
		return;
	}

	double offset = 0;
	double* lastringX = new double[slices];
	double* lastringY = new double[slices];
	double* thisringX = new double[slices];
	double* thisringY = new double[slices];

	double theta = 0, thetaD = 0, minh = 0, h = 0, lasth = 0, r = 0, phiD, phi;
	double top = 1.0, length = 1.0, s, t;
	double CRadius = height * 0.5 * (1.0 + pow((radius / height), 2.0));

	thetaD = TWOPI / (double)slices;
	minh = CRadius - height;

	h = CRadius;
	phi = 0;
	phiD = atan(radius / minh) / (double)stacks;
	

	if (offset >= 0)	// orient the cap properly
		top = 1.0;
	else
		top = -1.0;

	// fill first row
	for (int i = 0; i < slices; i++)
	{
		lastringX[i] = 0;
		lastringY[i] = 0;
	}

	// Draw our cap one triangle strip at a time

	glBegin(GL_TRIANGLE_STRIP);

    for (int i = 0; i < stacks; i++)
	{
		phi += phiD;
		lasth = h;
		h = CRadius*cos(phi);

		if (i==stacks - 1)
			h = minh;
		theta = 0;
		r = sqrt(CRadius * CRadius - h*h);	

		for (int j = 0; j < slices; j++)
		{
			thisringX[j] = r*cos(theta);
			thisringY[j] = r*sin(theta);
			theta+=thetaD;
		}

		int k = 0;
		for (int k = 0; k <= slices; k++)
		{
			int j = k % slices;
			s = (double)j / (double)slices; // x texture coordinate

			if (k==slices)
				s = 1.0;
			if (k==0)
				s = 0.0;
			if (top<0)
			{
				s = 1.0 - s;
				j = slices - 1 - j;
			}

			if (inverted)
			{
				t = 0.25 * ((double)(i+1.0) / (double)stacks);
				if (i==stacks-1)
					t = 0.25 - 0.001;
				if (top<0) t = 1.00 - t;
				length = sqrt(thisringX[j]*thisringX[j] + thisringY[j]*thisringY[j] + (top*top*h*h));
					glTexCoord2d(s, t);
					Normal(thisringX[j] / length, thisringY[j] / length, (top * h) / length);
					Point(thisringX[j], thisringY[j], (top*(h - minh)) + offset);

				t = 0.25 * ((double)i / (double)stacks); // y texture coordinate
				if (i==0) t = 0.001;
				if (top<0) t = 1.00 - t;			
				length = sqrt(lastringX[j]*lastringX[j] + lastringY[j]*lastringY[j] + (top*top*lasth*lasth));
					glTexCoord2d(s, t); 
					Normal(lastringX[j] / length, lastringY[j] / length,(top * lasth) / length);
					Point(lastringX[j], lastringY[j], (top*(lasth - minh)) + offset);
			}
			else
			{
				t = 0.25 * ((double)i / (double)stacks); // y texture coordinate
				if (i==0) t = 0.001;
				if (top<0) t = 1.00 - t;			
				length = sqrt(lastringX[j]*lastringX[j] + lastringY[j]*lastringY[j] + (lasth*lasth));
					glTexCoord2d(s, t); 
					Normal(lastringX[j] / length, lastringY[j] / length,(top * lasth) / length);
					Point(lastringX[j], lastringY[j], (top*(lasth - minh)) + offset);


				t = 0.25 * ((double)(i+1.0) / (double)stacks);
				if (i==stacks-1)
					t = 0.25 - 0.001;
				if (top<0) t = 1.00 - t;
				length = sqrt(thisringX[j]*thisringX[j] + thisringY[j]*thisringY[j] + h*h);
					glTexCoord2d(s, t);
					Normal(thisringX[j] / length, thisringY[j] / length, (top * h) / length);
					Point(thisringX[j], thisringY[j], (top*(h - minh)) + offset);
			}
		}

		for (int j = 0; j < slices; j++)
		{
			lastringX[j] = thisringX[j];
			lastringY[j] = thisringY[j];
		}
	}

	glEnd();


	delete [] lastringX;
	delete [] lastringY;
	delete [] thisringX;
	delete [] thisringY;
}

void GeometryEngine::FlushLists()
{
	for (int i = 0; i < raw_Lists.size(); i++)
	{
		glDeleteLists(raw_Lists[i], 5);
	}
	display_Lists.clear();
	raw_Lists.clear();
	visible.clear();
	radii.clear();
	default_radius = 0;
}

using namespace TNT;

void GeometryEngine::Point(float x, float y, float z)
{
	vector[0][0] = x;
	vector[1][0] = y;
	vector[2][0] = z;
	vector[3][0] = 1.0;

	temp = matmult(matrix, vector);

	glVertex3f(temp[0][0], temp[1][0], temp[2][0]);
}

void GeometryEngine::Normal(float x, float y, float z)
{
	vector[0][0] = x;
	vector[1][0] = y;
	vector[2][0] = z;
	vector[3][0] = 1.0;

	temp = matmult(matrix, vector);
	float mag = sqrt(SQR(temp[0][0]) + SQR(temp[1][0]) + SQR(temp[2][0]));
	glNormal3f(temp[0][0]/mag, temp[1][0]/mag, temp[2][0]/mag);
}

void GeometryEngine::CalcMatrix()
{
    //Array2D<float> base(4, 4, 0.0);
	double c, s;
	
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			matrix[i][j] = (i==j) ? 1.0 : 0.0;
    
	matrix[0][3] = c_pos.x;
	matrix[1][3] = c_pos.y;
	matrix[2][3] = c_pos.z;

	// derive rotation matrix from axis-angle quad "quat"
	if (quat.w != 0 || quat.x != 0 || quat.y != 0 || quat.z != 0)
	{		
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
				m_temp[i][j] = 0;

        float s = sin(quat.w*PI/180.0);
		float c = cos(quat.w*PI/180.0);
		matrix[0][0] = quat.x*quat.x*(1.0 - c) + c;
		matrix[0][1] = quat.x*quat.y*(1.0 - c) - quat.z*s;
		matrix[0][2] = quat.x*quat.z*(1.0 - c) + quat.y*s;

		matrix[1][0] = quat.y*quat.x*(1.0 - c) + quat.z*s;
		matrix[1][1] = quat.y*quat.y*(1.0 - c) + c;
		matrix[1][2] = quat.y*quat.z*(1.0 - c) - quat.x*s;

		matrix[2][0] = quat.x*quat.z*(1.0 - c) - quat.y*s;
		matrix[2][1] = quat.y*quat.z*(1.0 - c) + quat.x*s;
		matrix[2][2] = quat.z*quat.z*(1.0 - c) + c;

		//matrix = matmult(matrix, m_temp);
	}
	
	{
		// derive euler angle rotation matrices
		if (c_rot.x != 0)
		{
			for (int i = 0; i < 4; i++)
				for (int j = 0; j < 4; j++)
					m_temp[i][j] = 0;

			//Array2D<float> transEta(4, 4, 0.0);
			c = cos(c_rot.x);
			s = sin(c_rot.x);
			m_temp[0][0] = 1.0;
			m_temp[1][1] = c;
			m_temp[2][2] = c;
			m_temp[3][3] = 1.0;

			m_temp[1][2] = -s;
			m_temp[2][1] = s;
			matrix = matmult(matrix, m_temp);
		}
		if (c_rot.y != 0)
		{
			for (int i = 0; i < 4; i++)
				for (int j = 0; j < 4; j++)
					m_temp[i][j] = 0;

			//Array2D<float> transXi(4, 4, 0.0);
			c = cos(c_rot.y);
			s = sin(c_rot.y);
			m_temp[0][0] = c;
			m_temp[1][1] = 1.0;
			m_temp[2][2] = c;
			m_temp[3][3] = 1.0;
			m_temp[0][2] = s;
			m_temp[2][0] = -s;
			matrix = matmult(matrix, m_temp);
		}
		if (c_rot.z != 0)
		{
			for (int i = 0; i < 4; i++)
				for (int j = 0; j < 4; j++)
					m_temp[i][j] = 0;

			//Array2D<float> transLambda(4, 4, 0.0);
			c = cos(c_rot.z);
			s = sin(c_rot.z);
			m_temp[0][0] = c;
			m_temp[1][1] = c;
			m_temp[2][2] = 1.0;
			m_temp[3][3] = 1.0;
			m_temp[0][1] = -s;
			m_temp[1][0] = s;
			matrix = matmult(matrix, m_temp);
		}
	}
	//matrix = base;
}

void GeometryEngine::ReadTriangles(string filename)
{
	ifstream infile;
	infile.open(filename.c_str(), ios::in);
	if (infile.fail())
		return;

	glBegin(GL_TRIANGLES);

	glTexCoord2d(0, 0);
	float invalues[6];
	while (!infile.eof() && !infile.fail())
	{
		infile >> invalues[0];
		infile >> invalues[1];
		infile >> invalues[2];
		infile >> invalues[3];
		infile >> invalues[4];
		infile >> invalues[5];

		if (!infile.fail())
		{
			Normal(invalues[3], invalues[4], invalues[5]);
			Point(invalues[0], invalues[1], invalues[2]);			
		}
	}

	glEnd();
	infile.close();
}

void GeometryEngine::RenderDefaults()
{
	double radius = 0.005;
	default_display_lists = glGenLists(5);
	for (int i = 0; i < 5; i++)
	{
		glNewList(default_display_lists + i, GL_COMPILE);
		/*glBegin(GL_POINTS);
		glVertex3f(0, 0, 0);
		glEnd();*/

		glBegin(GL_POINTS);
		glNormal3d(0, 1, 0);
		glVertex3d(0, 0, 0);
		glEnd();

		//glColor3d(0, 0, 1);
		glBegin(GL_LINES);
		glNormal3d(1, 0, 0);
		glColor3d(0, 0, 1);
		glVertex3d(-radius, 0, 0);
		glVertex3d(radius, 0, 0);
		glNormal3d(0, 1, 0);
		glColor3d(1, 0, 0);
		glVertex3d(0, -radius, 0);
		glVertex3d(0, radius, 0);
		glNormal3d(0, 0, 1);
		glColor3d(0, 1, 0);
		glVertex3d(0, 0, -radius);
		glVertex3d(0, 0, radius);
		glEnd();
		glEndList();
	}
	//glNewList(default_display_lists + 5, GL_COMPILE);
	//glEndList();
}