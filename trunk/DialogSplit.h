#pragma once
#include "afxwin.h"
#include "ParticleSet.h"
#include "triple.h"

// DialogSplit dialog: add a colored `split' to the ParticleSet

class DialogSplit : public CDialog
{
	DECLARE_DYNAMIC(DialogSplit)

public:
	DialogSplit(ParticleSet* primary, bool griddialog = false, CWnd* pParent = NULL);   // standard constructor
	virtual ~DialogSplit();

// Dialog Data
	enum { IDD = IDD_DIALOGSPLIT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	BOOL radio_axis;
	CEdit originX;
	CEdit originY;
	CEdit originZ;
	virtual BOOL OnInitDialog();
protected:
	bool grid_dialog;
	triple color_1, color_2;
	void GetColor(triple& target);
	virtual void OnOK();
	ParticleSet * prim;
public:
	afx_msg void OnBnClickedButtoncolor1();
	afx_msg void OnBnClickedButtoncolor2();
	CStatic origin_text;
	CStatic picture;
};
