/*  GLWnd.h

	General purpose CWnd-derived openGL window.  Contains a texture
	loader and simple font support.  Subclasses of GLWnd should override
	functions such as Draw() and GLInit() according to their needs.

	By Vincent Hoon, 2005
*/	

#include <vector>

using namespace std;

#pragma once

// GLWnd window

class GLWnd : public CWnd
{
// Construction
public:
	GLWnd();

// Attributes
public:

// Operations
public:

// Overrides
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// Implementation
public:
	virtual ~GLWnd();
	afx_msg void OnPaint();
	bool redraw;

	static vector<unsigned int> textureCache; // Contains all loaded texture indices
	CDC* pDC;		// Device Context
	HGLRC hRC;		// Rendering Context

	static bool stereoMode; // If true, pixel format routine will attempt to gain stereo support
	static int useAA;

	bool minimized; // set to "true" when window is minimized.
protected:	

	int tick;
	int mx, my;		// default variables for mousetracking
	int w, h;		// tracks window size	
	static int pixel_format; // stores an antialiased (or whatever) pixel format
	double yoffset, oldoffset; // mousetracking variable
	GLfloat largest_supported_anisotropy;

	BITMAPINFO *BitmapInfo; /* Bitmap information */
	GLubyte    *BitmapBits; /* Bitmap data */
	BOOL LoadTexture(LPCTSTR fileName, UINT* m_texName);
	bool Lbutton, Mbutton, Rbutton; // indicates the status of various mouse buttons

	LARGE_INTEGER lasttime, thistime, temptime; // variables for high performance timer
	double freq;	// high performance timer frequency (resolution)
	UINT base; // font lists
	virtual void GLInit(void);
	bool SetupPixelFormat(); // initialize pixel format of window
	void BuildFont(HDC hdc, UINT &b, int size = 22); // create a bitmap font to be used by the display

private:
	GLYPHMETRICSFLOAT agmf[255]; // buffer for glyphs

public:
	void drawChars(const char* string, double _X, double _Y, double _Z, double b = 1); // draw bitmapped characters to the screen
	virtual bool Draw(void); // primary rendering function
	void ForceRedraw(); // redraw screen now, flip buffers
	void setDim(int w, int h) { this->w = w; this->h = h; } // set width and height variables

	// Generated message map functions
	DECLARE_MESSAGE_MAP()

	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
};

