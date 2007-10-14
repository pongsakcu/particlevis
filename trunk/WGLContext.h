#pragma once

#include "stdafx.h"
#include <GL/glew.h>
#include "glext.h"
#include "wglext.h"

class WGLContext
{
public:
	WGLContext(CDC* pDC);
	~WGLContext(void);

	HGLRC Create(int multisample = 0, bool stereo = false); // create an OpenGL DC
	HGLRC CreatePBuffer(int w, int h, int forceformat = -1 , bool stereoMode = false);
	int GetAAFormat(int samples); // return a multisampled pixel format

	CDC* DC;

	static PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB;
	static PFNWGLGETPIXELFORMATATTRIBIVARBPROC wglGetPixelFormatAttribivARB;
	static PFNWGLGETEXTENSIONSSTRINGARBPROC	wglGetExtensionsStringARB;

	bool IsWglExtensionSupported(const char *extstring);
	bool IsExtensionSupported(const char *extstring);

private:
	bool compatibility_mode;
	bool SetupPixelFormat(int bpp, int z_buffer_bpp, int samples, bool stereoMode, int forceformat = -1);
};
