#pragma once
#include "afxcmn.h"
#include "ChildView.h"
#include "afxwin.h"


// DialogVectorScaler dialog

class DialogVectorScaler : public CDialog
{
	DECLARE_DYNAMIC(DialogVectorScaler)

public:
	DialogVectorScaler(CWnd* pParent = NULL);   // standard constructor
	virtual ~DialogVectorScaler();

// Dialog Data
	enum { IDD = IDD_VECTORSCALER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	CChildView * parent;
	double tempTransLength, tempRotLength, tempFieldLength;
	int tempPathLength;
	void UpdateSliders();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	virtual BOOL OnInitDialog();
	afx_msg void OnNMReleasedcaptureSlidertrans(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMReleasedcaptureSliderrot(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMReleasedcaptureSliderfield(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMReleasedcaptureSliderpath(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonOK();

	CEdit transEdit;
	CEdit rotEdit;
	CEdit fieldEdit;	
	CEdit pathEdit;	
	CSliderCtrl sliderTrans;
	CSliderCtrl sliderRot;
	CSliderCtrl sliderField;
	CSliderCtrl sliderPath;	
	afx_msg void OnCaptureChanged(CWnd *pWnd);
};
