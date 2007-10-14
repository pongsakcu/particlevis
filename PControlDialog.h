#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include "stdafx.h"

// PControlDialog dialog

class CChildView;

class PControlDialog : public CDialog
{
	DECLARE_DYNAMIC(PControlDialog)

public:
	PControlDialog(CChildView* ref, CWnd* parent);   // standard constructor
	virtual ~PControlDialog();

// Dialog Data
	enum { IDD = IDD_CONTROLDIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	CChildView* main;

	DECLARE_MESSAGE_MAP()
public:

	void UpdateScrollBars();
	void UpdatePosition(int pos);

	afx_msg void OnBnClickedButtonstart();
	afx_msg void OnBnClickedButtonfwd();
	afx_msg void OnBnClickedButtonprev();
	afx_msg void OnBnClickedButtonlast();
	afx_msg void OnBnClickedButtonfirst();
	CEdit EditPosition;
	CEdit EditSpeed;
	CSliderCtrl SpeedSlider;
	CSliderCtrl PosSlider;
	afx_msg void OnBnClickedButtonmark();
	afx_msg void OnBnClickedButtonmark2();
	afx_msg void OnBnClickedButtonsetz();
	afx_msg void OnBnClickedButtonresetz();
	CButton StartStopButton;

protected:
	virtual void OnCancel();
	virtual void PostNcDestroy();
	void UpdateText(CWnd& control, CString newtext);
	bool recording;
public:
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	virtual BOOL OnInitDialog();
	CButton playReverse;
	CButton playRepeat;
	afx_msg void OnBnClickedCheck1();
	afx_msg void OnBnClickedCheck2();

	bool GetRecording(void)
	{
		return recording;
	}	
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnControllerBack();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
protected:
	// accelerator table
	HACCEL m_hAccel;
};
