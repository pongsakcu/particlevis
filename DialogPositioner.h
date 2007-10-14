#pragma once

#include "afxwin.h"
#include "stdafx.h"

class CChildView;

// DialogPositioner dialog

class DialogPositioner : public CDialog
{
	DECLARE_DYNAMIC(DialogPositioner)

public:
	DialogPositioner(CChildView* prim, CWnd* pParent = NULL);   // standard constructor
	virtual ~DialogPositioner();

// Dialog Data
	enum { IDD = IDD_METAPOSDIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	CChildView* primary;
	double cangle;

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedCampos();
	afx_msg void OnBnClickedCamtarg();
	afx_msg void OnBnClickedL1dir();
	afx_msg void OnBnClickedL2dir();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedForce();
	virtual BOOL OnInitDialog();
	int radioGroup;
	CButton camCone;
	afx_msg void OnBnClickedCheck1();
	void UpdateMode(int state);
	CEdit coneAngle;
	afx_msg void OnSetConeAngle();
};
