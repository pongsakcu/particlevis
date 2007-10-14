#pragma once
#include "afxwin.h"
#include "ParticleSet.h"


// DialogRandom dialog

class DialogRandom : public CDialog
{
	DECLARE_DYNAMIC(DialogRandom)

public:
	DialogRandom(ParticleSet* target, CWnd* pParent = NULL);   // standard constructor
	virtual ~DialogRandom();

// Dialog Data
	enum { IDD = IDD_DIALOGRANDOM };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	ParticleSet* primary;
public:
	CEdit PNumber;
	CButton Normal;
	afx_msg void OnBnClickedOk();
};
