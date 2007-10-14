// DialogSplit.cpp : implementation file
//

#include "stdafx.h"
#include "ParticleVis.h"
#include "DialogSplit.h"


// DialogSplit dialog: handles both grid and split operations.

IMPLEMENT_DYNAMIC(DialogSplit, CDialog)
DialogSplit::DialogSplit(ParticleSet* primary, bool griddialog /* = false*/, CWnd* pParent /*=NULL*/) :
CDialog(DialogSplit::IDD, pParent),
prim(primary),
radio_axis(FALSE),
color_1(Particle::pathColor1),
color_2(Particle::pathColor2),
grid_dialog(griddialog)
{
}

DialogSplit::~DialogSplit()
{
}

void DialogSplit::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Radio(pDX, IDC_RADIO_X, radio_axis);
	DDX_Control(pDX, IDC_EDITOX, originX);
	DDX_Control(pDX, IDC_EDITOY, originY);
	DDX_Control(pDX, IDC_EDITOZ, originZ);
	DDX_Control(pDX, IDC_STATIC_OTEXT, origin_text);
	DDX_Control(pDX, IDC_STATIC_ICON, picture);
}


BEGIN_MESSAGE_MAP(DialogSplit, CDialog)
	ON_BN_CLICKED(IDC_BUTTONCOLOR1, OnBnClickedButtoncolor1)
	ON_BN_CLICKED(IDC_BUTTONCOLOR2, OnBnClickedButtoncolor2)
END_MESSAGE_MAP()


// DialogSplit message handlers

BOOL DialogSplit::OnInitDialog()
{
	CDialog::OnInitDialog();	
	radio_axis = Y;	
	if (!grid_dialog)
	{
		originX.SetWindowText("0");
		originY.SetWindowText("0");
		originZ.SetWindowText("0");
	}
	else
	{
		float grid = Particle::GetMaxPosition() / 8.0;
		if (grid <= 0) grid = 0.1;
		char buffer[255];
		sprintf(buffer, "%.2g", grid);
		originX.SetWindowText(buffer);
		originY.SetWindowText(buffer);
		originZ.ShowWindow(SW_HIDE);
		origin_text.SetWindowText("Spacing:");
		picture.SetIcon(AfxGetApp()->LoadIcon(IDI_GRIDICON));
	}
	UpdateData(false);
	return TRUE;
}
	
void DialogSplit::OnOK()
{
	UpdateData(true);
	if (radio_axis < 0 || radio_axis > 2) return;
	char buffer[255];

	if (!grid_dialog)
	{
		triple origin;
		originX.GetWindowText(buffer, 255);
		double val = strtod(buffer, NULL);
		origin.x = val;

		originY.GetWindowText(buffer, 255);
		val = strtod(buffer, NULL);
		origin.y = val;

		originZ.GetWindowText(buffer, 255);
		val = strtod(buffer, NULL);
		origin.z = val;

		prim->AddSplit(radio_axis, origin, color_1, color_2);
	}
	else
	{
		double grid_w, grid_h;
		originX.GetWindowText(buffer, 255);
		double val = strtod(buffer, NULL);
		grid_w = val;

		originY.GetWindowText(buffer, 255);
		val = strtod(buffer, NULL);
		grid_h = val;

		prim->AddGrid(grid_w, grid_h, radio_axis, color_1, color_2);
	}
	CDialog::OnOK();
}

void DialogSplit::OnBnClickedButtoncolor1()
{
	GetColor(color_1);
}

void DialogSplit::OnBnClickedButtoncolor2()
{	
	GetColor(color_2);
}

void DialogSplit::GetColor(triple& target)
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
