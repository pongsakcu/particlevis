#pragma once

// Implementation of a "background" renderer that can be called from the command line.
// Vincent Hoon, 2006
// See copyright notice in copyright.txt for conditions of distribution and use.

#include "stdafx.h"
#include "WGLContext.h"
#include "ChildView.h"

class BackgroundRenderer
{
public:
	BackgroundRenderer(void);
	~BackgroundRenderer(void);
	bool BGRender(CChildView *child, char * path, char * output, int width, int height, int fram, int buffermode, bool ascii, char * xml);

private:
	void GLInit(WGLContext &context);
	int w, h;
};
