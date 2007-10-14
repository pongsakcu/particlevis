#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include "Camera.h"


// DialogStereo dialog

class DialogStereo : public CDialog
{
	DECLARE_DYNAMIC(DialogStereo)

public:
	DialogStereo(Camera& cam, CWnd* pParent = NULL);   // standard constructor
	virtual ~DialogStereo();

// Dialog Data
	enum { IDD = IDD_STEREODIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	Camera& cam;
	DECLARE_MESSAGE_MAP()
public:
	CSliderCtrl focalLength;
	CSliderCtrl eyeSep;
	CEdit focalOut;
	CEdit eyeOut;
	virtual BOOL OnInitDialog();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
};
