// DialogInterfaceColor.cpp : implementation file
//

#include "stdafx.h"
#include "ParticleVis.h"
#include "DialogInterfaceColor.h"
#include "Particle.h"
#include "ChildView.h"

// DialogInterfaceColor dialog

IMPLEMENT_DYNAMIC(DialogInterfaceColor, CDialog)
DialogInterfaceColor::DialogInterfaceColor(CWnd* pParent /*=NULL*/)
	: CDialog(DialogInterfaceColor::IDD, pParent)
{
}

DialogInterfaceColor::~DialogInterfaceColor()
{
}

void DialogInterfaceColor::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHECK1, disableMarkColor);	
}


BEGIN_MESSAGE_MAP(DialogInterfaceColor, CDialog)
	ON_BN_CLICKED(IDC_BGCBUTTON, OnBnClickedBgcbutton)
	ON_BN_CLICKED(IDC_MARKCBUTTON, OnBnClickedMarkcbutton)
	ON_BN_CLICKED(IDC_TEXTCBUTTON, OnBnClickedTextcbutton)
	ON_BN_CLICKED(IDC_BUTTON_PSTART, OnBnClickedButtonPStart)
	ON_BN_CLICKED(IDC_BUTTON_PEND, OnBnClickedButtonPEnd)
END_MESSAGE_MAP()


// DialogInterfaceColor message handlers

void DialogInterfaceColor::OnBnClickedBgcbutton()
{
	float clearcolor[4];
	glGetFloatv(GL_COLOR_CLEAR_VALUE, clearcolor);
	triple bg(clearcolor[0], clearcolor[1], clearcolor[2]);
	GetColor(bg);
	glClearColor(bg.x, bg.y, bg.z, 0.f);
}

void DialogInterfaceColor::OnBnClickedMarkcbutton()
{
	GetColor(Particle::markingColor);
}

void DialogInterfaceColor::OnBnClickedTextcbutton()
{
	GetColor(CChildView::textColor);
}

void DialogInterfaceColor::GetColor(triple& target)
{
	COLORREF CR = RGB(target.x * 255, target.y * 255, target.z * 255);
	CColorDialog cd(CR, CC_FULLOPEN);
	cd.DoModal();
	CR = cd.GetColor();
	int r = GetRValue(CR);
	int g = GetGValue(CR);
	int b = GetBValue(CR);
	if (r >= 0 && g >= 0 && b >= 0)
		target = triple(r / 255.0, g / 255.0, b / 255.0);
}

void DialogInterfaceColor::OnOK()
{
	if (disableMarkColor.GetCheck())
		Particle::useMarkColor = false;
	else
		Particle::useMarkColor = true;
	CDialog::OnOK();
}

void DialogInterfaceColor::OnBnClickedButtonPStart()
{
	GetColor(Particle::pathColor1);
}

void DialogInterfaceColor::OnBnClickedButtonPEnd()
{
	GetColor(Particle::pathColor2);
}

BOOL DialogInterfaceColor::OnInitDialog()
{
	CDialog::OnInitDialog();

	if (!Particle::useMarkColor)
		disableMarkColor.SetCheck(TRUE);
	else
		disableMarkColor.SetCheck(FALSE);

	return TRUE; 
}
