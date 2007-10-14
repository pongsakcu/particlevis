// DialogVectorScaler.cpp : implementation file
//

#include "stdafx.h"
#include "ParticleVis.h"
#include "DialogVectorScaler.h"
#include "Particle.h"
#include ".\dialogvectorscaler.h"


// DialogVectorScaler dialog

IMPLEMENT_DYNAMIC(DialogVectorScaler, CDialog)
DialogVectorScaler::DialogVectorScaler(CWnd* pParent /*=NULL*/)
	: CDialog(DialogVectorScaler::IDD, pParent)
{
	parent = (CChildView*)pParent;
	tempTransLength = Particle::transVelocityLength;
	tempRotLength = Particle::rotVelocityLength;
	tempFieldLength = Particle::fieldVelocityLength;
	tempPathLength = Particle::trailLength;
}

DialogVectorScaler::~DialogVectorScaler()
{
}

void DialogVectorScaler::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SLIDERTRANS, sliderTrans);
	DDX_Control(pDX, IDC_SLIDERROT, sliderRot);
	DDX_Control(pDX, IDC_SLIDERFIELD, sliderField);
	DDX_Control(pDX, IDC_SLIDERPATH, sliderPath);
	DDX_Control(pDX, IDC_BSIZE, transEdit);
	DDX_Control(pDX, IDC_P1, rotEdit);
	DDX_Control(pDX, IDC_P2, fieldEdit);
	DDX_Control(pDX, IDC_P4, pathEdit);	
}


BEGIN_MESSAGE_MAP(DialogVectorScaler, CDialog)
	ON_WM_HSCROLL()
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDERTRANS, OnNMReleasedcaptureSlidertrans)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDERROT, OnNMReleasedcaptureSliderrot)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDERFIELD, OnNMReleasedcaptureSliderfield)
	ON_BN_CLICKED(IDC_BUTTONOK, OnBnClickedButtonOK)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDERPATH, OnNMReleasedcaptureSliderpath)
	ON_WM_CAPTURECHANGED()
END_MESSAGE_MAP()


// DialogVectorScaler message handlers

void DialogVectorScaler::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	char buffer[64];

	int trans, rot, field, path;
	double delta_t = 0, delta_r = 0, delta_f = 0;
	int delta_p;
	trans = sliderTrans.GetPos();
	rot = sliderRot.GetPos();
	field = sliderField.GetPos();
	path = sliderPath.GetPos();
	
	delta_t = 1.0+((trans - 250.0) / 250.0)*0.75;
	delta_r = 1.0+((rot - 250.0) / 250.0)*0.75;
	delta_f = 1.0+((field - 250.0) / 250.0)*0.75;
	delta_p = path - 50;

	Particle::transVelocityLength =  tempTransLength * delta_t;
	Particle::rotVelocityLength = tempRotLength * delta_r;
	Particle::fieldVelocityLength = tempFieldLength * delta_f;
	Particle::trailLength = max(0, tempPathLength+delta_p);
	
	UpdateSliders();
	Particle::setup = false; // "dirty" flag

	if (parent)
		parent->ForceRedraw();	
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

BOOL DialogVectorScaler::OnInitDialog()
{
	CDialog::OnInitDialog();

	sliderTrans.SetRange(1, 500);	sliderTrans.SetPos(250);
	sliderRot.SetRange(1, 500);		sliderRot.SetPos(250);
	sliderField.SetRange(1, 500);	sliderField.SetPos(250);	
	sliderPath.SetRange(1, 100);	sliderPath.SetPos(50);
	
	UpdateSliders();
	return TRUE;
}

void DialogVectorScaler::UpdateSliders()
{
	char buffer[64];
	//sliderSize.SetPos( (int)(size*10.0) );
	//sliderLinear.SetPos( (int)(linear*10.0) + 1 );
	//sliderQuad.SetPos( (int)(quadratic*10.0) + 1 );

	sprintf(buffer, "%.3g", Particle::transVelocityLength);
	transEdit.SetWindowText(buffer);
	sprintf(buffer, "%.3g", Particle::rotVelocityLength);
	rotEdit.SetWindowText(buffer);
	sprintf(buffer, "%.3g", Particle::fieldVelocityLength);
	fieldEdit.SetWindowText(buffer);
	sprintf(buffer, "%d", Particle::trailLength);
	pathEdit.SetWindowText(buffer);
}

void DialogVectorScaler::OnNMReleasedcaptureSlidertrans(NMHDR *pNMHDR, LRESULT *pResult)
{
	tempTransLength = Particle::transVelocityLength;
	sliderTrans.SetPos(250);	
	*pResult = 0;
}

void DialogVectorScaler::OnNMReleasedcaptureSliderrot(NMHDR *pNMHDR, LRESULT *pResult)
{
	tempRotLength = Particle::rotVelocityLength;
	sliderRot.SetPos(250);
	*pResult = 0;
}

void DialogVectorScaler::OnNMReleasedcaptureSliderfield(NMHDR *pNMHDR, LRESULT *pResult)
{
	tempFieldLength = Particle::fieldVelocityLength;
	sliderField.SetPos(250);
	*pResult = 0;
}

void DialogVectorScaler::OnNMReleasedcaptureSliderpath(NMHDR *pNMHDR, LRESULT *pResult)
{
	tempPathLength = Particle::trailLength;
	sliderPath.SetPos(50);
	*pResult = 0;
}

void DialogVectorScaler::OnBnClickedButtonOK()
{
	CDialog::OnOK();
}

void DialogVectorScaler::OnCaptureChanged(CWnd *pWnd)
{
	if (parent)
		parent->ForceRedraw();	
	CDialog::OnCaptureChanged(pWnd);
}
