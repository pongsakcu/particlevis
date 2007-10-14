#pragma once
#include "afxcmn.h"
#include "afxwin.h"


// DialogSurfaceMap dialog

class DialogSurfaceMap : public CDialog
{
	DECLARE_DYNAMIC(DialogSurfaceMap)

public:
	DialogSurfaceMap(int& mode, double& scale, CWnd* pParent = NULL);   // standard constructor
	virtual ~DialogSurfaceMap();

// Dialog Data
	enum { IDD = IDD_DIALOGSURFACEMAP };
	DECLARE_MESSAGE_MAP()

	CSliderCtrl ScaleSlider;
	CEdit EditScale;
	virtual BOOL OnInitDialog();

protected:
	virtual void OnOK();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	int& map_mode;
	double& map_scale;
public:
	afx_msg void OnBnClickedRadiolin();
	afx_msg void OnBnClickedRadiolog();
	afx_msg void OnBnClickedRadiosqr();
	int radio_map_mode;
};
