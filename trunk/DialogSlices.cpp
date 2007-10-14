// DialogSlices.cpp : implementation file
//

#include "stdafx.h"
#include "ParticleVis.h"
#include "DialogSlices.h"



// DialogSlices dialog

IMPLEMENT_DYNAMIC(DialogSlices, CDialog)
DialogSlices::DialogSlices(GLPlanarSlices* slice, CWnd* pParent /*=NULL*/)
	: CDialog(DialogSlices::IDD, pParent)
{
	slices = slice;
}

DialogSlices::~DialogSlices()
{
}

void DialogSlices::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHECKDUMMY, useDummy);
	DDX_Control(pDX, IDC_CHECKNORM, useLocal);
	DDX_Control(pDX, IDC_CHECKNORMALPHA, useLocalAlpha);
	DDX_Control(pDX, IDC_CHECKADD, addBlend);
	DDX_Control(pDX, IDC_EDITALPHA, max_alpha);
	DDX_Control(pDX, IDC_EDITSIZE, slice_size);
	DDX_Control(pDX, IDC_EDITSLICES, num_slices);
	DDX_Control(pDX, IDC_EDITX, origin_X);
	DDX_Control(pDX, IDC_EDITY, origin_Y);
	DDX_Control(pDX, IDC_EDITZ, origin_Z);
	DDX_Control(pDX, IDC_EDITX2, rot_X);
	DDX_Control(pDX, IDC_EDITY2, rot_Y);
	DDX_Control(pDX, IDC_EDITZ2, rot_Z);
	DDX_Control(pDX, IDC_EDITX3, scale_X);
	DDX_Control(pDX, IDC_EDITY3, scale_Y);
	DDX_Control(pDX, IDC_EDITZ3, scale_Z);
}


BEGIN_MESSAGE_MAP(DialogSlices, CDialog)
END_MESSAGE_MAP()


// DialogSlices message handlers

BOOL DialogSlices::OnInitDialog()
{
	char buffer[255];
	CDialog::OnInitDialog();

	if (slices->GetDummyData())
		useDummy.SetCheck(BST_CHECKED);
	else
		useDummy.SetCheck(BST_UNCHECKED);
	
	if (slices->GetLocal())
		useLocal.SetCheck(BST_CHECKED);
	else
		useLocal.SetCheck(BST_UNCHECKED);

	if (slices->GetLocalAlpha())
		useLocalAlpha.SetCheck(BST_CHECKED);
	else
		useLocalAlpha.SetCheck(BST_UNCHECKED);

	if (slices->GetBlending() == BLEND_ADD)
		addBlend.SetCheck(BST_CHECKED);
	else
		addBlend.SetCheck(BST_UNCHECKED);

	sprintf(buffer, "%g", slices->GetAlpha());
	max_alpha.SetWindowText(buffer);

	sprintf(buffer, "%g", slices->GetRange());
	slice_size.SetWindowText(buffer);

	sprintf(buffer, "%d", slices->GetNumSlices());
	num_slices.SetWindowText(buffer);

	triple o = slices->GetOrigin();
	sprintf(buffer, "%g", o.x);
	origin_X.SetWindowText(buffer);
	sprintf(buffer, "%g", o.y);
	origin_Y.SetWindowText(buffer);
	sprintf(buffer, "%g", o.z);
	origin_Z.SetWindowText(buffer);

	triple rot = slices->GetRotation();
	sprintf(buffer, "%g", rot.x);
	rot_X.SetWindowText(buffer);
	sprintf(buffer, "%g", rot.y);
	rot_Y.SetWindowText(buffer);
	sprintf(buffer, "%g", rot.z);
	rot_Z.SetWindowText(buffer);

	triple scale = slices->GetScale();
	sprintf(buffer, "%g", scale.x);
	scale_X.SetWindowText(buffer);
	sprintf(buffer, "%g", scale.y);
	scale_Y.SetWindowText(buffer);
	sprintf(buffer, "%g", scale.z);
	scale_Z.SetWindowText(buffer);

	return TRUE;
}

void DialogSlices::OnOK()
{
	char buffer[64];
	double value;

	if (useDummy.GetCheck())
		slices->SetDummyData(true);
	else
		slices->SetDummyData(false);

	if (useLocal.GetCheck())
		slices->SetLocal(true);
	else
		slices->SetLocal(false);

	if (useLocalAlpha.GetCheck())
		slices->SetLocalAlpha(true);
	else
		slices->SetLocalAlpha(false);

	if (addBlend.GetCheck())
		slices->SetBlending(BLEND_ADD);
	else
		slices->SetBlending(BLEND_OVER);

	max_alpha.GetWindowText(buffer, 64);
	value = strtod(buffer, NULL);
	if (value >= 0)
		slices->SetAlpha(value);

	slice_size.GetWindowText(buffer, 64);
	value = strtod(buffer, NULL);
	if (value > 0)
		slices->SetRange(value);

	num_slices.GetWindowText(buffer, 64);
	value = strtod(buffer, NULL);
	if ((int)value > 0)
		slices->SetNumSlices((int)value);

	triple o;
	origin_X.GetWindowText(buffer, 64);
	value = strtod(buffer, NULL);
	o.x = value;
	origin_Y.GetWindowText(buffer, 64);
	value = strtod(buffer, NULL);
	o.y = value;
	origin_Z.GetWindowText(buffer, 64);
	value = strtod(buffer, NULL);
	o.z = value;
	slices->SetOrigin(o);

	triple rot;
	rot_X.GetWindowText(buffer, 64);
	value = strtod(buffer, NULL);
	rot.x = value;
	rot_Y.GetWindowText(buffer, 64);
	value = strtod(buffer, NULL);
	rot.y = value;
	rot_Z.GetWindowText(buffer, 64);
	value = strtod(buffer, NULL);
	rot.z = value;
	slices->SetRotation(rot);

	triple scale;
	scale_X.GetWindowText(buffer, 64);
	value = strtod(buffer, NULL);
	scale.x = value;
	scale_Y.GetWindowText(buffer, 64);
	value = strtod(buffer, NULL);
	scale.y = value;
	scale_Z.GetWindowText(buffer, 64);
	value = strtod(buffer, NULL);
	scale.z = value;
	slices->SetScale(scale);

	CDialog::OnOK();
}

void DialogSlices::OnBnClickedButtontexel()
{
	slices->RenderTextures();
}
