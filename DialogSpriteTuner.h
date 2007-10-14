#pragma once
#include "afxcmn.h"
#include "afxwin.h"


// DialogSpriteTuner dialog

class DialogSpriteTuner : public CDialog
{
	DECLARE_DYNAMIC(DialogSpriteTuner)

public:
	DialogSpriteTuner(CWnd* pParent = NULL);   // standard constructor
	virtual ~DialogSpriteTuner();

// Dialog Data
	enum { IDD = IDD_SPRITEDIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	double size, linear, quadratic;
	DECLARE_MESSAGE_MAP()
public:
	CSliderCtrl sliderSize;
	CSliderCtrl sliderLinear;
	CSliderCtrl sliderQuad;
	CEdit sizeBox;
	CEdit linearBox;
	CEdit quadBox;
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	virtual BOOL OnInitDialog();
	void UpdateSliders(void);
};
