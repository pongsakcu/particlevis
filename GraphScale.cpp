#include "stdafx.h"
#include "GraphScale.h"
#include "WGLFont.h"

#include <stdio.h>
#include <math.h>

GraphScale::GraphScale(void)
{
	for (int i = 0; i < 3; i++)
	{
		lowerVals[i] = 0;
		upperVals[i] = 100;
		upperCoord[i] = 100;
		camera[i] = 150;
		gridIncrement[i] = 1.0;
		gridWeight[i] = 1.0;
		subs[i] = 4;
		gridOffset[i] = 0;
	}
	drawXY = drawXZ = drawYZ = false;
	drawXScale = drawYScale = drawZScale = true;
	offset = 1.0;
}

GraphScale::~GraphScale(void)
{
	glDeleteLists(base, 96);
}

void GraphScale::SetWorldBox(double* vector)
{
	SetWorldBox(vector[0], vector[1], vector[2]);
}
void GraphScale::SetWorldBox(double _X, double _Y, double _Z)
{
	upperCoord[X] = _X;
	upperCoord[Y] = _Y;
	upperCoord[Z] = _Z;
}

void GraphScale::SetXRange(double lower, double upper)
{
	if (lower <= upper)
	{
		lowerVals[X] = lower;
		upperVals[X] = upper;
	}
}
void GraphScale::SetYRange(double lower, double upper)
{
	if (lower <= upper)
	{
		lowerVals[Y] = lower;
		upperVals[Y] = upper;
	}
}
void GraphScale::SetZRange(double lower, double upper)
{
	if (lower <= upper)
	{
		lowerVals[Z] = lower;
		upperVals[Z] = upper;
	}
}
void GraphScale::SetGraphOffset(double off)
{
	offset = off;
}
void GraphScale::SetCamera(double location[3])
{
	SetCamera(location[X], location[Y], location[Z]);
}
void GraphScale::SetCamera(double _X, double _Y, double _Z)
{
	camera[X] = _X;
	camera[Y] = _Y;
	camera[Z] = _Z;
}
void GraphScale::SetXGrid(double increment, int subdivisions, double weight, bool enabled)
{
	if (increment != 0)
		gridIncrement[X] = fabs(increment);
	if (subdivisions != 0)
		subs[X] = subdivisions;
	if (weight != 0)
		gridWeight[X] = weight;
	drawXScale = enabled;
}
void GraphScale::SetYGrid(double increment, int subdivisions, double weight, bool enabled)
{
	if (increment != 0)
		gridIncrement[Y] = fabs(increment);
	if (subdivisions != 0)
		subs[Y] = subdivisions;
	if (weight != 0)
		gridWeight[Y] = weight;
	drawYScale = enabled;
}
void GraphScale::SetZGrid(double increment, int subdivisions, double weight, bool enabled)
{
	if (increment != 0)
		gridIncrement[Z] = fabs(increment);
	if (subdivisions != 0)
		subs[Z] = subdivisions;
	if (weight != 0)
		gridWeight[Z] = weight;
	drawZScale = enabled;
}

void GraphScale::UpdateAll()
{
    if (camera[X] < 0) drawYZ = true;
		else drawYZ = false;
	if (camera[Y] < 0) drawXZ = true;
		else drawXZ = false;
	if (camera[Z] < 0) drawXY = true;
		else drawXY = false;
	for (int i = 0; i < 3; i++)
	{
		worldInc[i] = (upperCoord[i] / (upperVals[i] - lowerVals[i])) * gridIncrement[i];
		gridOffset[i] = -fmod(lowerVals[i], gridIncrement[i]) * (upperCoord[i] / (upperVals[i] - lowerVals[i]));
	}
}

bool GraphScale::Transform() // add the transforms to place data points into world coords
{
	double temp[3];
	for (int i = 0; i < 3; i++)
	{
		if (upperVals[i] == lowerVals[i]) temp[i] = 1;
		else temp[i] = (upperCoord[i] / (upperVals[i] - lowerVals[i]));
	}
	
	glScaled(temp[X], temp[Y], temp[Z]);
	glTranslated(-lowerVals[X], -lowerVals[Y], -lowerVals[Z]);
	return true;
}

// Draws the grids.  Drawmode actually does nothing since these are lines all the time.
bool GraphScale::Draw(int drawmode)
{
	UpdateAll();
	double temp = 0;

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(0.25, 0.25, 0.25, 0.25);
	DrawSubPlanes();

	glColor4f(0.5, 0.5, 0.5, 0.5);
	DrawPlanes();

	//glBlendFunc(GL_SRC_ALPHA_SATURATE, GL_ONE);
	glColor4f(1, 1, 1, 1);
	DrawScales();

	//glColor4f(1, 1, 1, 1);
	//drawChars("0WN3D", 0, 0, 0);

	return true;
}
void GraphScale::DrawScales()
{
	double temp[3], value = 0;
	char buffer[16];
	temp[0] = 0;
	temp[1] = 0;
	temp[2] = gridOffset[Z];

	for (int i = 0; i < 2; i++)
	{
		if (i==X) // on XY
		{
			if (!drawXScale)
				continue;
			temp[0] = 0;
			temp[1] = -0.01;//upperCoord[Y]*1.05;
			temp[2] = (drawXY) ? upperCoord[Z]*1.05 : 0;
		}
		if (i==Y) // on YZ
		{
			if (!drawYScale)
				continue;
			temp[0] = (drawYZ) ? upperCoord[X]*1.01 : -0.01;
			temp[1] = 0;
			temp[2] = upperCoord[Z]*1.01;
		}
		if (i==Z) // on XZ
		{
			if (!drawZScale)
				continue;
			temp[0] = upperCoord[X]*1.05;
			temp[1] = (drawXZ) ? upperCoord[Y]*1.05 : 0;
			temp[2] = gridOffset[Z];
		}
		do
		{
			//_gcvt(temp[i] * ((upperVals[i] - lowerVals[i])/upperCoord[i]), 4, buffer);
			value = temp[i] * ((upperVals[i] - lowerVals[i])/upperCoord[i]) + lowerVals[i];
			sprintf(buffer, "%3.3f", value);

			double buflen = strlen(buffer);
			
			if (temp[i] >= 0)
				drawChars(buffer,
				temp[X],
				temp[Y],
				temp[Z], i==Y);
			temp[i] += worldInc[i];
		} while (temp[i] <= upperCoord[i]);
	}
}
void GraphScale::DrawPlanes()
{
	double temp = 0.0;
	// Draw XY plane
	glPushMatrix();
	if (drawXY) glTranslated(0, 0, upperCoord[Z]);
	glLineWidth((GLfloat)gridWeight[Y]);
	glBegin(GL_LINES);
    do // parallel to X
	{
		glVertex3d(0,temp,0);
		glVertex3d(upperCoord[X],temp,0);
		temp += worldInc[Y];
	} while (temp <= upperCoord[Y]);
	glEnd();


	glLineWidth((GLfloat)gridWeight[X]);
	glBegin(GL_LINES);
	temp = 0;
    do // parallel to Y
	{
		glVertex3d(temp,0,0);
		glVertex3d(temp,upperCoord[Y],0);
		temp += worldInc[X];
	} while (temp <= upperCoord[X]);	
	glEnd();
	glPopMatrix();

	// Draw XZ plane
	temp = 0;
	glPushMatrix();
	if (drawXZ) glTranslated(0, upperCoord[Y], 0);
	glLineWidth((GLfloat)gridWeight[Z]);
	glBegin(GL_LINES);

	temp = gridOffset[Z];
    do // parallel to X
	{
		if (temp >= 0)
		{
			glVertex3d(0,0,temp);
			glVertex3d(upperCoord[X],0,temp);
		}
		temp += worldInc[Z];
	} while (temp <= upperCoord[Z]);
	temp = 0;
	glEnd();

	glLineWidth((GLfloat)gridWeight[X]);
	glBegin(GL_LINES);
    do // parallel to Z
	{
		glVertex3d(temp,0,0);
		glVertex3d(temp,0,upperCoord[Z]);
		temp += worldInc[X];
	} while (temp <= upperCoord[X]);
	glEnd();
	glPopMatrix();

	// Draw YZ plane
	temp = 0;
	glPushMatrix();
	if (drawYZ) glTranslated(upperCoord[X], 0, 0);

	glLineWidth((GLfloat)gridWeight[Z]);
	glBegin(GL_LINES);

	temp = gridOffset[Z];
    do // parallel to Y
	{
		if (temp >= 0)
		{
			glVertex3d(0,0,temp);
			glVertex3d(0,upperCoord[Y],temp);
		}
		temp += worldInc[Z];
	} while (temp <= upperCoord[Z]);

	glEnd();

	glLineWidth((GLfloat)gridWeight[Y]);
	glBegin(GL_LINES);
	temp = 0;
    do // parallel to Z
	{
		glVertex3d(0,temp,0);
		glVertex3d(0,temp,upperCoord[Z]);
		temp += worldInc[Y];
	} while (temp <= upperCoord[Y]);	
	glEnd();
	glPopMatrix();
}
void GraphScale::DrawSubPlanes()
{
	double temp = 0;
	int sub = 0;
	// Draw XY plane
	glPushMatrix();
	if (drawXY) glTranslated(0, 0, upperCoord[Z]);
	glLineWidth((GLfloat)(gridWeight[Y]/2.0));
	glBegin(GL_LINES);
    do // parallel to X
	{
		if (sub%subs[Y]!=0)
		{
			glVertex3d(0,temp,0);
			glVertex3d(upperCoord[X],temp,0);
		}
		temp += worldInc[Y]/subs[Y];
		sub++;
	} while (temp <= upperCoord[Y]);
	glEnd();


	glLineWidth((GLfloat)(gridWeight[X]/2.0));
	glBegin(GL_LINES);
	temp = 0;
	sub = 0;
    do // parallel to Y
	{
		if (sub%subs[X]!=0)
		{
			glVertex3d(temp,0,0);
			glVertex3d(temp,upperCoord[Y],0);
		}
		temp += worldInc[X]/subs[X];
		sub++;
	} while (temp <= upperCoord[X]);	
	glEnd();
	glPopMatrix();

	// Draw XZ plane
	temp = 0;
	sub = 0;
	glPushMatrix();
	if (drawXZ) glTranslated(0, upperCoord[Y], 0);
	glLineWidth((GLfloat)(gridWeight[Z]/2.0));
	glBegin(GL_LINES);
	temp = gridOffset[Z];
    do // parallel to X
	{
		if (sub%subs[Z]!=0 && temp >= 0)
		{
			glVertex3d(0,0,temp);
			glVertex3d(upperCoord[X],0,temp);
		}
		temp += worldInc[Z]/subs[Z];
		sub++;
	} while (temp <= upperCoord[Z]);
	temp = 0;
	sub = 0;
	glEnd();

	glLineWidth((GLfloat)(gridWeight[X]/2));
	glBegin(GL_LINES);
    do // parallel to Z
	{
		if (sub%subs[X]!=0 && temp >= 0)
		{
			glVertex3d(temp,0,0);
			glVertex3d(temp,0,upperCoord[Z]);
		}
		temp += worldInc[X]/subs[X];
		sub++;
	} while (temp <= upperCoord[X]);
	glEnd();
	glPopMatrix();


	// Draw YZ plane
	temp = 0;
	sub = 0;
	glPushMatrix();
	if (drawYZ) glTranslated(upperCoord[X], 0, 0);
	glBegin(GL_LINES);

	glLineWidth((GLfloat)gridWeight[Z]/(GLfloat)2);

	temp = gridOffset[Z];
    do // parallel to Y
	{
		if (sub%subs[Z]!=0 && temp >= 0)
		{
			glVertex3d(0,0,temp);
			glVertex3d(0,upperCoord[Y],temp);
		}
		temp += worldInc[Z]/subs[Z];
		sub++;
	} while (temp <= upperCoord[Z]);

	glEnd();

	glLineWidth((GLfloat)(gridWeight[Y]/2));
	glBegin(GL_LINES);

	temp = 0;
	sub = 0;
    do // parallel to Z
	{
		if (sub%subs[Y]!=0)
		{
			glVertex3d(0,temp,0);
			glVertex3d(0,temp,upperCoord[Z]);
		}
		temp += worldInc[Y]/subs[Y];
		sub++;
	} while (temp <= upperCoord[Y]);	
	glEnd();
	glPopMatrix();
}
void GraphScale::drawChars(char* string, double _X, double _Y, double _Z, bool rightaligned)
{
	glColor3d(1, 1, 1);

	font->DrawChars(string, _X, _Y, _Z, rightaligned ? ALIGNRIGHT | ALIGNMIDDLE : ALIGNCENTER | ALIGNTOP );

	/*glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, 1, 0, 1, -1, 1);

	glMatrixMode(GL_MODELVIEW);*/
	//glPushMatrix();

	//glLoadIdentity();
	/*if (rightaligned)
	{
		for (int i = 0; i < strlen(string); i++)
		{
			unsigned int c = string[i] - 32;
			_X -= (agmf[c].gmfCellIncX * fontsize);
		}
	}
	glTranslated(_X, _Y, _Z);
	glScaled(fontsize, fontsize*aspect, 1);

	glPushAttrib(GL_LIST_BIT);							// Pushes The Display List Bits
	glListBase(base - 32);								// Sets The Base Character to 32
	glCallLists((GLsizei)strlen(string), GL_UNSIGNED_BYTE, string);	// Draws The Display List Text
	glPopAttrib();										// Pops The Display List Bits

	glPopMatrix();*/

	/*glMatrixMode(GL_PROJECTION);
	glPopMatrix();*/
}
// stoled from nehe

void GraphScale::BuildFont(HDC hdc)								// Build Our Bitmap Font
{
	//HDC    hdc = NULL; 
	//HGLRC  hglrc, oldhglrc;
	 
	//oldhglrc = wglGetCurrentContext();

	// create a rendering context 
	//hglrc = wglCreateContext (hdc); 
	 
	// make it the calling thread's current rendering context 
	//wglMakeCurrent(hdc, hglrc);
								// Delete The Font


	HFONT	font;										// Windows Font ID
	HFONT	oldfont;									// Used For Good House Keeping

	base = glGenLists(96);								// Storage For 96 Characters

	font = CreateFont(	10,	   						// Height Of Font
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
	//wglUseFontBitmaps(hdc, 32, 96, base);					// Builds 96 Characters Starting At Character 32
	wglUseFontOutlines(hdc, 32, 96, base, 0, 0, WGL_FONT_POLYGONS, agmf); // alt call, uses outlines.  requires an alphabuffer for AA tho.
	SelectObject(hdc, oldfont);							// Reset font back
	DeleteObject(font);									// Delete our font

	//wglMakeCurrent(hdc, oldhglrc);
}