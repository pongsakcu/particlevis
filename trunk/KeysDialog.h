#pragma once


// KeysDialog dialog

class KeysDialog : public CDialog
{
	DECLARE_DYNAMIC(KeysDialog)

public:
	KeysDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~KeysDialog();

// Dialog Data
	enum { IDD = IDD_KEYSDIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
