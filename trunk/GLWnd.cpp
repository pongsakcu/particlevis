// GLWnd.cpp : implementation of the GLWnd class
//

#include "stdafx.h"
#include "GLWnd.h"
#include "WGLContext.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace std;

vector<unsigned int> GLWnd::textureCache;
bool GLWnd::stereoMode = false;
int GLWnd::useAA = 0;
int GLWnd::pixel_format = -1;

// GLWnd

GLWnd::GLWnd() :
tick(0),yoffset(0), oldoffset(0)
{
	w = h = 0;
	mx = my = 0;
	QueryPerformanceFrequency(&lasttime);
	freq = (double)lasttime.QuadPart;
	QueryPerformanceCounter(&lasttime);
	Lbutton = Mbutton = Rbutton = false;
	redraw = false;
	minimized = false;
}

GLWnd::~GLWnd()
{
}


BEGIN_MESSAGE_MAP(GLWnd, CWnd)
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONUP()
//	ON_WM_CHILDACTIVATE()
//ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()


// GLWnd message handlers

BOOL GLWnd::PreCreateWindow(CREATESTRUCT& cs) 
{
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	//cs.style &= ~WS_BORDER;
	cs.style |= WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS|CS_OWNDC, 
		::LoadCursor(NULL, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW+1), NULL);

	return TRUE;
}

// On paint, call Draw() to render graphics and swap buffers.
void GLWnd::OnPaint() 
{
	/*wglMakeCurrent(pDC->GetSafeHdc(), hRC); // activate our context
	Draw();	// render
	SwapBuffers(pDC->m_hDC); // swap buffers if present*/
	ValidateRect( NULL ); // validate screen region
}

bool GLWnd::Draw() // local drawing function, defaults to a big X
{
	tick++;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1, 1, -1, 1, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glRotated((float)(tick%360), 0, 0, 1);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	glBegin(GL_LINES);
		glVertex3d(-1, -1, 0);
		glVertex3d(1, 1, 0);
		glVertex3d(-1, 1, 0);
		glVertex3d(1, -1, 0);
	glEnd();

	//redraw = false;
	return true;
}


// GLInit creates and initializes the OpenGL context.
void GLWnd::GLInit(void)
{
	pDC = this->GetDC(); // Get device context
	
	if (NULL == pDC)
	{
		AfxMessageBox("Error acquiring device context.");
		exit(1);		// Creation failure
	}

	glClearColor(0, 0, 0, 0);	

	// set up default matrices
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 1, 0, 1, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// enable various feature flags
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	//glEnable(GL_POINT_SMOOTH);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POLYGON_SMOOTH);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	BuildFont(this->pDC->GetSafeHdc(), base, 20);
}

// Initialize pixel format.
// Set to try to acquire a 32-bit depth
bool GLWnd::SetupPixelFormat(void)
{
	return FALSE;
}

// Assure we have a 4:3 drawing area that is of maximal size
void GLWnd::OnSize(UINT nType, int cx, int cy)
{
	if ((cx > 0) && (cy>0))
	{
		wglMakeCurrent(pDC->GetSafeHdc(), hRC);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		
		glViewport(0, 0, cx, cy); // Reset drawing area
		

		/*if (ratio > (4.0/3.0)) // old forced-square mode
		{
			int width = (int)((double)cy * (4.0 / 3.0));
			glViewport( (cx-width)/2, 0, (GLsizei)width, (GLsizei)cy); // Reset drawing area
		}
		if (ratio < (4.0/3.0))
		{
			int height = (int)((double)cx * (3.0 / 4.0));
			glViewport(0, (cy-height)/2, (GLsizei)cx, (GLsizei)height); // Reset drawing area
			//gluPerspective(50, (double)cx/(double)cx, .25, 2);
		}*/

		w = cx;
		h = cy;
	}
}

// General texture loading routine: take a bitmap and load it into OpenGL
BOOL GLWnd::LoadTexture(LPCTSTR fileName, UINT* m_texName)
{
	wglMakeCurrent(pDC->m_hDC, hRC);

	// Load in texture and rebuild display list.  Returns TRUE for success.
	// Load the bitmap.
    HBITMAP hBmp = (HBITMAP) ::LoadImage (NULL, (LPCTSTR) fileName, IMAGE_BITMAP, 0, 0,
											LR_LOADFROMFILE | LR_CREATEDIBSECTION);

    if (hBmp == NULL)
    {
		CString crap = fileName;
		crap = "Error: unable to load bitmap file: " + crap;
		AfxMessageBox(crap);
		textureCache.push_back(-1);
        return FALSE;
	}

    // Get bitmap info.
    BITMAP BM;
    ::GetObject (hBmp, sizeof (BM), &BM);

    // Must be 24 bit
    if (BM.bmBitsPixel != 24)
	{
		AfxMessageBox("Error: bitmap must be 24 bit");
		textureCache.push_back(-1);
        return FALSE;
	}


    // Generate name for and bind texture.
	//glGenTextures(1, m_texName);
	glBindTexture(GL_TEXTURE_2D, *m_texName);

	//	Set up all the unpack options for single byte packing
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); 
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);

	int x = 0;
	int rowlen = (BM.bmWidth*3);
	unsigned char * data = (unsigned char*)BM.bmBits;
	unsigned char * reverseData = new unsigned char[rowlen*BM.bmHeight];
	for (int i = BM.bmHeight-1; i >= 0; i--)
	{
		for (int j = 0; j < BM.bmWidth*3; j++)
		{
			reverseData[i*rowlen + j] = *data++;
		}
		while (((unsigned long)data % 4) != 0) // assure that the point is on a 4-byte (word) boundary
			data++;
	}

    // Use glu to load the texture + mipmaps
    gluBuild2DMipmaps( GL_TEXTURE_2D, 3, BM.bmWidth, BM.bmHeight, GL_BGR_EXT,
						GL_UNSIGNED_BYTE, reverseData );

	// bind in texture
	glBindTexture(GL_TEXTURE_2D, *m_texName);

	// use textureCache to track all our texture names
	textureCache.push_back(*m_texName);
	textureCache.push_back(0);

	// free memory
	delete[] reverseData;
	DeleteObject(hBmp);
    return TRUE;
}
int GLWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	pDC = this->GetDC(); // Get device context
	
	if (NULL == pDC)
	{
		AfxMessageBox("Error acquiring device context.");
		exit(1);		// Creation failure
	}
	WGLContext default_context(pDC);
	hRC = default_context.Create(pixel_format, stereoMode); 
	if (!hRC)
	{
		AfxMessageBox("Failed to create GL context.");
		//exit(3);
		return -1;
	}

    if (useAA > 0 && pixel_format==-1)
	{
		pixel_format = default_context.GetAAFormat(useAA);
		if (pixel_format != -1)
		{
			// this is so absurd
			// wait no, let's destroy the entire window!
			wglMakeCurrent(pDC->GetSafeHdc(), hRC);
			wglDeleteContext(hRC);
			ReleaseDC(pDC);
			DestroyWindow();
			//return OnCreate(lpCreateStruct);
			return -1;
		}
	}

	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		AfxMessageBox("Error intializing GL extensions.");
		exit(0);
	}
	GLInit();
	return 0;
}

BOOL GLWnd::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

void GLWnd::OnMouseMove(UINT nFlags, CPoint point)
{
	mx = point.x;
	my = point.y;
	CWnd::OnMouseMove(nFlags, point);
}

void GLWnd::OnLButtonDown(UINT nFlags, CPoint point)
{
	this->SetCapture();
	Lbutton = true;
}

void GLWnd::OnLButtonUp(UINT nFlags, CPoint point)
{
	ReleaseCapture();
	Lbutton = false;
}

void GLWnd::OnMButtonDown(UINT nFlags, CPoint point)
{
	this->SetCapture();
	Mbutton = true;
	//mx = point.x;
	//my = point.y;
	//oldoffset = yoffset;
}

void GLWnd::OnMButtonUp(UINT nFlags, CPoint point)
{
	ReleaseCapture();
	Mbutton = false;
}

void GLWnd::OnRButtonDown(UINT nFlags, CPoint point)
{
	this->SetCapture();
	Rbutton = true;
}

void GLWnd::OnRButtonUp(UINT nFlags, CPoint point)
{
	ReleaseCapture();
	Rbutton = false;
}

void GLWnd::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
}

void GLWnd::drawChars(const char* string, double _X, double _Y, double _Z, double b)
{
	//glDisable(GL_TEXTURE_2D);
	glDisable( GL_DEPTH_TEST );
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA_SATURATE, GL_ONE);
	//glColor4d(1, 1, 1, 1);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, 1, 0, 1, 0, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	glLoadIdentity();
	glRasterPos3d(_X, _Y, _Z);
	//glTranslated(_X, _Y, 0);
	//glScaled(b*0.024, b*0.032, 1);

	glPushAttrib(GL_LIST_BIT);							// Pushes The Display List Bits
	glListBase(base - 32);								// Sets The Base Character to 32
	glCallLists((GLsizei)strlen(string), GL_UNSIGNED_BYTE, string);	// Draws The Display List Text
	glPopAttrib();										// Pops The Display List Bits


	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glEnable( GL_DEPTH_TEST );
}

void GLWnd::BuildFont(HDC hdc, UINT &b, int size) // Build our outline Font
{
	wglMakeCurrent(pDC->m_hDC, hRC);
	HFONT	font;										// Windows Font ID
	HFONT	oldfont;									// Used For Good House Keeping

	b = glGenLists(96);								// Storage For 96 Characters

	font = CreateFont(	size,	   						// Height Of Font
						0,								// Width Of Font
						0,								// Angle Of Escapement
						0,								// Orientation Angle
						FW_NORMAL,						// Font Weight
						FALSE,							// Italic
						FALSE,							// Underline
						FALSE,							// Strikeout
						ANSI_CHARSET,					// Character Set Identifier
						OUT_TT_PRECIS,					// Output Precision
						CLIP_DEFAULT_PRECIS,			// Clipping Precision
						ANTIALIASED_QUALITY,			// Output Quality		
						FF_DONTCARE|DEFAULT_PITCH,		// Family And Pitch
						"Verdana");					// Font Name

	oldfont = (HFONT)SelectObject(hdc, font);           // Selects The Font We Want
	wglUseFontBitmaps(hdc, 32, 96, b);					// Builds 96 Characters Starting At Character 32
	//wglUseFontOutlines(hdc, 32, 96, b, 0, 0, WGL_FONT_POLYGONS, agmf); // alt call, uses outlines.  requires an alphabuffer for AA tho.
	SelectObject(hdc, oldfont);							// Reset font back
	DeleteObject(font);									// Delete our font
}

void GLWnd::ForceRedraw()
{
	Invalidate(TRUE);
	redraw = true;
	Draw();
	//glFinish();
	if (!SwapBuffers(pDC->m_hDC))
		exit(1);
}



/* OpenGL Utility functions */

//PFNWGLGETEXTENSIONSSTRINGARBPROC	wglGetExtensionsStringARB;

// This function returns true if the passed extension is supported by OpenGL
bool isExtensionSupported(const char *extstring)
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

// Return true if WGL extension is supported
bool isWglExtensionSupported(const char *extstring)
{
    char *s = (char*) wglGetExtensionsStringARB(wglGetCurrentDC()); //Get extension-string
    char *temp = strstr(s, extstring);            //Search it
    return temp != NULL;
}