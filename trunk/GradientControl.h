#pragma once

#include "RGBGradient.h"

// GradientControl dialog

class GradientControl : public CStatic
{
	DECLARE_DYNAMIC(GradientControl)

public:
	GradientControl();   // standard constructor
	virtual ~GradientControl();
	void SetGradient(RGBGradient *grad) { gradient = grad; }

// Dialog Data
	enum { IDD = IDD_GRADIENTDIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	RGBGradient *gradient;

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
};
