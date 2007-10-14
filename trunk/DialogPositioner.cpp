// DialogPositioner.cpp : implementation file
//

#include "stdafx.h"
#include "ParticleVis.h"
#include "DialogPositioner.h"
#include "ChildView.h"
#include ".\dialogpositioner.h"


// DialogPositioner dialog

IMPLEMENT_DYNAMIC(DialogPositioner, CDialog)
DialogPositioner::DialogPositioner(CChildView* prim, CWnd* pParent /*=NULL*/)
	: CDialog(DialogPositioner::IDD, pParent), primary(prim)
	, radioGroup(0)
{
}

DialogPositioner::~DialogPositioner()
{
}

void DialogPositioner::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Radio(pDX, IDC_CAMPOS, radioGroup);
	DDX_Control(pDX, IDC_CHECK1, camCone);
	DDX_Control(pDX, IDC_CANGLE, coneAngle);
}


BEGIN_MESSAGE_MAP(DialogPositioner, CDialog)
	ON_BN_CLICKED(IDC_CAMPOS, OnBnClickedCampos)
	ON_BN_CLICKED(IDC_CAMTARG, OnBnClickedCamtarg)
	ON_BN_CLICKED(IDC_L1DIR, OnBnClickedL1dir)
	ON_BN_CLICKED(IDC_L2DIR, OnBnClickedL2dir)
	ON_BN_CLICKED(IDC_RADIOREGULAR, OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, OnBnClickedForce)
	ON_BN_CLICKED(IDC_CHECK1, OnBnClickedCheck1)
	ON_BN_CLICKED(IDC_BUTTON1, OnSetConeAngle)
END_MESSAGE_MAP()


// DialogPositioner message handlers

void DialogPositioner::OnBnClickedCampos()
{
	UpdateData(true);
	if (radioGroup == 0)
		primary->SetState(CAMPICK); // cam pick
}

void DialogPositioner::OnBnClickedCamtarg()
{
	UpdateData(true);
	if (radioGroup == 1)
		primary->SetState(CAMPPICK); // cam point pick
}

void DialogPositioner::OnBnClickedL1dir()
{
	UpdateData(true);
	if (radioGroup == 2)
		primary->SetState(LIGHT1PICK);
}

void DialogPositioner::OnBnClickedL2dir()
{
	UpdateData(true);
	if (radioGroup == 3)
		primary->SetState(LIGHT2PICK);
}

void DialogPositioner::OnBnClickedButton1()
{
	UpdateData(true);
	if (radioGroup == 4)
		primary->SetState(NORMAL);
}

void DialogPositioner::OnBnClickedForce()
{
	primary->ForceCamera();
	//regular.SetCheck(BST_CHECKED);
	radioGroup = 4;
	UpdateData(false);
}

BOOL DialogPositioner::OnInitDialog()
{
	CDialog::OnInitDialog();

	radioGroup = 4;
	cangle = primary->primary->coneangle;
	char buffer[255];
	sprintf(buffer, "%g", cangle);
	coneAngle.SetWindowText(buffer);	
	return TRUE;
}

void DialogPositioner::OnBnClickedCheck1() // conedraw
{
	if (camCone.GetCheck())
		primary->DrawCone(true);
	else
		primary->DrawCone(false);
}

void DialogPositioner::UpdateMode(int state) // reselect mode
{
	switch (state)
	{
	case NORMAL:
		radioGroup = 4;
		break;
	case CAMPICK:
		radioGroup = 0;
		break;
	case CAMPPICK:
		radioGroup = 1;
		break;
	case LIGHT1PICK:
		radioGroup = 2;
		break;
	case LIGHT2PICK:
		radioGroup = 3;
		break;
	}
	UpdateData(false);
}

void DialogPositioner::OnSetConeAngle()
{
	char buffer[255];
	coneAngle.GetWindowText(buffer, 255);
	cangle = strtod(buffer, NULL);
	primary->primary->coneangle = cangle;
	primary->primary->redrawCone = true;
}
