#include "graphdisplay.h"
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include "Particle.h"
#include <algorithm>

using namespace std;

GraphDisplay::GraphDisplay(WGLFont* font)
{
	main_font = font;
	display_list = -1;
	totalframes = -1;

	min_x_scale = 0;
	max_x_scale = 1.0;
	x_step_size = 0.1;
	min_y_scale = 0;
	max_y_scale = 1.0;
	y_step_size = 0.1;
	x_pos = 0.8;
	y_pos = 0;
	width = 0.2;
	height = 0.15;

	the_grid.font = font;

	//the_grid.BuildFont(hdc);
	the_grid.SetWorldBox(1, 1, 1);
	the_grid.SetFontSize(0.05, 1.5);
	the_grid.SetCamera(1, 1, 1);

	the_grid.SetXRange(0, 50);
	the_grid.SetXGrid(5, 5, 1.0);
	the_grid.SetYRange(0, 1);
	the_grid.SetYGrid(0.1, 2, 1.0);
	the_grid.SetZRange(0, 1);
	the_grid.SetZGrid(0.1, 1, 1.0);

	x_label = "X Var";
	y_label = "Y Var";

	scale_mode = 0;
	zoomed = false;
}

GraphDisplay::~GraphDisplay(void)
{
}

void GraphDisplay::Draw(int frame)
{
	// Destructive modification of GL matrices

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 1, 0, 1, -10, 10);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	if (zoomed)
	{
		glScaled(3, 3, 3);
		glTranslated(-x_zoom+.1667, -y_zoom+.1667, 0);
	}

	glTranslated(x_pos, y_pos, 0);
	glScaled(width, height, 1.0);


	glDisable(GL_DEPTH_TEST);
	//glEnable(GL_POLYGON_SMOOTH); // enable smoothing
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA_SATURATE, GL_ONE; // enable smoothing

	glColor4d(0, 0, 0, 0.15);
	glBegin(GL_TRIANGLE_STRIP);
		glVertex2d(0, 0);
		glVertex2d(1, 0);
		glVertex2d(0, 1);
		glVertex2d(1, 1);
	glEnd();

	int dims[4];
	glGetIntegerv(GL_VIEWPORT, dims);
	main_font->SetAspect((width*(double)dims[2]) / (height*(double)dims[3]));

	the_grid.Draw(0);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glLineWidth(1.0);		
	glColor4d(0, 1, 0, 0.25);	
	glBegin(GL_LINE_LOOP);
		glVertex2d(0, 0);
		glVertex2d(1, 0);
		glVertex2d(1, 1);
		glVertex2d(0, 1);
	glEnd();
	

	glPushMatrix();
	the_grid.Transform();
	if (zoomed)
	{
		//glTranslated(x_zoom, y_zoom, 0);
		//glScaled(2, 2, 2);
	}
	glColor3d(1, 1, 1);

	// draw datapoints
	vector<triple>::iterator it = data.begin();
	glBegin(GL_LINE_STRIP);
	while (it != data.end())
	{
		glVertex2d((*it).x, (*it).y);
		it++;
	}
	glEnd();


	glColor3d(1, 0, 0);
	if (scale_mode == 0) // display file data
	{
		int s = data.size();
		if (frame < data.size())
		{
			glEnable(GL_POINT_SMOOTH);
			glPointSize(7.0);
			glBegin(GL_POINTS);
				glVertex3d(data[frame].x, data[frame].y, 0.1);
			glEnd();
			glColor4d(1, 0, 0, 0.5);
			glBegin(GL_LINES);
				glVertex3d(data[frame].x, min_y_scale, 0.1);
				glVertex3d(data[frame].x, max_y_scale, 0.1);
			glEnd();
		}
	}
	else if (scale_mode > 0) // display legend
	{
		triple c;
		glBegin(GL_TRIANGLE_STRIP);
		for (int i = 0; i < 100; i++)
		{
			Particle::grad.ConvertToColor((double)i/99.0, c);
			glColor3f(c.x, c.y, c.z);
			glVertex3d(1.0, scale_max*(double)i/99.0, 0.1);
			glVertex3d(0, scale_max*(double)i/99.0, 0.1);
		}
		glEnd();
	}
	glPopMatrix();

	glColor3d(1, 1, 1);
	main_font->DrawChars(x_label.c_str(), 0.5, 1.04, 0, ALIGNCENTER);

	glMatrixMode(GL_MODELVIEW);
	glRotated(-90, 0, 0, 1);
	main_font->SetAspect((height*(double)dims[3]) / (width*(double)dims[2]));
	main_font->DrawChars(y_label.c_str(), -0.5, 1.03, 0, ALIGNCENTER);

	glDisable(GL_LINE_SMOOTH);
	glDisable(GL_POLYGON_SMOOTH);
	glDisable(GL_BLEND);
}

// Set position in normalized window coordinates, [0..1]
void GraphDisplay::SetPosition(double x, double y, double w, double h)
{
	if (x>=0 && x<=1) x_pos = x;
	if (y>=0 && y<=1) y_pos = y;
	if (w>=0 && w<=1) width = w;
	if (h>=0 && h<=1) height = h;
}

// Set position in pixels
void GraphDisplay::SetPosition(int x, int y, int x2, int y2)
{

	int left, right, top, bottom;
	
	if (x < x2)	{ left = x;  right = x2;}
	else		{ left = x2; right = x;	}

	if (y > y2)	{ top = y;  bottom = y2;}
	else		{ top = y2; bottom = y; }

	int dims[4];
	glGetIntegerv(GL_VIEWPORT, dims);

	x_pos = (double)left / (double)dims[2];
	y_pos = (double)bottom / (double)dims[3];
	width = (double)(right - left) / (double)dims[2];
	height = (double)(top-bottom) / (double)dims[3];
}

bool GraphDisplay::LoadFile(const char filename[MAX_PATH])
{
	// read in file, parse data
	double x_value, y_value;
	char * ptr;
	char buffer[256];
	ifstream filein;
	filein.open(filename, ios::in);

	filein.getline(buffer, 255);
	char * label_cstr = strtok(buffer, "\t");
	if (label_cstr)
		x_label = label_cstr;
	else
		return false;
	label_cstr = strtok(NULL, "\t");
	if (label_cstr)
		y_label = label_cstr;
	else
		return false;

	filein >> min_x_scale >> min_y_scale;
	filein >> max_x_scale >> max_y_scale;
	filein >> x_step_size >> y_step_size;

	if (filein.fail())
		return false;
	
	while (!filein.eof() && !filein.fail())
	{
		filein >> x_value;
		filein >> y_value;
		data.push_back(triple(x_value, y_value, 0));
	}
	
	the_grid.SetXGrid(x_step_size, 1, 1.0);
	the_grid.SetXRange(min_x_scale, max_x_scale);
	the_grid.SetYGrid(y_step_size, 1, 1.0);
	the_grid.SetYRange(min_y_scale, max_y_scale);
	
	return true;
}

void GraphDisplay::LoadLegend(int type)
{
	if (type==POSVELOCITY)
	{
		scale_max = Particle::maxVelocityMap;
		y_label = "Translational Velocity Magnitude";
	}
	if (type==ROTVELOCITY)
	{
		scale_max = Particle::maxRVelocityMap;
		y_label = "Angular Velocity Magnitude";
	}
	if (type==SURFACEMAP)
	{
		scale_max = Particle::map_scale;
		y_label = "Surface Map Magnitude";
	}

	the_grid.SetXGrid(1, 1, 1, false);
	the_grid.SetXRange(0, 1);
	x_label = "";

	the_grid.SetYGrid(scale_max / 10.0, 1, 1.0);
	the_grid.SetYRange(0, scale_max);
	

	SetPosition(0.4, 0.1, 0.4, 0.8);
	scale_mode = type+1;
}

bool GraphDisplay::LoadData(std::vector<float>& in_x, std::vector<float>& in_y)
{
	if (in_x.size() < 1 && in_y.size() < 1)
		return false;
	data.clear();
	vector<float>::iterator iter_x, iter_y;
	float x = 0.0, y = 0.0;
	max_x_scale = in_x[0];
	min_x_scale = in_x[0];
	max_y_scale = in_y[0];
	min_y_scale = in_y[0];
	iter_x = in_x.begin();
	iter_y = in_y.begin();
	while(iter_x != in_x.end() && iter_y != in_y.end())
	{
		x = *iter_x;
		y = *iter_y;
		if (y < min_y_scale)
			min_y_scale = y;
		if (y > max_y_scale)
			max_y_scale = y;
		if (x < min_x_scale)
			min_x_scale = x;
		if (x > max_x_scale)
			max_x_scale = x;
		data.push_back(triple(x, y, 0));

		iter_x++; iter_y++;
	}
	the_grid.SetXGrid(max_x_scale / 5.0, 2, 1.0);
	the_grid.SetXRange(0.0, max_x_scale);
	the_grid.SetYGrid((max_y_scale - min_y_scale) / 10.0, 2, 1.0);
	the_grid.SetYRange(min_y_scale, max_y_scale);
    return true;
}

void GraphDisplay::ZoomIn(int x, int y)
{
	int dims[4];
	glGetIntegerv(GL_VIEWPORT, dims);
	x_zoom = (double)x / (double)dims[2];
	y_zoom = 1.0 - ((double)y / (double)dims[3]);
	zoomed = true;
}

void GraphDisplay::ZoomOut()
{
	zoomed = false;
}