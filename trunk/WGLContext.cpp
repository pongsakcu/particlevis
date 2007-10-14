#include "WGLContext.h"

#define INIT_ENTRY_POINT( funcname, type ) \
funcname = (type) wglGetProcAddress(#funcname);

/*PFNWGLCREATEPBUFFERARBPROC		wglCreatePbufferARB;
PFNWGLGETPBUFFERDCARBPROC		wglGetPbufferDCARB;
PFNWGLRELEASEPBUFFERDCARBPROC	wglReleasePbufferDCARB;
PFNWGLDESTROYPBUFFERARBPROC		wglDestroyPbufferARB;
PFNWGLQUERYPBUFFERARBPROC		wglQueryPbufferARB;*/

PFNWGLCHOOSEPIXELFORMATARBPROC WGLContext::wglChoosePixelFormatARB;
PFNWGLGETPIXELFORMATATTRIBIVARBPROC WGLContext::wglGetPixelFormatAttribivARB;
PFNWGLGETEXTENSIONSSTRINGARBPROC	WGLContext::wglGetExtensionsStringARB;

/*PFNGLPOINTPARAMETERFEXTPROC    glPointParameterfEXT;
PFNGLPOINTPARAMETERFVEXTPROC   glPointParameterfvEXT;

PFNWGLCREATEPBUFFERARBPROC		wglCreatePbufferARB;
PFNWGLGETPBUFFERDCARBPROC		wglGetPbufferDCARB;
PFNWGLRELEASEPBUFFERDCARBPROC	wglReleasePbufferDCARB;
PFNWGLDESTROYPBUFFERARBPROC		wglDestroyPbufferARB;
PFNWGLQUERYPBUFFERARBPROC		wglQueryPbufferARB;*/

#define MAX_ATTRIBS 16

WGLContext::WGLContext(CDC* pDC)
{
	DC = pDC;
	INIT_ENTRY_POINT(wglGetExtensionsStringARB, PFNWGLGETEXTENSIONSSTRINGARBPROC);
    INIT_ENTRY_POINT(wglChoosePixelFormatARB, PFNWGLCHOOSEPIXELFORMATARBPROC);	
	if ((!wglGetExtensionsStringARB) && (!wglChoosePixelFormatARB))
		compatibility_mode = true;
	else
		compatibility_mode = false;
}

WGLContext::~WGLContext(void)
{
}

HGLRC WGLContext::Create(int forceformat /* = -1 */, bool stereoMode)
{
	if (!SetupPixelFormat(32, 32, 0, stereoMode, forceformat))
	{
		if (!SetupPixelFormat(24, 16, 0, stereoMode, forceformat))
		{
			AfxMessageBox("Error setting pixel format.");
			return NULL;	// Select graphics mode
		}
	}

	HGLRC hRC;
	if (0 == (hRC = wglCreateContext(DC->m_hDC)))
	{
		AfxMessageBox("Error creating GL context.");
		return NULL;	// Initialize rendering context
	}

	if (!wglMakeCurrent(DC->m_hDC, hRC))
	{
		AfxMessageBox("Error accessing GL context.");
		return NULL;	// Make it the active one
	}

	INIT_ENTRY_POINT(wglGetExtensionsStringARB, PFNWGLGETEXTENSIONSSTRINGARBPROC);
    INIT_ENTRY_POINT(wglChoosePixelFormatARB, PFNWGLCHOOSEPIXELFORMATARBPROC);
	if ((!wglGetExtensionsStringARB) && (!wglChoosePixelFormatARB))
		compatibility_mode = true;
	else
		compatibility_mode = false;

	return hRC;
}

bool WGLContext::SetupPixelFormat(int bpp, int z_buffer_bpp, int samples, bool stereoMode, int forceformat)
{
	int pixelformat = -1;
	PIXELFORMATDESCRIPTOR pfd = 
	{
		sizeof(PIXELFORMATDESCRIPTOR),		// Size
		1,					// Version number
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,				// Support true RGB color
		bpp,					// Color depth
		0,0,0,0,0,0,		// Color bits ignored
		0,					// No alpha buffer
		0,					// Ignore shift bit
		0,					// No accumulation buffer
		0,0,0,0,			// Accumulation bits ignored
		z_buffer_bpp,					// Z-buffer
		0,					// No auxilliary buffer
		PFD_MAIN_PLANE,		// main layer
		0,					// Reserved
		0,0,0					// Layer masks ignored
	};

	if (forceformat==-1)
	{
		//pfd.cColorBits=(unsigned char) 32;
		if (stereoMode)
			pfd.dwFlags |= PFD_STEREO;
		// pfd.cAlphaBits= 8;
			// Match closest pixel format
		if (0 == (pixelformat = ChoosePixelFormat(DC->GetSafeHdc(), &pfd)))
			return false;

		// Set closest match
		if (!SetPixelFormat(DC->GetSafeHdc(), pixelformat, &pfd))
			return false;
		else return true;
	}
	else
	{
		if (!SetPixelFormat(DC->GetSafeHdc(), forceformat, &pfd))
			return false;
		else return true;
	}
}

HGLRC WGLContext::CreatePBuffer(int w, int h, int forceformat /* = -1 */, bool stereoMode)
{
	if (IsWglExtensionSupported("WGL_ARB_pbuffer") && IsWglExtensionSupported("WGL_ARB_pixel_format"))
	{
		/* Initialize WGL_ARB_pbuffer entry points. */
		INIT_ENTRY_POINT(wglCreatePbufferARB, PFNWGLCREATEPBUFFERARBPROC);
		INIT_ENTRY_POINT(wglGetPbufferDCARB, PFNWGLGETPBUFFERDCARBPROC );
		INIT_ENTRY_POINT(wglReleasePbufferDCARB, PFNWGLRELEASEPBUFFERDCARBPROC );
		INIT_ENTRY_POINT(wglDestroyPbufferARB, PFNWGLDESTROYPBUFFERARBPROC );
		INIT_ENTRY_POINT(wglQueryPbufferARB, PFNWGLQUERYPBUFFERARBPROC );
		INIT_ENTRY_POINT(wglChoosePixelFormatARB, PFNWGLCHOOSEPIXELFORMATARBPROC);
	}
	else
		return false;

	// make a pixel format descriptor for our offscreen buffer
	PIXELFORMATDESCRIPTOR pfd = 
	{
	sizeof(PIXELFORMATDESCRIPTOR),		// Size
	1,					// Version number
	PFD_DRAW_TO_BITMAP |			// Support bitmap
		PFD_SUPPORT_OPENGL			// Support OpenGL
		,		 	// Support GDI?
	PFD_TYPE_RGBA,				// Support true RGB color
	32,					// Color depth
	0,0,0,0,0,0,				// Color bits ignored
	0,					// No alpha buffer
	0,					// Ignore shift bit
	0,					// No accumulation buffer
	0,0,0,0,				// Accumulation bits ignored
	32,					// No stencil buffer
	0,					// No auxilliary buffer
	PFD_MAIN_PLANE,				// main layer
	0,					// Reserved
	0,0,0					// Layer masks ignored
	};

	// Create a BITMAPINFOHEADER structure to describe the DIB.
	BITMAPINFOHEADER BIH;
	int iSize = sizeof(BITMAPINFOHEADER) ;
	memset(&BIH, 0, iSize);

	// Fill in the header info. 
	BIH.biSize = iSize;
	BIH.biWidth = w;
	BIH.biHeight = h;
	BIH.biPlanes = 1;
	BIH.biBitCount = 32;
	BIH.biCompression = BI_RGB;
	BIH.biClrUsed = 0;

	pfd.dwFlags = WGL_DRAW_TO_PBUFFER_ARB | PFD_SUPPORT_OPENGL;

	int pixelformat;
	int pb[] = {
		WGL_PBUFFER_LARGEST_ARB, true,
		WGL_TEXTURE_FORMAT_ARB, WGL_NO_TEXTURE_ARB,
		WGL_TEXTURE_TARGET_ARB, WGL_NO_TEXTURE_ARB,
		0};
	void *data = NULL;

	HDC	MYhDC = DC->m_hDC;
	HGLRC offscreen;
	HPBUFFERARB pbuf;
	HDC pbufDC;
	HBITMAP bitbuffer;

	// Query for a suitable pixel format
	int iattributes[2*MAX_ATTRIBS];
	float fattributes[2*MAX_ATTRIBS];
	int nfattribs = 0;
	int niattribs = 0;
	// arrays must be 0-terminated
	for ( int a = 0; a < 2*MAX_ATTRIBS; a++ )
	{
		iattributes[a] = 0;
		fattributes[a] = 0;
	}
	// pixel format must be p-buffer capable.
	iattributes[2*niattribs ] = WGL_DRAW_TO_PBUFFER_ARB;
	iattributes[2*niattribs+1] = true;
	niattribs++;
	// Support OpenGL (generally implicit)
	iattributes[2*niattribs ] = WGL_SUPPORT_OPENGL_ARB;
	iattributes[2*niattribs+1] = true;
	niattribs++;
	// Not double buffered (no need)
	iattributes[2*niattribs ] = WGL_DOUBLE_BUFFER_ARB;
	iattributes[2*niattribs+1] = false;
	niattribs++;
	// Give me acceleration or give me death
	iattributes[2*niattribs ] = WGL_ACCELERATION_ARB;
	iattributes[2*niattribs+1] = WGL_FULL_ACCELERATION_ARB;
	niattribs++;
	// 8 bits minimum on each channel
	iattributes[2*niattribs ] = WGL_RED_BITS_ARB;
	iattributes[2*niattribs+1] = 8;
	niattribs++;
	iattributes[2*niattribs ] = WGL_GREEN_BITS_ARB;
	iattributes[2*niattribs+1] = 8;
	niattribs++;
	iattributes[2*niattribs ] = WGL_BLUE_BITS_ARB;
	iattributes[2*niattribs+1] = 8;
	niattribs++;
	/*if (isWglExtensionSupported("GL_ARB_multisample"))
	{
		// 4X multisampling, hoowa
		iattributes[2*niattribs]=WGL_SAMPLE_BUFFERS_ARB;
		iattributes[2*niattribs+1]=GL_TRUE;
		niattribs++;
		iattributes[2*niattribs]=WGL_SAMPLES_ARB;
		iattributes[2*niattribs+1]=4;
		niattribs++;
		glEnable(GL_MULTISAMPLE_ARB);
	}*/
	// Now obtain a list of pixel formats that meet these minimum
	// requirements.
	int pformat[128];
	unsigned int nformats = 0;

	if (!wglChoosePixelFormatARB( wglGetCurrentDC(), iattributes, NULL, 128, pformat, &nformats))
	{
		// no suitable pixel format
		//cout << "No suitable pixel format found.  Try disabling anti-aliasing." << endl;
		//exit(1);
		return false;
	}
	pixelformat = GetPixelFormat(wglGetCurrentDC());
	pbuf = wglCreatePbufferARB(MYhDC, pformat[0], w, h, pb);
	pbufDC = wglGetPbufferDCARB( pbuf );
	offscreen = wglCreateContext(pbufDC);
	if (!wglMakeCurrent(pbufDC, offscreen))
	{
		return false;
		//exit(1);
	}
	CDC* offDC = new CDC();
	offDC->Attach(pbufDC);

	/*

	//else
	{
		offDC = new CDC();
		offDC->CreateCompatibleDC(this->GetDC());
		bitbuffer = CreateDIBSection(offDC->m_hDC, (BITMAPINFO*)&BIH, DIB_RGB_COLORS, &data, NULL, 0);

		wglDeleteContext(child->hRC);
		//::ReleaseDC(this->m_hWnd, MYhDC);
		ReleaseDC(this->GetDC());

		

		SelectObject(offDC->m_hDC, bitbuffer);
		pixelformat = ChoosePixelFormat(offDC->GetSafeHdc(), &pfd);
		SetPixelFormat(offDC->m_hDC, pixelformat, &pfd);

		offscreen = wglCreateContext(offDC->m_hDC);
		if (!wglMakeCurrent(offDC->m_hDC, offscreen))
			AfxMessageBox("Error: GL Context failed to initialize.");

		//this->pDC = offDC;
		//this->hRC = offscreen;
	}

	pDC = offDC;
	hRC = offscreen;
	*/

	return offscreen;
}


bool WGLContext::IsWglExtensionSupported(const char *extstring)
{
    char *s = (char*) wglGetExtensionsStringARB(wglGetCurrentDC()); //Get extension-string
    char *temp = strstr(s, extstring);            //Search it
	if (temp)
		return temp != NULL;
	else
		return false;
}

bool WGLContext::IsExtensionSupported(const char *extstring)
{
    char *s = (char*) glGetString(GL_EXTENSIONS); //Get extension-string
	if (s)
	{
		char *temp = strstr(s, extstring);            //Search it
		return temp != NULL;
	}
	else
		return false;
}

int WGLContext::GetAAFormat(int samples)
{
	if (!compatibility_mode)
	{
		int iattributes[2*MAX_ATTRIBS];
		float fattributes[2*MAX_ATTRIBS];
		int nfattribs = 0;
		int niattribs = 0;
		// attribute arrays must be 0-terminated
		for (int a = 0; a < 2*MAX_ATTRIBS; a++ )
		{
			iattributes[a] = 0;
			fattributes[a] = 0;
		}
		// Support OpenGL (generally implicit)
		iattributes[2*niattribs ] = WGL_SUPPORT_OPENGL_ARB;
		iattributes[2*niattribs+1] = true;
		niattribs++;

		// double-buffered
		iattributes[2*niattribs ] = WGL_DOUBLE_BUFFER_ARB;
		iattributes[2*niattribs+1] = true;
		niattribs++;

		// Give me acceleration or give me death
		iattributes[2*niattribs ] = WGL_ACCELERATION_ARB;
		iattributes[2*niattribs+1] = WGL_FULL_ACCELERATION_ARB;
		niattribs++;

		// 8 bits minimum on each channel
		iattributes[2*niattribs ] = WGL_RED_BITS_ARB;
		iattributes[2*niattribs+1] = 8;
		niattribs++;

		iattributes[2*niattribs ] = WGL_GREEN_BITS_ARB;
		iattributes[2*niattribs+1] = 8;
		niattribs++;

		iattributes[2*niattribs ] = WGL_BLUE_BITS_ARB;
		iattributes[2*niattribs+1] = 8;
		niattribs++;

		iattributes[2*niattribs]=WGL_SAMPLE_BUFFERS_ARB;
		iattributes[2*niattribs+1]=(samples>0) ? GL_TRUE : GL_FALSE;
		niattribs++;

		iattributes[2*niattribs]=WGL_SAMPLES_ARB;
		iattributes[2*niattribs+1]=samples;
		niattribs++;

		// Now obtain a list of pixel formats that meet these minimum
		// requirements.
		int pformat[128];
		unsigned int nformats = 0;

		if (!wglChoosePixelFormatARB( DC->GetSafeHdc(), iattributes, NULL, 128, pformat, &nformats))
		{
			// no suitable pixel format
			//cout << "No suitable pixel format found.  Try disabling anti-aliasing." << endl;
			return -1;
		}
		else
		{
			return pformat[0];
		}
	}
	return -1;
}