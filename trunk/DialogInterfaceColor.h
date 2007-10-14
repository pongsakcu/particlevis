#pragma once
#include "afxwin.h"
#include "triple.h"

// DialogInterfaceColor dialog: set colors for various elements in the interface and visualization

class DialogInterfaceColor : public CDialog
{
	DECLARE_DYNAMIC(DialogInterfaceColor)

public:
	DialogInterfaceColor(CWnd* pParent = NULL);   // standard constructor
	virtual ~DialogInterfaceColor();

// Dialog Data
	enum { IDD = IDD_COLORSDIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBgcbutton();
	afx_msg void OnBnClickedMarkcbutton();
	afx_msg void OnBnClickedTextcbutton();
	CButton disableMarkColor;
protected:
	virtual void OnOK();
	void GetColor(triple& target);
public:
	afx_msg void OnBnClickedButtonPStart();
	afx_msg void OnBnClickedButtonPEnd();
	virtual BOOL OnInitDialog();
};
