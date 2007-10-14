// DialogGradient.cpp : implementation file
//

#include "stdafx.h"
#include "ParticleVis.h"
#include "DialogGradient.h"
#include "ChildView.h"
#include <math.h>
#include <list>
#include <fstream>

// DialogGradient dialog

using std::ifstream;
using std::ofstream;

IMPLEMENT_DYNAMIC(DialogGradient, CDialog)
DialogGradient::DialogGradient(CWnd* pParent /*=NULL*/)
	: CDialog(DialogGradient::IDD, pParent)
{
	grad_resolution = 1024;
	thisgrad = new RGBGradient();
	thisgrad->AddControlPoint(0.0, triple(0, 0, 0));
	thisgrad->AddControlPoint(1.0, triple(1, 1, 1));
	thisgrad->CalculateGradient(grad_resolution);
	//cr = CRect(10, 60, 210, 100);
	//cr = CRect(10, 71, 210, 111);
	
	state = STATENORMAL;
	GradientBox.SetGradient(thisgrad);
	lastbitmap = NULL;
}

DialogGradient::DialogGradient(RGBGradient* input, CWnd* pParent /*=NULL*/)
	: CDialog(DialogGradient::IDD, pParent)
{
	grad_resolution = 1024;	
	thisgrad = input;	
	LoadGrad();
	//cr = CRect(10, 60, 210, 100);
	state = STATENORMAL;
	GradientBox.SetGradient(thisgrad);
	lastbitmap = NULL;
}

DialogGradient::~DialogGradient()
{
}

void DialogGradient::MapKnot(const ControlPoint &p)
{
	knots_IDs.push_back(p.id);
	knots.push_back( p.n );
}

void DialogGradient::LoadGrad()
{
	knots_IDs.clear(); // list of knot IDs
	knots.clear(); // list of indices (0..1)
	if (thisgrad->Allocated() == 0)
		thisgrad->CalculateGradient(grad_resolution);
	else
		grad_resolution = thisgrad->Allocated();
	
	list<ControlPoint>::const_iterator knot = thisgrad->PeekControlList();
	for (int i = 0; i < thisgrad->ControlSize(); i++)
	{
		MapKnot((*knot));
		knot++;
	}
}

void DialogGradient::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MYBOX, ColorBox);
	DDX_Control(pDX, IDC_STATICGRAD, GradientBox);
	DDX_Control(pDX, IDC_RESEDIT, resolution);
	DDX_Control(pDX, IDC_STATUSEDIT, status);
	DDX_Control(pDX, IDC_STATUSEDIT2, status2);
	DDX_Control(pDX, IDC_REMOVEBUTTON, removeButton);
	DDX_Control(pDX, IDC_ADDBUTTON, addButton);
	DDX_Control(pDX, IDC_LOADBUTTON, loadButton);
	DDX_Control(pDX, IDC_SAVEBUTTON, saveButton);
}


BEGIN_MESSAGE_MAP(DialogGradient, CDialog)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_BN_CLICKED(IDC_PICKCOLORBUTTON, OnBnClickedPickcolorbutton)
	ON_BN_CLICKED(IDC_ADDBUTTON, OnBnClickedAddbutton)
	ON_BN_CLICKED(IDC_REMOVEBUTTON, OnBnClickedRemovebutton)
	ON_EN_CHANGE(IDC_RESEDIT, OnEnChangeResedit)
	ON_BN_CLICKED(IDC_LOADBUTTON, OnBnClickedLoadbutton)
	ON_BN_CLICKED(IDC_SAVEBUTTON, OnBnClickedSavebutton)
END_MESSAGE_MAP()


// DialogGradient message handlers

void DialogGradient::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	CBrush newbrush, original;
    newbrush.CreateSolidBrush( RGB(current_color.x, current_color.y, current_color.z) );
	original.FromHandle((HBRUSH)dc.SelectObject(&newbrush) );

	//CRect cr(10, 60, 210, 100);
	//MapDialogRect(cr);

	CRect box;//(82, 7, 102, 27);
	ColorBox.GetWindowRect(box);
	dc.Rectangle(box);

	//dc.Rectangle(cr.left+10, cr.top+50, cr.left+310, cr.top+100);
	//dc.Rectangle(cr);
	//int left = cr.left, right = cr.right;
	/*for (int i = cr.left; i < cr.right; i++)
	{
		double index = (double)(i - cr.left) / (double)cr.Width();
		triple rgb = thisgrad->ConvertToColor(index);
		rgb = rgb * 255.0;
		COLORREF ref = RGB( (int)rgb.x, (int)rgb.y, (int)rgb.z );
		CPen color;
		//color.CreateSolidBrush( RGB( (int)rgb.x, (int)rgb.y, (int)rgb.z ));
		color.CreatePen(PS_SOLID, 1, ref);
		dc.SelectObject(&color);
		//dc.Rectangle(i, cr.top, i, cr.bottom);
		dc.MoveTo(i, cr.top);
		dc.LineTo(i, cr.bottom);
	}*/

	double width = (double)cr.Width();
	CPen black;
	black.CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
	dc.SelectObject(&black);
	for (int i = 0; i < knots.size(); i++)
	{
		dc.MoveTo((int)(knots[i]*width) + cr.left-4, cr.top-8);
		dc.LineTo((int)(knots[i]*width) + cr.left, cr.top);
		dc.LineTo((int)(knots[i]*width) + cr.left+4, cr.top-8);
		dc.LineTo((int)(knots[i]*width) + cr.left-4, cr.top-8);
	}
	dc.SelectObject(&original);
	CDialog::OnPaint();
}

void DialogGradient::OnLButtonDown(UINT nFlags, CPoint point)
{	
	double width = (double)cr.Width();
	if (point.x >= cr.left && point.x <= cr.right &&
		point.y >= cr.top && point.y <= cr.bottom)
	{
		if (state == STATENORMAL || state==STATEREMOVE)
		{
			for (int i = 0; i < knots.size(); i++)
			{
				if ((abs((int)(knots[i] * width + cr.left) - point.x) <= 8) &&
					point.y >= cr.top && point.y <= cr.bottom)
				{
					if (state==STATENORMAL)
					{
						state = STATEDRAG;
						selected_knot = i;
						SetCapture();
						//DrawStatus();
						break;
					}
					if (state==STATEREMOVE)
					{
						state = STATENORMAL;
						thisgrad->RemovePointByRef(knots_IDs[i]);
						thisgrad->CalculateGradient(grad_resolution);
						InvalidateRect(cr, FALSE);

						knots.erase( knots.begin() + i );
						knots_IDs.erase( knots_IDs.begin() + i );
						removeButton.SetState(FALSE);
						break;
					}
				}
			}
		}
		if (state==STATEADD)
		{
			state = STATENORMAL;
			double newindex = (double)(point.x - cr.left) / width;
			newindex = max(0.0, min(1.0, newindex));
			knots.push_back(newindex);
			knots_IDs.push_back(thisgrad->AddControlPoint(newindex, current_color));
			thisgrad->CalculateGradient(grad_resolution);
			InvalidateRect(cr, FALSE);
			addButton.SetState(FALSE);
		}
	}
	CDialog::OnLButtonDown(nFlags, point);
}

void DialogGradient::OnLButtonUp(UINT nFlags, CPoint point)
{
	ReleaseCapture();
	if (state==STATEDRAG)
		state = STATENORMAL;
	CDialog::OnLButtonUp(nFlags, point);
	if (state == STATENORMAL)
		status.SetWindowText("");
}

void DialogGradient::OnMouseMove(UINT nFlags, CPoint point)
{
	double width = (double)cr.Width();
	double newindex = (double)(point.x - cr.left) / width;
	ControlPoint temp;
	if (state == STATEDRAG)
	{
		if (newindex >= 0 && newindex <= 1)
		{
			temp = thisgrad->RemovePointByRef(knots_IDs[selected_knot]);
			temp.n = newindex;
			knots_IDs[selected_knot] = thisgrad->AddControlPoint(newindex, triple(temp.r, temp.g, temp.b));
			knots[selected_knot] = newindex;
			thisgrad->CalculateGradient(grad_resolution);		
			InvalidateRect(cr, FALSE);
		}
	}
	else
	{
		triple t = thisgrad->ConvertToColor(newindex);
		temp.r = t.x;
		temp.g = t.y;
		temp.b = t.z;
	}

	if (newindex >= 0 && newindex <= 1)
	{
		sprintf(buffer, "Index: %#4.4g", newindex);
		status.SetWindowText(buffer);
		sprintf(buffer, "Color: R %3.3d G %3.3d B %3.3d", (int)(temp.r * 255.0), (int)(temp.g * 255.0), (int)(temp.b * 255.0));
		status2.SetWindowText(buffer);
	}

	CDialog::OnMouseMove(nFlags, point);
}

void DialogGradient::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	int i = 0;
	if (nIDCtl==IDC_STATICGRAD)
	{
		CPaintDC dc(this); // device context for painting
		CBrush newbrush, original;
		newbrush.CreateSolidBrush( RGB(current_color.x, current_color.y, current_color.z) );
		original.FromHandle((HBRUSH)dc.SelectObject(&newbrush) );

		CRect box;
		ColorBox.GetWindowRect(box);
		dc.Rectangle(box);

		for (int i = cr.left; i < cr.right; i++)
		{
			double index = (double)(i - cr.left) / (double)cr.Width();
			triple rgb = thisgrad->ConvertToColor(index);
			rgb = rgb * 255.0;
			COLORREF ref = RGB( (int)rgb.x, (int)rgb.y, (int)rgb.z );
			CPen color;
			color.CreatePen(PS_SOLID, 1, ref);
			dc.SelectObject(&color);
			dc.MoveTo(i, cr.top);
			dc.LineTo(i, cr.bottom);
		}
	}
	else
		CDialog::OnDrawItem(nIDCtl, lpDrawItemStruct);
}

BOOL DialogGradient::PreCreateWindow(CREATESTRUCT& cs)
{
	//cs.style |= LBS_OWNERDRAWFIXED;
	return CDialog::PreCreateWindow(cs);
}

void DialogGradient::OnBnClickedPickcolorbutton()
{
	// Bring up a color dialog to select a new RGB value to use for Adding
	CColorDialog cd(RGB(current_color.x, current_color.y, current_color.z), 0, this);
	cd.DoModal();
	COLORREF CR = cd.GetColor();
	int r = GetRValue(CR);
	int g = GetGValue(CR);
	int b = GetBValue(CR);
	current_color = triple(r/255.0, g/255.0, b/255.0);
	UpdateColor();
}

void DialogGradient::OnBnClickedAddbutton()
{
	// set internal state, and keep Add button "depressed"
	state = STATEADD;
	addButton.SetState(TRUE);
	removeButton.SetState(FALSE);
}

void DialogGradient::OnBnClickedRemovebutton()
{
	// set internal state, and keep Remove button "depressed"
	state = STATEREMOVE;
	removeButton.SetState(TRUE);
	addButton.SetState(FALSE);
}

void DialogGradient::UpdateColor()
{
	// Load an appropriately colored bitmap into the ColorBox static control
	CRect r;
	ColorBox.GetClientRect(r);
	int w = r.Width();
	int h = r.Height();

	CDC dc;
	dc.CreateCompatibleDC(GetDC());
	CBitmap* color = new CBitmap();
	color->CreateCompatibleBitmap(GetDC(), w, h);
	CBrush brush;
	brush.CreateSolidBrush( RGB(current_color.x * 255, current_color.y * 255, current_color.z * 255));
    dc.SelectObject(*color);
	dc.SelectObject(brush);
	dc.Rectangle(0, 0, w, h);
    ColorBox.SetBitmap(*color);

	if (lastbitmap != NULL)
		delete lastbitmap;
	lastbitmap = color;
	Invalidate(true);
}

BOOL DialogGradient::OnInitDialog()
{
	CDialog::OnInitDialog();
	UpdateColor();

	sprintf(buffer, "%d", grad_resolution);
	resolution.SetWindowText(buffer);
	
	loadButton.SetIcon( AfxGetApp()->LoadIcon(IDI_OPENICON) );
	saveButton.SetIcon( AfxGetApp()->LoadIcon(IDI_WRITEICON) );

	// Manual rectangle sizer.  Evidently there's no easy way to do this otherwise.

	cr = CRect(10, 71, 210, 111);
	MapDialogRect(cr);
	return TRUE;
}

void DialogGradient::OnEnChangeResedit()
{
	resolution.GetWindowText(buffer, 128);
	double value = strtod(buffer, NULL);
	if (value > 0 && value <= 16777216)
	{
		grad_resolution = (int)value;
		thisgrad->CalculateGradient(grad_resolution);
		InvalidateRect(cr, FALSE);
	}
}
void DialogGradient::OnBnClickedLoadbutton()
{
	char filename[MAX_PATH];
	char buffer[2048];
	string input;
	if (BrowseForFile("Load Gradient from File", filename, false))
	{
		ifstream infile;
		infile.open(filename, std::ios::in);
		if (!infile.fail())
			std::getline(infile, input, '\n');
		infile.close();

		thisgrad->SerializeIn(input);
		OnEnChangeResedit();
		LoadGrad();
	}
}

void DialogGradient::OnBnClickedSavebutton()
{
	char filename[MAX_PATH];
	string output;
	if (thisgrad->SerializeOut(output) &&
		BrowseForFile("Save Gradient to File", filename, true))
	{
		ofstream outfile;
		outfile.open(filename, std::ios::out);
		if (!outfile.fail())
			outfile << output;
		outfile.close();
	}
}

// Essentially copied from ChildView.
bool DialogGradient::BrowseForFile(const char * title, char * fileNamePtr, bool write)
{
	CString gradpath = CChildView::exePath + "gradients";
	OPENFILENAME browse;
	ZeroMemory(&browse, sizeof(browse));
	browse.lStructSize = sizeof(browse);
	fileNamePtr[0] = '\0';
	browse.hwndOwner = this->m_hWnd;	
	browse.lpstrFile = fileNamePtr;
	browse.lpstrFilter = "Gradient Files\0*.rgb\0All Files\0*.*";
	browse.lpstrTitle = title;
	browse.nMaxFile = MAX_PATH;
	browse.nFilterIndex = 1;
	browse.lpstrInitialDir = gradpath;
	if (write)
	{
		browse.lpstrDefExt = "rgb";
		browse.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;			
		return GetSaveFileName(&browse)!=0;
	}
	else
	{
		browse.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
		return GetOpenFileName(&browse)!=0; // create the file dialog
	}
}
