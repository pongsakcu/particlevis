#include "glconsole.h"
#include <sstream>
#include <iostream>

GLConsole::GLConsole(WGLFont* font, int num_lines /* = 5 */, int alignment /* ALIGNRIGHT */)
{
	maxlines = num_lines;
	myfont = font;
	myalignment = alignment;
	lineheight = 15;
}

GLConsole::~GLConsole(void)
{
}


void GLConsole::Draw()
{
	int dims[4];
	glGetIntegerv(GL_VIEWPORT, dims);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, dims[2], 0, dims[3], -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	list<string>::iterator iter;
	int i = 0;
	double start = dims[3] - (lineheight * maxlines);
	for (iter = console_text.begin(); iter != console_text.end(); iter++)
		myfont->DrawChars((*iter).c_str(), dims[2]-5, start+lineheight*i++, 0, myalignment | ALIGNTOP);

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
}

void GLConsole::PushMessage(string msg)
{
	while (console_text.size() >= maxlines)
		console_text.pop_front();

	console_text.push_back(msg);
}

void GLConsole::PopMessage()
{
	if (console_text.size() >= 1)
		console_text.pop_front();
}

void GLConsole::PushVector(triple v, string header)
{
	using namespace std;
	stringstream bobthebuffer;
	bobthebuffer.precision(4);
	bobthebuffer.width(5);
	bobthebuffer << header << ": (";
	bobthebuffer.width(10);
	bobthebuffer << fixed << v.x;
	bobthebuffer.width(10);
	bobthebuffer << fixed << v.y;
	bobthebuffer.width(10);
	bobthebuffer << fixed << v.z << ")";
	PushMessage(bobthebuffer.str());
}

void GLConsole::ClearMessages()
{
	console_text.clear();
}