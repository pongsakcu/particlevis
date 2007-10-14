// BoundsDialog.cpp : implementation file
//

#include "stdafx.h"
#include "ParticleVis.h"
#include "BoundsDialog.h"

// BoundsDialog dialog

IMPLEMENT_DYNAMIC(BoundsDialog, CDialog)
BoundsDialog::BoundsDialog(ParticleSet* primary, double *bounds, CWnd* pParent /*=NULL*/)
	: CDialog(BoundsDialog::IDD, pParent), prim(primary), initialBounds(bounds)
{
}

BoundsDialog::~BoundsDialog()
{
}

void BoundsDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, XMin);
	DDX_Control(pDX, IDC_EDIT4, XMax);
	DDX_Control(pDX, IDC_EDIT2, YMin);
	DDX_Control(pDX, IDC_EDIT5, YMax);
	DDX_Control(pDX, IDC_EDIT3, ZMin);
	DDX_Control(pDX, IDC_EDIT6, ZMax);
	DDX_Control(pDX, IDC_CHECK2, enableX);
	DDX_Control(pDX, IDC_CHECK3, enableY);
	DDX_Control(pDX, IDC_CHECK4, enableZ);
	DDX_Control(pDX, IDC_TRANSCHECK, transBound);
	DDX_Control(pDX, IDC_MARKCHECK, markRegion);
}


BEGIN_MESSAGE_MAP(BoundsDialog, CDialog)
END_MESSAGE_MAP()


// BoundsDialog message handlers

BOOL BoundsDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	LoadValues();

	return TRUE;  // return TRUE unless you set the focus to a control
}

void BoundsDialog::OnOK()
{
	char buffer[64];
	char * temp;
	double value, value2;

	if (enableX.GetCheck())
	{
		XMin.GetWindowText(buffer, 64);
		value = strtod(buffer, &temp);
		XMax.GetWindowText(buffer, 64);
		value2 = strtod(buffer, &temp);

		if (value2 >= value)
			prim->SetBound(0, value, value2);
	}
	else
	{
		prim->SetOption(BOUNDCHECKX, false);
	}

	if (enableY.GetCheck())
	{
		YMin.GetWindowText(buffer, 64);
		value = strtod(buffer, &temp);
		YMax.GetWindowText(buffer, 64);
		value2 = strtod(buffer, &temp);

		if (value2 >= value)
			prim->SetBound(1, value, value2);
	}
	else
	{
		prim->SetOption(BOUNDCHECKY, false);
	}

	if (enableZ.GetCheck())
	{
		ZMin.GetWindowText(buffer, 64);
		value = strtod(buffer, &temp);
		ZMax.GetWindowText(buffer, 64);
		value2 = strtod(buffer, &temp);

		if (value2 >= value)
			prim->SetBound(2, value, value2);
	}
	else
	{
		prim->SetOption(BOUNDCHECKZ, false);
	}

	if (transBound.GetCheck())
		prim->SetOption(TRANSBOUNDS, true);
	else
		prim->SetOption(TRANSBOUNDS, false);

	if (markRegion.GetCheck())
		prim->SetOption(MARKBOUNDS, true);
	else
		prim->SetOption(MARKBOUNDS, false);

	CDialog::OnOK();
}

void BoundsDialog::LoadValues()
{
	char buffer[64];
	sprintf(buffer, "%g", initialBounds[0]);
	XMin.SetWindowText(buffer);

	sprintf(buffer, "%g", initialBounds[1]);
	XMax.SetWindowText(buffer);

	sprintf(buffer, "%g", initialBounds[2]);
	YMin.SetWindowText(buffer);

	sprintf(buffer, "%g", initialBounds[3]);
	YMax.SetWindowText(buffer);

	sprintf(buffer, "%g", initialBounds[4]);
	ZMin.SetWindowText(buffer);

	sprintf(buffer, "%g", initialBounds[5]);
	ZMax.SetWindowText(buffer);
    
	if (prim->GetOption(BOUNDCHECKX))
		enableX.SetCheck(1);
	else
		enableX.SetCheck(0);

	if (prim->GetOption(BOUNDCHECKY))
		enableY.SetCheck(1);
	else
		enableY.SetCheck(0);

	if (prim->GetOption(BOUNDCHECKZ))
		enableZ.SetCheck(1);
	else
		enableZ.SetCheck(0);

	if (prim->GetOption(TRANSBOUNDS))
		transBound.SetCheck(1);
	else
		transBound.SetCheck(0);

	if (prim->GetOption(MARKBOUNDS))
		markRegion.SetCheck(1);
	else
		markRegion.SetCheck(0);
}
