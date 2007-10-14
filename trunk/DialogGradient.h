#pragma once

#include "RGBGradient.h"
#include "afxwin.h"
#include "triple.h"
#include <vector>
#include "GradientControl.h"


using std::vector;

#define STATENORMAL 0
#define STATEADD 1
#define STATEREMOVE 2
#define STATEDRAG 3

// DialogGradient dialog


class DialogGradient : public CDialog
{
	DECLARE_DYNAMIC(DialogGradient)

public:
	DialogGradient(CWnd* pParent = NULL);   // standard constructor
	DialogGradient(RGBGradient* input, CWnd* pParent = NULL);   // standard constructor
	virtual ~DialogGradient();

// Dialog Data
	enum { IDD = IDD_GRADIENTDIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	triple current_color;
	int state;
	int selected_knot;
	int grad_resolution;
	vector<double> knots;
	vector<int> knots_IDs;
	void MapKnot(const ControlPoint &p);
	CRect cr;
	char buffer[128];

	bool BrowseForFile(const char * title, char * fileNamePtr, bool write);
	void LoadGrad();

public:
	GradientControl GradientBox;
	afx_msg void OnBnClickedPickcolorbutton();
	afx_msg void OnBnClickedAddbutton();
	afx_msg void OnBnClickedRemovebutton();
	virtual BOOL OnInitDialog();
	CEdit resolution;
	afx_msg void OnEnChangeResedit();
	CEdit status; // left status text
	CEdit status2;// right status text
	CButton removeButton; // remove color
	CButton addButton;    // add color

	afx_msg void OnPaint();
	RGBGradient* thisgrad;
	CStatic ColorBox;
	CBitmap * lastbitmap;

	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	void UpdateColor();

	DECLARE_MESSAGE_MAP()

	afx_msg void OnBnClickedLoadbutton();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedSavebutton();
	CButton loadButton;
	CButton saveButton;
};