#include "WGLFont.h"

WGLFont::WGLFont(HDC context)
{
	hDC = context;
	font_loaded = false;
	use_vectors = false;
}

WGLFont::~WGLFont(void)
{
}

void WGLFont::DrawChars(const char *text, double x, double y, double z, int alignment)
{

	if (!font_loaded)
		return;
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	if (use_vectors)
	{
		double w, h;
		if (font_aspect > 1.0)
		{
			w = font_size;
			h = font_size*font_aspect;
		}
		else
		{
			w = font_size/font_aspect;
			h = font_size;
		}

		if ((alignment&0x0F)==ALIGNRIGHT)
		{
			for (int i = 0; i < strlen(text); i++)
			{
				unsigned int c = text[i] - 32;
				x -= (agmf[c].gmfCellIncX * w);
			}
		}
		else if ((alignment&0x0F)==ALIGNCENTER)
		{
			for (int i = 0; i < strlen(text); i++)
			{
				unsigned int c = text[i] - 32;
				x -= (agmf[c].gmfCellIncX * w) / 2.0;
			}
		}
		if ((alignment&(0xF0))==ALIGNTOP)
		{
			y -= (agmf[text[0] - 32].gmfBlackBoxY * h);
		}
		if ((alignment&(0xF0))==ALIGNMIDDLE)
		{
			y -= (agmf[text[0] - 32].gmfBlackBoxY * h) / 2.0;
		}


		glTranslated(x, y, z);
		glScaled(w, h, 1);
	}
	else
	{
		if ((alignment&0x0F)==ALIGNRIGHT)
		{
			for (int i = 0; i < strlen(text); i++)
			{
				unsigned int c = text[i] - 32;
				x -= gmf[c].gmCellIncX;
			}
		}
		else if ((alignment&0x0F)==ALIGNCENTER)
		{
			for (int i = 0; i < strlen(text); i++)
			{
				unsigned int c = text[i] - 32;
				x -= gmf[c].gmCellIncX / 2;
			}
		}
		glRasterPos3d(x, y, z);
	}	

	glPushAttrib(GL_LIST_BIT);							// Pushes The Display List Bits
	glListBase(base - 32);								// Sets The Base Character to 32
	glCallLists((GLsizei)strlen(text), GL_UNSIGNED_BYTE, text);	// Draws The Display List Text
	glPopAttrib();										// Pops The Display List Bits

	glPopMatrix();
}

bool WGLFont::MakeFont(const char *name, double size, bool usevectors)
{
	HFONT	font;										// Windows Font ID
	HFONT	oldfont;									// Used For Good House Keeping

	base = glGenLists(96);								// Storage For 96 Characters

	font = CreateFont(	(int)size,	   					// Height Of Font
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
						name);					// Font Name

	if (font==0) return false;

	oldfont = (HFONT)SelectObject(hDC, font);           // Selects The Font We Want
	bool success;
	if (usevectors)
	{
		font_size = size;
		success = wglUseFontOutlines(hDC, 32, 96, base, 0, 0, WGL_FONT_POLYGONS, agmf); // alt call, uses outlines.  requires an alphabuffer for AA
	}
	else
	{
		//success = wglUseFontBitmaps(hDC, 32, 96, base);
		for (int f = 32; f <= 128; f++)
		{
			// WTF IS WITH THIS API GOOD LORD
			FIXED one; one.fract = 0; one.value = 1;
			FIXED zero;zero.fract=0; zero.value = 1;
			MAT2 mat;
			mat.eM11 = one;
			mat.eM12 = zero;
			mat.eM21 = zero;
			mat.eM22 = one;
			GetGlyphOutline(hDC, f, GGO_METRICS, &gmf[f-32], 0, NULL, &mat);
		}
		success = wglUseFontBitmaps(hDC, 32, 96, base);
	}
	
	SelectObject(hDC, oldfont);							// Reset font back
	DeleteObject(font);									// Delete our font
	use_vectors = usevectors;
	font_loaded = success;
	return success;
}