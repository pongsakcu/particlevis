#pragma once
#include "afxwin.h"

class CChildView;

// DialogVisibility dialog

class ParticleSet;

class DialogVisibility : public CDialog
{
	DECLARE_DYNAMIC(DialogVisibility)

public:
	DialogVisibility(CChildView* view, ParticleSet * pset);   // standard constructor
	virtual ~DialogVisibility();

// Dialog Data
	enum { IDD = IDD_VISDIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	CChildView * parent;
	ParticleSet * set;
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonSelect();
	afx_msg void OnBnClickedButtonInvert();
	afx_msg void OnBnClickedButtonReset();
	void Refresh(void);
	CButton hide_selected;
	CButton mark_selected;
	CButton flag_selected;
	INT visible_set;
	afx_msg void OnBnClickedCheckHide();
	afx_msg void OnBnClickedCheckMark();
	afx_msg void OnBnClickedCheckTrans();
	afx_msg void OnBnClickedRadioUsevis();
	afx_msg void OnBnClickedRadioUseocc();
	CButton select_visible;
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonHelp();
	afx_msg void OnBnClickedButtonselect2();
};
