// GradientControl.cpp : implementation file
//

#include "stdafx.h"
#include "ParticleVis.h"
#include "GradientControl.h"
#include ".\gradientcontrol.h"
#include <list>

using namespace::std;


// GradientControl dialog

IMPLEMENT_DYNAMIC(GradientControl, CStatic)
GradientControl::GradientControl()
	: CStatic()
{
	gradient = NULL;
}

GradientControl::~GradientControl()
{
}

void GradientControl::DoDataExchange(CDataExchange* pDX)
{
	CStatic::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(GradientControl, CStatic)
	ON_WM_PAINT()
	//ON_WM_DRAWITEM()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()


// GradientControl message handlers

void GradientControl::OnPaint()
{
	//GetClassInfoEx(
	//CStatic::OnPaint();
	CPaintDC dc(this); // device context for painting

	CRect cr;
	GetClientRect(cr);

	CBrush n;
	n.CreateSolidBrush( RGB(0, 0, 0) );
	CBrush old;
	old.FromHandle((HBRUSH)dc.SelectObject(n));
	if (gradient)
	{
		for (int i = cr.left; i < cr.right; i++)
		{
			double index = (double)(i - cr.left) / (double)cr.Width();
			triple rgb = gradient->ConvertToColor(index);
			rgb = rgb * 255.0;
			COLORREF ref = RGB( (int)rgb.x, (int)rgb.y, (int)rgb.z );
			CPen color;
			color.CreatePen(PS_SOLID, 1, ref);
			dc.SelectObject(&color);
			dc.MoveTo(i, cr.top);
			dc.LineTo(i, cr.bottom);
		}
	}


	double width = (double)cr.Width() - 1;
	CPen black;
	CBrush white;
	black.CreatePen(PS_SOLID, 1, RGB(32, 32, 32));
	white.CreateSolidBrush( RGB(224, 224, 224) );
	dc.SelectObject(&black);
	dc.SelectObject(&white);

	list<ControlPoint>::const_iterator knot = gradient->PeekControlList();
	for (int i = 0; i < gradient->ControlSize(); i++)
	{
		int offset = (int)((*knot).n * width);
		int lt = -4, rt = 4;
		if (offset < 4) lt = -offset;
		if (offset > (width-4)) rt = -(offset - (int)width);
		CPoint arrow[3];
		arrow[0].x = offset + cr.left + lt;
		arrow[0].y = cr.top;
		arrow[1].x = offset + cr.left;
		arrow[1].y = cr.top+8;
		arrow[2].x = offset + cr.left + rt;
		arrow[2].y = cr.top;

		dc.Polygon(arrow, 3);

		arrow[0].y = cr.bottom - 1;
		arrow[1].y = cr.bottom - 9;
		arrow[2].y = cr.bottom - 1;

		dc.Polygon(arrow, 3);
		knot++;
	}

	dc.SelectObject(old);
}

BOOL GradientControl::OnEraseBkgnd(CDC* pDC)
{
	return 1;
}

BOOL GradientControl::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.style |= WS_CLIPSIBLINGS;
	return CStatic::PreCreateWindow(cs);
}
