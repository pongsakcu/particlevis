// DialogSpriteTuner.cpp : implementation file
//

#include "stdafx.h"
#include "ParticleVis.h"
#include "DialogSpriteTuner.h"
#include ".\dialogspritetuner.h"
#include <GL/glew.h>
#include <sstream>

//extern PFNGLPOINTPARAMETERFEXTPROC    glPointParameterfEXT;
//extern PFNGLPOINTPARAMETERFVEXTPROC   glPointParameterfvEXT;

// DialogSpriteTuner dialog

IMPLEMENT_DYNAMIC(DialogSpriteTuner, CDialog)
DialogSpriteTuner::DialogSpriteTuner(CWnd* pParent /*=NULL*/)
	: CDialog(DialogSpriteTuner::IDD, pParent)
{
}

DialogSpriteTuner::~DialogSpriteTuner()
{
}

void DialogSpriteTuner::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SLIDERSIZE, sliderSize);
	DDX_Control(pDX, IDC_SLIDERLIN, sliderLinear);
	DDX_Control(pDX, IDC_SLIDERQUAD, sliderQuad);
	DDX_Control(pDX, IDC_BSIZE, sizeBox);
	DDX_Control(pDX, IDC_P1, linearBox);
	DDX_Control(pDX, IDC_P2, quadBox);
}


BEGIN_MESSAGE_MAP(DialogSpriteTuner, CDialog)
	ON_WM_HSCROLL()
END_MESSAGE_MAP()


// DialogSpriteTuner message handlers

using namespace std;

void DialogSpriteTuner::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	char buffer[64];

	float atten[3];
	int s, l, q;
	s = sliderSize.GetPos();
	l = sliderLinear.GetPos();
	q = sliderQuad.GetPos();

	size = (double)(s) / 5.0;
	linear = (double)(l - 1) / 10.0;
	quadratic = (double)(q - 1) / 10.0;

	atten[0] = 1.0;
	atten[1] = linear;
	atten[2] = quadratic;


	sprintf(buffer, "%g", size);
	sizeBox.SetWindowText(buffer);
	sprintf(buffer, "%g", linear);
	linearBox.SetWindowText(buffer);
	sprintf(buffer, "%g", quadratic);
	quadBox.SetWindowText(buffer);

	glPointSize(size);
	if (GLEW_ARB_point_parameters)
	{
		glPointParameterfvARB(GL_POINT_DISTANCE_ATTENUATION_ARB, atten);

		glPointParameterfARB( GL_POINT_SIZE_MAX_ARB, 1000.f );
		glPointParameterfARB( GL_POINT_SIZE_MIN_ARB, 1.f );
		glTexEnvf(GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, GL_TRUE);
		glEnable(GL_POINT_SPRITE_ARB);
	}
}

BOOL DialogSpriteTuner::OnInitDialog()
{
	CDialog::OnInitDialog();
	double psize = 1;
	float atten[3];
	glGetDoublev(GL_POINT_SIZE, &psize);
	glGetFloatv(GL_POINT_DISTANCE_ATTENUATION_ARB, atten);

    size = psize;
	linear = atten[1];
	quadratic = atten[2];

	sliderSize.SetRange(1, 500);
	sliderLinear.SetRange(1, 500);
	sliderQuad.SetRange(1, 500);
	UpdateSliders();
	return TRUE;
}

void DialogSpriteTuner::UpdateSliders(void)
{
	char buffer[64];
	sliderSize.SetPos( (int)(size*5.0) );
	sliderLinear.SetPos( (int)(linear*10.0) + 1 );
	sliderQuad.SetPos( (int)(quadratic*10.0) + 1 );

	sprintf(buffer, "%g", size);
	sizeBox.SetWindowText(buffer);
	sprintf(buffer, "%g", linear);
	linearBox.SetWindowText(buffer);
	sprintf(buffer, "%g", quadratic);
	quadBox.SetWindowText(buffer);
}