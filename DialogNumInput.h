#pragma once
#include "afxwin.h"
#include <string>

using namespace std;

// DialogNumInput dialog

class DialogNumInput : public CDialog
{
	DECLARE_DYNAMIC(DialogNumInput)

public:
	DialogNumInput(string _title, double& _datatarget, CWnd* pParent = NULL);   // standard constructor
	virtual ~DialogNumInput();

// Dialog Data
	enum { IDD = IDD_DIALOGINPUT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CEdit input;
	CEdit title_box;
protected:
	string title;
	double& data;
	virtual void OnOK();
public:
	virtual BOOL OnInitDialog();
};