// DialogStereo.cpp : implementation file
//

#include "stdafx.h"
#include "ParticleVis.h"
#include "DialogStereo.h"
#include <sstream>
#include <string>

using namespace std;

// DialogStereo dialog

IMPLEMENT_DYNAMIC(DialogStereo, CDialog)
DialogStereo::DialogStereo(Camera& incam, CWnd* pParent /*=NULL*/)
	: CDialog(DialogStereo::IDD, pParent), cam(incam)
{
}

DialogStereo::~DialogStereo()
{
}

void DialogStereo::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_FOCALSLIDER, focalLength);
	DDX_Control(pDX, IDC_EYESLIDER, eyeSep);
	DDX_Control(pDX, IDC_FOCALEDIT, focalOut);
	DDX_Control(pDX, IDC_EYEEDIT, eyeOut);
}


BEGIN_MESSAGE_MAP(DialogStereo, CDialog)
	ON_WM_HSCROLL()
END_MESSAGE_MAP()

// DialogStereo message handlers

BOOL DialogStereo::OnInitDialog()
{
	CDialog::OnInitDialog();
	eyeSep.SetRange(1, 200);
	focalLength.SetRange(1, 100);	

	eyeSep.SetPos( (int)(cam.getSeparation() * 5000.0) );
	focalLength.SetPos( (int)(cam.getFocalRatio() * 100.0) );
	OnHScroll(NULL, NULL, NULL);
	return TRUE;
}

void DialogStereo::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	int es = eyeSep.GetPos();
	int fl = focalLength.GetPos();

	cam.setSeparation( (double)es / 5000.0 );
	cam.setFocalRatio( (double)fl / 100.0 );

	stringstream out;
	out << (100.0*cam.getSeparation()) << "%";
	eyeOut.SetWindowText(out.str().c_str());

	stringstream out2;
	out2 << cam.getFocalRatio();
	focalOut.SetWindowText(out2.str().c_str());
}