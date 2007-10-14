// DialogSurfaceMap.cpp : implementation file
//

#include "stdafx.h"
#include "ParticleVis.h"
#include "DialogSurfaceMap.h"
#include ".\dialogsurfacemap.h"
#include "Particle.h"


// DialogSurfaceMap dialog

IMPLEMENT_DYNAMIC(DialogSurfaceMap, CDialog)
DialogSurfaceMap::DialogSurfaceMap(int& mode, double& scale, CWnd* pParent /*=NULL*/)
	: CDialog(DialogSurfaceMap::IDD, pParent), map_mode(mode), map_scale(scale)
	, radio_map_mode(0)
{
}

DialogSurfaceMap::~DialogSurfaceMap()
{
}

void DialogSurfaceMap::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDITSCALE, EditScale);
	DDX_Radio(pDX, IDC_RADIOLIN, radio_map_mode);
}


BEGIN_MESSAGE_MAP(DialogSurfaceMap, CDialog)
	ON_BN_CLICKED(IDC_RADIOLIN, OnBnClickedRadiolin)
	ON_BN_CLICKED(IDC_RADIOLOG, OnBnClickedRadiolog)
	ON_BN_CLICKED(IDC_RADIOSQR, OnBnClickedRadiosqr)
END_MESSAGE_MAP()


// DialogSurfaceMap message handlers

void DialogSurfaceMap::OnBnClickedRadiolin()
{
	radio_map_mode = 0;
}

void DialogSurfaceMap::OnBnClickedRadiolog()
{
	radio_map_mode = 1;
}

void DialogSurfaceMap::OnBnClickedRadiosqr()
{
	radio_map_mode = 2;
}

BOOL DialogSurfaceMap::OnInitDialog()
{
	CDialog::OnInitDialog();

	if (map_mode == LINEAR)
	{
		CheckRadioButton(IDC_RADIOLIN, IDC_RADIOSQR, IDC_RADIOLIN);
		radio_map_mode = 0;
	}
	if (map_mode == LOG)
	{
		CheckRadioButton(IDC_RADIOLIN, IDC_RADIOSQR, IDC_RADIOLOG);
		radio_map_mode = 1;
	}
	if (map_mode == QUAD)
	{
		CheckRadioButton(IDC_RADIOLIN, IDC_RADIOSQR, IDC_RADIOSQR);
		radio_map_mode = 2;
	}
	
	char buffer[255];
	sprintf(buffer, "%g", map_scale);
	EditScale.SetWindowText(buffer);
	return TRUE;
}

void DialogSurfaceMap::OnOK()
{
	//map_scale = ScaleSlider.GetPos();
	char buffer[255];
	EditScale.GetWindowText(buffer, 255);
	map_scale = strtod(buffer, NULL);

	if (radio_map_mode==0)
		map_mode = LINEAR;
	if (radio_map_mode==1)
		map_mode = LOG;
	if (radio_map_mode==2)
		map_mode = QUAD;

	CDialog::OnOK();
}