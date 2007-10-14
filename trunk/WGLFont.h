#pragma once

#include "stdafx.h"

// Use these constants to align vector text
#define ALIGNLEFT 0x00		// Default horizontal alignment
#define ALIGNRIGHT 0x02		// Align right edge font of font to X=0
#define ALIGNCENTER 0x03	// Align font to be centered in X
#define ALIGNTOP	0x10	// Align top edge of font to Y=0
#define ALIGNMIDDLE	0x20	// Align font to be centered in Y
#define ALIGNBOTTOM	0x00	// Default vertical alignment
// The horizontal and vertical constants can be ORed together

// A Windows OpenGL Font Class, able to generate characters
// in OpenGL using either bitmaps or polygons
//
// By Vincent Hoon, 2005

class WGLFont
{
public:
	WGLFont(HDC context);
	~WGLFont(void);

	bool MakeFont(const char* name, double size, bool usevectors);
	void DrawChars(const char* text, double x, double y, double z, int alignment = 0);

	// These functions only apply to vector fonts
	void SetSize(double s) { font_size = s; }
	void SetAspect(double a) { font_aspect = a; }
    
private:
	HDC hDC;
	GLuint base;
	GLYPHMETRICSFLOAT agmf[96];
	GLYPHMETRICS gmf[96];

	double font_size, font_aspect;
	bool use_vectors, font_loaded;
};
