#pragma once

/*

	The GraphDisplay class encapsulates a 2-dimensional graph to be
	used as a sub-window type display to supplement other information.
	It loads in a simple file of X-Y points and creates a line graph
	based on this data.  The graph can be positioned anywhere on the
	screen and is partially transparent.  Requires an OpenGL context.

	By Vincent Hoon, 2005

*/

#include "stdafx.h"
#include "triple.h"
#include "GraphScale.h"
#include "WGLFont.h"
#include <vector>

using std::vector;
using std::string;

class GraphDisplay
{
public:
	GraphDisplay(WGLFont* font);
	~GraphDisplay(void);
	void Draw(int frame);
	bool LoadFile(const char filename[MAX_PATH]);
	bool LoadData(std::vector<float>& in_x, std::vector<float>& in_y);
	void LoadLegend(int type);
	void SetScale(double min, double max, double stepsize);
	void SetPosition(double x, double y, double w, double h);
	void SetPosition(int x, int y, int x2, int y2);
	void ZoomIn(int x, int y);
	void ZoomOut();
	bool IsZoomed() { return zoomed; }

	string x_label, y_label;

private:
	double min_x_scale, max_x_scale, x_step_size;
	double min_y_scale, max_y_scale, y_step_size;
	int totalframes;
	int display_list;
	int scale_mode;
	double scale_max;
	double x_pos, y_pos, width, height;
	bool zoomed;
	double x_zoom, y_zoom;	
	GraphScale the_grid;

	WGLFont* main_font;
	vector<triple> data;
};