#pragma once

// GL utility class for drawing text in a console.
// Vincent Hoon, 2007
// See copyright notice in copyright.txt for conditions of distribution and use.

#include "stdafx.h"
#include "WGLFont.h"
#include <string>
#include <list>
#include "triple.h"

using std::string;
using std::list;

class GLConsole
{
public:
	GLConsole(WGLFont* font, int num_lines = 5, int alignment = ALIGNRIGHT);
	~GLConsole(void);
	void Draw();
	void PushMessage(string msg);
	void PushVector(triple v, string header);
	void PopMessage();
	void ClearMessages();
	void SetLineSize(int pixelheight) { lineheight = pixelheight; }
	void SetMaxLines(int num_lines) { maxlines = num_lines; }	

private:
	list<string> console_text;
	WGLFont* myfont;
	int maxlines, myalignment, lineheight;
};